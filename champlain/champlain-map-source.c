/*
 * Copyright (C) 2010 Jiri Techet <techet@gmail.com>
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
 * #ChamplainTile objects come from map sources which are represented by
 * #ChamplainMapSource.  This is should be considered an abstract
 * type as it does nothing of interest.
 *
 * When loading new tiles, #ChamplainView calls champlain_map_source_fill_tile()
 * on the current #ChamplainMapSource passing it a #ChamplainTile to be filled
 * with the image.
 *
 * Apart from being a base class of all map sources, #ChamplainMapSource
 * also supports cooperation of multiple map sources by arranging them into
 * chains. Every map source has the #ChamplainMapSource::next-source property
 * that determines the next map source in the chain. When a function of
 * a #ChamplainMapSource object is invoked, the map source may decide to
 * delegate the work to the next map source in the chain by invoking the
 * same function on it.

 * To undertand the concept of chains, consider for instance a chain
 * consisting of #ChamplainFileCache whose next source is
 * #ChamplainNetworkTileSource whose next source is #ChamplainErrorTileSource.
 * When champlain_map_source_fill_tile() is called on the first object of the
 * chain, #ChamplainFileCache, the cache checks whether it contains the
 * requested tile in its database. If it does, it returns the tile; otherwise,
 * it calls champlain_map_source_fill_tile() on the next source in the chain
 * (#ChamplainNetworkTileSource). The network tile source loads the tile
 * from the network. When successful, it returns the tile; otherwise it requests
 * the tile from the next source in the chain (#ChamplainErrorTileSource).
 * #ChamplainErrorTileSource always generates an error tile, no matter what
 * its next source is.
 */

#include "champlain-map-source.h"

#include <math.h>

G_DEFINE_TYPE (ChamplainMapSource, champlain_map_source, G_TYPE_INITIALLY_UNOWNED);

#define GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_MAP_SOURCE, ChamplainMapSourcePrivate))

enum
{
  /* normal signals */
  RELOAD_TILES,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_NEXT_SOURCE,
};

static guint champlain_map_source_signals[LAST_SIGNAL] = { 0, };

typedef struct _ChamplainMapSourcePrivate ChamplainMapSourcePrivate;

struct _ChamplainMapSourcePrivate
{
  ChamplainMapSource *next_source;

  gulong sig_handler_id;
};

static void reload_tiles_cb (ChamplainMapSource *orig, ChamplainMapSource *self);

static void
champlain_map_source_get_property (GObject *object,
                                   guint prop_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE(object);

  switch (prop_id)
    {
    case PROP_NEXT_SOURCE:
      g_value_set_object (value, priv->next_source);
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

  switch (prop_id)
    {
    case PROP_NEXT_SOURCE:
      champlain_map_source_set_next_source (map_source,
                                            g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
champlain_map_source_dispose (GObject *object)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE(object);

  if (priv->next_source)
    {
      g_object_unref (priv->next_source);

      priv->next_source = NULL;
    }

  G_OBJECT_CLASS (champlain_map_source_parent_class)->dispose (object);
}

static void
champlain_map_source_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_map_source_parent_class)->finalize (object);
}

static void
champlain_map_source_constructed  (GObject *object)
{
  if (G_OBJECT_CLASS (champlain_map_source_parent_class)->constructed)
    G_OBJECT_CLASS (champlain_map_source_parent_class)->constructed (object);
}

static void
champlain_map_source_class_init (ChamplainMapSourceClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (ChamplainMapSourcePrivate));

  object_class->finalize = champlain_map_source_finalize;
  object_class->dispose = champlain_map_source_dispose;
  object_class->get_property = champlain_map_source_get_property;
  object_class->set_property = champlain_map_source_set_property;
  object_class->constructed = champlain_map_source_constructed;

  klass->get_id = NULL;
  klass->get_name = NULL;
  klass->get_license = NULL;
  klass->get_license_uri = NULL;
  klass->get_min_zoom_level = NULL;
  klass->get_max_zoom_level = NULL;
  klass->get_tile_size = NULL;
  klass->get_projection = NULL;

  klass->fill_tile = NULL;

  /**
  * ChamplainMapSource:next-source:
  *
  * Next source in the loading chain.
  *
  * Since: 0.6
  */
  pspec = g_param_spec_object ("next-source",
                               "Next Source",
                               "Next source in the loading chain",
                               CHAMPLAIN_TYPE_MAP_SOURCE,
                               G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_NEXT_SOURCE, pspec);

  /**
  * ChamplainMapSource::reload-tiles:
  * @map_source: the #ChamplainMapSource that received the signal
  *
  * The ChamplainMapSource::reload-tiles signal is emitted when the map source
  * changed its style or data
  *
  * Since: 0.6
  */
  champlain_map_source_signals[RELOAD_TILES] =
    g_signal_new ("reload-tiles", G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE,
                  0, NULL);
}

