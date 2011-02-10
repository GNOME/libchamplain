/*
 * Copyright (C) 2008-2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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
 * SECTION:champlain-marker-layer
 * @short_description: A container for #ChamplainMarker
 *
 * A ChamplainMarkerLayer is little more than a #ClutterContainer. It keeps the
 * markers ordered so that they display correctly.
 *
 * Use #clutter_container_add to add markers to the layer and
 * #clutter_container_remove to remove them.
 */

#include "config.h"

#include "champlain-marker-layer.h"

#include "champlain-defines.h"
#include "champlain-enum-types.h"
#include "champlain-private.h"
#include "champlain-view.h"

#include <clutter/clutter.h>
#include <glib.h>

G_DEFINE_TYPE (ChamplainMarkerLayer, champlain_marker_layer, CHAMPLAIN_TYPE_LAYER)

#define GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CHAMPLAIN_TYPE_MARKER_LAYER, ChamplainMarkerLayerPrivate))

enum
{
  /* normal signals */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SELECTION_MODE,
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

//static guint signals[LAST_SIGNAL] = { 0, };

struct _ChamplainMarkerLayerPrivate
{
  ChamplainSelectionMode mode;
  ChamplainView *view;
  
  ClutterActor *path_actor;
  gboolean closed_path;
  ClutterColor *stroke_color;
  gboolean fill;
  ClutterColor *fill_color;
  gboolean stroke;
  gdouble stroke_width;
  gboolean visible;
  
