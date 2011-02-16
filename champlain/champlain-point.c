/*
 * Copyright (C) 2011 Jiri Techet <techet@gmail.com>
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
 * SECTION:champlain-point
 * @short_description: Simple point to mark a coordinate
 *
 * #ChamplainPoint is a simple variant of #ChamplainMarker. Contrary to
 * #ChamplainLabel, it is not capable of labelling the point with text and
 * only shows the location of the point as a circle on the map.
 */

#include "config.h"

#include "champlain.h"
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

/* static guint champlain_point_signals[LAST_SIGNAL] = { 0, }; */

struct _ChamplainPointPrivate
{
  ClutterColor *color;
  gdouble size;
  ClutterGroup *content_group;

  guint redraw_id;
};

G_DEFINE_TYPE (ChamplainPoint, champlain_point, CHAMPLAIN_TYPE_MARKER);

#define GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CHAMPLAIN_TYPE_POINT, ChamplainPointPrivate))

static void paint (ClutterActor *self);
static void pick (ClutterActor *self, 
    const ClutterColor *color);
static void get_preferred_width (ClutterActor *self,
    gfloat for_height,
    gfloat *min_width_p,
    gfloat *natural_width_p);
static void get_preferred_height (ClutterActor *self,
    gfloat for_width,
    gfloat *min_height_p,
    gfloat *natural_height_p);
static void allocate (ClutterActor *self,
    const ClutterActorBox *box,
    ClutterAllocationFlags flags);
static void map (ClutterActor *self);
static void unmap (ClutterActor *self);

static void draw_point (ChamplainPoint *point);


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
  ChamplainPoint *point = CHAMPLAIN_POINT (object);

  switch (prop_id)
    {
    case PROP_COLOR:
      champlain_point_set_color (point, clutter_value_get_color (value));
      break;

    case PROP_SIZE:
      champlain_point_set_size (point, g_value_get_double (value));
      break;
      
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
champlain_point_dispose (GObject *object)
{
  ChamplainPointPrivate *priv = CHAMPLAIN_POINT (object)->priv;
  
  if (priv->content_group)
    {
      clutter_actor_unparent (CLUTTER_ACTOR (priv->content_group));
      priv->content_group = NULL;
    }

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
champlain_point_class_init (ChamplainPointClass *klass)
{
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  g_type_class_add_private (klass, sizeof (ChamplainPointPrivate));

  object_class->finalize = champlain_point_finalize;
  object_class->dispose = champlain_point_dispose;
  object_class->get_property = champlain_point_get_property;
  object_class->set_property = champlain_point_set_property;

  actor_class->get_preferred_width = get_preferred_width;
  actor_class->get_preferred_height = get_preferred_height;
  actor_class->allocate = allocate;
  actor_class->paint = paint;
  actor_class->pick = pick;
  actor_class->map = map;
  actor_class->unmap = unmap;

  g_object_class_install_property (object_class, PROP_COLOR,
      clutter_param_spec_color ("color", "Color", "The point's color",
          &DEFAULT_COLOR, CHAMPLAIN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_SIZE,
      g_param_spec_double ("size", "Size", "The point size", 0, G_MAXDOUBLE,
          12, CHAMPLAIN_PARAM_READWRITE));
}


static void
draw_point (ChamplainPoint *point)
{
  ChamplainPointPrivate *priv = point->priv;
  ClutterActor *cairo_texture;
  cairo_t *cr;
  gdouble size = priv->size;
  gdouble radius = size / 2.0;
  const ClutterColor *color;
  
  clutter_group_remove_all (CLUTTER_GROUP (priv->content_group));
  cairo_texture = clutter_cairo_texture_new (size, size);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->content_group), cairo_texture);
  clutter_actor_set_anchor_point (CLUTTER_ACTOR (point), radius, radius);

  cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (cairo_texture));

  if (champlain_marker_get_selected (CHAMPLAIN_MARKER (point)))
    color = champlain_marker_get_selection_color ();
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
notify_selected (GObject *gobject,
    G_GNUC_UNUSED GParamSpec *pspec,
    G_GNUC_UNUSED gpointer user_data)
{
  draw_point (CHAMPLAIN_POINT (gobject));
}


static void
champlain_point_init (ChamplainPoint *point)
{
  ChamplainPointPrivate *priv = GET_PRIVATE (point);

  point->priv = priv;

  priv->color = clutter_color_copy (&DEFAULT_COLOR);
  priv->size = 12;
  priv->content_group = CLUTTER_GROUP (clutter_group_new ());
  clutter_actor_set_parent (CLUTTER_ACTOR (priv->content_group), CLUTTER_ACTOR (point));
  clutter_actor_queue_relayout (CLUTTER_ACTOR (point));
  
  draw_point (point);

  g_signal_connect (point, "notify::selected", G_CALLBACK (notify_selected), NULL);
}


/**
 * champlain_point_new:
 * 
 * Creates an instance of #ChamplainPoint with default size and color.
 *
 * Returns: a new #ChamplainPoint.
 *
 * Since: 0.10
 */
ClutterActor *
champlain_point_new (void)
{
  return CLUTTER_ACTOR (g_object_new (CHAMPLAIN_TYPE_POINT, NULL));
}


/**
 * champlain_point_new_full:
 * @size: Marker size
 * @color: Marker color
 * 
 * Creates an instance of #ChamplainPoint with the specified size and color.
 *
 * Returns: a new #ChamplainPoint.
 *
 * Since: 0.10
 */
ClutterActor *
champlain_point_new_full (gdouble size, 
    const ClutterColor *color)
{
  ChamplainPoint *point = CHAMPLAIN_POINT (champlain_point_new ());

  champlain_point_set_size (point, size);
  champlain_point_set_color (point, color);
  
  return CLUTTER_ACTOR (point);
}


static void
paint (ClutterActor *self)
{
  ChamplainPointPrivate *priv = GET_PRIVATE (self);
  
  clutter_actor_paint (CLUTTER_ACTOR (priv->content_group));
}


static void
pick (ClutterActor *self, 
    const ClutterColor *color)
{
  ChamplainPointPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_point_parent_class)->pick (self, color);

  clutter_actor_paint (CLUTTER_ACTOR (priv->content_group));
}


