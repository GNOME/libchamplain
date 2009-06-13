/*
 * Copyright (C) 2008, 2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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
 * SECTION:champlain-map-source
 * @short_description: A base class for map sources
 *
 * #ChamplainTiles come from map sources which are represented by
 * #ChamplainMapSource.  This is should be considered an abstract
 * type as it does nothing of interest.
 *
 * When loading new tiles, #ChamplainView calls #champlain_map_source_get_tile
 * on the current #ChamplainMapSource passing it a #ChamplainTile to be filled
 * with the image.  #ChamplainMapSources should check with #ChamplainCache
 * if the image is already locally available with #champlain_cache_fill_tile.
 */

#include "config.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_LOADING
#include "champlain-debug.h"

#include "champlain.h"
#include "champlain-defines.h"
#include "champlain-enum-types.h"
#include "champlain-map-source.h"
#include "champlain-marshal.h"
#include "champlain-private.h"
#include "champlain-zoom-level.h"

#include <glib.h>
#include <glib-object.h>
#include <math.h>
#include <string.h>

void champlain_map_source_real_fill_tile (ChamplainMapSource *map_source,
    ChamplainTile *tile);

enum
{
  /* normal signals */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_ID,
  PROP_NAME,
  PROP_LICENSE,
  PROP_LICENSE_URI,
  PROP_MAX_ZOOM_LEVEL,
  PROP_MIN_ZOOM_LEVEL,
  PROP_TILE_SIZE,
  PROP_MAP_PROJECTION,
};

/* static guint champlain_map_source_signals[LAST_SIGNAL] = { 0, }; */

G_DEFINE_TYPE (ChamplainMapSource, champlain_map_source, G_TYPE_OBJECT);

#define GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_MAP_SOURCE, ChamplainMapSourcePrivate))

struct _ChamplainMapSourcePrivate
{
  gchar *id;
  gchar *name;
  gchar *license;
  gchar *license_uri;
  guint max_zoom_level;
  guint min_zoom_level;
  guint tile_size;
  ChamplainMapProjection map_projection;
};