  ClutterGroup *content_group;
  GList *markers;
};

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


static void marker_selected_cb (ChamplainMarker *marker,
    G_GNUC_UNUSED GParamSpec *arg1,
    ChamplainMarkerLayer *layer);
    
static void redraw_path (ChamplainMarkerLayer *layer);

static void set_view (ChamplainLayer *layer,
    ChamplainView *view);


static void
champlain_marker_layer_get_property (GObject *object,
    guint property_id,
    G_GNUC_UNUSED GValue *value,
    GParamSpec *pspec)
{
  ChamplainMarkerLayer *self = CHAMPLAIN_MARKER_LAYER (object);
  ChamplainMarkerLayerPrivate *priv = self->priv;
  
  switch (property_id)
    {
    case PROP_SELECTION_MODE:
      g_value_set_enum (value, priv->mode);
      break;
      
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
champlain_marker_layer_set_property (GObject *object,
    guint property_id,
    G_GNUC_UNUSED const GValue *value,
    GParamSpec *pspec)
{
  ChamplainMarkerLayer *self = CHAMPLAIN_MARKER_LAYER (object);
  ChamplainMarkerLayerPrivate *priv = self->priv;
  
  switch (property_id)
    {
    case PROP_SELECTION_MODE:
      champlain_marker_layer_set_selection_mode (self, g_value_get_enum (value));
      break;
      
    case PROP_CLOSED_PATH:
      priv->closed_path = g_value_get_boolean (value);
      break;

    case PROP_FILL:
      champlain_marker_layer_set_path_fill (CHAMPLAIN_MARKER_LAYER (object),
          g_value_get_boolean (value));
      break;

    case PROP_STROKE:
      champlain_marker_layer_set_path_stroke (CHAMPLAIN_MARKER_LAYER (object),
          g_value_get_boolean (value));
      break;

    case PROP_FILL_COLOR:
      champlain_marker_layer_set_path_fill_color (CHAMPLAIN_MARKER_LAYER (object),
          clutter_value_get_color (value));
      break;

    case PROP_STROKE_COLOR:
      champlain_marker_layer_set_path_stroke_color (CHAMPLAIN_MARKER_LAYER (object),
          clutter_value_get_color (value));
      break;

    case PROP_STROKE_WIDTH:
      champlain_marker_layer_set_path_stroke_width (CHAMPLAIN_MARKER_LAYER (object),
          g_value_get_double (value));
      break;

    case PROP_VISIBLE:
        champlain_marker_layer_set_path_visible (CHAMPLAIN_MARKER_LAYER (object),
            g_value_get_boolean (value));
      break;
      
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
champlain_marker_layer_dispose (GObject *object)
{
  ChamplainMarkerLayer *self = CHAMPLAIN_MARKER_LAYER (object);
  ChamplainMarkerLayerPrivate *priv = self->priv;
  
  if (priv->view != NULL)
    {
      set_view (CHAMPLAIN_LAYER (self), NULL);
    }

  if (priv->content_group)
    {
      clutter_actor_unparent (CLUTTER_ACTOR (priv->content_group));
      priv->content_group = NULL;
    }

  G_OBJECT_CLASS (champlain_marker_layer_parent_class)->dispose (object);
}


static void
champlain_marker_layer_finalize (GObject *object)
{
  ChamplainMarkerLayer *self = CHAMPLAIN_MARKER_LAYER (object);
  ChamplainMarkerLayerPrivate *priv = self->priv;
  
  clutter_color_free (priv->stroke_color);
  clutter_color_free (priv->fill_color);
  g_list_free (priv->markers);

  G_OBJECT_CLASS (champlain_marker_layer_parent_class)->finalize (object);
}


static void
champlain_marker_layer_class_init (ChamplainMarkerLayerClass *klass)
{
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ChamplainLayerClass *layer_class = CHAMPLAIN_LAYER_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainMarkerLayerPrivate));
  
  object_class->finalize = champlain_marker_layer_finalize;
  object_class->dispose = champlain_marker_layer_dispose;
  object_class->get_property = champlain_marker_layer_get_property;
  object_class->set_property = champlain_marker_layer_set_property;
  
  actor_class->get_preferred_width = get_preferred_width;
  actor_class->get_preferred_height = get_preferred_height;
  actor_class->allocate = allocate;
  actor_class->paint = paint;
  actor_class->pick = pick;
  actor_class->map = map;
  actor_class->unmap = unmap;
  
  layer_class->set_view = set_view;
  
  /**
   * ChamplainMarkerLayer:selection-mode:
   *
   * Determines the type of selection that will be performed.
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_SELECTION_MODE,
      g_param_spec_enum ("selection-mode",
          "Selection Mode",
          "Determines the type of selection that will be performed.",
          CHAMPLAIN_TYPE_SELECTION_MODE,
          CHAMPLAIN_SELECTION_NONE,
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainMarkerLayer:path-closed:
   *
   * The shape is a closed path
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_CLOSED_PATH,
      g_param_spec_boolean ("path-closed",
          "Closed Path",
          "The Path is Closed",
          FALSE, CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainMarkerLayer:path-fill:
   *
   * The shape should be filled
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_FILL,
      g_param_spec_boolean ("path-fill",
          "Fill",
          "The shape is filled",
          FALSE, CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainMarkerLayer:path-stroke:
   *
   * The shape should be stroked
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_STROKE,
      g_param_spec_boolean ("path-stroke",
          "Stroke",
          "The shape is stroked",
          TRUE, CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainMarkerLayer:path-stroke-color:
   *
   * The path's stroke color
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_STROKE_COLOR,
      clutter_param_spec_color ("path-stroke-color",
          "Stroke Color",
          "The path's stroke color",
          &DEFAULT_STROKE_COLOR,
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainMarkerLayer:path-fill-color:
   *
   * The path's fill color
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_FILL_COLOR,
      clutter_param_spec_color ("path-fill-color",
          "Fill Color",
          "The path's fill color",
          &DEFAULT_FILL_COLOR,
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainMarkerLayer:path-stroke-width:
   *
   * The path's stroke width (in pixels)
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_STROKE_WIDTH,
      g_param_spec_double ("path-stroke-width",
          "Stroke Width",
          "The path's stroke width",
          0, 100.0,
          2.0,
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainMarkerLayer:path-visible:
   *
   * Wether the path is visible
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_VISIBLE,
      g_param_spec_boolean ("path-visible",
          "Visible",
          "The path's visibility",
          FALSE,
          CHAMPLAIN_PARAM_READWRITE));

}


static void
champlain_marker_layer_init (ChamplainMarkerLayer *self)
{
  ChamplainMarkerLayerPrivate *priv;
  
  self->priv = GET_PRIVATE (self);
  priv = self->priv;
  priv->mode = CHAMPLAIN_SELECTION_NONE;
  priv->view = NULL;

  priv->visible = FALSE;
  priv->fill = FALSE;
  priv->stroke = TRUE;
  priv->stroke_width = 2.0;
  priv->markers = NULL;

  priv->fill_color = clutter_color_copy (&DEFAULT_FILL_COLOR);
  priv->stroke_color = clutter_color_copy (&DEFAULT_STROKE_COLOR);

  priv->content_group = CLUTTER_GROUP (clutter_group_new ());
  clutter_actor_set_parent (CLUTTER_ACTOR (priv->content_group), CLUTTER_ACTOR (self));
  
  //TODO destroy + ref()
  priv->path_actor = clutter_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->content_group), priv->path_actor);
  
}


/**
 * champlain_marker_layer_new_full:
 * @mode: Selection mode
 *
 * Creates a new instance of #ChamplainMarkerLayer.
 *
 * Returns: a new #ChamplainMarkerLayer ready to be used as a container for the markers.
 *
 * Since: 0.10
 */
ChamplainMarkerLayer *
champlain_marker_layer_new_full (ChamplainSelectionMode mode)
{
  return g_object_new (CHAMPLAIN_TYPE_MARKER_LAYER, "selection-mode", mode, NULL);
}


static void
paint (ClutterActor *self)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (self);
  
  clutter_actor_paint (CLUTTER_ACTOR (priv->content_group));
}


static void
pick (ClutterActor *self, 
    const ClutterColor *color)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_marker_layer_parent_class)->pick (self, color);

  clutter_actor_paint (CLUTTER_ACTOR (priv->content_group));
}


static void
get_preferred_width (ClutterActor *self,
    gfloat for_height,
    gfloat *min_width_p,
    gfloat *natural_width_p)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (self);

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
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (self);

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

  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_marker_layer_parent_class)->allocate (self, box, flags);

  child_box.x1 = 0;
  child_box.x2 = box->x2 - box->x1;
  child_box.y1 = 0;
  child_box.y2 = box->y2 - box->y1;

  clutter_actor_allocate (CLUTTER_ACTOR (priv->content_group), &child_box, flags);
}


