/*
 * Copyright (C) 2009 Simon Wenner <simon@wenner.ch>
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

#include "champlain-memphis-map-source.h"
#include "champlain.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_MEMPHIS
#include "champlain-debug.h"

#include "champlain-cache.h"
#include "champlain-defines.h"
#include "champlain-enum-types.h"
#include "champlain-private.h"

#include <errno.h>
#include <glib/gstdio.h>
#include <clutter-cairo.h>
#include <memphis/memphis.h>

/* Tuning parameters */
#define MAX_THREADS 4
#define DEFAULT_TILE_SIZE 256 // FIXME find the bug
#define DEFAULT_RULES_PATH "default-rules.xml"
/* 0: Be quiet, 1: Normal output, 2: Be verbose */
#define MEMPHIS_INTERNAL_DEBUG_LEVEL 0

enum
{
  PROP_0,
  PROP_MAP_DATA_SOURCE,
  PROP_SESSION_ID
};

G_DEFINE_TYPE (ChamplainMemphisMapSource, champlain_memphis_map_source, CHAMPLAIN_TYPE_MAP_SOURCE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_MEMPHIS_MAP_SOURCE, ChamplainMemphisMapSourcePrivate))

typedef struct _ChamplainMemphisMapSourcePrivate ChamplainMemphisMapSourcePrivate;

struct _ChamplainMemphisMapSourcePrivate {
  ChamplainMapDataSource *map_data_source;
  gchar *session_id;
  MemphisRuleSet *rules;
  MemphisRenderer *renderer;
  GThreadPool *thpool;
};

typedef struct _TileData TileData;

struct _TileData {
  ChamplainTile *tile;
  cairo_surface_t *cst;
  gchar *session_id;
};

/* lock to protect the renderer state while rendering */
GStaticRWLock MemphisLock = G_STATIC_RW_LOCK_INIT;

static void
champlain_memphis_map_source_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainMemphisMapSource *self = CHAMPLAIN_MEMPHIS_MAP_SOURCE (object);
  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE (self);

  switch (property_id) {
  case PROP_MAP_DATA_SOURCE:
    g_value_set_object (value, priv->map_data_source);
    break;
  case PROP_SESSION_ID:
    g_value_set_string (value, priv->session_id);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
champlain_memphis_map_source_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainMemphisMapSource *self = CHAMPLAIN_MEMPHIS_MAP_SOURCE (object);

  switch (property_id) {
  case PROP_MAP_DATA_SOURCE:
    champlain_memphis_map_source_set_map_data_source (self,
        g_value_get_object (value));
  case PROP_SESSION_ID:
    champlain_memphis_map_source_set_session_id (self,
        g_value_get_string (value));
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
champlain_memphis_map_source_dispose (GObject *object)
{
  ChamplainMemphisMapSource *self = (ChamplainMemphisMapSource *) object;
  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);

  if (priv->thpool)
    g_thread_pool_free (priv->thpool, FALSE, TRUE);

  if (priv->map_data_source)
    g_object_unref (priv->map_data_source);
  memphis_renderer_free (priv->renderer);
  memphis_rule_set_free (priv->rules);

  G_OBJECT_CLASS (champlain_memphis_map_source_parent_class)->dispose (object);
}

static void
champlain_memphis_map_source_finalize (GObject *object)
{
  ChamplainMemphisMapSource *self = (ChamplainMemphisMapSource *) object;
  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);

  g_free (priv->session_id);

  G_OBJECT_CLASS (champlain_memphis_map_source_parent_class)->finalize (object);
}

