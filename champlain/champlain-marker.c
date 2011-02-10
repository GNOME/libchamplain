/*
 * Copyright (C) 2008 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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
 * SECTION:champlain-marker
 * @short_description: A marker to identify points of interest on a map
 *
 * Markers reprensent points of interest on a map. Markers need to be
 * placed on a layer (a #ChamplainMarkerLayer). Layers have to be added to a
 * #champlainview for the markers to show on the map.
 *
 * A marker is nothing more than a regular #clutteractor. You can draw on
 * it what ever you want.  Set the markers position
 * on the map using #champlain_marker_set_position.
 *
 * libchamplain has a more evoluted type of markers with text and image support.
 * See #ChamplainLabel.
 */

#include "config.h"

#include "champlain-marker.h"

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

enum
{
  /* normal signals */
  DRAG_MOTION_SIGNAL,
  DRAG_BEGIN_SIGNAL,
  DRAG_FINISH_SIGNAL,
  LAST_SIGNAL,
};

static guint signals[LAST_SIGNAL] = { 0, };

enum
{
  PROP_0,
  PROP_LONGITUDE,
  PROP_LATITUDE,
  PROP_SELECTED,
  PROP_SELECTABLE,
  PROP_DRAGGABLE,
};

/* static guint champlain_marker_signals[LAST_SIGNAL] = { 0, }; */

G_DEFINE_TYPE (ChamplainMarker, champlain_marker, CLUTTER_TYPE_ACTOR);

#define CHAMPLAIN_MARKER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CHAMPLAIN_TYPE_MARKER, ChamplainMarkerPrivate))

struct _ChamplainMarkerPrivate
{
  gdouble lon;
  gdouble lat;
  gboolean selected;
  gboolean selectable;
  gboolean draggable;
  
  gfloat click_x;
  gfloat click_y;
};