static void
map (ClutterActor *self)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_marker_layer_parent_class)->map (self);

  clutter_actor_map (CLUTTER_ACTOR (priv->content_group));
}


static void
unmap (ClutterActor *self)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_marker_layer_parent_class)->unmap (self);

  clutter_actor_unmap (CLUTTER_ACTOR (priv->content_group));
}


static void
set_selected_all_but_one (ChamplainMarkerLayer *layer,
    ChamplainMarker *not_selected,
    gboolean select)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);
  GList *elem;
  
  for (elem = priv->markers; elem != NULL; elem = elem->next)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (elem->data);
      
      if (marker != not_selected)
        {
          g_signal_handlers_block_by_func (marker, 
              G_CALLBACK (marker_selected_cb), 
              layer);

          champlain_marker_set_selected (marker, select);
          champlain_marker_set_selectable (marker, layer->priv->mode != CHAMPLAIN_SELECTION_NONE);

          g_signal_handlers_unblock_by_func (marker, 
              G_CALLBACK (marker_selected_cb), 
              layer);
        }
    }
}


static void
marker_selected_cb (ChamplainMarker *marker,
    G_GNUC_UNUSED GParamSpec *arg1,
    ChamplainMarkerLayer *layer)
{
  if (layer->priv->mode == CHAMPLAIN_SELECTION_SINGLE)
    {
      set_selected_all_but_one (layer, marker, FALSE);
    }
}


