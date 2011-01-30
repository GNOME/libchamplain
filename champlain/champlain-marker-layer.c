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
  CHANGED,
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

static guint signals[LAST_SIGNAL] = { 0, };

struct _ChamplainMarkerLayerPrivate
{
  ChamplainSelectionMode mode;
  ChamplainView *view;
  
  ClutterActor *polygon_actor;
  gboolean closed_path;
  ClutterColor *stroke_color;
  gboolean fill;
  ClutterColor *fill_color;
  gboolean stroke;
  gdouble stroke_width;
  gboolean visible;
};


static void marker_highlighted_cb (ChamplainMarker *marker,
    G_GNUC_UNUSED GParamSpec *arg1,
    ChamplainMarkerLayer *layer);
    
static void redraw_polygon (ChamplainMarkerLayer *layer);

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
      champlain_marker_layer_set_polygon_fill (CHAMPLAIN_MARKER_LAYER (object),
          g_value_get_boolean (value));
      break;

    case PROP_STROKE:
      champlain_marker_layer_set_polygon_stroke (CHAMPLAIN_MARKER_LAYER (object),
          g_value_get_boolean (value));
      break;

    case PROP_FILL_COLOR:
      champlain_marker_layer_set_polygon_fill_color (CHAMPLAIN_MARKER_LAYER (object),
          clutter_value_get_color (value));
      break;

    case PROP_STROKE_COLOR:
      champlain_marker_layer_set_polygon_stroke_color (CHAMPLAIN_MARKER_LAYER (object),
          clutter_value_get_color (value));
      break;

    case PROP_STROKE_WIDTH:
      champlain_marker_layer_set_polygon_stroke_width (CHAMPLAIN_MARKER_LAYER (object),
          g_value_get_double (value));
      break;

    case PROP_VISIBLE:
      if (g_value_get_boolean (value))
        champlain_marker_layer_show_polygon (CHAMPLAIN_MARKER_LAYER (object));
      else
        champlain_marker_layer_hide_polygon (CHAMPLAIN_MARKER_LAYER (object));
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

  G_OBJECT_CLASS (champlain_marker_layer_parent_class)->dispose (object);
}


static void
champlain_marker_layer_finalize (GObject *object)
{
  ChamplainMarkerLayer *self = CHAMPLAIN_MARKER_LAYER (object);
  ChamplainMarkerLayerPrivate *priv = self->priv;
  
  clutter_color_free (priv->stroke_color);
  clutter_color_free (priv->fill_color);

  G_OBJECT_CLASS (champlain_marker_layer_parent_class)->finalize (object);
}


