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

/**
 * SECTION:champlain-base-marker
 * @short_description: a base marker to identify points of interest on a map
 *
 * Base markers reprensent points of interest on a map. base markers need to be
 * placed on a layer (a #cluttergroup).  layers have to be added to a
 * #champlainview for the base_markers to show on the map.
 *
 * a basemarker is nothing more than a regular #clutteractor.  you can draw on
 * it what ever you want.  don't forget to set the anchor position in the base
 * marker using #clutter_actor_set_anchor_point. set the base_markers position
 * on the map using #champlain_base_marker_set_position.
 *
 * champlain has a more evoluted type of markers with text and image support.
 * see #champlainmarker.
 */

#include "config.h"

#include "champlain-base-marker.h"

#include "champlain.h"
#include "champlain-defines.h"
#include "champlain-marshal.h"
#include "champlain-private.h"
#include "champlain-map.h"
#include "champlain-tile.h"
#include "champlain-zoom-level.h"

#include <clutter/clutter.h>
#include <clutter-cairo/clutter-cairo.h>
#include <glib.h>
#include <glib-object.h>
#include <cairo.h>
#include <math.h>

enum
{
  /* normal signals */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_LONGITUDE,
  PROP_LATITUDE,
};

//static guint champlain_base_marker_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (ChamplainBaseMarker, champlain_base_marker, CLUTTER_TYPE_GROUP);

#define CHAMPLAIN_BASE_MARKER_GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_BASE_MARKER, ChamplainBaseMarkerPrivate))

static void
champlain_base_marker_get_property (GObject *object,
                               guint prop_id,
                               GValue *value,
                               GParamSpec *pspec)
{
    ChamplainBaseMarker *base_marker = CHAMPLAIN_BASE_MARKER (object);
    ChamplainBaseMarkerPrivate *priv = CHAMPLAIN_BASE_MARKER_GET_PRIVATE (base_marker);

    switch (prop_id)
      {
        case PROP_LONGITUDE:
          g_value_set_double (value, priv->lon);
          break;
        case PROP_LATITUDE:
          g_value_set_double (value, priv->lat);
          break;
        default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

static void
champlain_base_marker_set_property (GObject *object,
                               guint prop_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
    ChamplainBaseMarker *base_marker = CHAMPLAIN_BASE_MARKER (object);
    ChamplainBaseMarkerPrivate *priv = CHAMPLAIN_BASE_MARKER_GET_PRIVATE (base_marker);

    switch (prop_id)
    {
      case PROP_LONGITUDE:
        {
          gdouble lon = g_value_get_double (value);
          champlain_base_marker_set_position (base_marker, lon, priv->lat);
          break;
        }
      case PROP_LATITUDE:
        {
          gdouble lat = g_value_get_double (value);
          champlain_base_marker_set_position (base_marker, priv->lon, lat);
          break;
        }
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
champlain_base_marker_finalize (GObject *object)
{
  //ChamplainBaseMarker *base_marker = CHAMPLAIN_BASE_MARKER (object);
  //ChamplainBaseMarkerPrivate *priv = CHAMPLAIN_BASE_MARKER_GET_PRIVATE (base_marker);

  G_OBJECT_CLASS (champlain_base_marker_parent_class)->finalize (object);
}

static void
champlain_base_marker_class_init (ChamplainBaseMarkerClass *champlainBaseMarkerClass)
{
  g_type_class_add_private (champlainBaseMarkerClass, sizeof (ChamplainBaseMarkerPrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (champlainBaseMarkerClass);
  object_class->finalize = champlain_base_marker_finalize;
  object_class->get_property = champlain_base_marker_get_property;
  object_class->set_property = champlain_base_marker_set_property;

  /**
  * ChamplainBaseMarker:longitude:
  *
  * The longitude coordonate of the map
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class, PROP_LONGITUDE,
      g_param_spec_double ("longitude", "Longitude",
          "The longitude coordonate of the base_marker",
          -180.0f, 180.0f, 0.0f, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainBaseMarker:latitude:
  *
  * The latitude coordonate of the map
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class, PROP_LATITUDE,
      g_param_spec_double ("latitude", "Latitude",
          "The latitude coordonate of the base_marker",
          -90.0f, 90.0f, 0.0f, CHAMPLAIN_PARAM_READWRITE));

}

static void
champlain_base_marker_init (ChamplainBaseMarker *marker)
{
  ChamplainBaseMarkerPrivate *priv = CHAMPLAIN_BASE_MARKER_GET_PRIVATE (marker);
  marker->priv = priv;
}


/**
 * champlain_base_marker_new:
 *
 * Returns a new #ChamplainBaseMarker ready to be used as a #ClutterActor.
 *
 * Since: 0.4
 */
ClutterActor *
champlain_base_marker_new (void)
{
  ChamplainBaseMarker *base_marker;

  base_marker = CHAMPLAIN_BASE_MARKER (g_object_new (CHAMPLAIN_TYPE_BASE_MARKER, NULL));
  //ChamplainBaseMarkerPrivate *priv = CHAMPLAIN_BASE_MARKER_GET_PRIVATE (base_marker);

  return CLUTTER_ACTOR (base_marker);
}

/**
 * champlain_base_marker_set_position:
 * @base_marker: a #ChamplainBaseMarker
 * @longitude: the longitude to center the map at
 * @latitude: the longitude to center the map at
 *
 * Positions the base_marker on the map at the coordinates
 *
 * Since: 0.4
 */
void
champlain_base_marker_set_position (ChamplainBaseMarker *champlainBaseMarker, gdouble latitude, gdouble longitude)
{
  g_return_if_fail (CHAMPLAIN_IS_BASE_MARKER (champlainBaseMarker));

  ChamplainBaseMarkerPrivate *priv = CHAMPLAIN_BASE_MARKER_GET_PRIVATE (champlainBaseMarker);

  priv->lon = longitude;
  priv->lat = latitude;

  g_object_notify (G_OBJECT (champlainBaseMarker), "latitude");
  g_object_notify (G_OBJECT (champlainBaseMarker), "longitude");
}
