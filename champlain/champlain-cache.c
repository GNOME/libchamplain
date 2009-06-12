/*
* Copyright (C) 2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * SECTION:champlain-cache
 * @short_description: Manages cached tiles
 *
 * #ChamplainCache is an object to interogate the cache for previously downloaded
 * tiles. ChamplainCache is a singleton, there should be only one instance shared
 * by all map sources.
 *
 * Tiles most frequently asked gain in "popularity".  This popularity will be taken
 * into account when purging the cache.
 *
 * Unless you are implementing your own ChamplainMapSource, the only function you
 * should need are #champlain_cache_purge and #champlain_cache_purge_on_idle.
 */

#include "champlain-cache.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_CACHE
#include "champlain-debug.h"
#include "champlain-enum-types.h"
#include "champlain-private.h"

#include <errno.h>
#include <glib.h>
#include <gio/gio.h>
#include <string.h>
#include <sqlite3.h>

G_DEFINE_TYPE (ChamplainCache, champlain_cache, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_CACHE, ChamplainCachePrivate))

enum
{
  PROP_0,
  PROP_SIZE_LIMIT,
};

typedef struct _ChamplainCachePrivate ChamplainCachePrivate;

static ChamplainCache *instance = NULL;

struct _ChamplainCachePrivate {
  guint size_limit;

  sqlite3 *data;
  sqlite3_stmt *stmt_select;
  sqlite3_stmt *stmt_update;

  GSList *popularity_queue;
  guint popularity_id;
};

static gboolean inc_popularity (gpointer data);
static void delete_tile (ChamplainCache *self,
    const gchar *filename);

