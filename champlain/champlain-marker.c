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
 * SECTION:champlain-marker
 * @short_description: A marker to identify points of interest on a map
 *
 * Markers reprensent points of interest on a map. Markers need to be placed on
 * a layer (a #ClutterGroup).  Layers have to be added to a #ChamplainView for
 * the markers to show on the map.
 *
 * A marker is nothing more than a regular #ClutterActor.  You can draw on it
 * what ever you want.  Don't forget to set the anchor position in the marker
 * using #champlain_marker_set_anchor.  Set the markers position on the map
 * using #champlain_marker_set_position.
 *
 * Champlain has a default type of markers with text. To create one,
 * use #champlain_marker_new_with_label.
 */

#include "config.h"

#include "champlain-marker.h"

#include "champlain.h"
#include "champlain-base-marker.h"
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
};

//static guint champlain_marker_signals[LAST_SIGNAL] = { 0, };

struct _ChamplainMarkerPrivate
{
  gboolean tmp;
};

G_DEFINE_TYPE (ChamplainMarker, champlain_marker, CHAMPLAIN_TYPE_BASE_MARKER);

#define CHAMPLAIN_MARKER_GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_MARKER, ChamplainMarkerPrivate))

static void
champlain_marker_get_property (GObject *object,
                               guint prop_id,
                               GValue *value,
                               GParamSpec *pspec)
{
    //ChamplainMarker *marker = CHAMPLAIN_MARKER (object);
    //ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (marker);

    switch (prop_id)
      {
        default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

static void
champlain_marker_set_property (GObject *object,
                               guint prop_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
    //ChamplainMarker *marker = CHAMPLAIN_MARKER (object);
    //ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (marker);

    switch (prop_id)
    {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
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
champlain_marker_class_init (ChamplainMarkerClass *markerClass)
{
  g_type_class_add_private (markerClass, sizeof (ChamplainMarkerPrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (markerClass);
  object_class->finalize = champlain_marker_finalize;
  object_class->get_property = champlain_marker_get_property;
  object_class->set_property = champlain_marker_set_property;

}

static void
champlain_marker_init (ChamplainMarker *marker)
{
  ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (marker);
  marker->priv = priv;
}

/**
 * champlain_marker_new:
 *
 * Returns a new #ChamplainMarker ready to be used as a #ClutterActor.
 *
 * Since: 0.2
 */
ClutterActor *
champlain_marker_new (void)
{
  ChamplainMarker *marker;

  marker = CHAMPLAIN_MARKER (g_object_new (CHAMPLAIN_TYPE_MARKER, NULL));
  //ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (marker);

  return CLUTTER_ACTOR (marker);
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
  ChamplainMarker *marker = CHAMPLAIN_MARKER (champlain_marker_new ());
  ClutterColor default_text_color = { 0x22, 0x22, 0x22, 0xFF },
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
  clutter_label_set_color (CLUTTER_LABEL (actor), text_color);
  clutter_container_add_actor (CLUTTER_CONTAINER (marker), actor);

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
  clutter_container_add_actor (CLUTTER_CONTAINER (marker), bg);
  clutter_actor_raise (actor, bg);

  clutter_actor_set_anchor_point (CLUTTER_ACTOR (marker), 0, text_height + point);

  return CLUTTER_ACTOR (marker);
}

/**
 * champlain_marker_new_with_image:
 * @filename: The filename of the image.
 * @error: Return location for an error.
 *
 * Returns a new #ChamplainMarker with a drawn marker containing the given
 * image.
 *
 */
ClutterActor *
champlain_marker_new_with_image (const gchar *filename, GError **error)
{
  if (filename == NULL)
    return NULL;

  ChamplainMarker *marker = CHAMPLAIN_MARKER (champlain_marker_new ());
  ClutterActor *actor = clutter_texture_new_from_file (filename, error);

  if (actor == NULL){
    g_object_unref (G_OBJECT (marker));
    return NULL;
  }

  clutter_container_add_actor (CLUTTER_CONTAINER (marker), actor);

  return CLUTTER_ACTOR (marker);
}

/**
 * champlain_marker_new_with_image_full:
 * @filename: The name of an image file to load.
 * @width: Width of the image in pixel or -1.
 * @height: Height of the image in pixel or -1.
 * @anchor_x: X coordinate of the anchor point.
 * @anchor_y: Y coordinate of the anchor point.
 * @error: Return location for an error.
 *
 * Returns a new #ChamplainMarker with a drawn marker containing the given
 * image.
 *
 */
ClutterActor *
champlain_marker_new_with_image_full (const gchar *filename,
                                      gint width,
                                      gint height,
                                      gint anchor_x,
                                      gint anchor_y,
                                      GError **error)
{
  if (filename == NULL)
    return NULL;

  ChamplainMarker *marker = CHAMPLAIN_MARKER (champlain_marker_new ());
  ClutterActor *actor = clutter_texture_new_from_file (filename, error);

  if (actor == NULL)
    {
      g_object_unref (G_OBJECT (marker));
      return NULL;
    }

  clutter_actor_set_size (actor, width, height);

  clutter_container_add_actor (CLUTTER_CONTAINER (marker), actor);
  clutter_actor_set_anchor_point (CLUTTER_ACTOR (marker), anchor_x,
      anchor_y);

  return CLUTTER_ACTOR (marker);
}

