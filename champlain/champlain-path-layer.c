/*
 * Copyright (C) 2008-2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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
 * SECTION:champlain-path-layer
 * @short_description: A layer displaying line path between inserted #ChamplainLocation
 * objects
 *
 * This layer shows a connection between inserted objects implementing the
 * #ChamplainLocation interface. This means that both #ChamplainMarker
 * objects and #ChamplainCoordinate objects can be inserted into the layer.
 * Of course, custom objects implementing the #ChamplainLocation interface
 * can be used as well.
 */

#include "config.h"

#include "champlain-path-layer.h"

#include "champlain-defines.h"
#include "champlain-enum-types.h"
#include "champlain-private.h"
#include "champlain-view.h"
#include "champlain-group.h"

#include <clutter/clutter.h>
#include <glib.h>

G_DEFINE_TYPE (ChamplainPathLayer, champlain_path_layer, CHAMPLAIN_TYPE_LAYER)

#define GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CHAMPLAIN_TYPE_PATH_LAYER, ChamplainPathLayerPrivate))

enum
{
  /* normal signals */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_CLOSED_PATH,
  PROP_STROKE_WIDTH,
  PROP_STROKE_COLOR,
  PROP_FILL,
  PROP_FILL_COLOR,
  PROP_STROKE,
  PROP_VISIBLE,
};

static ClutterColor DEFAULT_FILL_COLOR = { 0xcc, 0x00, 0x00, 0xaa };
static ClutterColor DEFAULT_STROKE_COLOR = { 0xa4, 0x00, 0x00, 0xff };

/* static guint signals[LAST_SIGNAL] = { 0, }; */

struct _ChamplainPathLayerPrivate
{
  ChamplainView *view;

  gboolean closed_path;
  ClutterColor *stroke_color;
  gboolean fill;
  ClutterColor *fill_color;
  gboolean stroke;
  gdouble stroke_width;
  gboolean visible;

  ClutterGroup *content_group;
  ClutterActor *path_actor;
  GList *nodes;
  gboolean redraw_scheduled;
};


static gboolean redraw_path (ChamplainPathLayer *layer);
static void schedule_redraw (ChamplainPathLayer *layer);

static void set_view (ChamplainLayer *layer,
    ChamplainView *view);

static ChamplainBoundingBox *get_bounding_box (ChamplainLayer *layer);