static void
champlain_marker_layer_class_init (ChamplainMarkerLayerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ChamplainLayerClass *layer_class = CHAMPLAIN_LAYER_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainMarkerLayerPrivate));
  
  object_class->finalize = champlain_marker_layer_finalize;
  object_class->dispose = champlain_marker_layer_dispose;
  object_class->get_property = champlain_marker_layer_get_property;
  object_class->set_property = champlain_marker_layer_set_property;
  
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
   * ChamplainPolygon:close-path:
   *
   * The shape is a closed path
   *
   * Since: 0.4
   */
  g_object_class_install_property (object_class,
      PROP_CLOSED_PATH,
      g_param_spec_boolean ("closed-path",
          "Closed Path",
          "The Path is Closed",
          FALSE, CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainPolygon:fill:
   *
   * The shape should be filled
   *
   * Since: 0.4
   */
  g_object_class_install_property (object_class,
      PROP_FILL,
      g_param_spec_boolean ("fill",
          "Fill",
          "The shape is filled",
          FALSE, CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainPolygon:stroke:
   *
   * The shape should be stroked
   *
   * Since: 0.4
   */
  g_object_class_install_property (object_class,
      PROP_STROKE,
      g_param_spec_boolean ("stroke",
          "Stroke",
          "The shape is stroked",
          TRUE, CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainPolygon:stroke-color:
   *
   * The polygon's stroke color
   *
   * Since: 0.4
   */
  g_object_class_install_property (object_class,
      PROP_STROKE_COLOR,
      clutter_param_spec_color ("stroke-color",
          "Stroke Color",
          "The polygon's stroke color",
          &DEFAULT_STROKE_COLOR,
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainPolygon:text-color:
   *
   * The polygon's fill color
   *
   * Since: 0.4
   */
  g_object_class_install_property (object_class,
      PROP_FILL_COLOR,
      clutter_param_spec_color ("fill-color",
          "Fill Color",
          "The polygon's fill color",
          &DEFAULT_FILL_COLOR,
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainPolygon:stroke-width:
   *
   * The polygon's stroke width (in pixels)
   *
   * Since: 0.4
   */
  g_object_class_install_property (object_class,
      PROP_STROKE_WIDTH,
      g_param_spec_double ("stroke-width",
          "Stroke Width",
          "The polygon's stroke width",
          0, 100.0,
          2.0,
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainPolygon:visible:
   *
   * Wether the polygon is visible
   *
   * Since: 0.4
   */
  g_object_class_install_property (object_class,
      PROP_VISIBLE,
      g_param_spec_boolean ("visible",
          "Visible",
          "The polygon's visibility",
          TRUE,
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainMarkerLayer::changed
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
champlain_marker_layer_init (ChamplainMarkerLayer *self)
{
  ChamplainMarkerLayerPrivate *priv;
  
  self->priv = GET_PRIVATE (self);
  priv = self->priv;
  priv->mode = CHAMPLAIN_SELECTION_NONE;
  priv->view = NULL;

  priv->visible = TRUE;
  priv->fill = FALSE;
  priv->stroke = TRUE;
  priv->stroke_width = 2.0;

  priv->fill_color = clutter_color_copy (&DEFAULT_FILL_COLOR);
  priv->stroke_color = clutter_color_copy (&DEFAULT_STROKE_COLOR);
  
  //TODO destroy + ref()
  priv->polygon_actor = clutter_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (self), priv->polygon_actor);
  
}


/**
 * champlain_marker_layer_new_full:
 * @mode: Selection mode
 *
 * Creates a new instance of #ChamplainMarkerLayer.
 *
 * Returns: a new #ChamplainMarkerLayer ready to be used as a #ClutterContainer for the markers.
 *
 * Since: 0.2.2
 */
ChamplainMarkerLayer *
champlain_marker_layer_new_full (ChamplainSelectionMode mode)
{
  return g_object_new (CHAMPLAIN_TYPE_MARKER_LAYER, "selection-mode", mode, NULL);
}


static void
set_highlighted_all_but_one (ChamplainMarkerLayer *layer,
    ChamplainMarker *not_highlighted,
    gboolean highlight)
{
  int i;
  
  for (i = 1; i < clutter_group_get_n_children (CLUTTER_GROUP (layer)); i++)
    {
      ClutterActor *actor = clutter_group_get_nth_child (CLUTTER_GROUP (layer), i);
      ChamplainMarker *marker = CHAMPLAIN_MARKER (actor);
      
      if (marker != not_highlighted)
        {
          g_signal_handlers_block_by_func (marker, 
              G_CALLBACK (marker_highlighted_cb), 
              layer);

          champlain_marker_set_highlighted (marker, highlight);
          champlain_marker_set_selectable (marker, layer->priv->mode != CHAMPLAIN_SELECTION_NONE);

          g_signal_handlers_unblock_by_func (marker, 
              G_CALLBACK (marker_highlighted_cb), 
              layer);
        }
    }
}


static void
marker_highlighted_cb (ChamplainMarker *marker,
    G_GNUC_UNUSED GParamSpec *arg1,
    ChamplainMarkerLayer *layer)
{
  if (layer->priv->mode == CHAMPLAIN_SELECTION_SINGLE)
    {
      set_highlighted_all_but_one (layer, marker, FALSE);
    }
}


static void
set_marker_position (ChamplainMarkerLayer *layer, ChamplainMarker *marker)
{
  ChamplainMarkerLayerPrivate *priv = layer->priv;
  gint x, y;
  
  /* layer not yet added to the view */
  if (priv->view == NULL)
    return;

  x = champlain_view_longitude_to_x (priv->view, 
    champlain_marker_get_longitude (marker));
  x += champlain_view_get_viewport_x (priv->view);
  y = champlain_view_latitude_to_y (priv->view, 
    champlain_marker_get_latitude (marker));
  y += champlain_view_get_viewport_y (priv->view);

  clutter_actor_set_position (CLUTTER_ACTOR (marker), x, y);
}


static void
marker_position_notify (ChamplainMarker *marker,
    G_GNUC_UNUSED GParamSpec *pspec,
    ChamplainMarkerLayer *layer)
{
  set_marker_position (layer, marker);
  redraw_polygon (layer);
}


static void
marker_move_by_cb (ChamplainMarker *marker,
    gfloat dx,
    gfloat dy,
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


/**
 * champlain_marker_layer_add_marker:
 * @layer: a #ChamplainMarkerLayer
 * @marker: a #ChamplainMarker
 *
 * Adds the marker to the layer.
 *
 * Since: 0.4
 */
void
champlain_marker_layer_add_marker (ChamplainMarkerLayer *layer,
    ChamplainMarker *marker)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  champlain_marker_set_selectable (marker, layer->priv->mode != CHAMPLAIN_SELECTION_NONE);

  g_signal_connect (G_OBJECT (marker), "notify::highlighted",
      G_CALLBACK (marker_highlighted_cb), layer);

  g_signal_connect (G_OBJECT (marker), "notify::latitude",
      G_CALLBACK (marker_position_notify), layer);

  g_signal_connect (G_OBJECT (marker), "move-by",
      G_CALLBACK (marker_move_by_cb), layer);

  clutter_container_add_actor (CLUTTER_CONTAINER (layer), CLUTTER_ACTOR (marker));
  set_marker_position (layer, marker);
  redraw_polygon (layer);
}


/**
 * champlain_marker_layer_remove_marker:
 * @layer: a #ChamplainMarkerLayer
 * @marker: a #ChamplainMarker
 *
 * Removes the marker from the layer.
 *
 * Since: 0.4
 */
void
champlain_marker_layer_remove_marker (ChamplainMarkerLayer *layer,
    ChamplainMarker *marker)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  g_signal_handlers_disconnect_by_func (G_OBJECT (marker),
      G_CALLBACK (marker_highlighted_cb), layer);

  g_signal_handlers_disconnect_by_func (G_OBJECT (marker),
      G_CALLBACK (marker_position_notify), layer);

  clutter_container_remove_actor (CLUTTER_CONTAINER (layer), CLUTTER_ACTOR (marker));
  redraw_polygon (layer);
}


/**
 * champlain_marker_layer_animate_in_all_markers:
 * @layer: a #ChamplainMarkerLayer
 *
 * Fade in all markers with an animation
 *
 * Since: 0.4
 */
void
champlain_marker_layer_animate_in_all_markers (ChamplainMarkerLayer *layer)
{
  guint i;
  guint delay = 0;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  for (i = 1; i < clutter_group_get_n_children (CLUTTER_GROUP (layer)); i++)
    {
      ClutterActor *actor = clutter_group_get_nth_child (CLUTTER_GROUP (layer), i);
      ChamplainMarker *marker = CHAMPLAIN_MARKER (actor);

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
 * Since: 0.4
 */
void
champlain_marker_layer_animate_out_all_markers (ChamplainMarkerLayer *layer)
{
  guint i;
  guint delay = 0;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  for (i = 1; i < clutter_group_get_n_children (CLUTTER_GROUP (layer)); i++)
    {
      ClutterActor *actor = clutter_group_get_nth_child (CLUTTER_GROUP (layer), i);
      ChamplainMarker *marker = CHAMPLAIN_MARKER (actor);

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
 * Since: 0.4
 */
void
champlain_marker_layer_show_all_markers (ChamplainMarkerLayer *layer)
{
  guint i;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  for (i = 1; i < clutter_group_get_n_children (CLUTTER_GROUP (layer)); i++)
    {
      ClutterActor *marker = CLUTTER_ACTOR (clutter_group_get_nth_child (CLUTTER_GROUP (layer), i));

      clutter_actor_show (marker);
    }
}

/**
 * champlain_marker_layer_hide_all_markers:
 * @layer: a #ChamplainMarkerLayer
 *
 * Calls clutter_actor_hide on all markers
 *
 * Since: 0.4
 */
void
champlain_marker_layer_hide_all_markers (ChamplainMarkerLayer *layer)
{
  guint i;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  for (i = 1; i < clutter_group_get_n_children (CLUTTER_GROUP (layer)); i++)
    {
      ClutterActor *marker = CLUTTER_ACTOR (clutter_group_get_nth_child (CLUTTER_GROUP (layer), i));

      clutter_actor_hide (marker);
    }
}


void
champlain_marker_layer_set_all_markers_movable (ChamplainMarkerLayer *layer)
{
  guint i;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  for (i = 1; i < clutter_group_get_n_children (CLUTTER_GROUP (layer)); i++)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (clutter_group_get_nth_child (CLUTTER_GROUP (layer), i));

      champlain_marker_set_movable (marker, TRUE);
    }
}


void
champlain_marker_layer_set_all_markers_unmovable (ChamplainMarkerLayer *layer)
{
  guint i;

  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  for (i = 1; i < clutter_group_get_n_children (CLUTTER_GROUP (layer)); i++)
    {
      ChamplainMarker *marker = CHAMPLAIN_MARKER (clutter_group_get_nth_child (CLUTTER_GROUP (layer), i));

      champlain_marker_set_movable (marker, FALSE);
    }
}



/**
 * champlain_marker_layer_get_selected_markers:
 * @layer: a #ChamplainMarkerLayer
 *
 * Gets the list of selected markers.
 *
 * Returns: (transfer container) (element-type ChamplainMarker): the list of selected #ChamplainMarker or NULL if none is selected.
 * You should free the list but not the elements of the list.
 *
 * Since: 0.10
 */
GSList *
champlain_marker_layer_get_selected_markers (ChamplainMarkerLayer *layer)
{
  GSList *lst = NULL;
    
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer), NULL);
    
  gint i, n_children;
  
  n_children = clutter_group_get_n_children (CLUTTER_GROUP (layer));
  for (i = 1; i < n_children; i++)
    {
      ClutterActor *actor = clutter_group_get_nth_child (CLUTTER_GROUP (layer), i);
      
      lst = g_slist_prepend (lst, actor);
    }

  return lst;
}


/**
 * champlain_marker_layer_unselect_all:
 * @layer: a #ChamplainMarkerLayer
 *
 * Unselects all markers.
 *
 * Since: 0.4
 */
void
champlain_marker_layer_unselect_all (ChamplainMarkerLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  set_highlighted_all_but_one (layer, NULL, FALSE);
}


/**
 * champlain_marker_layer_select_all:
 * @layer: a #ChamplainMarkerLayer
 *
 * Selects all markers in the layer. This call will only work if the selection
 * mode is set CHAMPLAIN_SELETION_MULTIPLE.
 *
 * Since: 0.4
 */
void
champlain_marker_layer_select_all (ChamplainMarkerLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  set_highlighted_all_but_one (layer, NULL, TRUE);
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
 * Since: 0.4
 */
void
champlain_marker_layer_set_selection_mode (ChamplainMarkerLayer *layer,
    ChamplainSelectionMode mode)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  gboolean highlight;
  
  if (layer->priv->mode == mode)
    return;
  layer->priv->mode = mode;

  highlight = mode != CHAMPLAIN_SELECTION_NONE &&
              mode != CHAMPLAIN_SELECTION_SINGLE;

  set_highlighted_all_but_one (layer, NULL, highlight);

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
 * Since: 0.4
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
  gint i, n_children;
  
  g_return_if_fail (CHAMPLAIN_IS_VIEW (priv->view));

  n_children = clutter_group_get_n_children (CLUTTER_GROUP (layer));
  for (i = 1; i < n_children; i++)
    {
      ClutterActor *actor = clutter_group_get_nth_child (CLUTTER_GROUP (layer), i);
      ChamplainMarker *marker = CHAMPLAIN_MARKER (actor);
      
      set_marker_position (layer, marker);
    }

  redraw_polygon (layer);
}

static void
relocate_cb (G_GNUC_UNUSED GObject *gobject,
    ChamplainMarkerLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));
    
  relocate (layer);
}



static void
redraw_polygon (ChamplainMarkerLayer *layer)
{
  ChamplainMarkerLayerPrivate *priv = layer->priv;
  ClutterActor *cairo_texture;
  cairo_t *cr;
  gfloat width, height;
  int i, n_children;
  ChamplainView *view = priv->view;
  gdouble x, y;
  
  /* layer not yet added to the view */
  if (view == NULL)
    return;
    
  clutter_actor_get_size (CLUTTER_ACTOR (view), &width, &height);

  if (!priv->visible || width == 0.0 || height == 0.0)
    return;

  clutter_group_remove_all (CLUTTER_GROUP (priv->polygon_actor));
  cairo_texture = clutter_cairo_texture_new (width, height);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->polygon_actor), cairo_texture);
  
  x = champlain_view_get_viewport_x (priv->view);
  y = champlain_view_get_viewport_y (priv->view);

  clutter_actor_set_position (priv->polygon_actor, x, y);

  cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (cairo_texture));

  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  n_children = clutter_group_get_n_children (CLUTTER_GROUP (layer));
  for (i = 1; i < n_children; i++)
    {
      ClutterActor *actor = clutter_group_get_nth_child (CLUTTER_GROUP (layer), i);
      ChamplainMarker *marker = CHAMPLAIN_MARKER (actor);
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
redraw_polygon_cb (G_GNUC_UNUSED GObject *gobject,
    G_GNUC_UNUSED GParamSpec *arg1,
    ChamplainMarkerLayer *layer)
{
  redraw_polygon (layer);
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
        G_CALLBACK (redraw_polygon_cb), layer);
        
      relocate (marker_layer);
      redraw_polygon (marker_layer);
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
    ChamplainMarker *markers[],
    gboolean animate)
{
  DEBUG_LOG ()

  gdouble min_lat, min_lon, max_lat, max_lon;
  ChamplainMarker *marker = NULL;
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


/**
 * champlain_polygon_set_fill_color:
 * @polygon: The polygon
 * @color: (allow-none): The polygon's fill color or NULL to reset to the
 *         default color. The color parameter is copied.
 *
 * Set the polygon's fill color.
 *
 * Since: 0.4
 */
void
champlain_marker_layer_set_polygon_fill_color (ChamplainMarkerLayer *layer,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  ChamplainMarkerLayerPrivate *priv = layer->priv;

  if (priv->fill_color != NULL)
    clutter_color_free (priv->fill_color);

  if (color == NULL)
    color = &DEFAULT_FILL_COLOR;

  priv->fill_color = clutter_color_copy (color);
  g_object_notify (G_OBJECT (layer), "fill-color");
}


/**
 * champlain_polygon_set_stroke_color:
 * @polygon: The polygon
 * @color: (allow-none): The polygon's stroke color or NULL to reset to the
 *         default color. The color parameter is copied.
 *
 * Set the polygon's stroke color.
 *
 * Since: 0.4
 */
void
champlain_marker_layer_set_polygon_stroke_color (ChamplainMarkerLayer *layer,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  ChamplainMarkerLayerPrivate *priv = layer->priv;

  if (priv->stroke_color != NULL)
    clutter_color_free (priv->stroke_color);

  if (color == NULL)
    color = &DEFAULT_STROKE_COLOR;

  priv->stroke_color = clutter_color_copy (color);
  g_object_notify (G_OBJECT (layer), "stroke-color");
}


/**
 * champlain_polygon_get_fill_color:
 * @polygon: The polygon
 *
 * Gets the polygon's fill color.
 *
 * Returns: the polygon's fill color.
 *
 * Since: 0.4
 */
ClutterColor *
champlain_marker_layer_get_polygon_fill_color (ChamplainMarkerLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer), NULL);

  return layer->priv->fill_color;
}


/**
 * champlain_polygon_get_stroke_color:
 * @polygon: The polygon
 *
 * Gets the polygon's stroke color.
 *
 * Returns: the polygon's stroke color.
 *
 * Since: 0.4
 */
ClutterColor *
champlain_marker_layer_get_polygon_stroke_color (ChamplainMarkerLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer), NULL);

  return layer->priv->stroke_color;
}


/**
 * champlain_polygon_set_stroke:
 * @polygon: The polygon
 * @value: if the polygon is stroked
 *
 * Sets the polygon to have a stroke
 *
 * Since: 0.4
 */
void
champlain_marker_layer_set_polygon_stroke (ChamplainMarkerLayer *layer,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  layer->priv->stroke = value;
  g_object_notify (G_OBJECT (layer), "stroke");
}


/**
 * champlain_polygon_get_stroke:
 * @polygon: The polygon
 *
 * Checks whether the polygon has a stroke.
 *
 * Returns: TRUE if the polygon has a stroke, FALSE otherwise.
 *
 * Since: 0.4
 */
gboolean
champlain_marker_layer_get_polygon_stroke (ChamplainMarkerLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer), FALSE);

  return layer->priv->stroke;
}


/**
 * champlain_polygon_set_fill:
 * @polygon: The polygon
 * @value: if the polygon is filled
 *
 * Sets the polygon to have be filled
 *
 * Since: 0.4
 */
void
champlain_marker_layer_set_polygon_fill (ChamplainMarkerLayer *layer,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  layer->priv->fill = value;
  g_object_notify (G_OBJECT (layer), "fill");
}


/**
 * champlain_polygon_get_fill:
 * @polygon: The polygon
 *
 * Checks whether the polygon is filled.
 *
 * Returns: TRUE if the polygon is filled, FALSE otherwise.
 *
 * Since: 0.4
 */
gboolean
champlain_marker_layer_get_polygon_fill (ChamplainMarkerLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer), FALSE);

  return layer->priv->fill;
}


/**
 * champlain_polygon_set_stroke_width:
 * @polygon: The polygon
 * @value: the width of the stroke (in pixels)
 *
 * Sets the width of the stroke
 *
 * Since: 0.4
 */
void
champlain_marker_layer_set_polygon_stroke_width (ChamplainMarkerLayer *layer,
    gdouble value)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  layer->priv->stroke_width = value;
  g_object_notify (G_OBJECT (layer), "stroke-width");
}


/**
 * champlain_polygon_get_stroke_width:
 * @polygon: The polygon
 *
 * Gets the width of the stroke.
 *
 * Returns: the width of the stroke
 *
 * Since: 0.4
 */
gdouble
champlain_marker_layer_get_polygon_stroke_width (ChamplainMarkerLayer *layer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer), 0);

  return layer->priv->stroke_width;
}


/**
 * champlain_polygon_show:
 * @polygon: The polygon
 *
 * Makes the polygon visible
 *
 * Since: 0.4
 */
void
champlain_marker_layer_show_polygon (ChamplainMarkerLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  layer->priv->visible = TRUE;
  clutter_actor_show (CLUTTER_ACTOR (layer->priv->polygon_actor));
  g_object_notify (G_OBJECT (layer->priv->polygon_actor), "visible");
}


/**
 * champlain_polygon_hide:
 * @polygon: The polygon
 *
 * Hides the polygon
 *
 * Since: 0.4
 */
void
champlain_marker_layer_hide_polygon (ChamplainMarkerLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER_LAYER (layer));

  layer->priv->visible = FALSE;
  clutter_actor_hide (CLUTTER_ACTOR (layer->priv->polygon_actor));
  g_object_notify (G_OBJECT (layer->priv->polygon_actor), "visible");
}