static void
champlain_marker_get_property (GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainMarker *marker = CHAMPLAIN_MARKER (object);
  ChamplainMarkerPrivate *priv = marker->priv;

  switch (prop_id)
    {
    case PROP_LONGITUDE:
      g_value_set_double (value, priv->lon);
      break;

    case PROP_LATITUDE:
      g_value_set_double (value, priv->lat);
      break;

    case PROP_SELECTED:
      g_value_set_boolean (value, priv->selected);
      break;

    case PROP_SELECTABLE:
      g_value_set_boolean (value, priv->selectable);
      break;
      
    case PROP_DRAGGABLE:
      g_value_set_boolean (value, priv->draggable);
      break;
      
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
  ChamplainMarker *marker = CHAMPLAIN_MARKER (object);
  ChamplainMarkerPrivate *priv = marker->priv;

  switch (prop_id)
    {
    case PROP_LONGITUDE:
    {
      gdouble lon = g_value_get_double (value);
      champlain_marker_set_position (marker, priv->lat, lon);
      break;
    }

    case PROP_LATITUDE:
    {
      gdouble lat = g_value_get_double (value);
      champlain_marker_set_position (marker, lat, priv->lon);
      break;
    }

    case PROP_SELECTED:
    {
      gboolean bvalue = g_value_get_boolean (value);
      champlain_marker_set_selected (marker, bvalue);
      break;
    }

    case PROP_SELECTABLE:
    {
      gboolean bvalue = g_value_get_boolean (value);
      champlain_marker_set_selectable (marker, bvalue);
      break;
    }

    case PROP_DRAGGABLE:
    {
      gboolean bvalue = g_value_get_boolean (value);
      champlain_marker_set_draggable (marker, bvalue);
      break;
    }

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
champlain_marker_dispose (GObject *object)
{
  G_OBJECT_CLASS (champlain_marker_parent_class)->dispose (object);
}


static void
champlain_marker_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_marker_parent_class)->finalize (object);
}


static void
champlain_marker_class_init (ChamplainMarkerClass *marker_class)
{
  g_type_class_add_private (marker_class, sizeof (ChamplainMarkerPrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (marker_class);
  object_class->finalize = champlain_marker_finalize;
  object_class->dispose = champlain_marker_dispose;
  object_class->get_property = champlain_marker_get_property;
  object_class->set_property = champlain_marker_set_property;

  /**
   * ChamplainMarker:longitude:
   *
   * The longitude coordonate of the map
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class, PROP_LONGITUDE,
      g_param_spec_double ("longitude", "Longitude",
          "The longitude coordonate of the marker",
          -180.0f, 180.0f, 0.0f, CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainMarker:latitude:
   *
   * The latitude coordonate of the map
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class, PROP_LATITUDE,
      g_param_spec_double ("latitude", "Latitude",
          "The latitude coordonate of the marker",
          -90.0f, 90.0f, 0.0f, CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainMarker:selected:
   *
   * The selected state of the marker
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class, PROP_SELECTED,
      g_param_spec_boolean ("selected", "Selected",
          "The sighlighted state of the marker",
          FALSE, CHAMPLAIN_PARAM_READWRITE));
          
  /**
   * ChamplainMarker:selectable:
   *
   * The selectable state of the marker
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class, PROP_SELECTABLE,
      g_param_spec_boolean ("selectable", "Selectable",
          "The draggable state of the marker",
          FALSE, CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainMarker:draggable:
   *
   * The draggable state of the marker
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class, PROP_DRAGGABLE,
      g_param_spec_boolean ("draggable", "Draggable",
          "The draggable state of the marker",
          FALSE, CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainMarker::drag-motion:
   * @dx: by how much the marker has been moved in the x direction 
   * @dy: by how much the marker has been moved in the y direction 
   *
   * Emmitted when the marker is dragged by mouse. dx and dy specify by how much
   * the marker has been dragged since last time.
   *
   * Since: 0.10
   */
  signals[DRAG_MOTION_SIGNAL] =
    g_signal_new ("drag-motion", G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL,
        _champlain_marshal_VOID__DOUBLE_DOUBLE, G_TYPE_NONE, 2, G_TYPE_DOUBLE, G_TYPE_DOUBLE);
        
  /**
   * ChamplainMarker::drag-begin:
   *
   * Emitted when marker dragging begins.
   *
   * Since: 0.10
   */
  signals[DRAG_BEGIN_SIGNAL] =
    g_signal_new ("drag-begin", G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL,
        g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /**
   * ChamplainMarker::drag-finish:
   *
   * Emitted when marker dragging ends.
   *
   * Since: 0.10
   */
  signals[DRAG_FINISH_SIGNAL] =
    g_signal_new ("drag-finish", G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL,
        g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
        
}


static gboolean
motion_event_cb (ClutterActor        *stage,
                 ClutterMotionEvent  *event,
                 ChamplainMarker *marker)
{
  ChamplainMarkerPrivate *priv = marker->priv;
  gfloat x, y;

  if (event->type != CLUTTER_MOTION)
    return FALSE;

  if (clutter_actor_transform_stage_point (CLUTTER_ACTOR (marker),
                                           event->x,
                                           event->y,
                                           &x, &y))
    {
      g_signal_emit_by_name (marker, "drag-motion", x - priv->click_x, y - priv->click_y);
    }

  return TRUE;
}


static gboolean
button_release_event_cb (ClutterActor        *stage,
                         ClutterButtonEvent  *event,
                         ChamplainMarker *marker)
{
  if ((event->type != CLUTTER_BUTTON_RELEASE) ||
      (event->button != 1))
    return FALSE;

  g_signal_handlers_disconnect_by_func (stage,
                                        motion_event_cb,
                                        marker);
  g_signal_handlers_disconnect_by_func (stage,
                                        button_release_event_cb,
                                        marker);
  
  clutter_set_motion_events_enabled (TRUE);
  g_signal_emit_by_name (marker, "drag-finish");

  return TRUE;
}


static gboolean
button_press_event_cb (ClutterActor        *actor,
                       ClutterEvent        *event,
                       ChamplainMarker *marker)
{
  ChamplainMarkerPrivate *priv = marker->priv;
  ClutterButtonEvent *bevent = (ClutterButtonEvent *)event;
  ClutterActor *stage = clutter_actor_get_stage (actor);
  gboolean swallow_event = FALSE;

  if (event->type != CLUTTER_BUTTON_PRESS ||
      bevent->button != 1 ||
      !stage)
    {
      return FALSE;
    }

  if (priv->selectable)
    {
      champlain_marker_set_selected (marker, TRUE);
      swallow_event = TRUE;
    }
          
  if (clutter_actor_transform_stage_point (actor, bevent->x, bevent->y,
                                           &priv->click_x, &priv->click_y))
    {
      if (priv->draggable) 
        {
          g_signal_connect (stage,
                            "captured-event",
                            G_CALLBACK (motion_event_cb),
                            marker);
                            
          g_signal_connect (stage,
                            "captured-event",
                            G_CALLBACK (button_release_event_cb),
                            marker);

          clutter_set_motion_events_enabled (FALSE);
          g_signal_emit_by_name (marker, "drag-begin");

          swallow_event = TRUE;
        }
    }

  if (swallow_event)
    clutter_actor_raise (CLUTTER_ACTOR (marker), NULL);

  return swallow_event;
}


static void
champlain_marker_init (ChamplainMarker *marker)
{
  ChamplainMarkerPrivate *priv = CHAMPLAIN_MARKER_GET_PRIVATE (marker);

  marker->priv = priv;

  priv->lat = 0;
  priv->lon = 0;
  priv->selected = FALSE;
  priv->selectable = TRUE;
  priv->draggable = FALSE;
  
  clutter_actor_set_reactive (CLUTTER_ACTOR (marker), TRUE);
  
  g_signal_connect (marker,
                    "button-press-event",
                    G_CALLBACK (button_press_event_cb),
                    marker);
}


/**
 * champlain_marker_set_position:
 * @marker: a #ChamplainMarker
 * @latitude: the longitude to center the map at
 * @longitude: the longitude to center the map at
 *
 * Positions the marker on the map at the coordinates
 *
 * Since: 0.10
 */
void
champlain_marker_set_position (ChamplainMarker *marker,
    gdouble latitude,
    gdouble longitude)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  ChamplainMarkerPrivate *priv = marker->priv;

  priv->lon = longitude;
  priv->lat = latitude;

  g_object_notify (G_OBJECT (marker), "latitude");
  g_object_notify (G_OBJECT (marker), "longitude");
}


/**
 * champlain_marker_get_latitude:
 * @marker: a #ChamplainMarker
 *
 * Gets the latitude of the marker.
 *
 * Returns: the latitude of the marker.
 *
 * Since: 0.10
 */
gdouble
champlain_marker_get_latitude (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), 0.0);

  return marker->priv->lat;
}


/**
 * champlain_marker_get_longitude:
 * @marker: a #ChamplainMarker
 *
 * Gets the longitude of the marker.
 *
 * Returns: the longitude of the marker.
 *
 * Since: 0.10
 */
gdouble
champlain_marker_get_longitude (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), 0.0);

  return marker->priv->lon;
}