static void
champlain_path_layer_get_property (GObject *object,
    guint property_id,
    G_GNUC_UNUSED GValue *value,
    GParamSpec *pspec)
{
  ChamplainPathLayer *self = CHAMPLAIN_PATH_LAYER (object);
  ChamplainPathLayerPrivate *priv = self->priv;

  switch (property_id)
    {
    case PROP_CLOSED_PATH:
      g_value_set_boolean (value, priv->closed_path);
      break;

    case PROP_FILL:
      g_value_set_boolean (value, priv->fill);
      break;

    case PROP_STROKE:
      g_value_set_boolean (value, priv->stroke);
      break;

    case PROP_FILL_COLOR:
      clutter_value_set_color (value, priv->fill_color);
      break;

    case PROP_STROKE_COLOR:
      clutter_value_set_color (value, priv->stroke_color);
      break;

    case PROP_STROKE_WIDTH:
      g_value_set_double (value, priv->stroke_width);
      break;

    case PROP_VISIBLE:
      g_value_set_boolean (value, priv->visible);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
champlain_path_layer_set_property (GObject *object,
    guint property_id,
    G_GNUC_UNUSED const GValue *value,
    GParamSpec *pspec)
{
  switch (property_id)
    {
    case PROP_CLOSED_PATH:
      champlain_path_layer_set_closed (CHAMPLAIN_PATH_LAYER (object),
          g_value_get_boolean (value));
      break;

    case PROP_FILL:
      champlain_path_layer_set_fill (CHAMPLAIN_PATH_LAYER (object),
          g_value_get_boolean (value));
      break;

    case PROP_STROKE:
      champlain_path_layer_set_stroke (CHAMPLAIN_PATH_LAYER (object),
          g_value_get_boolean (value));
      break;

    case PROP_FILL_COLOR:
      champlain_path_layer_set_fill_color (CHAMPLAIN_PATH_LAYER (object),
          clutter_value_get_color (value));
      break;

    case PROP_STROKE_COLOR:
      champlain_path_layer_set_stroke_color (CHAMPLAIN_PATH_LAYER (object),
          clutter_value_get_color (value));
      break;

    case PROP_STROKE_WIDTH:
      champlain_path_layer_set_stroke_width (CHAMPLAIN_PATH_LAYER (object),
          g_value_get_double (value));
      break;

    case PROP_VISIBLE:
      champlain_path_layer_set_visible (CHAMPLAIN_PATH_LAYER (object),
          g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
paint (ClutterActor *self)
{
  ChamplainPathLayerPrivate *priv = GET_PRIVATE (self);

  clutter_actor_paint (CLUTTER_ACTOR (priv->content_group));
}


static void
pick (ClutterActor *self,
    const ClutterColor *color)
{
  ChamplainPathLayerPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_path_layer_parent_class)->pick (self, color);

  clutter_actor_paint (CLUTTER_ACTOR (priv->content_group));
}


static void
get_preferred_width (ClutterActor *self,
    gfloat for_height,
    gfloat *min_width_p,
    gfloat *natural_width_p)
{
  ChamplainPathLayerPrivate *priv = GET_PRIVATE (self);

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
  ChamplainPathLayerPrivate *priv = GET_PRIVATE (self);

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

  ChamplainPathLayerPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_path_layer_parent_class)->allocate (self, box, flags);

  child_box.x1 = 0;
  child_box.x2 = box->x2 - box->x1;
  child_box.y1 = 0;
  child_box.y2 = box->y2 - box->y1;

  clutter_actor_allocate (CLUTTER_ACTOR (priv->content_group), &child_box, flags);
}


static void
map (ClutterActor *self)
{
  ChamplainPathLayerPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_path_layer_parent_class)->map (self);

  clutter_actor_map (CLUTTER_ACTOR (priv->content_group));
}


static void
unmap (ClutterActor *self)
{
  ChamplainPathLayerPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_path_layer_parent_class)->unmap (self);

  clutter_actor_unmap (CLUTTER_ACTOR (priv->content_group));
}


static void
champlain_path_layer_dispose (GObject *object)
{
  ChamplainPathLayer *self = CHAMPLAIN_PATH_LAYER (object);
  ChamplainPathLayerPrivate *priv = self->priv;

  if (priv->nodes)
    champlain_path_layer_remove_all (CHAMPLAIN_PATH_LAYER (object));

  if (priv->view != NULL)
    set_view (CHAMPLAIN_LAYER (self), NULL);

  if (priv->content_group)
    {
      clutter_actor_unparent (CLUTTER_ACTOR (priv->content_group));
      priv->content_group = NULL;
    }

  G_OBJECT_CLASS (champlain_path_layer_parent_class)->dispose (object);
}


static void
champlain_path_layer_finalize (GObject *object)
{
  ChamplainPathLayer *self = CHAMPLAIN_PATH_LAYER (object);
  ChamplainPathLayerPrivate *priv = self->priv;

  clutter_color_free (priv->stroke_color);
  clutter_color_free (priv->fill_color);

  G_OBJECT_CLASS (champlain_path_layer_parent_class)->finalize (object);
}


static void
champlain_path_layer_class_init (ChamplainPathLayerClass *klass)
{
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ChamplainLayerClass *layer_class = CHAMPLAIN_LAYER_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainPathLayerPrivate));

  object_class->finalize = champlain_path_layer_finalize;
  object_class->dispose = champlain_path_layer_dispose;
  object_class->get_property = champlain_path_layer_get_property;
  object_class->set_property = champlain_path_layer_set_property;

  actor_class->get_preferred_width = get_preferred_width;
  actor_class->get_preferred_height = get_preferred_height;
  actor_class->allocate = allocate;
  actor_class->paint = paint;
  actor_class->pick = pick;
  actor_class->map = map;
  actor_class->unmap = unmap;

  layer_class->set_view = set_view;
  layer_class->get_bounding_box = get_bounding_box;

  /**
   * ChamplainPathLayer:closed:
   *
   * The shape is a closed path
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_CLOSED_PATH,
      g_param_spec_boolean ("closed",
          "Closed Path",
          "The Path is Closed",
          FALSE, CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainPathLayer:fill:
   *
   * The shape should be filled
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_FILL,
      g_param_spec_boolean ("fill",
          "Fill",
          "The shape is filled",
          FALSE, CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainPathLayer:stroke:
   *
   * The shape should be stroked
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_STROKE,
      g_param_spec_boolean ("stroke",
          "Stroke",
          "The shape is stroked",
          TRUE, CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainPathLayer:stroke-color:
   *
   * The path's stroke color
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_STROKE_COLOR,
      clutter_param_spec_color ("stroke-color",
          "Stroke Color",
          "The path's stroke color",
          &DEFAULT_STROKE_COLOR,
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainPathLayer:fill-color:
   *
   * The path's fill color
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_FILL_COLOR,
      clutter_param_spec_color ("fill-color",
          "Fill Color",
          "The path's fill color",
          &DEFAULT_FILL_COLOR,
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainPathLayer:stroke-width:
   *
   * The path's stroke width (in pixels)
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_STROKE_WIDTH,
      g_param_spec_double ("stroke-width",
          "Stroke Width",
          "The path's stroke width",
          0, 100.0,
          2.0,
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainPathLayer:visible:
   *
   * Wether the path is visible
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_VISIBLE,
      g_param_spec_boolean ("visible",
          "Visible",
          "The path's visibility",
          TRUE,
          CHAMPLAIN_PARAM_READWRITE));
}


static void
champlain_path_layer_init (ChamplainPathLayer *self)
{
  ChamplainPathLayerPrivate *priv;

  self->priv = GET_PRIVATE (self);
  priv = self->priv;
  priv->view = NULL;

  priv->visible = TRUE;
  priv->fill = FALSE;
  priv->stroke = TRUE;
  priv->stroke_width = 2.0;
  priv->nodes = NULL;
  priv->redraw_scheduled = FALSE;

  priv->fill_color = clutter_color_copy (&DEFAULT_FILL_COLOR);
  priv->stroke_color = clutter_color_copy (&DEFAULT_STROKE_COLOR);

  priv->content_group = CLUTTER_GROUP (clutter_group_new ());
  clutter_actor_set_parent (CLUTTER_ACTOR (priv->content_group), CLUTTER_ACTOR (self));

  priv->path_actor = clutter_cairo_texture_new (256, 256);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->content_group), priv->path_actor);

  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
}


/**
 * champlain_path_layer_new:
 *
 * Creates a new instance of #ChamplainPathLayer.
 *
 * Returns: a new instance of #ChamplainPathLayer.
 *
 * Since: 0.10
 */
