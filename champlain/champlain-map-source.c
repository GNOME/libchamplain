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

static void reload_tiles_cb(ChamplainMapSource *orig, ChamplainMapSource *self);

static void
champlain_map_source_get_property (GObject *object,
                                   guint prop_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE(object);

  switch(prop_id)
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

  switch(prop_id)
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

  klass->fill_tile = NULL;

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
  * The ::reload-tiles signal is emitted when the map source changed
  * its style or data
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

ChamplainMapSource *
champlain_map_source_get_next_source (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), NULL);

  ChamplainMapSourcePrivate *priv = GET_PRIVATE(map_source);
  return priv->next_source;
}

static
void reload_tiles_cb(ChamplainMapSource *orig, ChamplainMapSource *self)
{
  /* propagate the signal up the chain */
  g_signal_emit_by_name (self, "reload-tiles", NULL);
}

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

      g_object_unref(priv->next_source);
    }

  if (next_source)
    {
      g_return_if_fail (CHAMPLAIN_IS_MAP_SOURCE (next_source));

      g_object_ref_sink(next_source);

      priv->sig_handler_id = g_signal_connect (next_source, "reload-tiles",
                             G_CALLBACK (reload_tiles_cb), map_source);
    }

  priv->next_source = next_source;

  g_object_notify (G_OBJECT (map_source), "next-source");
}

const gchar *
champlain_map_source_get_id (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), NULL);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_id (map_source);
}

const gchar *
champlain_map_source_get_name (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), NULL);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_name (map_source);
}

const gchar *
champlain_map_source_get_license (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), NULL);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_license (map_source);
}

const gchar *
champlain_map_source_get_license_uri (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), NULL);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_license_uri (map_source);
}

guint
champlain_map_source_get_min_zoom_level (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_min_zoom_level (map_source);
}

guint
champlain_map_source_get_max_zoom_level (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_max_zoom_level (map_source);
}

guint
champlain_map_source_get_tile_size (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  return CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->get_tile_size (map_source);
}

guint
champlain_map_source_get_x (ChamplainMapSource *map_source,
                            guint zoom_level,
                            gdouble longitude)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  // FIXME: support other projections
  return ((longitude + 180.0) / 360.0 * pow(2.0, zoom_level)) * champlain_map_source_get_tile_size (map_source);
}

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

guint
champlain_map_source_get_row_count (ChamplainMapSource *map_source,
                                    guint zoom_level)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source), 0);

  //ChamplainMapSourcePrivate *priv = map_source->priv;
  // FIXME: support other projections
  return pow (2, zoom_level);
}

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

void
champlain_map_source_fill_tile (ChamplainMapSource *map_source,
                                ChamplainTile *tile)
{
  g_return_if_fail (CHAMPLAIN_IS_MAP_SOURCE (map_source));

  CHAMPLAIN_MAP_SOURCE_GET_CLASS (map_source)->fill_tile (map_source, tile);
}