/**
 * champlain_marker_set_selected:
 * @marker: a #ChamplainMarker
 * @value: the selected state
 *
 * Sets the marker as selected or not. This will affect the "Selected" look
 * of the marker.
 *
 * Since: 0.10
 */
void
champlain_marker_set_selected (ChamplainMarker *marker,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  marker->priv->selected = value;

  g_object_notify (G_OBJECT (marker), "selected");
}


/**
 * champlain_marker_get_selected:
 * @marker: a #ChamplainMarker
 *
 * Checks whether the marker is selected.
 *
 * Returns: the selected or not state of the marker.
 *
 * Since: 0.10
 */
gboolean
champlain_marker_get_selected (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), FALSE);

  return marker->priv->selected;
}


/**
 * champlain_marker_set_selectable:
 * @marker: a #ChamplainMarker
 * @value: the selectable state
 *
 * Sets the marker as selectable or not. 
 *
 * Since: 0.10
 */
void
champlain_marker_set_selectable (ChamplainMarker *marker,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  marker->priv->selectable = value;

  g_object_notify (G_OBJECT (marker), "selectable");
}


/**
 * champlain_marker_get_selectable:
 * @marker: a #ChamplainMarker
 *
 * Checks whether the marker is selectable.
 *
 * Returns: the selectable or not state of the marker.
 *
 * Since: 0.10
 */
gboolean
champlain_marker_get_selectable (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), FALSE);

  return marker->priv->selectable;
}

/**
 * champlain_marker_set_draggable:
 * @marker: a #ChamplainMarker
 * @value: the draggable state
 *
 * Sets the marker as draggable or not. 
 *
 * Since: 0.10
 */
