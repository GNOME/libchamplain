/*
* Copyright (C) 2008 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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

#include "champlain-cache.h"

#include "champlain-enum-types.h"
#include "champlain-private.h"

#include <glib.h>
#include <gio/gio.h>
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
};

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

  error = sqlite3_close (priv->data);
  if (error != SQLITE_OK)
    g_warning ("Sqlite returned error %d when closing cache.db", error);

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

  g_object_class_install_property (object_class,
      PROP_SIZE_LIMIT,
      g_param_spec_uint ("size-limit",
        "Size Limit",
        "The cache's size limit (Mb)",
        1,
        G_MAXINT,
        10,
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

  champlain_cache_set_size_limit (self, 10);

  error = sqlite3_open_v2 (filename, &priv->data,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (error != SQLITE_OK)
    {
      g_warning ("Sqlite returned error %d when opening cache.db", error);
      goto cleanup;
    }

  sqlite3_exec (priv->data, "CREATE TABLE etags (filename char(500) PRIMARY KEY, etag char (30))", NULL, NULL, &error_msg);
  if (error_msg != NULL)
    {
      g_warning ("Creating table Etag failed: %s", error_msg);
      sqlite3_free (error_msg);
      goto cleanup;
    }

cleanup:
  g_free (filename);
}

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

guint
champlain_cache_get_size_limit (ChamplainCache *self)
{
  g_return_val_if_fail(CHAMPLAIN_CACHE (self), FALSE);

  ChamplainCachePrivate *priv = GET_PRIVATE (self);

  return priv->size_limit;
}

void
champlain_cache_set_size_limit (ChamplainCache *self,
    guint size_limit)
{
  g_return_if_fail(CHAMPLAIN_CACHE (self));

  ChamplainCachePrivate *priv = GET_PRIVATE (self);

  priv->size_limit = size_limit;
  g_object_notify (G_OBJECT (self), "size-limit");
}

static int
set_etag (void *tile,
    int row_count,
    char** values,
    char** headers)
{
  g_print ("%s: %s\n", headers[0], values[0]);
  champlain_tile_set_etag (CHAMPLAIN_TILE (tile), values[0]);
  return 0; // 0 meaning success
}

gboolean
champlain_cache_fill_tile (ChamplainCache *self,
    ChamplainTile *tile)
{
  g_return_val_if_fail(CHAMPLAIN_CACHE (self), FALSE);
  g_return_val_if_fail(CHAMPLAIN_TILE (tile), FALSE);

  GFileInfo *info;
  GFile *file;
  GError *error = NULL;
  ClutterActor *actor;
  const gchar *filename;
  gchar *error_msg = NULL;
  gchar *query = NULL;
  GTimeVal *modified_time;

  ChamplainCachePrivate *priv = GET_PRIVATE (self);
  modified_time = g_new0 (GTimeVal, 1);
  filename = champlain_tile_get_filename (tile);

  if (!g_file_test (filename, G_FILE_TEST_EXISTS))
    return FALSE;

  file = g_file_new_for_path (filename);
  info = g_file_query_info (file,
      G_FILE_ATTRIBUTE_TIME_MODIFIED "," G_FILE_ATTRIBUTE_ETAG_VALUE,
      G_FILE_QUERY_INFO_NONE, NULL, NULL);
  g_file_info_get_modification_time (info, modified_time);
  champlain_tile_set_modified_time (tile, modified_time);

  /* Retrieve etag */
  query = g_strdup_printf ("SELECT etag FROM etags WHERE filename = '%s'",
      filename);
  sqlite3_exec (priv->data, query, set_etag, tile, &error_msg);
  if (error_msg != NULL)
    {
      g_warning ("Retrieving Etag failed: %s", error_msg);
      sqlite3_free (error_msg);
    }

  /* Load the cached version */
  actor = clutter_texture_new_from_file (filename, &error);
  champlain_tile_set_actor (tile, actor);

  g_object_unref (file);
  g_object_unref (info);
  g_free (query);

  return TRUE;
}

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

void
champlain_cache_update_tile (ChamplainCache *self,
    ChamplainTile *tile)
{
  g_return_if_fail(CHAMPLAIN_CACHE (self));
  gchar *query, *error = NULL;

  ChamplainCachePrivate *priv = GET_PRIVATE (self);

  query = g_strdup_printf ("REPLACE INTO etags (filename, etag) VALUES ('%s', '%s');",
      champlain_tile_get_filename (tile),
      champlain_tile_get_etag (tile));
  sqlite3_exec (priv->data, query, NULL, NULL, &error);
  if (error != NULL)
    {
      g_warning ("Saving Etag failed: %s", error);
      sqlite3_free (error);
    }


  g_free (query);
}