static void
champlain_map_source_init (ChamplainMapSource *map_source)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE(map_source);
  priv->next_source = NULL;
  priv->sig_handler_id = 0;
}

/**
 * champlain_map_source_get_next_source:
 * @map_source: a #ChamplainMapSource
 *
 * Get the next source in the chain.
 *
 * Returns: the next source in the chain.
 *
 * Since: 0.6
 */
ChamplainMapSource *
champlain_map_source_get_next_source (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), NULL);

  ChamplainMapSourcePrivate *priv = GET_PRIVATE(map_source);
  return priv->next_source;
}

static
void reload_tiles_cb (ChamplainMapSource *orig, ChamplainMapSource *self)
{
  /* propagate the signal up the chain */
  g_signal_emit_by_name (self, "reload-tiles", NULL);
}

/**
 * champlain_map_source_set_next_source:
 * @map_source: a #ChamplainMapSource
 * @next_source: the next #ChamplainMapSource in the chain
 *
 * Sets the next map source in the chain.
 *
 * Since: 0.6
 */
void
champlain_map_source_set_next_source (ChamplainMapSource *map_source,
                                      ChamplainMapSource *next_source)
{
  g_return_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source));

  ChamplainMapSourcePrivate *priv = GET_PRIVATE(map_source);

  if (priv->next_source != NULL)
    {
      if (g_signal_handler_is_connected (priv->next_source, priv->sig_handler_id))
        g_signal_handler_disconnect (priv->next_source, priv->sig_handler_id);

      g_object_unref (priv->next_source);
    }

  if (next_source)
    {
      g_return_if_fail (CHAMPLAIN_IS_MAP_SOURCE (next_source));

      g_object_ref_sink (next_source);

      priv->sig_handler_id = g_signal_connect (next_source, "reload-tiles",
                             G_CALLBACK (reload_tiles_cb), map_source);
    }

  priv->next_source = next_source;

  g_object_notify (G_OBJECT (map_source), "next-source");
}

/**
 * champlain_map_source_get_id:
 * @map_source: a #ChamplainMapSource
 *
 * Gets map source's id.
 *
 * Returns: the map source's id.
 *
 * Since: 0.4
 */
const gchar *
champlain_map_source_get_id (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), NULL);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_id (map_source);
}

/**
 * champlain_map_source_get_name:
 * @map_source: a #ChamplainMapSource
 *
 * Gets map source's name.
 *
 * Returns: the map source's name.
 *
 * Since: 0.4
 */
const gchar *
champlain_map_source_get_name (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), NULL);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_name (map_source);
}

/**
 * champlain_map_source_get_license:
 * @map_source: a #ChamplainMapSource
 *
 * Gets map source's license.
 *
 * Returns: the map source's license.
 *
 * Since: 0.4
 */
const gchar *
champlain_map_source_get_license (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), NULL);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_license (map_source);
}

/**
 * champlain_map_source_get_license_uri:
 * @map_source: a #ChamplainMapSource
 *
 * Gets map source's license URI.
 *
 * Returns: the map source's license URI.
 *
 * Since: 0.4
 */
const gchar *
champlain_map_source_get_license_uri (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), NULL);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_license_uri (map_source);
}

/**
 * champlain_map_source_get_min_zoom_level:
 * @map_source: a #ChamplainMapSource
 *
 * Gets map source's minimum zoom level.
 *
 * Returns: the miminum zoom level this map source supports
 *
 * Since: 0.4
 */
guint
champlain_map_source_get_min_zoom_level (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_min_zoom_level (map_source);
}

/**
 * champlain_map_source_get_max_zoom_level:
 * @map_source: a #ChamplainMapSource
 *
 * Gets map source's maximum zoom level.
 *
 * Returns: the maximum zoom level this map source supports
 *
 * Since: 0.4
 */
guint
champlain_map_source_get_max_zoom_level (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_max_zoom_level (map_source);
}

/**
 * champlain_map_source_get_tile_size:
 * @map_source: a #ChamplainMapSource
 *
 * Gets map source's tile size.
 *
 * Returns: the tile's size (width and height) in pixels for this map source
 *
 * Since: 0.4
 */
guint
champlain_map_source_get_tile_size (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_tile_size (map_source);
}

/**
 * champlain_map_source_get_projection:
 * @map_source: a #ChamplainMapSource
 *
 * Gets map source's projection.
 *
 * Returns: the map source's projection.
 *
 * Since: 0.4
 */