static void
map_data_changed_cb (ChamplainMapDataSource *map_data_source,
    ChamplainMemphisMapSource *map_source)
{
  g_assert (CHAMPLAIN_IS_MAP_DATA_SOURCE (map_data_source) &&
      CHAMPLAIN_IS_MEMPHIS_MAP_SOURCE (map_source));

  MemphisMap *map;
  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(map_source);

  map = champlain_map_data_source_get_map_data (map_data_source);

  DEBUG ("DataSource has been changed!");

  g_static_rw_lock_writer_lock (&MemphisLock);
  memphis_renderer_set_map (priv->renderer, map);
  g_static_rw_lock_writer_unlock (&MemphisLock);

  g_signal_emit_by_name (CHAMPLAIN_MAP_SOURCE (map_source),
      "reload-tiles", NULL);
}

static void
fill_tile (ChamplainMapSource *map_source, ChamplainTile *tile)
{
  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(map_source);
  ChamplainCache* cache = champlain_cache_dup_default ();
  guint size;
  GError *error = NULL;
  gchar *filename;
  gboolean in_cache = FALSE;

  size = champlain_map_source_get_tile_size (map_source);
  champlain_tile_set_size (tile, size);

  filename = champlain_cache_get_filename (cache, map_source, tile,
      priv->session_id);
  champlain_tile_set_filename (tile, filename);

  in_cache = champlain_cache_fill_tile (cache, tile);

  /* check for cached version */
  if (in_cache == TRUE)
    {
      DEBUG ("Tile was cached (%u, %u, %u)", champlain_tile_get_x (tile),
          champlain_tile_get_y (tile),
          champlain_tile_get_zoom_level (tile));
      champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);
    }
  else
    {
      DEBUG ("Render tile (%u, %u, %u)", champlain_tile_get_x (tile),
          champlain_tile_get_y (tile),
          champlain_tile_get_zoom_level (tile));
      champlain_tile_set_state (tile, CHAMPLAIN_STATE_LOADING);

      /* So we don't loose a tile if it is in the thread pool queue for a long time */
      tile = g_object_ref (tile);

      g_thread_pool_push (priv->thpool, tile, &error);
      if (error)
        {
          g_error ("Thread pool error: %s", error->message);
          g_error_free (error);
        }
    }
  g_object_unref (cache);
}

static gboolean
set_tile_content (gpointer data)
{
  TileData *tdata = (TileData *) data;
  ChamplainTile *tile = tdata->tile;
  cairo_surface_t *cst = tdata->cst;
  gchar *session = tdata->session_id;
  cairo_t *cr_clutter;
  ClutterActor *actor;
  guint size;
  ChamplainCache *cache = champlain_cache_dup_default ();

  /* update the cache */
  champlain_cache_update_tile_with_session (cache, tile, 20, session);

  g_object_unref (cache);

  /* draw the clutter texture */
  size = champlain_tile_get_size (tile);
  actor = clutter_cairo_new (size, size);

  cr_clutter = clutter_cairo_create (CLUTTER_CAIRO(actor));
  cairo_set_source_surface (cr_clutter, cst, 0, 0);
  cairo_paint (cr_clutter);
  cairo_destroy (cr_clutter);
  cairo_surface_destroy (cst);

  champlain_tile_set_content (tile, actor, TRUE);

  champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);
  g_object_unref (tile);
  g_object_unref (tile);

  g_free (tdata);
  return FALSE;
}

static gboolean
delete_session_cache (gpointer data)
{
  ChamplainMemphisMapSource *self = CHAMPLAIN_MEMPHIS_MAP_SOURCE (data);
  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);
  ChamplainCache* cache = champlain_cache_dup_default ();

  champlain_cache_delete_session (cache, CHAMPLAIN_MAP_SOURCE (self),
      priv->session_id);

  DEBUG ("Delete '%s' session cache", priv->session_id);

  g_object_unref (cache);

  g_signal_emit_by_name (CHAMPLAIN_MAP_SOURCE (self),
      "reload-tiles", NULL);

  return FALSE;
}