ChamplainPathLayer *
champlain_path_layer_new ()
{
  return g_object_new (CHAMPLAIN_TYPE_PATH_LAYER, NULL);
}


static void
position_notify (ChamplainLocation *location,
    G_GNUC_UNUSED GParamSpec *pspec,
    ChamplainPathLayer *layer)
{
  schedule_redraw (layer);
}


static void
add_node (ChamplainPathLayer *layer,
    ChamplainLocation *location,
    gboolean append,
    guint position)
{
  ChamplainPathLayerPrivate *priv = GET_PRIVATE (layer);

  g_signal_connect (G_OBJECT (location), "notify::latitude",
      G_CALLBACK (position_notify), layer);

  g_object_ref_sink (location);

  if (append)
    priv->nodes = g_list_append (priv->nodes, location);
  else
    priv->nodes = g_list_insert (priv->nodes, location, position);
  schedule_redraw (layer);
}


/**
 * champlain_path_layer_add_node:
 * @layer: a #ChamplainPathLayer
 * @location: a #ChamplainLocation
 *
 * Adds a #ChamplainLocation object to the layer.
 *
 * Since: 0.10
 */
void
champlain_path_layer_add_node (ChamplainPathLayer *layer,
    ChamplainLocation *location)
{
  g_return_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer));
  g_return_if_fail (CHAMPLAIN_IS_LOCATION (location));

  add_node (layer, location, TRUE, 0);
}


/**
 * champlain_path_layer_remove_all:
 * @layer: a #ChamplainPathLayer
 *
 * Removes all #ChamplainLocation objects from the layer.
 *
 * Since: 0.10
 */
void
champlain_path_layer_remove_all (ChamplainPathLayer *layer)
{
  ChamplainPathLayerPrivate *priv = GET_PRIVATE (layer);
  GList *elem;

  g_return_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer));

  for (elem = priv->nodes; elem != NULL; elem = elem->next)
    {
      GObject *node = G_OBJECT (elem->data);

      g_signal_handlers_disconnect_by_func (node,
          G_CALLBACK (position_notify), layer);

      g_object_unref (node);
    }

  g_list_free (priv->nodes);
  priv->nodes = NULL;
  schedule_redraw (layer);
}


