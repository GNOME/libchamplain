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
 * a layer (a #ChamplainLayer).  Layers have to be added to a #ChamplainView for
 * the markers to show on the map.
 *
 * A marker is nothing more than a regular #ClutterActor.  You can draw on it
 * what ever you want. Set the markers position on the map
 * using #champlain_marker_set_position.
 *
 * Champlain has a default type of markers with text. To create one,
 * use #champlain_marker_new_with_text.
 */

#include "config.h"

#include "champlain-marker.h"

#include "champlain.h"
#include "champlain-base-marker.h"
#include "champlain-defines.h"
#include "champlain-marshal.h"
#include "champlain-private.h"
#include "champlain-tile.h"

#include <clutter/clutter.h>
#include <glib.h>
#include <glib-object.h>
#include <cairo.h>
#include <math.h>
#include <string.h>

#define DEFAULT_FONT_NAME "Sans 11"

static ClutterColor SELECTED_COLOR = { 0x00, 0x33, 0xcc, 0xff };

static ClutterColor DEFAULT_COLOR = { 0x33, 0x33, 0x33, 0xff };

enum
{
  /* normal signals */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_COLOR,
  PROP_SIZE,
};

/* static guint champlain_marker_signals[LAST_SIGNAL] = { 0, }; */

struct _ChamplainPointPrivate
{
  ClutterColor *color;
  gdouble size;

  guint redraw_id;
};

G_DEFINE_TYPE (ChamplainPoint, champlain_point, CHAMPLAIN_TYPE_BASE_MARKER);

#define GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CHAMPLAIN_TYPE_POINT, ChamplainPointPrivate))

static void draw_marker (ChamplainPoint *marker);

/**
 * champlain_marker_set_highlight_color:
 * @color: a #ClutterColor
 *
 * Changes the highlight color, this is to ensure a better integration with
 * the desktop, this is automatically done by GtkChamplainEmbed.
 *
 * Since: 0.4
 */
void
champlain_point_set_highlight_color (ClutterColor *color)
{
  SELECTED_COLOR.red = color->red;
  SELECTED_COLOR.green = color->green;
  SELECTED_COLOR.blue = color->blue;
  SELECTED_COLOR.alpha = color->alpha;
}


/**
 * champlain_marker_get_highlight_color:
 *
 * Gets the highlight color.
 *
 * Returns: the highlight color. Should not be freed.
 *
 * Since: 0.4.1
 */
const ClutterColor *
champlain_point_get_highlight_color ()
{
  return &SELECTED_COLOR;
}



static void
champlain_point_get_property (GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainPointPrivate *priv = CHAMPLAIN_POINT (object)->priv;

  switch (prop_id)
    {
    case PROP_COLOR:
      clutter_value_set_color (value, priv->color);
      break;
      
    case PROP_SIZE:
      g_value_set_double (value, priv->size);
      break;
      
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
champlain_point_set_property (GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainPoint *marker = CHAMPLAIN_POINT (object);

  switch (prop_id)
    {
    case PROP_COLOR:
      champlain_point_set_color (marker, clutter_value_get_color (value));
      break;

    case PROP_SIZE:
      champlain_point_set_size (marker, g_value_get_double (value));
      break;
      
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
champlain_point_dispose (GObject *object)
{
//  ChamplainPointPrivate *priv = CHAMPLAIN_POINT (object)->priv;

  G_OBJECT_CLASS (champlain_point_parent_class)->dispose (object);
}


static void
champlain_point_finalize (GObject *object)
{
  ChamplainPointPrivate *priv = CHAMPLAIN_POINT (object)->priv;

  if (priv->color)
    {
      clutter_color_free (priv->color);
      priv->color = NULL;
    }

  G_OBJECT_CLASS (champlain_point_parent_class)->finalize (object);
}


static void
champlain_point_class_init (ChamplainPointClass *markerClass)
{
  g_type_class_add_private (markerClass, sizeof (ChamplainPointPrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (markerClass);
  object_class->finalize = champlain_point_finalize;
  object_class->dispose = champlain_point_dispose;
  object_class->get_property = champlain_point_get_property;
  object_class->set_property = champlain_point_set_property;

  /**
   * ChamplainMarker:color:
   *
   * The marker's color
   *
   * Since: 0.4
   */
  g_object_class_install_property (object_class, PROP_COLOR,
      clutter_param_spec_color ("color", "Color", "The marker's color",
          &DEFAULT_COLOR, CHAMPLAIN_PARAM_READWRITE));

  /**
   * TODO
   */
  g_object_class_install_property (object_class, PROP_SIZE,
      g_param_spec_double ("size", "Size", "The point size", 0, G_MAXDOUBLE,
          12, CHAMPLAIN_PARAM_READWRITE));
}


static void
draw_marker (ChamplainPoint *marker)
{
  ChamplainPointPrivate *priv = marker->priv;
  ClutterActor *cairo_texture;
  cairo_t *cr;
  gdouble size = priv->size;
  gdouble radius = size / 2.0;
  ClutterColor *color;
  
  clutter_group_remove_all (CLUTTER_GROUP (marker));
  cairo_texture = clutter_cairo_texture_new (size, size);
  clutter_container_add_actor (CLUTTER_CONTAINER (marker), cairo_texture);
  clutter_actor_set_anchor_point (CLUTTER_ACTOR (marker), radius, radius);

  cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (cairo_texture));

  if (champlain_base_marker_get_highlighted (CHAMPLAIN_BASE_MARKER (marker)))
    color = &SELECTED_COLOR;
  else
    color = priv->color;  
  
  cairo_set_source_rgba (cr,
      color->red / 255.0,
      color->green / 255.0,
      color->blue / 255.0,
      color->alpha / 255.0);
      
  cairo_arc (cr, radius, radius, radius, 0, 2 * M_PI);
  cairo_fill (cr);
  

  cairo_fill_preserve (cr);
  cairo_set_line_width (cr, 1.0);
  cairo_stroke (cr);

  cairo_destroy (cr);
}


static void
notify_highlighted (GObject *gobject,
    G_GNUC_UNUSED GParamSpec *pspec,
    G_GNUC_UNUSED gpointer user_data)
{
  draw_marker (CHAMPLAIN_POINT (gobject));
}



static void
champlain_point_init (ChamplainPoint *marker)
{
  ChamplainPointPrivate *priv = GET_PRIVATE (marker);

  marker->priv = priv;

  priv->color = clutter_color_copy (&DEFAULT_COLOR);
  priv->size = 12;
  
  draw_marker (marker);

  g_signal_connect (marker, "notify::highlighted", G_CALLBACK (notify_highlighted), NULL);
}


/**
 * champlain_marker_new:
 *
 * Creates a new instance of #ChamplainMarker.
 *
 * Returns: a new #ChamplainMarker ready to be used as a #ClutterActor.
 *
 * Since: 0.2
 */
ClutterActor *
champlain_point_new (void)
{
  return CLUTTER_ACTOR (g_object_new (CHAMPLAIN_TYPE_POINT, NULL));
}




/**
 * champlain_marker_new_full:
 * @text: The text
 * @actor: The image
 *
 * Creates a new instance of #ChamplainMarker consisting of a custom #ClutterActor.
 *
 * Returns: a new #ChamplainMarker with a drawn marker containing the given
 * image.
 *
 * Since: 0.4
 */
ClutterActor *
champlain_point_new_full (gdouble size, 
    const ClutterColor *color)
{
  ChamplainPoint *marker = CHAMPLAIN_POINT (champlain_point_new ());

  champlain_point_set_size (marker, size);
  champlain_point_set_color (marker, color);
  
  return CLUTTER_ACTOR (marker);
}


/**
 */
void
champlain_point_set_size (ChamplainPoint *point,
    gdouble size)
{
  g_return_if_fail (CHAMPLAIN_IS_POINT (point));

  point->priv->size = size;
  g_object_notify (G_OBJECT (point), "size");
  draw_marker (point);
}


/**
 */
gdouble
champlain_point_get_size (ChamplainPoint *point)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POINT (point), 0);

  return point->priv->size;
}


/**
 * champlain_point_set_color:
 * @point: The point
 * @color: (allow-none): The point's background color or NULL to reset the background to the
 *         default color. The color parameter is copied.
 *
 * Set the point's background color.
 *
 * Since: 0.4
 */
void
champlain_point_set_color (ChamplainPoint *point,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_POINT (point));

  ChamplainPointPrivate *priv = point->priv;

  if (priv->color != NULL)
    clutter_color_free (priv->color);

  if (color == NULL)
    color = &DEFAULT_COLOR;

  priv->color = clutter_color_copy (color);
  g_object_notify (G_OBJECT (point), "color");
  draw_marker (point);
}

/**
 * champlain_point_get_color:
 * @point: The point
 *
 * Gets the point's color.
 *
 * Returns: the point's color.
 *
 * Since: 0.4
 */
ClutterColor *
champlain_point_get_color (ChamplainPoint *point)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POINT (point), NULL);

  return point->priv->color;
}