static void
set_marker_position (ChamplainMarkerLayer *layer, ChamplainMarker *marker)
{
  ChamplainMarkerLayerPrivate *priv = layer->priv;
  gdouble x, y, origin_x, origin_y;
  
  /* layer not yet added to the view */
  if (priv->view == NULL)
    return;

  champlain_view_get_viewport_origin (priv->view, &origin_x, &origin_y);
  x = champlain_view_longitude_to_x (priv->view, 
    champlain_marker_get_longitude (marker)) + origin_x;
  y = champlain_view_latitude_to_y (priv->view, 
    champlain_marker_get_latitude (marker)) + origin_y;

  clutter_actor_set_position (CLUTTER_ACTOR (marker), x, y);
}


static void
marker_position_notify (ChamplainMarker *marker,
    G_GNUC_UNUSED GParamSpec *pspec,
    ChamplainMarkerLayer *layer)
{
  set_marker_position (layer, marker);
  redraw_path (layer);
}


static void
marker_move_by_cb (ChamplainMarker *marker,
    gdouble dx,
    gdouble dy,
    ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = layer->priv;
  ChamplainView *view = priv->view;
  gdouble x, y, lat, lon;

  x = champlain_view_longitude_to_x (view, champlain_marker_get_longitude (marker));
  y = champlain_view_latitude_to_y (view, champlain_marker_get_latitude (marker));
  
  x += dx;
  y += dy;

  lon = champlain_view_x_to_longitude (view, x);
  lat = champlain_view_y_to_latitude (view, y);
    
  champlain_marker_set_position (marker, lat, lon);
}


static void
add_marker (ChamplainMarkerLayer *layer,
    ChamplainMarker *marker,
    gboolean append,
    guint position)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  champlain_marker_set_selectable (marker, layer->priv->mode != CHAMPLAIN_SELECTION_NONE);

  g_signal_connect (G_OBJECT (marker), "notify::selected",
      G_CALLBACK (marker_selected_cb), layer);

  g_signal_connect (G_OBJECT (marker), "notify::latitude",
      G_CALLBACK (marker_position_notify), layer);

  g_signal_connect (G_OBJECT (marker), "drag-motion",
      G_CALLBACK (marker_move_by_cb), layer);

  clutter_container_add_actor (CLUTTER_CONTAINER (priv->content_group), CLUTTER_ACTOR (marker));
  set_marker_position (layer, marker);
  if (append)
    priv->markers = g_list_append (priv->markers, marker);
  else  
    priv->markers = g_list_insert (priv->markers, marker, position);
  redraw_path (layer);
}


/**
 * champlain_marker_layer_add_marker:
 * @layer: a #ChamplainMarkerLayer
 * @marker: a #ChamplainMarker
 *
 * Adds the marker to the layer.
 *
 * Since: 0.10
 */
void
champlain_marker_layer_add_marker (ChamplainMarkerLayer *layer,
    ChamplainMarker *marker)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  add_marker (layer, marker, TRUE, 0);
}


/**
 * champlain_marker_layer_remove_all:
 * @layer: a #ChamplainMarkerLayer
 *
 * Removes all markers from the layer.
 *
 * Since: 0.10
 */
void champlain_marker_layer_remove_all (ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);
  GList *elem;
  
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));


  for (elem = priv->markers; elem != NULL; elem = elem->next)
    {
      GObject *marker = G_OBJECT (elem->data);
      
      g_signal_handlers_disconnect_by_func (marker,
          G_CALLBACK (marker_selected_cb), layer);

      g_signal_handlers_disconnect_by_func (marker,
          G_CALLBACK (marker_position_notify), layer);
    }

  clutter_group_remove_all (CLUTTER_GROUP (priv->content_group));
  g_list_free (priv->markers);
  priv->markers = NULL;

  priv->path_actor = clutter_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->content_group), priv->path_actor);
}


/**
 * champlain_marker_layer_get_markers:
 * @layer: a #ChamplainMarkerLayer
 *
 * Gets the list of all markers inserted into the layer.
 * 
 * Returns: (transfer none) (element-type ChamplainMarker): the list
 *
 * Since: 0.10
 */
GList *
champlain_marker_layer_get_markers (ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);

  return priv->markers;
}


/**
 * champlain_marker_layer_remove_marker:
 * @layer: a #ChamplainMarkerLayer
 * @marker: a #ChamplainMarker
 *
 * Removes the marker from the layer.
 *
 * Since: 0.10
 */
