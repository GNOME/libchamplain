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
 * SECTION:champlain-layer
 * @short_description: A container for #ChamplainMarker
 *
 * A ChamplainLayer is little more than a #ClutterContainer. It keeps the
 * markers ordered so that they display correctly.
 *
 * Use #clutter_container_add to add markers to the layer and
 * #clutter_container_remove to remove them.
 */

#include "config.h"

#include "champlain-layer.h"

#include "champlain-defines.h"
#include "champlain-base-marker.h"
#include "champlain-enum-types.h"
#include "champlain-private.h"
#include "champlain-view.h"

#include <clutter/clutter.h>
#include <glib.h>

G_DEFINE_TYPE (ChamplainLayer, champlain_layer, CLUTTER_TYPE_GROUP)

#define GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CHAMPLAIN_TYPE_LAYER, ChamplainLayerPrivate))

enum
{
  /* normal signals */
  CHANGED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SELECTION_MODE
};

static guint signals[LAST_SIGNAL] = { 0, };

struct _ChamplainLayerPrivate
{
  ChamplainSelectionMode mode;
  GList *selection;
  ChamplainView *view;
};

static void
champlain_layer_get_property (GObject *object,
    guint property_id,
    G_GNUC_UNUSED GValue *value,
    GParamSpec *pspec)
{
  ChamplainLayer *self = CHAMPLAIN_LAYER (object);
  ChamplainLayerPrivate *priv = self->priv;
  
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
champlain_layer_set_property (GObject *object,
    guint property_id,
    G_GNUC_UNUSED const GValue *value,
    GParamSpec *pspec)
{
  ChamplainLayer *self = CHAMPLAIN_LAYER (object);
  
  switch (property_id)
    {
    case PROP_SELECTION_MODE:
      champlain_layer_set_selection_mode (self, g_value_get_enum (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
champlain_layer_dispose (GObject *object)
{
  ChamplainLayer *self = CHAMPLAIN_LAYER (object);
  ChamplainLayerPrivate *priv = self->priv;
  
  if (priv->selection != NULL)
    {
      champlain_layer_unselect_all (self);
    }

  if (priv->view != NULL)
    {
      champlain_layer_set_view (self, NULL);
    }

  G_OBJECT_CLASS (champlain_layer_parent_class)->dispose (object);
}


static void
champlain_layer_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_layer_parent_class)->finalize (object);
}


static void
champlain_layer_class_init (ChamplainLayerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainLayerPrivate));
  
  object_class->finalize = champlain_layer_finalize;
  object_class->dispose = champlain_layer_dispose;
  object_class->get_property = champlain_layer_get_property;
  object_class->set_property = champlain_layer_set_property;
  
  /**
   * ChamplainLayer:selection-mode:
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
   * ChamplainLayer::changed
   *
   * The changed signal is emitted when the selected marker(s) change.
   *
   * Since: 0.4.1
   */
  signals[CHANGED] =
    g_signal_new ("changed", G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL,
        g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

}


static void
champlain_layer_init (ChamplainLayer *self)
{
  self->priv = GET_PRIVATE (self);
  self->priv->mode = CHAMPLAIN_SELECTION_NONE;
  self->priv->selection = NULL;
  self->priv->view = NULL;
}


/**
 * champlain_layer_new_full:
 * @mode: Selection mode
 *
 * Creates a new instance of #ChamplainLayer.
 *
 * Returns: a new #ChamplainLayer ready to be used as a #ClutterContainer for the markers.
 *
 * Since: 0.2.2
 */
ChamplainLayer *
champlain_layer_new_full (ChamplainSelectionMode mode)
{
  return g_object_new (CHAMPLAIN_TYPE_LAYER, "selection-mode", mode, NULL);
}


static void
marker_select (ChamplainLayer *layer,
    ChamplainBaseMarker *marker)
{
  /* Add selection */
  g_object_ref (marker);
  champlain_base_marker_set_highlighted (marker, TRUE);
  layer->priv->selection = g_list_prepend (layer->priv->selection, marker);

  g_signal_emit_by_name (layer, "changed", NULL);
}


static void
select (ChamplainLayer *layer,
    ChamplainBaseMarker *marker,
    gboolean mouse,
    gboolean append)
{
  if (layer->priv->mode == CHAMPLAIN_SELECTION_NONE)
    return;

  if (layer->priv->mode == CHAMPLAIN_SELECTION_SINGLE || (mouse && !append))
    {
      /* Clear previous selection */
      champlain_layer_unselect_all (layer);
      marker_select (layer, marker);
    }
  else if (layer->priv->mode == CHAMPLAIN_SELECTION_MULTIPLE)
    {
      if (champlain_layer_marker_is_selected (layer, marker))
        {
          if (mouse)
            champlain_layer_unselect (layer, marker);
        }
      else
        marker_select (layer, marker);
    }
}


static gboolean
marker_clicked_cb (ClutterActor *actor,
    ClutterButtonEvent *event,
    gpointer user_data)
{
  select (CHAMPLAIN_LAYER (user_data),
      CHAMPLAIN_BASE_MARKER (actor),
      TRUE,
      (event->modifier_state & CLUTTER_CONTROL_MASK));

  return TRUE;
}


/**
 * champlain_layer_add_marker:
 * @layer: a #ChamplainLayer
 * @marker: a #ChamplainBaseMarker
 *
 * Adds the marker to the layer.
 *
 * Since: 0.4
 */
void
champlain_layer_add_marker (ChamplainLayer *layer,
    ChamplainBaseMarker *marker)
{
  ClutterActor *actor = CLUTTER_ACTOR (marker);
    
  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));
  g_return_if_fail (CHAMPLAIN_IS_BASE_MARKER (marker));

  clutter_actor_set_reactive (actor, TRUE);

  g_signal_connect (G_OBJECT (marker), "button-release-event",
      G_CALLBACK (marker_clicked_cb), layer);

  clutter_container_add_actor (CLUTTER_CONTAINER (layer), CLUTTER_ACTOR (marker));
}


