/*
 * Copyright (C) 2008 Pierre-Luc Beaudoin <pierre-luc@squidy.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"

#include "champlain_defines.h"
#include "champlainmarker.h"
#include "champlain_private.h"
#include "champlain.h"
#include "champlain-marshal.h"
#include "map.h"
#include "tile.h"
#include "zoomlevel.h"

#include <clutter/clutter.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk-clutter-embed.h>
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
  PROP_ANCHOR_X,
  PROP_ANCHOR_Y,
};

static guint champlain_marker_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (ChamplainMarker, champlain_marker, CLUTTER_TYPE_GROUP);

static void 
champlain_marker_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec)
{
    ChamplainMarker* marker = CHAMPLAIN_MARKER(object);
    ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (marker);

    switch(prop_id) 
      {
        case PROP_LONGITUDE:
          g_value_set_double(value, priv->lon);
          break;
        case PROP_LATITUDE:
          g_value_set_double(value, priv->lat);
          break;
        default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      }
}

static void 
champlain_marker_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec *pspec)
{
    ChamplainMarker* marker = CHAMPLAIN_MARKER(object);
    ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (marker);

    switch(prop_id) 
    {
      case PROP_LONGITUDE:
        {
          gdouble lon = g_value_get_double(value);
          champlain_marker_set_position(marker, lon, priv->lat);
          break;
        }
      case PROP_LATITUDE:
        {
          gdouble lat = g_value_get_double(value);
          champlain_marker_set_position(marker, priv->lon, lat);
          break;
        }
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
champlain_marker_finalize (GObject * object)
{
  ChamplainMarker *marker = CHAMPLAIN_MARKER (object);
  ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (marker);

  G_OBJECT_CLASS (champlain_marker_parent_class)->finalize (object);
}

static void
champlain_marker_class_init (ChamplainMarkerClass *champlainMarkerClass)
{
  g_type_class_add_private (champlainMarkerClass, sizeof (ChamplainMarkerPrivate));

  GObjectClass *objectClass = G_OBJECT_CLASS (champlainMarkerClass);
  objectClass->finalize = champlain_marker_finalize;
  objectClass->get_property = champlain_marker_get_property;
  objectClass->set_property = champlain_marker_set_property;
  
  /**
  * ChamplainMarker:longitude:
  *
  * The longitude coordonate of the map
  *
  * Since: 0.2
  */
  g_object_class_install_property(objectClass, PROP_LONGITUDE,
                                  g_param_spec_double("longitude",
                                                     "Longitude",
                                                     "The longitude coordonate of the marker",
                                                     -180.0f,
                                                     180.0f,
                                                     0.0f,
                                                     CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainMarker:latitude:
  *
  * The latitude coordonate of the map
  *
  * Since: 0.2
  */
  g_object_class_install_property(objectClass, PROP_LATITUDE,
                                  g_param_spec_double("latitude",
                                                     "Latitude",
                                                     "The latitude coordonate of the marker",
                                                     -90.0f,
                                                     90.0f,
                                                     0.0f,
                                                     CHAMPLAIN_PARAM_READWRITE));

}

static void
champlain_marker_init (ChamplainMarker *champlainMarker)
{
  ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (champlainMarker);
  priv->anchor.x = 0;
  priv->anchor.y = 0;
}


/**
 * champlain_marker_new:
 *
 * Returns a new #ChamplainWidget ready to be used as a #ClutterActor.
 *
 * Since: 0.2
 */
ClutterActor *
champlain_marker_new ()
{
  ChamplainMarker *marker;
  
  marker = CHAMPLAIN_MARKER (g_object_new (CHAMPLAIN_TYPE_MARKER, NULL));
  ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (marker);
  
  return CLUTTER_ACTOR (marker);
}

/**
 * champlain_marker_set_position:
 * @marker: a #ChamplainMarker
 * @longitude: the longitude to center the map at
 * @latitude: the longitude to center the map at
 *
 * Positions the marker on the map at the coordinates
 *
 * Since: 0.2
 */
void
champlain_marker_set_position (ChamplainMarker *champlainMarker, gdouble longitude, gdouble latitude)
{
  ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (champlainMarker);

  priv->lon = longitude;
  priv->lat = latitude;
  
  g_object_notify(G_OBJECT(champlainMarker), "longitude");
  g_object_notify(G_OBJECT(champlainMarker), "latitude");
}


/**
 * champlain_marker_set_anchor:
 * @marker: a #ChamplainMarker
 * @x: the position in the actor that represents the longitude
 * @y: the position in the actor that represents the latitude
 *
 * Marks the point (x, y) as the place where the #ChamplainMarker position is at (longitude, latitude).
 *
 * Since: 0.2
 */
void 
champlain_marker_set_anchor (ChamplainMarker *champlainMarker, gint x, gint y)
{
  ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (champlainMarker);
  
  priv->anchor.x = x;
  priv->anchor.y = y;
  
  clutter_actor_set_position(CLUTTER_ACTOR(champlainMarker), -x, -y);
}