static void
memphis_worker_thread (gpointer data, gpointer user_data)
{
  ChamplainTile *tile = CHAMPLAIN_TILE (data);
  ChamplainMemphisMapSource *map_source = CHAMPLAIN_MEMPHIS_MAP_SOURCE (user_data);
  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(map_source);
  cairo_t *cr;
  cairo_surface_t *cst;
  guint x, y, z, size;
  TileData *tdata;

  x = champlain_tile_get_x (tile);
  y = champlain_tile_get_y (tile);
  z = champlain_tile_get_zoom_level (tile);
  size = champlain_tile_get_size (tile);

  /* create a clutter-independant surface to draw on */
  cst = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, size, size);
  cr = cairo_create (cst);

  DEBUG ("Draw Tile (%d, %d, %d)", x, y, z);

  g_static_rw_lock_reader_lock (&MemphisLock);
  memphis_renderer_draw_tile (priv->renderer, cr, x, y, z);
  g_static_rw_lock_reader_unlock (&MemphisLock);
  cairo_destroy (cr);

  const gchar *filename = champlain_tile_get_filename (tile);
  /* Create, if needed, the cache's dirs */
  char *path = g_path_get_dirname (filename);

  if (g_mkdir_with_parents (path, 0700) == -1)
    {
      if (errno != EEXIST)
        {
          g_warning ("Unable to create the image cache path '%s': %s",
                     path, g_strerror (errno));
        }
    }
  g_free (path);

  /* Write png image for caching */
  if (cairo_surface_write_to_png (cst, filename) != CAIRO_STATUS_SUCCESS)
    {
      g_warning ("Unable to write image '%s' to the cache", filename);
    }

  /* Write the tile content and cache entry */
  tdata = g_new (TileData, 1);
  tdata->tile = tile;
  tdata->cst = cst;
  tdata->session_id = priv->session_id;

  clutter_threads_add_idle_full (G_PRIORITY_DEFAULT, set_tile_content,
      tdata, NULL);
}

static void
champlain_memphis_map_source_class_init (ChamplainMemphisMapSourceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainMemphisMapSourcePrivate));

  object_class->get_property = champlain_memphis_map_source_get_property;
  object_class->set_property = champlain_memphis_map_source_set_property;
  object_class->dispose = champlain_memphis_map_source_dispose;
  object_class->finalize = champlain_memphis_map_source_finalize;

  ChamplainMapSourceClass *map_source_class = CHAMPLAIN_MAP_SOURCE_CLASS (klass);
  map_source_class->fill_tile = fill_tile;

  /**
  * ChamplainMemphisMapSource:MapDataSource:
  *
  * The data source of the renderer
  *
  * Since: 0.6
  */
  g_object_class_install_property (object_class,
      PROP_MAP_DATA_SOURCE,
      g_param_spec_object ("map-data-source",
        "Map data source",
        "The data source of the renderer",
        CHAMPLAIN_TYPE_MAP_DATA_SOURCE,
        G_PARAM_READWRITE));

  /**
  * ChamplainMemphisMapSource:Session:
  *
  * The session id of the tile cache
  *
  * Since: 0.6
  */
  g_object_class_install_property (object_class,
      PROP_SESSION_ID,
      g_param_spec_string ("session-id",
        "Cache session id",
        "The session id of the cache",
        "default",
        G_PARAM_READWRITE));
}

static void
champlain_memphis_map_source_init (ChamplainMemphisMapSource *self)
{
  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);

  priv->map_data_source = NULL;
  priv->rules = NULL;
  priv->renderer = NULL;
  priv->thpool = NULL;
  priv->session_id = g_strdup ("default");
}

