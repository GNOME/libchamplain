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

#include "config.h"

#include "champlaindefines.h"
#include "champlainmarker.h"
#include "champlainprivate.h"
#include "champlain.h"
#include "champlain-marshal.h"
#include "map.h"
#include "tile.h"
#include "zoomlevel.h"

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
  PROP_ANCHOR_X,
  PROP_ANCHOR_Y,
};

//static guint champlain_marker_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (ChamplainMarker, champlain_marker, CLUTTER_TYPE_GROUP);

static void
champlain_marker_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    ChamplainMarker *marker = CHAMPLAIN_MARKER(object);
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
champlain_marker_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    ChamplainMarker *marker = CHAMPLAIN_MARKER(object);
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
champlain_marker_finalize (GObject *object)
{
  //ChamplainMarker *marker = CHAMPLAIN_MARKER (object);
  //ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (marker);

  G_OBJECT_CLASS (champlain_marker_parent_class)->finalize (object);
}

static void
champlain_marker_class_init (ChamplainMarkerClass *champlainMarkerClass)
{
  g_type_class_add_private (champlainMarkerClass, sizeof (ChamplainMarkerPrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (champlainMarkerClass);
  object_class->finalize = champlain_marker_finalize;
  object_class->get_property = champlain_marker_get_property;
  object_class->set_property = champlain_marker_set_property;

  /**
  * ChamplainMarker:longitude:
  *
  * The longitude coordonate of the map
  *
  * Since: 0.2
  */
  g_object_class_install_property(object_class, PROP_LONGITUDE,
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
  g_object_class_install_property(object_class, PROP_LATITUDE,
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
 * Returns a new #ChamplainMarker ready to be used as a #ClutterActor.
 *
 * Since: 0.2
 */
ClutterActor *
champlain_marker_new ()
{
  ChamplainMarker *marker;

  marker = CHAMPLAIN_MARKER (g_object_new (CHAMPLAIN_TYPE_MARKER, NULL));
  //ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (marker);

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
champlain_marker_set_position (ChamplainMarker *champlainMarker, gdouble latitude, gdouble longitude)
{
  g_return_if_fail(CHAMPLAIN_IS_MARKER(champlainMarker));

  ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (champlainMarker);

  priv->lon = longitude;
  priv->lat = latitude;

  g_object_notify(G_OBJECT(champlainMarker), "latitude");
  g_object_notify(G_OBJECT(champlainMarker), "longitude");
}

/**
 * champlain_marker_new_with_label:
 * @label: the text of the label
 * @font: the font to use to draw the text, for example "Courrier Bold 11", can be NULL
 * @text_color: a #ClutterColor, the color of the text, can be NULL
 * @marker_color: a #ClutterColor, the color of the marker, can be NULL
 *
 * Returns a new #ChamplainMarker with a drawn marker containing the given text.
 *
 * Since: 0.2
 */
ClutterActor *
champlain_marker_new_with_label (const gchar *label,
                                 const gchar *font,
                                 ClutterColor *text_color,
                                 ClutterColor *marker_color)
{
  ChamplainMarker *champlainMarker = CHAMPLAIN_MARKER(champlain_marker_new ());
  ClutterColor default_text_color = { 0x22, 022, 0x22, 0xFF },
               default_marker_color = { 0x2A, 0xB1, 0x26, 0xEE },
               darker_color;
  ClutterActor *actor, *bg;
  cairo_t *cr;
  guint text_width, text_height, point;
  const gint padding = 5;

  if (!font)
    font = "Sans 11";
  if (!text_color)
    text_color = &default_text_color;
  if (!marker_color)
    marker_color = &default_marker_color;

  actor = clutter_label_new_with_text (font, label);
  clutter_actor_set_position (actor, padding, padding / 2.0);
  text_width = clutter_actor_get_width (actor) + 2 * padding;
  text_height = clutter_actor_get_height (actor)+ padding;
  clutter_label_set_color (CLUTTER_LABEL(actor), text_color);
  clutter_container_add_actor (CLUTTER_CONTAINER(champlainMarker), actor);

  point = (text_height + 2 * padding) / 4.0;

  bg = clutter_cairo_new (text_width, text_height + point);
  cr = clutter_cairo_create (CLUTTER_CAIRO (bg));

  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_move_to (cr, 0, 0);
  cairo_line_to (cr, text_width, 0);
  cairo_line_to (cr, text_width, text_height);
  cairo_line_to (cr, point, text_height);
  cairo_line_to (cr, 0, text_height + point);
  cairo_close_path (cr);

  cairo_set_line_width (cr, 1.0);
  cairo_set_source_rgba (cr,
                        marker_color->red / 255.0,
                        marker_color->green / 255.0,
                        marker_color->blue / 255.0,
                        marker_color->alpha / 255.0);
  cairo_fill_preserve (cr);
  clutter_color_darken (marker_color, &darker_color);
  cairo_set_source_rgba (cr,
                        darker_color.red / 255.0,
                        darker_color.green / 255.0,
                        darker_color.blue / 255.0,
                        darker_color.alpha / 255.0);
  cairo_stroke (cr);


  cairo_destroy (cr);
  clutter_container_add_actor (CLUTTER_CONTAINER(champlainMarker), bg);
  clutter_actor_raise (actor, bg);

  clutter_actor_set_anchor_point (CLUTTER_ACTOR(champlainMarker), 0, text_height + point);

  return CLUTTER_ACTOR (champlainMarker);
}