void
champlain_marker_set_draggable (ChamplainMarker *marker,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  marker->priv->draggable = value;

  g_object_notify (G_OBJECT (marker), "draggable");
}


/**
 * champlain_marker_get_draggable:
 * @marker: a #ChamplainMarker
 *
 * Checks whether the marker is draggable.
 *
 * Returns: the draggable or not state of the marker.
 *
 * Since: 0.10
 */
gboolean
champlain_marker_get_draggable (ChamplainMarker *marker)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MARKER (marker), FALSE);

  return marker->priv->draggable;
}

/**
 * champlain_marker_animate_in:
 * @marker: The marker
 *
 * Animates the marker as if it were falling from the sky onto the map.
 *
 * Since: 0.10
 */
void
champlain_marker_animate_in (ChamplainMarker *marker)
{
  champlain_marker_animate_in_with_delay (marker, 0);
}


/**
 * champlain_marker_animate_in_with_delay :
 * @marker: The marker
 * @delay: The delay in milliseconds
 *
 * Animates the marker as if it were falling from the sky onto the map after
 * delay.
 *
 * Since: 0.10
 */
void
champlain_marker_animate_in_with_delay (ChamplainMarker *marker,
    guint delay)
{
  ClutterTimeline *timeline;
  gfloat y;

  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  clutter_actor_show (CLUTTER_ACTOR (marker));
  clutter_actor_set_opacity (CLUTTER_ACTOR (marker), 0);
  clutter_actor_set_scale (CLUTTER_ACTOR (marker), 1.5, 1.5);
  clutter_actor_get_position (CLUTTER_ACTOR (marker), NULL, &y);
  clutter_actor_move_by (CLUTTER_ACTOR (marker), 0, -100);

  timeline = clutter_timeline_new (1000);
  clutter_timeline_set_delay (timeline, delay);
  clutter_actor_animate_with_timeline (CLUTTER_ACTOR (marker),
      CLUTTER_EASE_OUT_BOUNCE, timeline, "opacity", 255, "y", y,
      "scale-x", 1.0, "scale-y", 1.0, NULL);
}


/**
 * champlain_marker_animate_out:
 * @marker: The marker
 *
 * Animates the marker as if it were drawn through the sky.
 *
 * Since: 0.10
 */
void
champlain_marker_animate_out (ChamplainMarker *marker)
{
  champlain_marker_animate_out_with_delay (marker, 0);
}


static gboolean
on_idle (ChamplainMarker *marker)
{
  /* Notify the view that the position changed so that the marker's
   * position is reset, it has to happen on idle as Clutter seems to
   * set actors position after calling animation_completed */
  clutter_actor_hide (CLUTTER_ACTOR (marker));

  g_object_notify (G_OBJECT (marker), "latitude");
  g_object_notify (G_OBJECT (marker), "longitude");
  return FALSE;
}


static void
on_animation_completed (G_GNUC_UNUSED ClutterAnimation *animation,
    ChamplainMarker *marker)
{
  g_idle_add_full (G_PRIORITY_DEFAULT,
      (GSourceFunc) on_idle,
      g_object_ref (marker),
      (GDestroyNotify) g_object_unref);
}


/**
 * champlain_marker_animate_out_with_delay :
 * @marker: The marker
 * @delay: The delay in milliseconds
 *
 * Animates the marker as if it were drawn through the sky after
 * delay.
 *
 * Since: 0.10
 */
void
champlain_marker_animate_out_with_delay (ChamplainMarker *marker,
    guint delay)
{
  ClutterAnimation *animation;
  ClutterTimeline *timeline;
  gfloat y;

  g_return_if_fail (CHAMPLAIN_IS_MARKER (marker));

  clutter_actor_get_position (CLUTTER_ACTOR (marker), NULL, &y);
  clutter_actor_set_opacity (CLUTTER_ACTOR (marker), 200);

  timeline = clutter_timeline_new (750);
  clutter_timeline_set_delay (timeline, delay);
  animation = clutter_actor_animate_with_timeline (CLUTTER_ACTOR (marker),
      CLUTTER_EASE_IN_BACK, timeline, "opacity", 0, "y", y - 100,
      "scale-x", 2.0, "scale-y", 2.0, NULL);
  g_signal_connect (animation, "completed",
      G_CALLBACK (on_animation_completed), marker);
}