static void
get_preferred_width (ClutterActor *self,
    gfloat for_height,
    gfloat *min_width_p,
    gfloat *natural_width_p)
{
  ChamplainPointPrivate *priv = GET_PRIVATE (self);

  clutter_actor_get_preferred_width (CLUTTER_ACTOR (priv->content_group),
      for_height,
      min_width_p,
      natural_width_p);
}


static void
get_preferred_height (ClutterActor *self,
    gfloat for_width,
    gfloat *min_height_p,
    gfloat *natural_height_p)
{
  ChamplainPointPrivate *priv = GET_PRIVATE (self);

  clutter_actor_get_preferred_height (CLUTTER_ACTOR (priv->content_group),
      for_width,
      min_height_p,
      natural_height_p);
}


static void
allocate (ClutterActor *self,
    const ClutterActorBox *box,
    ClutterAllocationFlags flags)
{
  ClutterActorBox child_box;

  ChamplainPointPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_point_parent_class)->allocate (self, box, flags);

  child_box.x1 = 0;
  child_box.x2 = box->x2 - box->x1;
  child_box.y1 = 0;
  child_box.y2 = box->y2 - box->y1;

  clutter_actor_allocate (CLUTTER_ACTOR (priv->content_group), &child_box, flags);
}


static void
map (ClutterActor *self)
{
  ChamplainPointPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_point_parent_class)->map (self);

  clutter_actor_map (CLUTTER_ACTOR (priv->content_group));
}


static void
unmap (ClutterActor *self)
{
  ChamplainPointPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_point_parent_class)->unmap (self);

  clutter_actor_unmap (CLUTTER_ACTOR (priv->content_group));
}


/**
 * champlain_point_set_size:
 * @point: a #ChamplainPoint
 * @size: The size of the point.
 *
 * Set the size of the point.
 *
 * Since: 0.10
 */
void
champlain_point_set_size (ChamplainPoint *point,
    gdouble size)
{
  g_return_if_fail (CHAMPLAIN_IS_POINT (point));

  point->priv->size = size;
  g_object_notify (G_OBJECT (point), "size");
  draw_point (point);
}


/**
 * champlain_point_get_size:
 * @point: a #ChamplainPoint
 *
 * Gets the size of the point.
 *
 * Returns: the size.
 *
 * Since: 0.10
 */
gdouble
champlain_point_get_size (ChamplainPoint *point)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POINT (point), 0);

  return point->priv->size;
}


/**
 * champlain_point_set_color:
 * @point: a #ChamplainPoint
 * @color: (allow-none): The color of the point or NULL to reset the background to the
 *         default color. The color parameter is copied.
 *
 * Set the color of the point.
 *
 * Since: 0.10
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
  draw_point (point);
}


/**
 * champlain_point_get_color:
 * @point: a #ChamplainPoint
 *
 * Gets the color of the point.
 *
 * Returns: the color.
 *
 * Since: 0.10
 */
ClutterColor *
champlain_point_get_color (ChamplainPoint *point)
{
  g_return_val_if_fail (CHAMPLAIN_IS_POINT (point), NULL);

  return point->priv->color;
}