void
champlain_marker_layer_remove_marker (ChamplainMarkerLayer *layer,
    ChamplainMarker *marker)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);
  
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  g_signal_handlers_disconnect_by_func (G_OBJECT (marker),
      G_CALLBACK (marker_selected_cb), layer);

  g_signal_handlers_disconnect_by_func (G_OBJECT (marker),
      G_CALLBACK (marker_position_notify), layer);

  clutter_container_remove_actor (CLUTTER_CONTAINER (priv->content_group), CLUTTER_ACTOR (marker));
  priv->markers = g_list_remove (priv->markers, marker);
  redraw_path (layer);
}


/**
 * champlain_marker_layer_insert_marker:
 * @layer: a #ChamplainMarkerLayer
 * @marker: a #ChamplainMarker
 * @position: position in the list where the marker should be inserted
 *
 * Inserts a marker to the specified position.
 *
 * Since: 0.10
 */
void 
champlain_marker_layer_insert_marker (ChamplainMarkerLayer *layer,
    ChamplainMarker *marker,
    guint position)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  add_marker (layer, marker, FALSE, position);
}

/**
 * champlain_marker_layer_animate_in_all_markers:
 * @layer: a #ChamplainMarkerLayer
 *
 * Fade in all markers with an animation
 *
 * Since: 0.10
 */
void
champlain_marker_layer_animate_in_all_markers (ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);
  GList *elem;
  guint delay = 0;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));
  
  for (elem = priv->markers; elem != NULL; elem = elem->next)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (elem->data);

      champlain_marker_animate_in_with_delay (marker, delay);
      delay += 50;
    }
}


/**
 * champlain_marker_layer_animate_out_all_markers:
 * @layer: a #ChamplainMarkerLayer
 *
 * Fade out all markers with an animation
 *
 * Since: 0.10
 */
void
champlain_marker_layer_animate_out_all_markers (ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);
  
  GList *elem;
  guint delay = 0;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));
  
  for (elem = priv->markers; elem != NULL; elem = elem->next)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (elem->data);

      champlain_marker_animate_out_with_delay (marker, delay);
      delay += 50;
    }
}


/**
 * champlain_marker_layer_show_all_markers:
 * @layer: a #ChamplainMarkerLayer
 *
 * Calls clutter_actor_show on all markers
 *
 * Since: 0.10
 */
void
champlain_marker_layer_show_all_markers (ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);
  GList *elem;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  for (elem = priv->markers; elem != NULL; elem = elem->next)
    {
      ClutterActor *actor = CLUTTER_ACTOR (elem->data);

      clutter_actor_show (actor);
    }
}

/**
 * champlain_marker_layer_hide_all_markers:
 * @layer: a #ChamplainMarkerLayer
 *
 * Calls clutter_actor_hide on all markers
 *
 * Since: 0.10
 */
void
champlain_marker_layer_hide_all_markers (ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);
  GList *elem;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  for (elem = priv->markers; elem != NULL; elem = elem->next)
    {
      ClutterActor *actor = CLUTTER_ACTOR (elem->data);

      clutter_actor_hide (actor);
    }
}


/**
 * champlain_marker_layer_set_all_markers_movable:
 * @layer: a #ChamplainMarkerLayer
 *
 * Sets all markers movable
 *
 * Since: 0.10
 */
void
champlain_marker_layer_set_all_markers_movable (ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);
  GList *elem;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  for (elem = priv->markers; elem != NULL; elem = elem->next)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (elem->data);

      champlain_marker_set_movable (marker, TRUE);
    }
}


/**
 * champlain_marker_layer_set_all_markers_unmovable:
 * @layer: a #ChamplainMarkerLayer
 *
 * Sets all markers unmovable
 *
 * Since: 0.10
 */
void
champlain_marker_layer_set_all_markers_unmovable (ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);
  GList *elem;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  for (elem = priv->markers; elem != NULL; elem = elem->next)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (elem->data);

      champlain_marker_set_movable (marker, FALSE);
    }
}