/**
 * champlain_layer_remove_marker:
 * @layer: a #ChamplainLayer
 * @marker: a #ChamplainBaseMarker
 *
 * Removes the marker from the layer.
 *
 * Since: 0.4
 */
void
champlain_layer_remove_marker (ChamplainLayer *layer,
    ChamplainBaseMarker *marker)
{
  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));
  g_return_if_fail (CHAMPLAIN_IS_BASE_MARKER (marker));

  g_signal_handlers_disconnect_by_func (G_OBJECT (marker),
      G_CALLBACK (marker_clicked_cb), layer);

  clutter_container_remove_actor (CLUTTER_CONTAINER (layer), CLUTTER_ACTOR (marker));
}


/**
 * champlain_layer_animate_in_all_markers:
 * @layer: a #ChamplainLayer
 *
 * Fade in all markers with an animation
 *
 * Since: 0.4
 */
void
champlain_layer_animate_in_all_markers (ChamplainLayer *layer)
{
  guint i;
  guint delay = 0;

  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));

  for (i = 0; i < clutter_group_get_n_children (CLUTTER_GROUP (layer)); i++)
    {
      ChamplainBaseMarker *marker = CHAMPLAIN_BASE_MARKER (clutter_group_get_nth_child (CLUTTER_GROUP (layer), i));

      champlain_base_marker_animate_in_with_delay (marker, delay);
      delay += 50;
    }
}


/**
 * champlain_layer_animate_out_all_markers:
 * @layer: a #ChamplainLayer
 *
 * Fade out all markers with an animation
 *
 * Since: 0.4
 */
void
champlain_layer_animate_out_all_markers (ChamplainLayer *layer)
{
  guint i;
  guint delay = 0;

  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));

  for (i = 0; i < clutter_group_get_n_children (CLUTTER_GROUP (layer)); i++)
    {
      ChamplainBaseMarker *marker = CHAMPLAIN_BASE_MARKER (clutter_group_get_nth_child (CLUTTER_GROUP (layer), i));

      champlain_base_marker_animate_out_with_delay (marker, delay);
      delay += 50;
    }
}


//TODO: do we need it? (we can hide the whole layer)
/**
 * champlain_layer_show_all_markers:
 * @layer: a #ChamplainLayer
 *
 * Calls clutter_actor_show on all markers
 *
 * Since: 0.4
 */
void
champlain_layer_show_all_markers (ChamplainLayer *layer)
{
  guint i;

  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));

  for (i = 0; i < clutter_group_get_n_children (CLUTTER_GROUP (layer)); i++)
    {
      ClutterActor *marker = CLUTTER_ACTOR (clutter_group_get_nth_child (CLUTTER_GROUP (layer), i));

      clutter_actor_show (marker);
    }
}

//TODO: do we need it? (we can hide the whole layer)
/**
 * champlain_layer_hide_all_markers:
 * @layer: a #ChamplainLayer
 *
 * Calls clutter_actor_hide on all markers
 *
 * Since: 0.4
 */
void
champlain_layer_hide_all_markers (ChamplainLayer *layer)
{
  guint i;

  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));

  for (i = 0; i < clutter_group_get_n_children (CLUTTER_GROUP (layer)); i++)
    {
      ClutterActor *marker = CLUTTER_ACTOR (clutter_group_get_nth_child (CLUTTER_GROUP (layer), i));

      clutter_actor_hide (marker);
    }
}


