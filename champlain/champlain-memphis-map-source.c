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
  PROP_MAP_DATA_SOURCE
};

G_DEFINE_TYPE (ChamplainMemphisMapSource, champlain_memphis_map_source, CHAMPLAIN_TYPE_MAP_SOURCE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_MEMPHIS_MAP_SOURCE, ChamplainMemphisMapSourcePrivate))

typedef struct _ChamplainMemphisMapSourcePrivate ChamplainMemphisMapSourcePrivate;

struct _ChamplainMemphisMapSourcePrivate {
  ChamplainMapDataSource *map_data_source;
  MemphisRuleSet *rules;
  MemphisRenderer *renderer;
  GThreadPool *thpool;
};

/* lock to protect the renderer state while rendering */
GStaticRWLock MemphisLock = G_STATIC_RW_LOCK_INIT;

static void
champlain_memphis_map_source_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  ChamplainMemphisMapSource *self = CHAMPLAIN_MEMPHIS_MAP_SOURCE (object);
  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE (self);

  switch (property_id) {
  case PROP_MAP_DATA_SOURCE:
    g_value_set_object (value, priv->map_data_source);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
champlain_memphis_map_source_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  ChamplainMemphisMapSource *self = CHAMPLAIN_MEMPHIS_MAP_SOURCE (object);

  switch (property_id) {
  case PROP_MAP_DATA_SOURCE:
    champlain_memphis_map_source_set_map_data_source (self,
        g_value_get_object (value));
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
  //ChamplainMemphisMapSource *self = (ChamplainMemphisMapSource *) object;
  //ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);

  G_OBJECT_CLASS (champlain_memphis_map_source_parent_class)->finalize (object);
}

static void
map_data_changed_cb (ChamplainMapDataSource *map_data_source,
    GParamSpec *arg1,
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
}

static void
fill_tile (ChamplainMapSource *map_source, ChamplainTile *tile)
{
  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(map_source);
  guint size;
  GError *error = NULL;

  size = champlain_map_source_get_tile_size (map_source);
  champlain_tile_set_size (tile, size);

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

typedef struct _TileData TileData;
struct _TileData {
  ChamplainTile *tile;
  cairo_surface_t *cst;
};

static gboolean
set_tile_content (gpointer data)
{
  TileData *tdata = (TileData *) data;
  ChamplainTile *tile = tdata->tile;
  cairo_surface_t *cst = tdata->cst;
  cairo_t *cr_clutter;
  ClutterActor *actor;
  guint size;

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

static void
memphis_worker_thread (gpointer data, gpointer user_data)
{
  ChamplainTile *tile = (ChamplainTile *) data;
  MemphisRenderer *renderer = (MemphisRenderer *) user_data;
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
  memphis_renderer_draw_tile (renderer, cr, x, y, z);
  g_static_rw_lock_reader_unlock (&MemphisLock);
  cairo_destroy (cr);

  tdata = g_new (TileData, 1);
  tdata->tile = tile;
  tdata->cst = cst;

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
      g_param_spec_string ("map-data-source",
        "Map data source",
        "The data source of the renderer",
        NULL,
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
      "projection", CHAMPLAIN_MAP_PROJECTION_MERCATOR,
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

  priv->thpool = g_thread_pool_new (memphis_worker_thread, priv->renderer,
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

  memphis_renderer_set_resolution (priv->renderer, size);
  g_object_set (G_OBJECT (self), "tile-size", size, NULL);
}

void
champlain_memphis_map_source_load_rules (
    ChamplainMemphisMapSource *self,
    gchar *rules_path)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_MAP_SOURCE (self));

  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);

  g_static_rw_lock_writer_lock (&MemphisLock);
  if (rules_path)
    memphis_rule_set_load_from_file (priv->rules, rules_path);
  else
    memphis_rule_set_load_from_file (priv->rules, DEFAULT_RULES_PATH);
  g_static_rw_lock_writer_unlock (&MemphisLock);

  g_signal_emit_by_name (CHAMPLAIN_MAP_SOURCE (self),
      "reload-tiles", NULL);
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
