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
#include "champlain-group.h"

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
};


struct _ChamplainMarkerLayerPrivate
{
  ChamplainSelectionMode mode;
  ChamplainView *view;
  
  ClutterGroup *content_group;
};


static void marker_selected_cb (ChamplainMarker *marker,
    G_GNUC_UNUSED GParamSpec *arg1,
    ChamplainMarkerLayer *layer);
    
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
  
  switch (property_id)
    {
    case PROP_SELECTION_MODE:
      champlain_marker_layer_set_selection_mode (self, g_value_get_enum (value));
      break;
      
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
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

}


static void
champlain_marker_layer_init (ChamplainMarkerLayer *self)
{
  ChamplainMarkerLayerPrivate *priv;
  
  self->priv = GET_PRIVATE (self);
  priv = self->priv;
  priv->mode = CHAMPLAIN_SELECTION_NONE;
  priv->view = NULL;

  //TODO destroy + ref()
  priv->content_group = CLUTTER_GROUP (clutter_group_new ());
  clutter_actor_set_parent (CLUTTER_ACTOR (priv->content_group), CLUTTER_ACTOR (self));

  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));
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
set_selected_all_but_one (ChamplainMarkerLayer *layer,
    ChamplainMarker *not_selected,
    gboolean select)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);
  GList *elem;
  GList *markers;
  
  markers = clutter_container_get_children (CLUTTER_CONTAINER (priv->content_group));

  for (elem = markers; elem != NULL; elem = elem->next)
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

  g_list_free (markers);
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
    champlain_location_get_longitude (CHAMPLAIN_LOCATION (marker))) + origin_x;
  y = champlain_view_latitude_to_y (priv->view, 
    champlain_location_get_latitude (CHAMPLAIN_LOCATION (marker))) + origin_y;

  clutter_actor_set_position (CLUTTER_ACTOR (marker), x, y);
}


static void
marker_position_notify (ChamplainMarker *marker,
    G_GNUC_UNUSED GParamSpec *pspec,
    ChamplainMarkerLayer *layer)
{
  set_marker_position (layer, marker);
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

  x = champlain_view_longitude_to_x (view, champlain_location_get_longitude (CHAMPLAIN_LOCATION (marker)));
  y = champlain_view_latitude_to_y (view, champlain_location_get_latitude (CHAMPLAIN_LOCATION (marker)));
  
  x += dx;
  y += dy;

  lon = champlain_view_x_to_longitude (view, x);
  lat = champlain_view_y_to_latitude (view, y);
    
  champlain_location_set_position (CHAMPLAIN_LOCATION (marker), lat, lon);
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
  GList *markers;
  
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  markers = clutter_container_get_children (CLUTTER_CONTAINER (priv->content_group));

  for (elem = markers; elem != NULL; elem = elem->next)
    {
      GObject *marker = G_OBJECT (elem->data);
      
      g_signal_handlers_disconnect_by_func (marker,
          G_CALLBACK (marker_selected_cb), layer);

      g_signal_handlers_disconnect_by_func (marker,
          G_CALLBACK (marker_position_notify), layer);
    }

  champlain_group_remove_all (CHAMPLAIN_GROUP (priv->content_group));
  g_list_free (markers);
}


/**
 * champlain_marker_layer_get_markers:
 * @layer: a #ChamplainMarkerLayer
 *
 * Gets a copy of the list of all markers inserted into the layer. You should
 * free the list but not its contents.
 * 
 * Returns: (transfer container) (element-type ChamplainMarker): the list
 *
 * Since: 0.10
 */
GList *
champlain_marker_layer_get_markers (ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);

  return clutter_container_get_children (CLUTTER_CONTAINER (priv->content_group));
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
  GList *markers;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  markers = clutter_container_get_children (CLUTTER_CONTAINER (priv->content_group));
  
  for (elem = markers; elem != NULL; elem = elem->next)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (elem->data);

      champlain_marker_animate_in_with_delay (marker, delay);
      delay += 50;
    }

  g_list_free (markers);
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
  GList *markers;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  markers = clutter_container_get_children (CLUTTER_CONTAINER (priv->content_group));
  
  for (elem = markers; elem != NULL; elem = elem->next)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (elem->data);

      champlain_marker_animate_out_with_delay (marker, delay);
      delay += 50;
    }

  g_list_free (markers);
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
  GList *markers;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  markers = clutter_container_get_children (CLUTTER_CONTAINER (priv->content_group));

  for (elem = markers; elem != NULL; elem = elem->next)
    {
      ClutterActor *actor = CLUTTER_ACTOR (elem->data);

      clutter_actor_show (actor);
    }

  g_list_free (markers);
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
  GList *markers;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  markers = clutter_container_get_children (CLUTTER_CONTAINER (priv->content_group));

  for (elem = markers; elem != NULL; elem = elem->next)
    {
      ClutterActor *actor = CLUTTER_ACTOR (elem->data);

      clutter_actor_hide (actor);
    }

  g_list_free (markers);
}


/**
 * champlain_marker_layer_set_all_markers_draggable:
 * @layer: a #ChamplainMarkerLayer
 *
 * Sets all markers draggable
 *
 * Since: 0.10
 */
void
champlain_marker_layer_set_all_markers_draggable (ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);
  GList *elem;
  GList *markers;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  markers = clutter_container_get_children (CLUTTER_CONTAINER (priv->content_group));

  for (elem = markers; elem != NULL; elem = elem->next)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (elem->data);

      champlain_marker_set_draggable (marker, TRUE);
    }

  g_list_free (markers);
}


/**
 * champlain_marker_layer_set_all_markers_undraggable:
 * @layer: a #ChamplainMarkerLayer
 *
 * Sets all markers undraggable
 *
 * Since: 0.10
 */
void
champlain_marker_layer_set_all_markers_undraggable (ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = GET_PRIVATE (layer);
  GList *elem;
  GList *markers;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  markers = clutter_container_get_children (CLUTTER_CONTAINER (priv->content_group));

  for (elem = markers; elem != NULL; elem = elem->next)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (elem->data);

      champlain_marker_set_draggable (marker, FALSE);
    }

  g_list_free (markers);
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
  GList *markers;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  markers = clutter_container_get_children (CLUTTER_CONTAINER (priv->content_group));

  for (elem = markers; elem != NULL; elem = elem->next)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (elem->data);
      
      set_marker_position (layer, marker);
    }

  g_list_free (markers);
}


static void
relocate_cb (G_GNUC_UNUSED GObject *gobject,
    ChamplainMarkerLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));
    
  relocate (layer);
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

      relocate (marker_layer);
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
  GList *markers;

  g_return_val_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer), NULL);
  
  bbox = champlain_bounding_box_new ();
  bbox->left = CHAMPLAIN_MAX_LONGITUDE;
  bbox->right = CHAMPLAIN_MIN_LONGITUDE;
  bbox->bottom = CHAMPLAIN_MAX_LATITUDE;
  bbox->top = CHAMPLAIN_MIN_LATITUDE;

  markers = clutter_container_get_children (CLUTTER_CONTAINER (priv->content_group));

  for (elem = markers; elem != NULL; elem = elem->next)
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

  g_list_free (markers);

  return bbox;
}