/**
 * champlain_path_layer_get_nodes:
 * @layer: a #ChamplainPathLayer
 *
 * Gets a copy of the list of all #ChamplainLocation objects inserted into the layer. You should
 * free the list but not its contents.
 *
 * Returns: (transfer container) (element-type ChamplainLocation): the list
 *
 * Since: 0.10
 */
GList *
champlain_path_layer_get_nodes (ChamplainPathLayer *layer)
{
  ChamplainPathLayerPrivate *priv = GET_PRIVATE (layer);

  return g_list_copy (priv->nodes);
}


/**
 * champlain_path_layer_remove_node:
 * @layer: a #ChamplainPathLayer
 * @location: a #ChamplainLocation
 *
 * Removes the #ChamplainLocation object from the layer.
 *
 * Since: 0.10
 */
void
champlain_path_layer_remove_node (ChamplainPathLayer *layer,
    ChamplainLocation *location)
{
  ChamplainPathLayerPrivate *priv = GET_PRIVATE (layer);

  g_return_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer));
  g_return_if_fail (CHAMPLAIN_IS_LOCATION (location));

  g_signal_handlers_disconnect_by_func (G_OBJECT (location),
      G_CALLBACK (position_notify), layer);

  priv->nodes = g_list_remove (priv->nodes, location);
  g_object_unref (location);
  schedule_redraw (layer);
}


/**
 * champlain_path_layer_insert_node:
 * @layer: a #ChamplainPathLayer
 * @location: a #ChamplainLocation
 * @position: position in the list where the #ChamplainLocation object should be inserted
 *
 * Inserts a #ChamplainLocation object to the specified position.
 *
 * Since: 0.10
 */
void
champlain_path_layer_insert_node (ChamplainPathLayer *layer,
    ChamplainLocation *location,
    guint position)
{
  g_return_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer));
  g_return_if_fail (CHAMPLAIN_IS_LOCATION (location));

  add_node (layer, location, FALSE, position);
}


static void
relocate_cb (G_GNUC_UNUSED GObject *gobject,
    ChamplainPathLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer));

  schedule_redraw (layer);
}


static gboolean
redraw_path (ChamplainPathLayer *layer)
{
  ChamplainPathLayerPrivate *priv = layer->priv;
  cairo_t *cr;
  gfloat width, height;
  GList *elem;
  ChamplainView *view = priv->view;
  gint x, y;
  guint last_width, last_height;

  priv->redraw_scheduled = FALSE;

  /* layer not yet added to the view */
  if (view == NULL)
    return FALSE;

  clutter_actor_get_size (CLUTTER_ACTOR (view), &width, &height);

  if (!priv->visible || width == 0.0 || height == 0.0)
    return FALSE;

  clutter_cairo_texture_get_surface_size (CLUTTER_CAIRO_TEXTURE (priv->path_actor), &last_width, &last_height);

  if ((guint) width != last_width || (guint) height != last_height)
    clutter_cairo_texture_set_surface_size (CLUTTER_CAIRO_TEXTURE (priv->path_actor), width, height);

  champlain_view_get_viewport_origin (priv->view, &x, &y);
  clutter_actor_set_position (priv->path_actor, x, y);

  cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (priv->path_actor));

  /* Clear the drawing area */
  cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
  cairo_paint (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  for (elem = priv->nodes; elem != NULL; elem = elem->next)
    {
      ChamplainLocation *location = CHAMPLAIN_LOCATION (elem->data);
      gfloat x, y;

      x = champlain_view_longitude_to_x (view, champlain_location_get_longitude (location));
      y = champlain_view_latitude_to_y (view, champlain_location_get_latitude (location));

      cairo_line_to (cr, x, y);
    }

  if (priv->closed_path)
    cairo_close_path (cr);

  cairo_set_source_rgba (cr,
      priv->fill_color->red / 255.0,
      priv->fill_color->green / 255.0,
      priv->fill_color->blue / 255.0,
      priv->fill_color->alpha / 255.0);

  if (priv->fill)
    cairo_fill_preserve (cr);

  cairo_set_source_rgba (cr,
      priv->stroke_color->red / 255.0,
      priv->stroke_color->green / 255.0,
      priv->stroke_color->blue / 255.0,
      priv->stroke_color->alpha / 255.0);

  cairo_set_line_width (cr, priv->stroke_width);

  if (priv->stroke)
    cairo_stroke (cr);

  cairo_destroy (cr);

  return FALSE;
}