static void
champlain_map_source_get_property (GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainMapSource *map_source = CHAMPLAIN_MAP_SOURCE(object);
  ChamplainMapSourcePrivate *priv = map_source->priv;

  switch(prop_id)
    {
      case PROP_ID:
        g_value_set_string (value, priv->id);
        break;
      case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;
      case PROP_LICENSE:
        g_value_set_string (value, priv->license);
        break;
      case PROP_LICENSE_URI:
        g_value_set_string (value, priv->license_uri);
        break;
      case PROP_MAX_ZOOM_LEVEL:
        g_value_set_uint (value, priv->max_zoom_level);
        break;
      case PROP_MIN_ZOOM_LEVEL:
        g_value_set_uint (value, priv->min_zoom_level);
        break;
      case PROP_TILE_SIZE:
        g_value_set_uint (value, priv->tile_size);
        break;
      case PROP_MAP_PROJECTION:
        g_value_set_enum (value, priv->map_projection);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
champlain_map_source_set_property (GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainMapSource *map_source = CHAMPLAIN_MAP_SOURCE(object);
  ChamplainMapSourcePrivate *priv = map_source->priv;

  switch(prop_id)
    {
      case PROP_ID:
        champlain_map_source_set_id (map_source,
            g_value_get_string (value));
      case PROP_NAME:
        champlain_map_source_set_name (map_source,
            g_value_get_string (value));
        break;
      case PROP_LICENSE:
        priv->license = g_value_dup_string (value);
        break;
      case PROP_LICENSE_URI:
        priv->license_uri = g_value_dup_string (value);
        break;
      case PROP_MAX_ZOOM_LEVEL:
        priv->max_zoom_level = g_value_get_uint (value);
        break;
      case PROP_MIN_ZOOM_LEVEL:
        priv->min_zoom_level = g_value_get_uint (value);
        break;
      case PROP_TILE_SIZE:
        priv->tile_size = g_value_get_uint (value);
        break;
      case PROP_MAP_PROJECTION:
        priv->map_projection = g_value_get_enum (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
champlain_map_source_finalize (GObject *object)
{
  ChamplainMapSource *map_source = CHAMPLAIN_MAP_SOURCE (object);
  ChamplainMapSourcePrivate *priv = map_source->priv;

  g_free (priv->name);
  g_free (priv->license);
  g_free (priv->license_uri);
  G_OBJECT_CLASS (champlain_map_source_parent_class)->finalize (object);
}

static void
champlain_map_source_class_init (ChamplainMapSourceClass *klass)
{
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (ChamplainMapSourcePrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = champlain_map_source_finalize;
  object_class->get_property = champlain_map_source_get_property;
  object_class->set_property = champlain_map_source_set_property;

  klass->fill_tile = champlain_map_source_real_fill_tile;

  /**
  * ChamplainMapSource:id:
  *
  * The name of the map source
  *
  * Since: 0.4
  */
  pspec = g_param_spec_string ("id",
                               "Id",
                               "The id of the map source",
                               "",
                               (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_ID, pspec);

  /**
  * ChamplainMapSource:name:
  *
  * The name of the map source
  *
  * Since: 0.4
  */
  pspec = g_param_spec_string ("name",
                               "Name",
                               "The name of the map source",
                               "",
                               (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_NAME, pspec);

  /**
  * ChamplainMapSource:license:
  *
  * The usage license of the map source
  *
  * Since: 0.4
  */
  pspec = g_param_spec_string ("license",
                               "License",
                               "The usage license of the map source",
                               "",
                               (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_LICENSE, pspec);

  /**
  * ChamplainMapSource:license-uri:
  *
  * The usage license's uri for more information 
  *
  * Since: 0.4
  */
  pspec = g_param_spec_string ("license-uri",
                               "License-uri",
                               "The usage license's uri for more information",
                               "",
                               (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_LICENSE_URI, pspec);

  /**
  * ChamplainMapSource:max-zoom-level:
  *
  * The maximum zoom level
  *
  * Since: 0.4
  */
  pspec = g_param_spec_uint ("max-zoom-level",
                             "Maximum Zoom Level",
                             "The maximum zoom level",
                             0,
                             50,
                             18,
                             (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_MAX_ZOOM_LEVEL, pspec);

  /**
  * ChamplainMapSource:min-zoom-level:
  *
  * The minimum zoom level
  *
  * Since: 0.4
  */
  pspec = g_param_spec_uint ("min-zoom-level",
                             "Minimum Zoom Level",
                             "The minimum zoom level",
                             0,
                             50,
                             0,
                             (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_MIN_ZOOM_LEVEL, pspec);

  /**
  * ChamplainMapSource:tile-size:
  *
  * The tile size of the map source
  *
  * Since: 0.4
  */
  pspec = g_param_spec_uint ("tile-size",
                             "Tile Size",
                             "The tile size",
                             0,
                             2048,
                             256,
                             (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_TILE_SIZE, pspec);

  /**
  * ChamplainMapSource:projection
  *
  * The map projection of the map source
  *
  * Since: 0.4
  */
  pspec = g_param_spec_enum ("projection",
                             "Projection",
                             "The map projection",
                             CHAMPLAIN_TYPE_MAP_PROJECTION,
                             CHAMPLAIN_MAP_PROJECTION_MERCATOR,
                             (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_MAP_PROJECTION, pspec);
}

static void
champlain_map_source_init (ChamplainMapSource *champlainMapSource)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (champlainMapSource);
  champlainMapSource->priv = priv;
}

/**
 * champlain_map_source_get_max_zoom_level:
 * @map_source: a #ChamplainMapSource
 *
 * Returns the maximum zoom level this map source supports
 *
 * Since: 0.4
 */
gint
champlain_map_source_get_max_zoom_level (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  ChamplainMapSourcePrivate *priv = map_source->priv;
  return priv->max_zoom_level;
}

/**
 * champlain_map_source_get_min_zoom_level:
 * @map_source: a #ChamplainMapSource
 *
 * Returns the miminum zoom level this map source supports
 *
 * Since: 0.4
 */
gint
champlain_map_source_get_min_zoom_level (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  ChamplainMapSourcePrivate *priv = map_source->priv;
  return priv->min_zoom_level;
}

/**
 * champlain_map_source_get_tile_size:
 * @map_source: a #ChamplainMapSource
 *
 * Returns the tile's size (width and height) in pixels for this map source
 *
 * Since: 0.4
 */
guint
champlain_map_source_get_tile_size (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  ChamplainMapSourcePrivate *priv = map_source->priv;
  return priv->tile_size;
}

/**
 * champlain_map_source_get_x:
 * @map_source: a #ChamplainMapSource
 * @zoom_level: the zoom level
 * @longitude: a longitude
 *
 * Returns the x position on the map using this map source's projection.
 * (0, 0) is located at the top left.
 *
 * Since: 0.4
 */
guint
champlain_map_source_get_x (ChamplainMapSource *map_source,
    gint zoom_level,
    gdouble longitude)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  ChamplainMapSourcePrivate *priv = map_source->priv;
  // FIXME: support other projections
  return ((longitude + 180.0) / 360.0 * pow(2.0, zoom_level)) * priv->tile_size;
}

/**
 * champlain_map_source_get_y:
 * @map_source: a #ChamplainMapSource
 * @zoom_level: the zoom level
 * @latitude: a latitude
 *
 * Returns the y position on the map using this map source's projection.
 * (0, 0) is located at the top left.
 *
 * Since: 0.4
 */
guint
champlain_map_source_get_y (ChamplainMapSource *map_source,
    gint zoom_level,
    gdouble latitude)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  ChamplainMapSourcePrivate *priv = map_source->priv;
  // FIXME: support other projections
  return ((1.0 - log (tan (latitude * M_PI / 180.0) + 1.0 /
          cos (latitude * M_PI / 180.0)) /
        M_PI) / 2.0 * pow (2.0, zoom_level)) * priv->tile_size;
}

/**
 * champlain_map_source_get_row_count:
 * @map_source: a #ChamplainMapSource
 * @zoom_level: the zoom level
 *
 * Returns the number of tiles in a row at this zoom level for this map source.
 *
 * Since: 0.4
 */
guint
champlain_map_source_get_row_count (ChamplainMapSource *map_source,
    gint zoom_level)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  //ChamplainMapSourcePrivate *priv = map_source->priv;
  // FIXME: support other projections
  return pow (2, zoom_level);
}

/**
 * champlain_map_source_get_column_count:
 * @map_source: a #ChamplainMapSource
 * @zoom_level: the zoom level
 *
 * Returns the number of tiles in a column at this zoom level for this map
 * source.
 *
 * Since: 0.4
 */
guint
champlain_map_source_get_column_count (ChamplainMapSource *map_source,
    gint zoom_level)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  //ChamplainMapSourcePrivate *priv = map_source->priv;
  // FIXME: support other projections
  return pow (2, zoom_level);
}

/**
 * champlain_map_source_fill_tile:
 * @map_source: a #ChamplainMapSource
 * @tile: A #ChamplainTile
 *
 * Fills the tile with image data (either from cache, network or rendered
 * locally).
 *
 * Since: 0.4
 */
void
champlain_map_source_fill_tile (ChamplainMapSource *map_source,
    ChamplainTile *tile)
{
  g_return_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source));

  CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->fill_tile (map_source, tile);
}

void
champlain_map_source_real_fill_tile (ChamplainMapSource *map_source,
    ChamplainTile *tile)
{
  g_error ("Should not be reached");
}

/**
 * champlain_map_source_get_longitude:
 * @map_source: a #ChamplainMapSource
 * @zoom_level: the zoom level
 * @x: a x position
 *
 * Returns the longitude corresponding to this x position in the map source's
 * projection.
 *
 * Since: 0.4
 */
gdouble
champlain_map_source_get_longitude (ChamplainMapSource *map_source,
    gint zoom_level,
    guint x)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);
  //ChamplainMapSourcePrivate *priv = map_source->priv;
  // FIXME: support other projections
  gdouble dx = (float)x / champlain_map_source_get_tile_size (map_source);
  return dx / pow (2.0, zoom_level) * 360.0 - 180;
}

/**
 * champlain_map_source_get_latitude:
 * @map_source: a #ChamplainMapSource
 * @zoom_level: the zoom level
 * @y: a y position
 *
 * Returns the latitude corresponding to this y position in the map source's
 * projection.
 *
 * Since: 0.4
 */
gdouble
champlain_map_source_get_latitude (ChamplainMapSource *map_source,
    gint zoom_level,
    guint y)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);
  //ChamplainMapSourcePrivate *priv = map_source->priv;
  // FIXME: support other projections
  gdouble dy = (float)y / champlain_map_source_get_tile_size (map_source);
  double n = M_PI - 2.0 * M_PI * dy / pow (2.0, zoom_level);
  return 180.0 / M_PI * atan (0.5 * (exp (n) - exp (-n)));
}

/**
 * champlain_map_source_get_name:
 * @map_source: a #ChamplainMapSource
 *
 * Returns the map source's name.
 *
 * Since: 0.4
 */
const gchar *
champlain_map_source_get_name (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  ChamplainMapSourcePrivate *priv = map_source->priv;
  return priv->name;
}

/**
 * champlain_map_source_set_name:
 * @map_source: a #ChamplainMapSource
 * @name: a name
 *
 * Sets the map source's name.
 *
 * Since: 0.4
 */
void
champlain_map_source_set_name (ChamplainMapSource *map_source,
    const gchar *name)
{
  g_return_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source));

  ChamplainMapSourcePrivate *priv = map_source->priv;

  g_free (priv->name);
  priv->name = g_strdup (name);
  g_object_notify (G_OBJECT (map_source), "name");
}

/**
 * champlain_map_source_get_license:
 * @map_source: a #ChamplainMapSource
 *
 * Returns the map source's license.
 *
 * Since: 0.4
 */
const gchar *
champlain_map_source_get_license (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  ChamplainMapSourcePrivate *priv = map_source->priv;
  return priv->license;
}

/**
 * champlain_map_source_set_license:
 * @map_source: a #ChamplainMapSource
 * @license: the licence
 *
 * Sets the map source's license.
 *
 * Since: 0.4
 */
void
champlain_map_source_set_license (ChamplainMapSource *map_source,
    const gchar *license)
{
  g_return_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source));

  ChamplainMapSourcePrivate *priv = map_source->priv;

  g_free (priv->license);
  priv->license = g_strdup (license);
  g_object_notify (G_OBJECT (map_source), "license");
}