/**
 * champlain_marker_layer_unselect_all_markers:
 * @layer: a #ChamplainMarkerLayer
 *
 * Unselects all markers.
 *
 * Since: 0.10
 */
void
champlain_marker_layer_unselect_all_markers (ChamplainMarkerLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  set_selected_all_but_one (layer, NULL, FALSE);
}


/**
 * champlain_marker_layer_select_all_markers:
 * @layer: a #ChamplainMarkerLayer
 *
 * Selects all markers in the layer. 
 *
 * Since: 0.10
 */
void
champlain_marker_layer_select_all_markers (ChamplainMarkerLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  set_selected_all_but_one (layer, NULL, TRUE);
}


/**
 * champlain_marker_layer_set_selection_mode:
 * @layer: a #ChamplainMarkerLayer
 * @mode: a #ChamplainSelectionMode value
 *
 * Sets the selection mode of the layer.
 *
 * NOTE: changing selection mode to CHAMPLAIN_SELECTION_NONE or
 * CHAMPLAIN_SELECTION_SINGLE will clear all previously selected markers.
 *
 * Since: 0.10
 */
void
champlain_marker_layer_set_selection_mode (ChamplainMarkerLayer *layer,
    ChamplainSelectionMode mode)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  gboolean select;
  
  if (layer->priv->mode == mode)
    return;
  layer->priv->mode = mode;

  select = mode != CHAMPLAIN_SELECTION_NONE &&
           mode != CHAMPLAIN_SELECTION_SINGLE;

  set_selected_all_but_one (layer, NULL, select);

  g_object_notify (G_OBJECT (layer), "selection-mode");
}


/**
 * champlain_marker_layer_get_selection_mode:
 * @layer: a #ChamplainMarkerLayer
 *
 * Gets the selection mode of the layer.
 *
 * Returns: the selection mode of the layer.
 *
 * Since: 0.10
 */
ChamplainSelectionMode
champlain_marker_layer_get_selection_mode (ChamplainMarkerLayer *layer)
{
  g_return_val_if_fail (
      CHAMPLAIN_IS_MARKER_LAYER (layer),
      CHAMPLAIN_SELECTION_SINGLE);
  return layer->priv->mode;
}


static void
relocate (ChamplainMarkerLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));
    
  ChamplainMarkerLayerPrivate *priv = layer->priv;
  GList *elem;
  
  g_return_if_fail (CHAMPLAIN_IS_VIEW (priv->view));

  for (elem = priv->markers; elem != NULL; elem = elem->next)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (elem->data);
      
      set_marker_position (layer, marker);
    }

  redraw_path (layer);
}

static void
relocate_cb (G_GNUC_UNUSED GObject *gobject,
    ChamplainMarkerLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));
    
  relocate (layer);
}



static void
redraw_path (ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = layer->priv;
  ClutterActor *cairo_texture;
  cairo_t *cr;
  gfloat width, height;
  GList *elem;
  ChamplainView *view = priv->view;
  gdouble x, y;
  
  /* layer not yet added to the view */
  if (view == NULL)
    return;
    
  clutter_actor_get_size (CLUTTER_ACTOR (view), &width, &height);

  if (!priv->visible || width == 0.0 || height == 0.0)
    return;

  clutter_group_remove_all (CLUTTER_GROUP (priv->path_actor));
  cairo_texture = clutter_cairo_texture_new (width, height);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->path_actor), cairo_texture);
  
  champlain_view_get_viewport_origin (priv->view, &x, &y);

  clutter_actor_set_position (priv->path_actor, x, y);

  cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (cairo_texture));

  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  for (elem = priv->markers; elem != NULL; elem = elem->next)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (elem->data);
      gfloat x, y;

      x = champlain_view_longitude_to_x (view, champlain_marker_get_longitude (marker));
      y = champlain_view_latitude_to_y (view, champlain_marker_get_latitude (marker));

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
}


static void
redraw_path_cb (G_GNUC_UNUSED GObject *gobject,
    G_GNUC_UNUSED GParamSpec *arg1,
    ChamplainMarkerLayer *layer)
{
  redraw_path (layer);
}


