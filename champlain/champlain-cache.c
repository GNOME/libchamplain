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

struct _ChamplainCachePrivate {
  guint size_limit;

  sqlite3 *data;
  sqlite3_stmt *stmt_select;
  sqlite3_stmt *stmt_update;
};

static void
inc_popularity (ChamplainCache *self,
    ChamplainTile *tile);

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
    sqlite3_finalize (priv->stmt_select);
  
  if (priv->stmt_update)
    sqlite3_finalize (priv->stmt_update);

  error = sqlite3_close (priv->data);
  if (error != SQLITE_OK)
    DEBUG ("Sqlite returned error %d when closing cache.db", error);

  G_OBJECT_CLASS (champlain_cache_parent_class)->dispose (object);
}

static void
champlain_cache_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_cache_parent_class)->finalize (object);
}

static void
champlain_cache_class_init (ChamplainCacheClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainCachePrivate));

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
  gchar *filename, *error_msg = NULL;
  gint error;

  ChamplainCachePrivate *priv = GET_PRIVATE (self);

  filename = g_build_filename (g_get_user_cache_dir (), "champlain",
      "cache.db", NULL);

  champlain_cache_set_size_limit (self, 100000000);

  priv->stmt_select = NULL;
  priv->stmt_update = NULL;

  error = sqlite3_open_v2 (filename, &priv->data,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (error != SQLITE_OK)
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
      DEBUG ("Sqlite returned error code %d when creating select Etag statement", error);
      goto cleanup;
    }

  error = sqlite3_prepare_v2 (priv->data, 
      "UPDATE tiles SET popularity = popularity + 1 WHERE filename = ?", -1,
      &priv->stmt_update, NULL);
  if (error != SQLITE_OK)
    {
      priv->stmt_update = NULL;
      DEBUG ("Sqlite returned error code %d when creating update popularity statement", error);
      goto cleanup;
    }

cleanup:
  g_free (filename);
}

/**
 * champlain_cache_get_default:
 *
 * Returns the #ChamplainCache singleton
 *
 * Since: 0.4
 */
ChamplainCache*
champlain_cache_get_default (void)
{
  static ChamplainCache * instance = NULL;

  if (G_UNLIKELY (instance == NULL))
    {
      instance = g_object_new (CHAMPLAIN_TYPE_CACHE, NULL);
      return instance;
    }

  return g_object_ref (instance);
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
  GTimer *timer;
  gulong microseconds;

  ChamplainCachePrivate *priv = GET_PRIVATE (self);
  modified_time = g_new0 (GTimeVal, 1);
  filename = champlain_tile_get_filename (tile);

  if (!g_file_test (filename, G_FILE_TEST_EXISTS))
    return FALSE;
  
  timer = g_timer_new ();
  g_timer_start (timer);

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
  if (sql_rc != SQLITE_OK)
    {
      DEBUG ("Failed to prepare the SQL query for finding the Etag of '%s', got error: %s", 
          filename, sqlite3_errmsg (priv->data));
      goto cleanup;
    }

  sql_rc = sqlite3_step (priv->stmt_select);
  if (sql_rc == SQLITE_ROW)
    {
      const gchar *etag = (const gchar *) sqlite3_column_text (priv->stmt_select, 0);
      champlain_tile_set_etag (CHAMPLAIN_TILE (tile), etag);
      cache_hit = TRUE;
    }
  else if (sql_rc == SQLITE_DONE)
    {
      DEBUG ("Can't find the Etag of '%s', got error: %s", 
          filename, sqlite3_errmsg (priv->data));
      goto cleanup;
    }
  else
    {
      DEBUG ("Failed to finding the Etag of '%s', got error: %s",
          filename, sqlite3_errmsg (priv->data));
      goto cleanup;
    }
 
  /* Load the cached version */
  actor = clutter_texture_new_from_file (filename, &error);
  champlain_tile_set_content (tile, actor, FALSE);

  inc_popularity (self, tile);
  g_timer_stop (timer);
  g_timer_elapsed (timer, &microseconds);
  g_timer_destroy (timer);
  g_print ("%s: %6lu %s hit:%s\n", __FUNCTION__, microseconds, filename, cache_hit ? "TRUE" : "FALSE");

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
  g_time_val_add (now, (-60ul * 60ul * 1000ul * 1000ul)); // Cache expires 1 hour
  validate_cache = modified_time->tv_sec < now->tv_sec;

  g_free (now);
  return validate_cache;
}

static void
inc_popularity (ChamplainCache *self,
    ChamplainTile *tile)
{
  g_return_if_fail (CHAMPLAIN_CACHE (self));
  int sql_rc = SQLITE_OK;
  const gchar *filename = NULL;

  ChamplainCachePrivate *priv = GET_PRIVATE (self);

  filename = champlain_tile_get_filename (tile);
  
  sql_rc = sqlite3_bind_text (priv->stmt_update, 1, filename, -1, SQLITE_STATIC);
  if (sql_rc != SQLITE_OK)
    {
      DEBUG ("Failed to prepare the SQL query for updating the popularity of '%s', got error: %s", 
          filename, sqlite3_errmsg (priv->data));
      goto cleanup;
    }

  sql_rc = sqlite3_step (priv->stmt_update);
  if (sql_rc != SQLITE_DONE)
    {
      DEBUG ("Failed to update the popularity of '%s', got error: %s",
          filename, sqlite3_errmsg (priv->data));
      goto cleanup;
    }

cleanup:
  sqlite3_reset (priv->stmt_update);
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

  query = g_strdup_printf ("DELETE FROM tiles WHERE filename = '%s';", filename);
  sqlite3_exec (priv->data, query, NULL, NULL, &error);
  if (error != NULL)
    {
      DEBUG ("Deleting tile from db failed: %s", error);
      sqlite3_free (error);
    }
  g_free (query);

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

  query = g_strdup_printf ("REPLACE INTO tiles (filename, etag, size) VALUES ('%s', '%s', %d);",
      champlain_tile_get_filename (tile),
      champlain_tile_get_etag (tile),
      filesize);
  sqlite3_exec (priv->data, query, NULL, NULL, &error);
  if (error != NULL)
    {
      DEBUG ("Saving Etag and size failed: %s", error);
      sqlite3_free (error);
    }
  g_free (query);
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

  query = g_strdup_printf ("SELECT SUM (size) FROM tiles;");
  rc = sqlite3_prepare (priv->data, query, strlen (query), &stmt, NULL);
  if (rc != SQLITE_OK)
    {
      DEBUG ("Can't compute cache size %s", sqlite3_errmsg(priv->data));
    }
  g_free (query);

  rc = sqlite3_step (stmt);
  current_size = sqlite3_column_int (stmt, 0);
  if (current_size < priv->size_limit)
    {
      DEBUG ("Cache doesn't need to be purged at %d bytes", current_size);
      sqlite3_finalize (stmt);
      return;
    }

  sqlite3_finalize (stmt);

  /* Ok, delete the less popular tiles until size_limit reached */
  query = g_strdup_printf ("SELECT filename, size, popularity FROM tiles ORDER BY popularity;");
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
  g_free (query);

  query = g_strdup_printf ("UPDATE tiles SET popularity = popularity - %d;",
      highest_popularity);
  sqlite3_exec (priv->data, query, NULL, NULL, &error);
  if (error != NULL)
    {
      DEBUG ("Updating popularity failed: %s", error);
      sqlite3_free (error);
    }
  g_free (query);
}