/**
 * champlain_layer_get_selected_markers:
 * @layer: a #ChamplainLayer
 *
 * Gets the list of selected markers.
 *
 * Returns: (transfer none) (element-type ChamplainBaseMarker): the list of selected #ChamplainBaseMarker or NULL if none is selected.
 * You shouldn't free that list.
 *
 * Since: 0.4
 */
const GList *
champlain_layer_get_selected_markers (ChamplainLayer *layer)
{
  return layer->priv->selection;
}


/**
 * champlain_layer_select:
 * @layer: a #ChamplainLayer
 * @marker: a #ChamplainBaseMarker
 *
 * Selects the marker.
 *
 * Since: 0.4
 */
void
champlain_layer_select (ChamplainLayer *layer,
    ChamplainBaseMarker *marker)
{
  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));
  g_return_if_fail (CHAMPLAIN_IS_BASE_MARKER (marker));

  select (layer, marker, FALSE, FALSE);
}


/**
 * champlain_layer_unselect:
 * @layer: a #ChamplainLayer
 * @marker: a #ChamplainBaseMarker
 *
 * Unselect the marker.
 *
 * Since: 0.4
 */
void
champlain_layer_unselect (ChamplainLayer *layer,
    ChamplainBaseMarker *marker)
{
  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));
  g_return_if_fail (CHAMPLAIN_IS_BASE_MARKER (marker));

  GList *selection;

  selection = g_list_find (layer->priv->selection, marker);
  if (selection != NULL)
    {
      champlain_base_marker_set_highlighted (marker, FALSE);
      g_object_unref (selection->data);
      layer->priv->selection = g_list_delete_link (layer->priv->selection, selection);

      g_signal_emit_by_name (layer, "changed", NULL);
    }
}


/**
 * champlain_layer_marker_is_selected:
 * @layer: a #ChamplainLayer
 * @marker: a #ChamplainBaseMarker
 *
 * Checks whether the marker is selected.
 *
 * Returns: whether the marker is selected or not.
 *
 * Since: 0.4
 */
gboolean
champlain_layer_marker_is_selected (ChamplainLayer *layer,
    ChamplainBaseMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_LAYER (layer), FALSE);
  g_return_val_if_fail (CHAMPLAIN_IS_BASE_MARKER (marker), FALSE);

  GList *selection;

  selection = g_list_find (layer->priv->selection, marker);
  return selection != NULL;
}


/**
 * champlain_layer_unselect_all:
 * @layer: a #ChamplainLayer
 *
 * Unselects all markers.
 *
 * Since: 0.4
 */
void
champlain_layer_unselect_all (ChamplainLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));

  GList *selection = layer->priv->selection;
  
  if (selection == NULL)
    return;

  while (selection != NULL)
    {
      champlain_base_marker_set_highlighted (selection->data, FALSE);
      g_object_unref (selection->data);
      selection = g_list_delete_link (selection, selection);
    }
  layer->priv->selection = NULL;

  g_signal_emit_by_name (layer, "changed", NULL);
}


/**
 * champlain_layer_select_all:
 * @layer: a #ChamplainLayer
 *
 * Selects all markers in the layer. This call will only work if the selection
 * mode is set CHAMPLAIN_SELETION_MULTIPLE.
 *
 * Since: 0.4
 */
void
champlain_layer_select_all (ChamplainLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));

  gint n_children;
  gint i;

  if (layer->priv->mode == CHAMPLAIN_SELECTION_NONE)
    return;

  if (layer->priv->mode == CHAMPLAIN_SELECTION_SINGLE)
    return;

  n_children = clutter_group_get_n_children (CLUTTER_GROUP (layer));
  for (i = 0; i < n_children; i++)
    {
      ClutterActor *actor = clutter_group_get_nth_child (
          CLUTTER_GROUP (layer), i);
      if (CHAMPLAIN_IS_BASE_MARKER (actor))
        {
          ChamplainBaseMarker *marker = CHAMPLAIN_BASE_MARKER (actor);
          select (layer, marker, FALSE, FALSE);
        }
    }
}


/**
 * champlain_layer_set_selection_mode:
 * @layer: a #ChamplainLayer
 * @mode: a #ChamplainSelectionMode value
 *
 * Sets the selection mode of the layer.
 *
 * NOTE: changing selection mode to CHAMPLAIN_SELECTION_NONE or
 * CHAMPLAIN_SELECTION_SINGLE will clear all previously selected markers.
 *
 * Since: 0.4
 */