static void 
set_view (ChamplainLayer *layer,
    ChamplainView *view)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer) && (CHAMPLAIN_IS_VIEW (view) || view == NULL));
  
  ChamplainMarkerLayer *marker_layer = CHAMPLAIN_MARKER_LAYER (layer);
  
  if (marker_layer->priv->view != NULL)
    {
      g_signal_handlers_disconnect_by_func (marker_layer->priv->view,
        G_CALLBACK (relocate_cb), marker_layer);
      g_object_unref (marker_layer->priv->view);
    }
  
  marker_layer->priv->view = view;

  if (view != NULL)
    {
      g_object_ref (view);
  
      g_signal_connect (view, "layer-relocated",
        G_CALLBACK (relocate_cb), layer);

      g_signal_connect (view, "notify::latitude",
        G_CALLBACK (redraw_path_cb), layer);
        
      relocate (marker_layer);
      redraw_path (marker_layer);
    }
}

/**
 * champlain_marker_layer_get_bounding_box:
 * @layer: a #ChamplainMarkerLayer
 *
 * Gets the bounding box occupied by the markers in the layer
 *
 * Returns: The bounding box.
 * 
 * FIXME: This doesn't take into account the marker's actor size yet
 *
 * Since: 0.10
 */
ChamplainBoundingBox *
champlain_marker_layer_get_bounding_box (ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);
  GList *elem;
  ChamplainBoundingBox *bbox;
  
  bbox = champlain_bounding_box_new ();
  bbox->left = CHAMPLAIN_MAX_LONGITUDE;
  bbox->right = CHAMPLAIN_MIN_LONGITUDE;
  bbox->bottom = CHAMPLAIN_MAX_LATITUDE;
  bbox->top = CHAMPLAIN_MIN_LATITUDE;

  for (elem = priv->markers; elem != NULL; elem = elem->next)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (elem->data);
      gdouble lat, lon;
      
      g_object_get (G_OBJECT (marker), "latitude", &lat, "longitude", &lon,
          NULL);

      if (lon < bbox->left)
        bbox->left = lon;

      if (lat < bbox->bottom)
        bbox->bottom = lat;

      if (lon > bbox->right)
        bbox->right = lon;

      if (lat > bbox->top)
        bbox->top = lat;
    }

  return bbox;
}


/**
 * champlain_marker_layer_set_path_fill_color:
 * @layer: a #ChamplainMarkerLayer
 * @color: (allow-none): The path's fill color or NULL to reset to the
 *         default color. The color parameter is copied.
 *
 * Set the path's fill color.
 *
 * Since: 0.10
 */
void
champlain_marker_layer_set_path_fill_color (ChamplainMarkerLayer *layer,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  ChamplainMarkerLayerPrivate *priv = layer->priv;

  if (priv->fill_color != NULL)
    clutter_color_free (priv->fill_color);

  if (color == NULL)
    color = &DEFAULT_FILL_COLOR;

  priv->fill_color = clutter_color_copy (color);
  g_object_notify (G_OBJECT (layer), "path-fill-color");
}


/**
 * champlain_marker_layer_set_path_stroke_color:
 * @layer: a #ChamplainMarkerLayer
 * @color: (allow-none): The path's stroke color or NULL to reset to the
 *         default color. The color parameter is copied.
 *
 * Set the path's stroke color.
 *
 * Since: 0.10
 */
void
champlain_marker_layer_set_path_stroke_color (ChamplainMarkerLayer *layer,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  ChamplainMarkerLayerPrivate *priv = layer->priv;

  if (priv->stroke_color != NULL)
    clutter_color_free (priv->stroke_color);

  if (color == NULL)
    color = &DEFAULT_STROKE_COLOR;

  priv->stroke_color = clutter_color_copy (color);
  g_object_notify (G_OBJECT (layer), "path-stroke-color");
}


/**
 * champlain_marker_layer_get_path_fill_color:
 * @layer: a #ChamplainMarkerLayer
 *
 * Gets the path's fill color.
 *
 * Returns: the path's fill color.
 *
 * Since: 0.10
 */