ChamplainMemphisMapSource *
champlain_memphis_map_source_new_full (ChamplainMapSourceDesc *desc,
    ChamplainMapDataSource *map_data_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_DATA_SOURCE (map_data_source) &&
      CHAMPLAIN_MAP_SOURCE_DESC (desc), NULL);

  ChamplainMemphisMapSource *source;
  ChamplainMemphisMapSourcePrivate *priv;
  MemphisMap *map;

  source = g_object_new (CHAMPLAIN_TYPE_MEMPHIS_MAP_SOURCE,
      "id", desc->id,
      "name", desc->name,
      "license", desc->license,
      "license-uri", desc->license_uri,
      "projection", desc->projection,
      "min-zoom-level", desc->min_zoom_level,
      "max-zoom-level", desc->max_zoom_level,
      "tile-size", DEFAULT_TILE_SIZE,
      NULL);

  priv = GET_PRIVATE(source);
  priv->map_data_source = g_object_ref (map_data_source);

  priv->rules = memphis_rule_set_new ();
  map = champlain_map_data_source_get_map_data (priv->map_data_source);

  priv->renderer = memphis_renderer_new_full (priv->rules, map);
  memphis_renderer_set_resolution (priv->renderer, DEFAULT_TILE_SIZE);
  memphis_renderer_set_debug_level (priv->renderer,
      MEMPHIS_INTERNAL_DEBUG_LEVEL);

  memphis_rule_set_load_from_file (priv->rules, DEFAULT_RULES_PATH);
  // TODO: Do we want to ship a default rule-set?

  priv->thpool = g_thread_pool_new (memphis_worker_thread, source,
      MAX_THREADS, FALSE, NULL);

  g_signal_connect (priv->map_data_source, "map-data-changed",
      G_CALLBACK (map_data_changed_cb), source);

  return source;
}

void // TODO: this is a dangerous function! Remove it?
champlain_memphis_map_source_set_tile_size (ChamplainMemphisMapSource *self,
    guint size)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_MAP_SOURCE (self));

  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);

  g_static_rw_lock_writer_lock (&MemphisLock);
  memphis_renderer_set_resolution (priv->renderer, size);
  g_static_rw_lock_writer_unlock (&MemphisLock);
  g_object_set (G_OBJECT (self), "tile-size", size, NULL);
}

void
champlain_memphis_map_source_load_rules (
    ChamplainMemphisMapSource *self,
    const gchar *rules_path)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_MAP_SOURCE (self));

  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);

  g_static_rw_lock_writer_lock (&MemphisLock);
  if (rules_path)
    memphis_rule_set_load_from_file (priv->rules, rules_path);
  else
    memphis_rule_set_load_from_file (priv->rules, DEFAULT_RULES_PATH);
  g_static_rw_lock_writer_unlock (&MemphisLock);

  champlain_memphis_map_source_delete_session_cache (self);
}

void
champlain_memphis_map_source_set_map_data_source (
    ChamplainMemphisMapSource *self,
    ChamplainMapDataSource *map_data_source)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_MAP_SOURCE (self) &&
      CHAMPLAIN_IS_MAP_DATA_SOURCE (map_data_source));

  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);
  MemphisMap *map;

  priv->map_data_source = map_data_source;
  map = champlain_map_data_source_get_map_data (priv->map_data_source);

  g_static_rw_lock_writer_lock (&MemphisLock);
  memphis_renderer_set_map (priv->renderer, map);
  g_static_rw_lock_writer_unlock (&MemphisLock);
}

ChamplainMapDataSource *
champlain_memphis_map_source_get_map_data_source (
    ChamplainMemphisMapSource *self)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MEMPHIS_MAP_SOURCE (self), NULL);

  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);
  return priv->map_data_source;
}

void
champlain_memphis_map_source_delete_session_cache (ChamplainMemphisMapSource *self)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_MAP_SOURCE (self));

  clutter_threads_add_idle_full (G_PRIORITY_DEFAULT, delete_session_cache,
      self, NULL);
}

void
champlain_memphis_map_source_set_session_id (ChamplainMemphisMapSource *self,
    const gchar *session_id)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_MAP_SOURCE (self)
      && session_id != NULL);

  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);

  if (priv->session_id)
    g_free (priv->session_id);

  priv->session_id = g_strdup (session_id);
}

const gchar *
champlain_memphis_map_source_get_session_id (ChamplainMemphisMapSource *self)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MEMPHIS_MAP_SOURCE (self), NULL);

  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);
  return priv->session_id;
}