void
champlain_layer_set_selection_mode (ChamplainLayer *layer,
    ChamplainSelectionMode mode)
{
  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));

  if (layer->priv->mode == mode)
    return;
  layer->priv->mode = mode;

  /* Switching to single mode shouldn't keep the selection */
  if (mode == CHAMPLAIN_SELECTION_NONE ||
      mode == CHAMPLAIN_SELECTION_SINGLE)
    champlain_layer_unselect_all (layer);

  g_object_notify (G_OBJECT (layer), "selection-mode");
}


/**
 * champlain_layer_get_selection_mode:
 * @layer: a #ChamplainLayer
 *
 * Gets the selection mode of the layer.
 *
 * Returns: the selection mode of the layer.
 *
 * Since: 0.4
 */
ChamplainSelectionMode
champlain_layer_get_selection_mode (ChamplainLayer *layer)
{
  g_return_val_if_fail (
      CHAMPLAIN_IS_LAYER (layer),
      CHAMPLAIN_SELECTION_SINGLE);
  return layer->priv->mode;
}


static void
relocate_cb (G_GNUC_UNUSED GObject *gobject,
    ChamplainLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));
    
  ChamplainLayerPrivate *priv = layer->priv;
  gint i, n_children;
  
  g_return_if_fail (CHAMPLAIN_IS_VIEW (priv->view));

  n_children = clutter_group_get_n_children (CLUTTER_GROUP (layer));
  for (i = 0; i < n_children; i++)
    {
      ClutterActor *actor = clutter_group_get_nth_child (
          CLUTTER_GROUP (layer), i);
      if (CHAMPLAIN_IS_BASE_MARKER (actor))
        {
          ChamplainBaseMarker *marker = CHAMPLAIN_BASE_MARKER (actor);
          gint x, y;

          x = champlain_view_longitude_to_layer_x (priv->view, 
            champlain_base_marker_get_longitude (marker));
          y = champlain_view_latitude_to_layer_y (priv->view, 
            champlain_base_marker_get_latitude (marker));

          clutter_actor_set_position (CLUTTER_ACTOR (marker), x, y);
        }
    }
}


static gboolean
button_release_cb (G_GNUC_UNUSED ClutterActor *actor,
    ClutterEvent *event,
    ChamplainLayer *layer)
{
  gboolean found = FALSE;
  
  if (clutter_event_get_button (event) != 1)
    return FALSE;

  if (champlain_layer_get_selected_markers (layer) != NULL)
    {
      champlain_layer_unselect_all (layer);
      found = TRUE;
    }

  return found;
}





void champlain_layer_set_view (ChamplainLayer *layer,
    ChamplainView *view)
{
  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer) && (CHAMPLAIN_IS_VIEW (view) || view == NULL));
  
  if (layer->priv->view != NULL)
    {
      g_signal_handlers_disconnect_by_func (layer->priv->view,
        G_CALLBACK (relocate_cb), layer);
      g_object_unref (layer->priv->view);
    }
  
  layer->priv->view = view;

  if (view != NULL)
    {
      g_object_ref (view);
  
      g_signal_connect (view, "layer-relocated",
        G_CALLBACK (relocate_cb), layer);
        
      g_signal_connect_after (view, "button-release-event",
        G_CALLBACK (button_release_cb), layer);
  
    }
}

/**
 * champlain_view_ensure_markers_visible:
 * @view: a #ChamplainView
 * @markers: (array zero-terminated=1): a NULL terminated array of #ChamplainMarker elements
 * @animate: a #gboolean
 *
 * Changes the map's zoom level and center to make sure those markers are
 * visible.
 *
 * FIXME: This doesn't take into account the marker's actor size yet
 *
 * Since: 0.4
 */
/*void
champlain_view_ensure_markers_visible (ChamplainView *view,
    ChamplainBaseMarker *markers[],
    gboolean animate)
{
  DEBUG_LOG ()

  gdouble min_lat, min_lon, max_lat, max_lon;
  ChamplainBaseMarker *marker = NULL;
  gint i = 0;

  min_lat = min_lon = 200;
  max_lat = max_lon = -200;

  marker = markers[i];
  while (marker != NULL)
    {
      gdouble lat, lon;
      g_object_get (G_OBJECT (marker), "latitude", &lat, "longitude", &lon,
          NULL);

      if (lon < min_lon)
        min_lon = lon;

      if (lat < min_lat)
        min_lat = lat;

      if (lon > max_lon)
        max_lon = lon;

      if (lat > max_lat)
        max_lat = lat;

      marker = markers[i++];
    }
  champlain_view_ensure_visible (view, min_lat, min_lon, max_lat, max_lon, animate);
}*/