/**
 * champlain_map_source_get_license_uri:
 * @map_source: a #ChamplainMapSource
 *
 * Returns the map source's license URI.
 *
 * Since: 0.4
 */
const gchar *
champlain_map_source_get_license_uri (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  ChamplainMapSourcePrivate *priv = map_source->priv;

  return priv->license_uri;
}

/**
 * champlain_map_source_set_license_uri:
 * @map_source: a #ChamplainMapSource
 * @license_uri: the licence URI
 *
 * Sets the map source's license URI.
 *
 * Since: 0.4
 */
void
champlain_map_source_set_license_uri (ChamplainMapSource *map_source,
    const gchar *license_uri)
{
  g_return_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source));

  ChamplainMapSourcePrivate *priv = map_source->priv;

  g_free (priv->license_uri);
  priv->license = g_strdup (license_uri);
  g_object_notify (G_OBJECT (map_source), "license-uri");
}

/**
 * champlain_map_source_get_projection:
 * @map_source: a #ChamplainMapSource
 *
 * Returns the map source's projection.
 *
 * Since: 0.4
 */
ChamplainMapProjection
champlain_map_source_get_projection (ChamplainMapSource *map_source)
{
  ChamplainMapSourcePrivate *priv = map_source->priv;

  return priv->map_projection;
}