ChamplainMapProjection
champlain_map_source_get_projection (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), CHAMPLAIN_MAP_PROJECTION_MERCATOR);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_projection (map_source);
}

/**
 * champlain_map_source_get_x:
 * @map_source: a #ChamplainMapSource
 * @zoom_level: the zoom level
 * @longitude: a longitude
 *
 * Gets the x position on the map using this map source's projection.
 * (0, 0) is located at the top left.
 *
 * Returns: the x position
 *
 * Since: 0.4
 */
guint
champlain_map_source_get_x (ChamplainMapSource *map_source,
                            guint zoom_level,
                            gdouble longitude)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  // FIXME: support other projections
  return ((longitude + 180.0) / 360.0 * pow (2.0, zoom_level)) * champlain_map_source_get_tile_size (map_source);
}

/**
 * champlain_map_source_get_y:
 * @map_source: a #ChamplainMapSource
 * @zoom_level: the zoom level
 * @latitude: a latitude
 *
 * Gets the y position on the map using this map source's projection.
 * (0, 0) is located at the top left.
 *
 * Returns: the y position
 *
 * Since: 0.4
 */
guint
champlain_map_source_get_y (ChamplainMapSource *map_source,
                            guint zoom_level,
                            gdouble latitude)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  // FIXME: support other projections
  return ((1.0 - log (tan (latitude * M_PI / 180.0) + 1.0 /
                      cos (latitude * M_PI / 180.0)) /
           M_PI) / 2.0 * pow (2.0, zoom_level)) * champlain_map_source_get_tile_size (map_source);
}

/**
 * champlain_map_source_get_longitude:
 * @map_source: a #ChamplainMapSource
 * @zoom_level: the zoom level
 * @x: a x position
 *
 * Gets the longitude corresponding to this x position in the map source's
 * projection.
 *
 * Returns: the longitude
 *
 * Since: 0.4
 */
gdouble
champlain_map_source_get_longitude (ChamplainMapSource *map_source,
                                    guint zoom_level,
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
 * Gets the latitude corresponding to this y position in the map source's
 * projection.
 *
 * Returns: the latitude
 *
 * Since: 0.4
 */
gdouble
champlain_map_source_get_latitude (ChamplainMapSource *map_source,
                                   guint zoom_level,
                                   guint y)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);
  //ChamplainMapSourcePrivate *priv = map_source->priv;
  // FIXME: support other projections
  gdouble dy = (float)y / champlain_map_source_get_tile_size (map_source);
  gdouble n = M_PI - 2.0 * M_PI * dy / pow (2.0, zoom_level);
  return 180.0 / M_PI * atan (0.5 * (exp (n) - exp (-n)));
}

/**
 * champlain_map_source_get_row_count:
 * @map_source: a #ChamplainMapSource
 * @zoom_level: the zoom level
 *
 * Gets the number of tiles in a row at this zoom level for this map source.
 *
 * Returns: the number of tiles in a row
 *
 * Since: 0.4
 */
guint
champlain_map_source_get_row_count (ChamplainMapSource *map_source,
                                    guint zoom_level)
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
 * Gets the number of tiles in a column at this zoom level for this map
 * source.
 *
 * Returns: the number of tiles in a column
 *
 * Since: 0.4
 */
guint
champlain_map_source_get_column_count (ChamplainMapSource *map_source,
                                       guint zoom_level)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  //ChamplainMapSourcePrivate *priv = map_source->priv;
  // FIXME: support other projections
  return pow (2, zoom_level);
}

#define EARTH_RADIUS 6378137.0 /* meters, Equatorial radius */

/**
 * champlain_map_source_get_meters_per_pixel:
 * @map_source: a #ChamplainMapSource
 * @zoom_level: the zoom level
 * @latitude: a latitude
 * @longitude: a longitude
 *
 * Gets meters per pixel at the position on the map using this map source's projection.
 *
 * Returns: the meters per pixel
 *
 * Since: 0.4.3
 */
gdouble
champlain_map_source_get_meters_per_pixel (ChamplainMapSource *map_source,
    guint zoom_level,
    gdouble latitude,
    gdouble longitude)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0.0);

  /* Width is in pixels. (1 px)
     m/px = radius_at_latitude / width_in_pixels
     k = radius of earth = 6 378.1 km
     radius_at_latitude = 2π * k * sin (π/2-θ)
  */

  gdouble tile_size = champlain_map_source_get_tile_size (map_source);
  // FIXME: support other projections
  return 2.0 * M_PI * EARTH_RADIUS * sin (M_PI/2.0 - M_PI / 180.0 * latitude) /
         (tile_size * champlain_map_source_get_row_count (map_source, zoom_level));
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