static void
schedule_redraw (ChamplainPathLayer *layer)
{
  if (!layer->priv->redraw_scheduled)
    {
      layer->priv->redraw_scheduled = TRUE;
      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
          (GSourceFunc) redraw_path,
          g_object_ref (layer),
          (GDestroyNotify) g_object_unref);
    }
}


static void
redraw_path_cb (G_GNUC_UNUSED GObject *gobject,
    G_GNUC_UNUSED GParamSpec *arg1,
    ChamplainPathLayer *layer)
{
  schedule_redraw (layer);
}


static void
set_view (ChamplainLayer *layer,
    ChamplainView *view)
{
  g_return_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer) && (CHAMPLAIN_IS_VIEW (view) || view == NULL));

  ChamplainPathLayer *path_layer = CHAMPLAIN_PATH_LAYER (layer);

  if (path_layer->priv->view != NULL)
    {
      g_signal_handlers_disconnect_by_func (path_layer->priv->view,
          G_CALLBACK (relocate_cb), path_layer);

      g_signal_handlers_disconnect_by_func (path_layer->priv->view,
          G_CALLBACK (redraw_path_cb), path_layer);

      g_object_unref (path_layer->priv->view);
    }

  path_layer->priv->view = view;

  if (view != NULL)
    {
      g_object_ref (view);

      g_signal_connect (view, "layer-relocated",
          G_CALLBACK (relocate_cb), layer);

      g_signal_connect (view, "notify::latitude",
          G_CALLBACK (redraw_path_cb), layer);

      schedule_redraw (path_layer);
    }
}


static ChamplainBoundingBox *
get_bounding_box (ChamplainLayer *layer)
{
  ChamplainPathLayerPrivate *priv = GET_PRIVATE (layer);
  GList *elem;
  ChamplainBoundingBox *bbox;

  bbox = champlain_bounding_box_new ();

  for (elem = priv->nodes; elem != NULL; elem = elem->next)
    {
      ChamplainLocation *location = CHAMPLAIN_LOCATION (elem->data);
      gdouble lat, lon;

      lat = champlain_location_get_latitude (location);
      lon = champlain_location_get_longitude (location);

      champlain_bounding_box_extend (bbox, lat, lon);
    }

  if (bbox->left == bbox->right)
    {
      bbox->left -= 0.0001;
      bbox->right += 0.0001;
    }

  if (bbox->bottom == bbox->top)
    {
      bbox->bottom -= 0.0001;
      bbox->top += 0.0001;
    }

  return bbox;
}


/**
 * champlain_path_layer_set_fill_color:
 * @layer: a #ChamplainPathLayer
 * @color: (allow-none): The path's fill color or NULL to reset to the
 *         default color. The color parameter is copied.
 *
 * Set the path's fill color.
 *
 * Since: 0.10
 */
void
champlain_path_layer_set_fill_color (ChamplainPathLayer *layer,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer));

  ChamplainPathLayerPrivate *priv = layer->priv;

  if (priv->fill_color != NULL)
    clutter_color_free (priv->fill_color);

  if (color == NULL)
    color = &DEFAULT_FILL_COLOR;

  priv->fill_color = clutter_color_copy (color);
  g_object_notify (G_OBJECT (layer), "fill-color");
}


/**
 * champlain_path_layer_set_stroke_color:
 * @layer: a #ChamplainPathLayer
 * @color: (allow-none): The path's stroke color or NULL to reset to the
 *         default color. The color parameter is copied.
 *
 * Set the path's stroke color.
 *
 * Since: 0.10
 */
void
champlain_path_layer_set_stroke_color (ChamplainPathLayer *layer,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer));

  ChamplainPathLayerPrivate *priv = layer->priv;

  if (priv->stroke_color != NULL)
    clutter_color_free (priv->stroke_color);

  if (color == NULL)
    color = &DEFAULT_STROKE_COLOR;

  priv->stroke_color = clutter_color_copy (color);
  g_object_notify (G_OBJECT (layer), "stroke-color");
}


/**
 * champlain_path_layer_get_fill_color:
 * @layer: a #ChamplainPathLayer
 *
 * Gets the path's fill color.
 *
 * Returns: the path's fill color.
 *
 * Since: 0.10
 */
ClutterColor *
champlain_path_layer_get_fill_color (ChamplainPathLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer), NULL);

  return layer->priv->fill_color;
}


/**
 * champlain_path_layer_get_stroke_color:
 * @layer: a #ChamplainPathLayer
 *
 * Gets the path's stroke color.
 *
 * Returns: the path's stroke color.
 *
 * Since: 0.10
 */
