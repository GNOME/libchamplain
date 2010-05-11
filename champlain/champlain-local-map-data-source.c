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

/**
 * SECTION:champlain-local-map-data-source
 * @short_description: Loads local map data for #ChamplainMemphisTileSource
 *
 * This map data source loads local <ulink role="online-location"
 * url="http://wiki.openstreetmap.org/wiki/.osm">
 * OpenStreetMap XML data files</ulink> (*.osm).
 *
 */

#include "champlain-local-map-data-source.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_MEMPHIS
#include "champlain-debug.h"
#include "champlain-bounding-box.h"
#include "champlain-enum-types.h"
#include "champlain-tile.h"

#include <memphis/memphis.h>

G_DEFINE_TYPE (ChamplainLocalMapDataSource, champlain_local_map_data_source, CHAMPLAIN_TYPE_MAP_DATA_SOURCE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_LOCAL_MAP_DATA_SOURCE, ChamplainLocalMapDataSourcePrivate))

struct _ChamplainLocalMapDataSourcePrivate {
    MemphisMap* map;
};

static void
champlain_local_map_data_source_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  switch (property_id)
    {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_local_map_data_source_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  switch (property_id)
    {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_local_map_data_source_dispose (GObject *object)
{
  ChamplainLocalMapDataSource *self = (ChamplainLocalMapDataSource *) object;
  ChamplainLocalMapDataSourcePrivate *priv = self->priv;

  if (priv->map)
    {
      memphis_map_free (priv->map);
      priv->map = NULL;
    }

  G_OBJECT_CLASS (champlain_local_map_data_source_parent_class)->dispose (object);
}

static void
champlain_local_map_data_source_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_local_map_data_source_parent_class)->finalize (object);
}

static MemphisMap *
get_map_data (ChamplainMapDataSource *self)
{
  ChamplainLocalMapDataSourcePrivate *priv = CHAMPLAIN_LOCAL_MAP_DATA_SOURCE (self)->priv;

  return priv->map;
}

static void
champlain_local_map_data_source_class_init (ChamplainLocalMapDataSourceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainLocalMapDataSourcePrivate));

  object_class->get_property = champlain_local_map_data_source_get_property;
  object_class->set_property = champlain_local_map_data_source_set_property;
  object_class->dispose = champlain_local_map_data_source_dispose;
  object_class->finalize = champlain_local_map_data_source_finalize;

  ChamplainMapDataSourceClass *map_data_source_class = CHAMPLAIN_MAP_DATA_SOURCE_CLASS (klass);
  map_data_source_class->get_map_data = get_map_data;
}

static void
champlain_local_map_data_source_init (ChamplainLocalMapDataSource *self)
{
  ChamplainLocalMapDataSourcePrivate *priv = GET_PRIVATE(self);

  self->priv = priv;

  priv->map = memphis_map_new ();
}

/**
 * champlain_local_map_data_source_new:
 *
 * Creates a new instance of #ChamplainLocalMapDataSource.
 *
 * Returns: a new #ChamplainLocalMapDataSource.
 *
 * Since: 0.6
 */
ChamplainLocalMapDataSource *
champlain_local_map_data_source_new (void)
{
  return g_object_new (CHAMPLAIN_TYPE_LOCAL_MAP_DATA_SOURCE, NULL);
}

/**
 * champlain_local_map_data_source_load_map_data:
 * @map_data_source: a #ChamplainLocalMapDataSource
 * @map_path: a path to a map data file
 *
 * Loads the OpenStreetMap XML file at the given path.
 *
 * Since: 0.6
 */
void
champlain_local_map_data_source_load_map_data (ChamplainLocalMapDataSource *self,
    const gchar *map_path)
{
  g_return_if_fail (CHAMPLAIN_IS_LOCAL_MAP_DATA_SOURCE (self)
      && map_path);

  /* TODO: Remove test when memphis handles invalid paths properly */
  if (!g_file_test (map_path, G_FILE_TEST_EXISTS))
    {
      g_critical ("Error: \"%s\" does not exist.", map_path);
      return;
    }

  ChamplainLocalMapDataSourcePrivate *priv = self->priv;
  ChamplainBoundingBox *bbox;
  MemphisMap *map = memphis_map_new ();
  GError *err = NULL;

  memphis_map_load_from_file (map, map_path, &err);
  if (err != NULL)
    {
      g_critical ("Can't load map file: \"%s\"", err->message);
      memphis_map_free (map);
      g_error_free (err);
      return;
    }

  if (priv->map)
    memphis_map_free (priv->map);

  priv->map = map;

  bbox = champlain_bounding_box_new ();
  memphis_map_get_bounding_box (priv->map, &bbox->left, &bbox->top,
      &bbox->right, &bbox->bottom);
  g_object_set (G_OBJECT (self), "bounding-box", bbox, NULL);
  champlain_bounding_box_free (bbox);

  g_object_set (G_OBJECT (self), "state", CHAMPLAIN_STATE_DONE, NULL);
}