static void
champlain_cache_get_property (GObject *object,
                             guint property_id,
                             GValue *value,
                             GParamSpec *pspec)
{
  ChamplainCache *self = CHAMPLAIN_CACHE (object);
  switch (property_id)
    {
      case PROP_SIZE_LIMIT:
        g_value_set_uint (value, champlain_cache_get_size_limit (self));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_cache_set_property (GObject *object,
                             guint property_id,
                             const GValue *value,
                             GParamSpec *pspec)
{
  ChamplainCache *self = CHAMPLAIN_CACHE (object);
  switch (property_id)
    {
      case PROP_SIZE_LIMIT:
        champlain_cache_set_size_limit (self, g_value_get_uint (value));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_cache_dispose (GObject *object)
{
  gint error;

  ChamplainCachePrivate *priv = GET_PRIVATE (object);

  if (priv->stmt_select)
    {
      sqlite3_finalize (priv->stmt_select);
      priv->stmt_select = NULL;
    }

  if (priv->stmt_update)
    {
      sqlite3_finalize (priv->stmt_update);
      priv->stmt_update = NULL;
    }

  if (priv->data != NULL)
  {
    error = sqlite3_close (priv->data);
    if (error != SQLITE_OK)
      DEBUG ("Sqlite returned error %d when closing cache.db", error);
    priv->data = NULL;
  }

  G_OBJECT_CLASS (champlain_cache_parent_class)->dispose (object);
}

static void
champlain_cache_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_cache_parent_class)->finalize (object);
}

static GObject *
champlain_cache_constructor (GType type,
    guint n_construct_params,
    GObjectConstructParam *construct_params)
{
  GObject *retval;

  if (instance == NULL)
    {
      retval = G_OBJECT_CLASS (champlain_cache_parent_class)->constructor
          (type, n_construct_params, construct_params);

      instance = CHAMPLAIN_CACHE (retval);
      g_object_add_weak_pointer (retval, (gpointer) &instance);
    }
  else
    {
      retval = g_object_ref (instance);
    }

  return retval;
}

static void
champlain_cache_class_init (ChamplainCacheClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainCachePrivate));

  object_class->constructor = champlain_cache_constructor;
  object_class->get_property = champlain_cache_get_property;
  object_class->set_property = champlain_cache_set_property;
  object_class->dispose = champlain_cache_dispose;
  object_class->finalize = champlain_cache_finalize;

  /**
  * ChamplainCache:size-limit:
  *
  * The cache size limit in bytes.
  *
  * Note: this new value will not be applied until you call #champlain_cache_purge
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_SIZE_LIMIT,
      g_param_spec_uint ("size-limit",
        "Size Limit",
        "The cache's size limit (Mb)",
        1,
        G_MAXINT,
        100000000,
        G_PARAM_READWRITE));
}

static void
champlain_cache_init (ChamplainCache *self)
{
  gchar *path, *filename, *error_msg = NULL;
  gint error;

  ChamplainCachePrivate *priv = GET_PRIVATE (self);
  priv->popularity_queue = NULL;
  priv->popularity_id = 0;

  filename = g_build_filename (g_get_user_cache_dir (), "champlain",
      "cache.db", NULL);

  champlain_cache_set_size_limit (self, 100000000);

  priv->stmt_select = NULL;
  priv->stmt_update = NULL;

  /* Create, if needed, the cache's dirs */
  path = g_path_get_dirname (filename);
  if (g_mkdir_with_parents (path, 0700) == -1)
    {
      if (errno != EEXIST)
        {
          g_warning ("Unable to create the image cache path '%s': %s",
                     path, g_strerror (errno));
        }
    }

  error = sqlite3_open_v2 (filename, &priv->data,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (error == SQLITE_ERROR)
    {
      DEBUG ("Sqlite returned error %d when opening cache.db", error);
      goto cleanup;
    }

  sqlite3_exec (priv->data,
      "CREATE TABLE IF NOT EXISTS tiles ("
      "filename TEXT PRIMARY KEY, etag TEXT, "
      "popularity INT DEFAULT 1, "
      "size INT DEFAULT 0)",
      NULL, NULL, &error_msg);
  if (error_msg != NULL)
    {
      DEBUG ("Creating table 'tiles' failed: %s", error_msg);
          sqlite3_free (error_msg);
      goto cleanup;
    }

  error = sqlite3_prepare_v2 (priv->data,
      "SELECT etag FROM tiles WHERE filename = ?", -1,
      &priv->stmt_select, NULL);
  if (error != SQLITE_OK)
    {
      priv->stmt_select = NULL;
      DEBUG ("Failed to prepare the select Etag statement, error:%d: %s",
          error, sqlite3_errmsg (priv->data));
      goto cleanup;
    }

  error = sqlite3_prepare_v2 (priv->data,
      "UPDATE tiles SET popularity = popularity + 1 WHERE filename = ?", -1,
      &priv->stmt_update, NULL);
  if (error != SQLITE_OK)
    {
      priv->stmt_update = NULL;
      DEBUG ("Failed to prepare the update popularity statement, error: %s",
          sqlite3_errmsg (priv->data));
      goto cleanup;
    }

cleanup:
  g_free (filename);
  g_free (path);
}

/**
 * champlain_cache_dup_default:
 *
 * Returns the #ChamplainCache singleton, use #g_object_unref when not neeeded
 * anymore.
 *
 *
 * Since: 0.4
 */
ChamplainCache*
champlain_cache_dup_default (void)
{
  return g_object_new (CHAMPLAIN_TYPE_CACHE, NULL);
}

ChamplainCache*
champlain_cache_get_default (void)
{
  return champlain_cache_dup_default ();
}

/**
 * champlain_cache_get_size_limit:
 * @self: the #ChamplainCache
 *
 * Returns the cache size limit in bytes
 *
 * Since: 0.4
 */
guint
champlain_cache_get_size_limit (ChamplainCache *self)
{
  g_return_val_if_fail(CHAMPLAIN_CACHE (self), FALSE);

  ChamplainCachePrivate *priv = GET_PRIVATE (self);

  return priv->size_limit;
}

/**
 * champlain_cache_set_size_limit:
 * @self: the #ChamplainCache
 * @size_limit: the cache limit in bytes
 *
 * Sets the cache size limit in bytes
 *
 * Since: 0.4
 */
void
champlain_cache_set_size_limit (ChamplainCache *self,
    guint size_limit)
{
  g_return_if_fail (CHAMPLAIN_CACHE (self));

  ChamplainCachePrivate *priv = GET_PRIVATE (self);

  priv->size_limit = size_limit;
  g_object_notify (G_OBJECT (self), "size-limit");
}

/**
 * champlain_cache_fill_tile:
 * @self: the #ChamplainCache
 * @tile: the #ChamplainTile to fill
 *
 * Loads data from disk for the given tile
 *
 * Returns TRUE if the tile was in cache, false if it needs to be
 * loaded from network
 *
 * Since: 0.4
 */
gboolean
champlain_cache_fill_tile (ChamplainCache *self,
    ChamplainTile *tile)
{
  g_return_val_if_fail(CHAMPLAIN_CACHE (self), FALSE);
  g_return_val_if_fail(CHAMPLAIN_TILE (tile), FALSE);

  GFileInfo *info = NULL;
  GFile *file = NULL;
  GError *error = NULL;
  ClutterActor *actor = NULL;
  const gchar *filename = NULL;
  GTimeVal *modified_time = NULL;
  int sql_rc = SQLITE_OK;
  gboolean cache_hit = FALSE;

  ChamplainCachePrivate *priv = GET_PRIVATE (self);

  modified_time = g_new0 (GTimeVal, 1);
  filename = champlain_tile_get_filename (tile);

  if (!g_file_test (filename, G_FILE_TEST_EXISTS))
    goto cleanup;

  file = g_file_new_for_path (filename);
  info = g_file_query_info (file,
      G_FILE_ATTRIBUTE_TIME_MODIFIED "," G_FILE_ATTRIBUTE_ETAG_VALUE,
      G_FILE_QUERY_INFO_NONE, NULL, NULL);
  g_file_info_get_modification_time (info, modified_time);
  champlain_tile_set_modified_time (tile, modified_time);

  g_object_unref (file);
  g_object_unref (info);

  /* Retrieve etag */
  sql_rc = sqlite3_bind_text (priv->stmt_select, 1, filename, -1, SQLITE_STATIC);
  if (sql_rc == SQLITE_ERROR)
    {
      DEBUG ("Failed to prepare the SQL query for finding the Etag of '%s', error: %s",
          filename, sqlite3_errmsg (priv->data));
      goto cleanup;
    }

  sql_rc = sqlite3_step (priv->stmt_select);
  if (sql_rc == SQLITE_ROW)
    {
      const gchar *etag = (const gchar *) sqlite3_column_text (priv->stmt_select, 0);
      champlain_tile_set_etag (CHAMPLAIN_TILE (tile), etag);
    }
  else if (sql_rc == SQLITE_DONE)
    {
      DEBUG ("'%s' does't have an etag",
          filename);
    }
  else if (sql_rc == SQLITE_ERROR)
    {
      DEBUG ("Failed to finding the Etag of '%s', %d error: %s",
          filename, sql_rc, sqlite3_errmsg (priv->data));
      goto cleanup;
    }

  /* Load the cached version */
  actor = clutter_texture_new_from_file (filename, &error);
  if (error == NULL)
    {
        champlain_tile_set_content (tile, actor, FALSE);
        cache_hit = TRUE;
    }
  else
    {
      DEBUG ("Failed to load tile %s, error: %s",
          filename, error->message);
      /* remote from the history */
      delete_tile (self, filename);
      goto cleanup;
    }

  priv->popularity_queue = g_slist_prepend (priv->popularity_queue,
      g_strdup (filename));

  if (priv->popularity_id == 0)
  {
    g_object_ref (self);
    priv->popularity_id = g_idle_add (inc_popularity, self);
  }

cleanup:
  sqlite3_reset (priv->stmt_select);

  return cache_hit;
}

/**
 * champlain_cache_tile_is_expired:
 * @self: the #ChamplainCache
 * @tile: the #ChamplainTile to fill
 *
 * Returns TRUE if the tile should be
 * reloaded from network
 *
 * Since: 0.4
 */
gboolean
champlain_cache_tile_is_expired (ChamplainCache *self,
    ChamplainTile *tile)
{
  g_return_val_if_fail(CHAMPLAIN_CACHE (self), FALSE);
  g_return_val_if_fail(CHAMPLAIN_TILE (tile), FALSE);

  GTimeVal *now = g_new0 (GTimeVal, 1);
  const GTimeVal *modified_time = champlain_tile_get_modified_time (tile);
  gboolean validate_cache = FALSE;

  g_get_current_time (now);
  g_time_val_add (now, (-24ul * 60ul * 60ul * 1000ul * 1000ul)); // Cache expires 1 day
  validate_cache = modified_time->tv_sec < now->tv_sec;

  g_free (now);
  return validate_cache;
}

static gboolean
inc_popularity (gpointer data)
{
  int sql_rc = SQLITE_OK;
  gchar *filename = NULL;
  ChamplainCache *cache = CHAMPLAIN_CACHE (data);
  ChamplainCachePrivate *priv = GET_PRIVATE (cache);
  GSList *last;

  if (priv->popularity_queue == NULL)
    {
      g_object_unref (cache);
      priv->popularity_id = 0;
      return FALSE;
    }

  last = g_slist_last (priv->popularity_queue);
  filename = last->data;

  sql_rc = sqlite3_bind_text (priv->stmt_update, 1, filename, -1, SQLITE_STATIC);
  if (sql_rc != SQLITE_OK)
    {
      DEBUG ("Failed to set values to the popularity query of '%s', error: %s",
          filename, sqlite3_errmsg (priv->data));
      goto cleanup;
    }

  sql_rc = sqlite3_step (priv->stmt_update);
  if (sql_rc != SQLITE_DONE)
    {
      DEBUG ("Failed to update the popularity of '%s', error: %s",
          filename, sqlite3_errmsg (priv->data));
      goto cleanup;
    }

cleanup:
  sqlite3_reset (priv->stmt_update);

  priv->popularity_queue = g_slist_remove  (priv->popularity_queue, filename);
  g_free (filename);

  /* Ask to be called again until the list is emptied */
  return TRUE;
}

static void
delete_tile (ChamplainCache *self,
    const gchar *filename)
{
  g_return_if_fail (CHAMPLAIN_CACHE (self));
  gchar *query, *error = NULL;
  GError *gerror;
  GFile *file;

  ChamplainCachePrivate *priv = GET_PRIVATE (self);

  query = sqlite3_mprintf ("DELETE FROM tiles WHERE filename = %Q", filename);
  sqlite3_exec (priv->data, query, NULL, NULL, &error);
  if (error != NULL)
    {
      DEBUG ("Deleting tile from db failed: %s", error);
      sqlite3_free (error);
    }
  sqlite3_free (query);

  if (!g_file_test (filename, G_FILE_TEST_EXISTS))
    return;

  file = g_file_new_for_path (filename);
  if (!g_file_delete (file, NULL, &gerror))
    {
        DEBUG ("Deleting tile from disk failed: %s", gerror->message);
        g_error_free (gerror);
    }
  g_object_unref (file);
}

/**
 * champlain_cache_update_tile:
 * @self: the #ChamplainCache
 * @tile: the #ChamplainTile to fill
 * @filesize: the filesize on the disk
 *
 * Update the tile's information in the cache such as Etag and filesize.
 * Also increase the tile's popularity.
 *
 * Since: 0.4
 */
void
champlain_cache_update_tile (ChamplainCache *self,
    ChamplainTile *tile,
    guint filesize)
{
  g_return_if_fail (CHAMPLAIN_CACHE (self));
  gchar *query, *error = NULL;

  ChamplainCachePrivate *priv = GET_PRIVATE (self);

  query = sqlite3_mprintf ("REPLACE INTO tiles (filename, etag, size) VALUES (%Q, %Q, %d)",
      champlain_tile_get_filename (tile),
      champlain_tile_get_etag (tile),
      filesize);
  sqlite3_exec (priv->data, query, NULL, NULL, &error);
  if (error != NULL)
    {
      DEBUG ("Saving Etag and size failed: %s", error);
      sqlite3_free (error);
    }
  sqlite3_free (query);
}

static gboolean
purge_on_idle (gpointer data)
{
  champlain_cache_purge (CHAMPLAIN_CACHE (data));
  return FALSE;
}

/**
 * champlain_cache_purge_on_idle:
 * @self: the #ChamplainCache
 *
 * Purge the cache from the less popular tiles until cache's size limit is reached.
 * This is a non blocking call as the purge will happen when the application is idle
 *
 * Since: 0.4
 */
void
champlain_cache_purge_on_idle (ChamplainCache *self)
{
  g_return_if_fail (CHAMPLAIN_CACHE (self));
  g_idle_add (purge_on_idle, self);
}

/**
 * champlain_cache_purge:
 * @self: the #ChamplainCache
 *
 * Purge the cache from the less popular tiles until cache's size limit is reached.
 *
 * Since: 0.4
 */
void
champlain_cache_purge (ChamplainCache *self)
{
  g_return_if_fail (CHAMPLAIN_CACHE (self));

  ChamplainCachePrivate *priv = GET_PRIVATE (self);
  gchar *query;
  sqlite3_stmt *stmt;
  int rc = 0;
  guint current_size = 0;
  guint highest_popularity = 0;
  gchar *error;

  query = "SELECT SUM (size) FROM tiles";
  rc = sqlite3_prepare (priv->data, query, strlen (query), &stmt, NULL);
  if (rc != SQLITE_OK)
    {
      DEBUG ("Can't compute cache size %s", sqlite3_errmsg (priv->data));
    }

  rc = sqlite3_step (stmt);
  if (rc != SQLITE_ROW)
    {
      DEBUG ("Failed to count the total cache consumption %s",
          sqlite3_errmsg (priv->data));
      sqlite3_finalize (stmt);
      return;
    }

  current_size = sqlite3_column_int (stmt, 0);
  if (current_size < priv->size_limit)
    {
      DEBUG ("Cache doesn't need to be purged at %d bytes", current_size);
      sqlite3_finalize (stmt);
      return;
    }

  sqlite3_finalize (stmt);

  /* Ok, delete the less popular tiles until size_limit reached */
  query = "SELECT filename, size, popularity FROM tiles ORDER BY popularity";
  rc = sqlite3_prepare (priv->data, query, strlen (query), &stmt, NULL);
  if (rc != SQLITE_OK)
    {
      DEBUG ("Can't fetch tiles to delete: %s", sqlite3_errmsg(priv->data));
    }

  rc = sqlite3_step (stmt);
  while (rc == SQLITE_ROW && current_size > priv->size_limit)
    {
      const char *filename = sqlite3_column_text (stmt, 0);
      guint size;

      filename = sqlite3_column_text (stmt, 0);
      size = sqlite3_column_int (stmt, 1);
      highest_popularity = sqlite3_column_int (stmt, 2);
      DEBUG ("Deleting %s of size %d", filename, size);

      delete_tile (self, filename);

      current_size -= size;

      rc = sqlite3_step (stmt);
    }
  DEBUG ("Cache size is now %d", current_size);

  sqlite3_finalize (stmt);

  query = sqlite3_mprintf ("UPDATE tiles SET popularity = popularity - %d",
      highest_popularity);
  sqlite3_exec (priv->data, query, NULL, NULL, &error);
  if (error != NULL)
    {
      DEBUG ("Updating popularity failed: %s", error);
      sqlite3_free (error);
    }
  sqlite3_free (query);
}