ClutterColor *
champlain_path_layer_get_stroke_color (ChamplainPathLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer), NULL);

  return layer->priv->stroke_color;
}


/**
 * champlain_path_layer_set_stroke:
 * @layer: a #ChamplainPathLayer
 * @value: if the path is stroked
 *
 * Sets the path to be stroked
 *
 * Since: 0.10
 */
void
champlain_path_layer_set_stroke (ChamplainPathLayer *layer,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer));

  layer->priv->stroke = value;
  g_object_notify (G_OBJECT (layer), "stroke");
}


/**
 * champlain_path_layer_get_stroke:
 * @layer: a #ChamplainPathLayer
 *
 * Checks whether the path is stroked.
 *
 * Returns: TRUE if the path is stroked, FALSE otherwise.
 *
 * Since: 0.10
 */
gboolean
champlain_path_layer_get_stroke (ChamplainPathLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer), FALSE);

  return layer->priv->stroke;
}


/**
 * champlain_path_layer_set_fill:
 * @layer: a #ChamplainPathLayer
 * @value: if the path is filled
 *
 * Sets the path to be filled
 *
 * Since: 0.10
 */
void
champlain_path_layer_set_fill (ChamplainPathLayer *layer,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer));

  layer->priv->fill = value;
  g_object_notify (G_OBJECT (layer), "fill");
}


/**
 * champlain_path_layer_get_fill:
 * @layer: a #ChamplainPathLayer
 *
 * Checks whether the path is filled.
 *
 * Returns: TRUE if the path is filled, FALSE otherwise.
 *
 * Since: 0.10
 */
gboolean
champlain_path_layer_get_fill (ChamplainPathLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer), FALSE);

  return layer->priv->fill;
}


/**
 * champlain_path_layer_set_stroke_width:
 * @layer: a #ChamplainPathLayer
 * @value: the width of the stroke (in pixels)
 *
 * Sets the width of the stroke
 *
 * Since: 0.10
 */
void
champlain_path_layer_set_stroke_width (ChamplainPathLayer *layer,
    gdouble value)
{
  g_return_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer));

  layer->priv->stroke_width = value;
  g_object_notify (G_OBJECT (layer), "stroke-width");
}


/**
 * champlain_path_layer_get_stroke_width:
 * @layer: a #ChamplainPathLayer
 *
 * Gets the width of the stroke.
 *
 * Returns: the width of the stroke
 *
 * Since: 0.10
 */
gdouble
champlain_path_layer_get_stroke_width (ChamplainPathLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer), 0);

  return layer->priv->stroke_width;
}


/**
 * champlain_path_layer_set_visible:
 * @layer: a #ChamplainPathLayer
 * @value: TRUE to make the path visible
 *
 * Sets path visibility.
 *
 * Since: 0.10
 */
void
champlain_path_layer_set_visible (ChamplainPathLayer *layer,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer));

  layer->priv->visible = value;
  if (value)
    clutter_actor_show (CLUTTER_ACTOR (layer->priv->path_actor));
  else
    clutter_actor_hide (CLUTTER_ACTOR (layer->priv->path_actor));
  g_object_notify (G_OBJECT (layer), "visible");
}


/**
 * champlain_path_layer_get_visible:
 * @layer: a #ChamplainPathLayer
 *
 * Gets path visibility.
 *
 * Returns: TRUE when the path is visible, FALSE otherwise
 *
 * Since: 0.10
 */
gboolean
champlain_path_layer_get_visible (ChamplainPathLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer), FALSE);

  return layer->priv->visible;
}


/**
 * champlain_path_layer_set_closed:
 * @layer: a #ChamplainPathLayer
 * @value: TRUE to make the path closed
 *
 * Makes the path closed.
 *
 * Since: 0.10
 */
void
champlain_path_layer_set_closed (ChamplainPathLayer *layer,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer));

  layer->priv->closed_path = value;
  g_object_notify (G_OBJECT (layer), "closed");
}


/**
 * champlain_path_layer_get_closed:
 * @layer: a #ChamplainPathLayer
 *
 * Gets information whether the path is closed.
 *
 * Returns: TRUE when the path is closed, FALSE otherwise
 *
 * Since: 0.10
 */
gboolean
champlain_path_layer_get_closed (ChamplainPathLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_PATH_LAYER (layer), FALSE);

  return layer->priv->closed_path;
}
