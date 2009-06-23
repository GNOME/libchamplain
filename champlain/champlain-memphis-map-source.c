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
#include "champlain-debug.h"
#include "champlain-cache.h"
#include "champlain-defines.h"
#include "champlain-enum-types.h"
#include "champlain-marshal.h"
#include "champlain-private.h"

#include <clutter-cairo.h>
#include <memphis/memphis.h>

/* Tuning parameters */
#define MAX_THREADS 4
#define DEFAULT_TILE_SIZE 256
#define DEFAULT_RULES_PATH "rule.xml"
#define MEMPHIS_DEBUG_LEVEL 1

G_DEFINE_TYPE (ChamplainMemphisMapSource, champlain_memphis_map_source, CHAMPLAIN_TYPE_MAP_SOURCE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_MEMPHIS_MAP_SOURCE, ChamplainMemphisMapSourcePrivate))

typedef struct _ChamplainMemphisMapSourcePrivate ChamplainMemphisMapSourcePrivate;

struct _ChamplainMemphisMapSourcePrivate {
  ChamplainMapDataSource *map_data_source;
  MemphisRuleSet *rules;
  MemphisRenderer *renderer;
  GThreadPool *thpool;
  //GStaticRWLock rwlock;
};

static void
champlain_memphis_map_source_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  // TODO
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
champlain_memphis_map_source_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  // TODO
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
champlain_memphis_map_source_dispose (GObject *object)
{
  ChamplainMemphisMapSource *self = (ChamplainMemphisMapSource *) object;
  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);

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

  g_thread_pool_free (priv->thpool, TRUE, TRUE);

  G_OBJECT_CLASS (champlain_memphis_map_source_parent_class)->finalize (object);
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

  memphis_renderer_draw_tile (renderer, cr, x, y, z);
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
  memphis_rule_set_load_from_file (priv->rules, DEFAULT_RULES_PATH); //???

  map = champlain_map_data_source_get_map_data (priv->map_data_source);

  priv->renderer = memphis_renderer_new_full (priv->rules, map);
  memphis_renderer_set_resolution (priv->renderer, DEFAULT_TILE_SIZE);
  memphis_renderer_set_debug_level (priv->renderer, MEMPHIS_DEBUG_LEVEL);

  priv->thpool = g_thread_pool_new (memphis_worker_thread, priv->renderer,
      MAX_THREADS, FALSE, NULL);

  return source;
}

void
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

  if (rules_path)
    memphis_rule_set_load_from_file (priv->rules, rules_path);
  else
    memphis_rule_set_load_from_file (priv->rules, DEFAULT_RULES_PATH);
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
  memphis_renderer_set_map (priv->renderer, map);
}

ChamplainMapDataSource *
champlain_memphis_map_source_get_map_data_source (
    ChamplainMemphisMapSource *self)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MEMPHIS_MAP_SOURCE (self), NULL);

  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);
  return priv->map_data_source;
}
