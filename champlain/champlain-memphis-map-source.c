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
#define DEFAULT_TILE_SIZE 512

G_DEFINE_TYPE (ChamplainMemphisMapSource, champlain_memphis_map_source, CHAMPLAIN_TYPE_MAP_SOURCE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_MEMPHIS_MAP_SOURCE, ChamplainMemphisMapSourcePrivate))

typedef struct _ChamplainMemphisMapSourcePrivate ChamplainMemphisMapSourcePrivate;

struct _ChamplainMemphisMapSourcePrivate {
  ChamplainMapDataSource *source;
  MemphisMap *map; // remove me
  MemphisRuleSet *rules;
  MemphisRenderer *renderer;
  GThreadPool* thpool;
  //GStaticRWLock rwlock;
};

static void
champlain_memphis_map_source_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
champlain_memphis_map_source_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
champlain_memphis_map_source_dispose (GObject *object)
{
  ChamplainMemphisMapSource *self = (ChamplainMemphisMapSource *) object;
  ChamplainMemphisMapSourcePrivate *priv = GET_PRIVATE(self);

  g_thread_pool_free (priv->thpool, TRUE, TRUE);
  memphis_renderer_free (priv->renderer);
  memphis_map_free (priv->map);
  memphis_rule_set_free (priv->rules);
  
  G_OBJECT_CLASS (champlain_memphis_map_source_parent_class)->dispose (object);
}

static void
champlain_memphis_map_source_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_memphis_map_source_parent_class)->finalize (object);
}

static void
fill_tile (ChamplainMapSource *map_source,
    ChamplainTile *tile)
{

}

static void
memphis_worker_thread (gpointer data, gpointer user_data)
{

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
  
  priv->map = memphis_map_new ();
  priv->rules = memphis_rule_set_new ();
  priv->renderer = memphis_renderer_new_full (priv->rules, priv->map);
  priv->thpool = g_thread_pool_new (memphis_worker_thread, priv->renderer,
      MAX_THREADS, FALSE, NULL);
}

ChamplainMemphisMapSource*
champlain_memphis_map_source_new_full (
    ChamplainMapDataSource *data_source)
{
  return g_object_new (CHAMPLAIN_TYPE_MEMPHIS_MAP_SOURCE, NULL);
}

void
champlain_memphis_map_source_set_tile_size (
    ChamplainMemphisMapSource *map_source)
{

}

void
champlain_memphis_map_source_set_rules_path (
    ChamplainMemphisMapSource *map_source,
    gchar *path)
{

}

void champlain_memphis_map_source_set_map_source (
    ChamplainMemphisMapSource *map_source,
    ChamplainMapDataSource *data_source)
{

}