ClutterColor *
champlain_marker_layer_get_path_fill_color (ChamplainMarkerLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer), NULL);

  return layer->priv->fill_color;
}


/**
 * champlain_marker_layer_get_path_stroke_color:
 * @layer: a #ChamplainMarkerLayer
 *
 * Gets the path's stroke color.
 *
 * Returns: the path's stroke color.
 *
 * Since: 0.10
 */
ClutterColor *
champlain_marker_layer_get_path_stroke_color (ChamplainMarkerLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer), NULL);

  return layer->priv->stroke_color;
}


/**
 * champlain_marker_layer_set_path_stroke:
 * @layer: a #ChamplainMarkerLayer
 * @value: if the path is stroked
 *
 * Sets the path to be stroked
 *
 * Since: 0.10
 */
void
champlain_marker_layer_set_path_stroke (ChamplainMarkerLayer *layer,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  layer->priv->stroke = value;
  g_object_notify (G_OBJECT (layer), "path-stroke");
}


/**
 * champlain_marker_layer_get_path_stroke:
 * @layer: a #ChamplainMarkerLayer
 *
 * Checks whether the path is stroked.
 *
 * Returns: TRUE if the path is stroked, FALSE otherwise.
 *
 * Since: 0.10
 */
gboolean
champlain_marker_layer_get_path_stroke (ChamplainMarkerLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer), FALSE);

  return layer->priv->stroke;
}


/**
 * champlain_marker_layer_set_path_fill:
 * @layer: a #ChamplainMarkerLayer
 * @value: if the path is filled
 *
 * Sets the path to be filled
 *
 * Since: 0.10
 */
void
champlain_marker_layer_set_path_fill (ChamplainMarkerLayer *layer,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  layer->priv->fill = value;
  g_object_notify (G_OBJECT (layer), "path-fill");
}


/**
 * champlain_marker_layer_get_path_fill:
 * @layer: a #ChamplainMarkerLayer
 *
 * Checks whether the path is filled.
 *
 * Returns: TRUE if the path is filled, FALSE otherwise.
 *
 * Since: 0.10
 */
gboolean
champlain_marker_layer_get_path_fill (ChamplainMarkerLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer), FALSE);

  return layer->priv->fill;
}


/**
 * champlain_marker_layer_set_path_stroke_width:
 * @layer: a #ChamplainMarkerLayer
 * @value: the width of the stroke (in pixels)
 *
 * Sets the width of the stroke
 *
 * Since: 0.10
 */
void
champlain_marker_layer_set_path_stroke_width (ChamplainMarkerLayer *layer,
    gdouble value)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  layer->priv->stroke_width = value;
  g_object_notify (G_OBJECT (layer), "path-stroke-width");
}


/**
 * champlain_marker_layer_get_path_stroke_width:
 * @layer: a #ChamplainMarkerLayer
 *
 * Gets the width of the stroke.
 *
 * Returns: the width of the stroke
 *
 * Since: 0.10
 */
gdouble
champlain_marker_layer_get_path_stroke_width (ChamplainMarkerLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer), 0);

  return layer->priv->stroke_width;
}


/**
 * champlain_marker_layer_set_path_visible:
 * @layer: a #ChamplainMarkerLayer
 * @value: TRUE to make the path visible
 *
 * Sets path visibility.
 *
 * Since: 0.10
 */
void
champlain_marker_layer_set_path_visible (ChamplainMarkerLayer *layer,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  layer->priv->visible = value;
  if (value)
    clutter_actor_show (CLUTTER_ACTOR (layer->priv->path_actor));
  else
    clutter_actor_hide (CLUTTER_ACTOR (layer->priv->path_actor));
  g_object_notify (G_OBJECT (layer), "path-visible");
}


/**
 * champlain_marker_layer_get_path_visible:
 * @layer: a #ChamplainMarkerLayer
 *
 * Gets path visibility.
 *
 * Returns: TRUE when the path is visible, FALSE otherwise
 *
 * Since: 0.10
 */
gboolean 
champlain_marker_layer_get_path_visible (ChamplainMarkerLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer), FALSE);

  return layer->priv->visible;
}