/**
 * champlain_map_source_set_projection:
 * @map_source: a #ChamplainMapSource
 * @projection: a #ChamplainMapProjection
 *
 * Sets the map source's projection.
 *
 * Since: 0.4
 */
void
champlain_map_source_set_projection (ChamplainMapSource *map_source,
    ChamplainMapProjection projection)
{
  g_return_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source));

  ChamplainMapSourcePrivate *priv = map_source->priv;

  priv->map_projection = projection;
  g_object_notify (G_OBJECT (map_source), "projection");
}

/**
 * champlain_map_source_get_id:
 * @map_source: a #ChamplainMapSource
 *
 * Returns the map source's id.
 *
 * Since: 0.4
 */
const gchar *
champlain_map_source_get_id (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  ChamplainMapSourcePrivate *priv = map_source->priv;
  return priv->id;
}

/**
 * champlain_map_source_set_id:
 * @map_source: a #ChamplainMapSource
 * @id: a id
 *
 * Sets the map source's id.
 *
 * Since: 0.4
 */
void
champlain_map_source_set_id (ChamplainMapSource *map_source,
    const gchar *id)
{
  g_return_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source));

  ChamplainMapSourcePrivate *priv = map_source->priv;

  g_free (priv->id);
  priv->id = g_strdup (id);
  g_object_notify (G_OBJECT (map_source), "id");
}

