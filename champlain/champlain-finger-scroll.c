/* champlain-finger-scroll.c: Finger scrolling container actor
 *
 * Copyright (C) 2008 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Written by: Chris Lord <chris@openedhand.com>
 */

#include "champlain-finger-scroll.h"
#include "champlain-enum-types.h"
#include "champlain-marshal.h"
#include "champlain-adjustment.h"
#include "champlain-viewport.h"
#include <clutter/clutter.h>
#include <math.h>

static void clutter_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (ChamplainFingerScroll, champlain_finger_scroll, CLUTTER_TYPE_ACTOR,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init))


#define FINGER_SCROLL_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                  CHAMPLAIN_TYPE_FINGER_SCROLL, \
                                  ChamplainFingerScrollPrivate))

typedef struct {
  /* Units to store the origin of a click when scrolling */
  gfloat x;
  gfloat y;
  GTimeVal    time;
} ChamplainFingerScrollMotion;

struct _ChamplainFingerScrollPrivate
{
  /* Scroll mode */
  gboolean kinetic;

  GArray                *motion_buffer;
  guint                  last_motion;

  /* Variables for storing acceleration information for kinetic mode */
  ClutterTimeline       *deceleration_timeline;
  gdouble                dx;
  gdouble                dy;
  gdouble                decel_rate;

  ClutterActor *child;
};

enum {
  PROP_MODE = 1,
  PROP_DECEL_RATE,
  PROP_BUFFER,
};

enum
{
  /* normal signals */
  PANNING_COMPLETED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static void
champlain_finger_scroll_get_property (GObject *object, guint property_id,
                                 GValue *value, GParamSpec *pspec)
{
  ChamplainFingerScrollPrivate *priv = CHAMPLAIN_FINGER_SCROLL (object)->priv;

  switch (property_id)
    {
    case PROP_MODE :
      g_value_set_boolean (value, priv->kinetic);
      break;
    case PROP_DECEL_RATE :
      g_value_set_double (value, priv->decel_rate);
      break;
    case PROP_BUFFER :
      g_value_set_uint (value, priv->motion_buffer->len);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_finger_scroll_set_property (GObject *object, guint property_id,
                                 const GValue *value, GParamSpec *pspec)
{
  ChamplainFingerScrollPrivate *priv = CHAMPLAIN_FINGER_SCROLL (object)->priv;

  switch (property_id)
    {
    case PROP_MODE :
      priv->kinetic = g_value_get_boolean (value);
      g_object_notify (object, "mode");
      break;
    case PROP_DECEL_RATE :
      priv->decel_rate = g_value_get_double (value);
      g_object_notify (object, "decel-rate");
      break;
    case PROP_BUFFER :
      g_array_set_size (priv->motion_buffer, g_value_get_uint (value));
      g_object_notify (object, "motion-buffer");
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_finger_scroll_dispose (GObject *object)
{
  ChamplainFingerScrollPrivate *priv = CHAMPLAIN_FINGER_SCROLL (object)->priv;

  if (priv->child)
    clutter_container_remove_actor (CLUTTER_CONTAINER (object), priv->child);

  if (priv->deceleration_timeline)
    {
      clutter_timeline_stop (priv->deceleration_timeline);
      g_object_unref (priv->deceleration_timeline);
      priv->deceleration_timeline = NULL;
    }

  G_OBJECT_CLASS (champlain_finger_scroll_parent_class)->dispose (object);
}

static void
champlain_finger_scroll_finalize (GObject *object)
{
  ChamplainFingerScrollPrivate *priv = CHAMPLAIN_FINGER_SCROLL (object)->priv;

  g_array_free (priv->motion_buffer, TRUE);

  G_OBJECT_CLASS (champlain_finger_scroll_parent_class)->finalize (object);
}


static void
champlain_finger_scroll_paint (ClutterActor *actor)
{
  ChamplainFingerScrollPrivate *priv = CHAMPLAIN_FINGER_SCROLL (actor)->priv;

  if (priv->child)
    clutter_actor_paint (priv->child);
}


static void
champlain_finger_scroll_pick (ClutterActor *actor, const ClutterColor *color)
{
  /* Chain up so we get a bounding box pained (if we are reactive) */
  CLUTTER_ACTOR_CLASS (champlain_finger_scroll_parent_class)->pick (actor, color);

  /* Trigger pick on children */
  champlain_finger_scroll_paint (actor);
}


static void
champlain_finger_scroll_get_preferred_width (ClutterActor *actor,
                                      gfloat   for_height,
                                      gfloat  *min_width_p,
                                      gfloat  *natural_width_p)
{

  ChamplainFingerScrollPrivate *priv = CHAMPLAIN_FINGER_SCROLL (actor)->priv;

  if (!priv->child)
    return;


  /* Our natural width is the natural width of the child */
  clutter_actor_get_preferred_width (priv->child,
                                     for_height,
                                     NULL,
                                     natural_width_p);

}

static void
champlain_finger_scroll_get_preferred_height (ClutterActor *actor,
                                       gfloat   for_width,
                                       gfloat  *min_height_p,
                                       gfloat  *natural_height_p)
{

  ChamplainFingerScrollPrivate *priv = CHAMPLAIN_FINGER_SCROLL (actor)->priv;

  if (!priv->child)
    return;


  /* Our natural height is the natural height of the child */
  clutter_actor_get_preferred_height (priv->child,
                                      for_width,
                                      NULL,
                                      natural_height_p);
}

static void
champlain_finger_scroll_allocate (ClutterActor          *actor,
                           const ClutterActorBox *box,
                           ClutterAllocationFlags flags)
{
  ClutterActorBox child_box;

  ChamplainFingerScrollPrivate *priv = CHAMPLAIN_FINGER_SCROLL (actor)->priv;

  /* Chain up */
  CLUTTER_ACTOR_CLASS (champlain_finger_scroll_parent_class)->
    allocate (actor, box, flags);

  /* Child */
  child_box.x1 = 0;
  child_box.x2 = box->x2 - box->x1;
  child_box.y1 = 0;
  child_box.y2 = box->y2 - box->y1;

  if (priv->child)
    {
      clutter_actor_allocate (priv->child, &child_box, flags);
      clutter_actor_set_clip (priv->child,
                              child_box.x1,
                              child_box.y1,
                              child_box.x2 - child_box.x1,
                              child_box.y2 - child_box.y1);
    }

}

static void
champlain_finger_scroll_class_init (ChamplainFingerScrollClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainFingerScrollPrivate));

  object_class->get_property = champlain_finger_scroll_get_property;
  object_class->set_property = champlain_finger_scroll_set_property;
  object_class->dispose = champlain_finger_scroll_dispose;
  object_class->finalize = champlain_finger_scroll_finalize;

  actor_class->paint = champlain_finger_scroll_paint;
  actor_class->pick = champlain_finger_scroll_pick;
  actor_class->get_preferred_width = champlain_finger_scroll_get_preferred_width;
  actor_class->get_preferred_height = champlain_finger_scroll_get_preferred_height;
  actor_class->allocate = champlain_finger_scroll_allocate;

  g_object_class_install_property (object_class,
                                   PROP_MODE,
                                   g_param_spec_boolean ("mode",
                                                      "ChamplainFingerScrollMode",
                                                      "Scrolling mode",
                                                      FALSE,
                                                      G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                                   PROP_DECEL_RATE,
                                   g_param_spec_double ("decel-rate",
                                                        "Deceleration rate",
                                                        "Rate at which the view "
                                                        "will decelerate in "
                                                        "kinetic mode.",
                                                        G_MINFLOAT + 1,
                                                        G_MAXFLOAT,
                                                        1.1,
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                                   PROP_BUFFER,
                                   g_param_spec_uint ("motion-buffer",
                                                      "Motion buffer",
                                                      "Amount of motion "
                                                      "events to buffer",
                                                      1, G_MAXUINT, 3,
                                                      G_PARAM_READWRITE));

  signals[PANNING_COMPLETED] =
      g_signal_new ("panning-completed", G_OBJECT_CLASS_TYPE (object_class),
          G_SIGNAL_RUN_LAST, 0, NULL, NULL,
          g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
champlain_finger_scroll_add_actor (ClutterContainer *container,
                            ClutterActor     *actor)
{
  ChamplainFingerScroll *self = CHAMPLAIN_FINGER_SCROLL (container);
  ChamplainFingerScrollPrivate *priv = self->priv;

  if (priv->child)
    {
      g_warning ("Attempting to add an actor of type %s to "
                 "a ChamplainFingerScroll that already contains "
                 "an actor of type %s.",
                 g_type_name (G_OBJECT_TYPE (actor)),
                 g_type_name (G_OBJECT_TYPE (priv->child)));
    }
  else
    {
      if (CHAMPLAIN_IS_VIEWPORT(actor))
        {
          priv->child = actor;
          clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));

          /* Notify that child has been set */
          g_signal_emit_by_name (container, "actor-added", priv->child);

          clutter_actor_queue_relayout (CLUTTER_ACTOR (container));
        }
      else
        {
          g_warning ("Attempting to add an actor to "
                     "a ChamplainFingerScroll, but the actor does "
                     "not implement ChamplainViewport.");
        }
    }
}

static void
champlain_finger_scroll_remove_actor (ClutterContainer *container,
                               ClutterActor     *actor)
{
  ChamplainFingerScrollPrivate *priv = CHAMPLAIN_FINGER_SCROLL (container)->priv;

  if (actor == priv->child)
    {
      g_object_ref (priv->child);


      clutter_actor_unparent (priv->child);

      g_signal_emit_by_name (container, "actor-removed", priv->child);

      g_object_unref (priv->child);
      priv->child = NULL;

      if (CLUTTER_ACTOR_IS_VISIBLE (container))
        clutter_actor_queue_relayout (CLUTTER_ACTOR (container));
    }
}

static void
champlain_finger_scroll_foreach (ClutterContainer *container,
                          ClutterCallback   callback,
                          gpointer          callback_data)
{
  ChamplainFingerScrollPrivate *priv = CHAMPLAIN_FINGER_SCROLL (container)->priv;

  if (priv->child)
    callback (priv->child, callback_data);
}

static void
champlain_finger_scroll_lower (ClutterContainer *container,
                        ClutterActor     *actor,
                        ClutterActor     *sibling)
{
  /* single child */
}

static void
champlain_finger_scroll_raise (ClutterContainer *container,
                        ClutterActor     *actor,
                        ClutterActor     *sibling)
{
  /* single child */
}

static void
champlain_finger_scroll_sort_depth_order (ClutterContainer *container)
{
  /* single child */
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = champlain_finger_scroll_add_actor;
  iface->remove = champlain_finger_scroll_remove_actor;
  iface->foreach = champlain_finger_scroll_foreach;
  iface->lower = champlain_finger_scroll_lower;
  iface->raise = champlain_finger_scroll_raise;
  iface->sort_depth_order = champlain_finger_scroll_sort_depth_order;
}



static gboolean
motion_event_cb (ClutterActor *actor,
                 ClutterMotionEvent *event,
                 ChamplainFingerScroll *scroll)
{
  gfloat x, y;

  ChamplainFingerScrollPrivate *priv = scroll->priv;

  if (clutter_actor_transform_stage_point (actor,
                                           event->x,
                                           event->y,
                                           &x, &y))
    {
      ChamplainFingerScrollMotion *motion;

      if (priv->child)
        {
          gdouble dx, dy;
          ChamplainAdjustment *hadjust, *vadjust;

          champlain_viewport_get_adjustments (CHAMPLAIN_VIEWPORT (priv->child),
                                           &hadjust,
                                           &vadjust);

          motion = &g_array_index (priv->motion_buffer,
                                   ChamplainFingerScrollMotion, priv->last_motion);
          dx = (motion->x - x) +
               champlain_adjustment_get_value (hadjust);
          dy = (motion->y - y) +
               champlain_adjustment_get_value (vadjust);

          champlain_adjustment_set_value (hadjust, dx);
          champlain_adjustment_set_value (vadjust, dy);
        }

      priv->last_motion ++;
      if (priv->last_motion == priv->motion_buffer->len)
        {
          priv->motion_buffer = g_array_remove_index (priv->motion_buffer, 0);
          g_array_set_size (priv->motion_buffer, priv->last_motion);
          priv->last_motion --;
        }

      motion = &g_array_index (priv->motion_buffer,
                               ChamplainFingerScrollMotion, priv->last_motion);
      motion->x = x;
      motion->y = y;
      g_get_current_time (&motion->time);
    }

  return TRUE;
}

static void
clamp_adjustments (ChamplainFingerScroll *scroll)
{
  ChamplainFingerScrollPrivate *priv = scroll->priv;
  
  if (priv->child)
    {
      guint fps, n_frames;
      ChamplainAdjustment *hadj, *vadj;
      gboolean snap;

      champlain_viewport_get_adjustments (CHAMPLAIN_VIEWPORT (priv->child),
                                       &hadj, &vadj);

      /* FIXME: Hard-coded value here */
      fps = clutter_get_default_frame_rate ();
      n_frames = fps / 6;

      snap = TRUE;
      if (champlain_adjustment_get_elastic (hadj))
        snap = !champlain_adjustment_clamp (hadj, TRUE, n_frames, fps);

      /* Snap to the nearest step increment on hadjustment */
      if (snap)
        {
          gdouble d, value, lower, step_increment;

          champlain_adjustment_get_values (hadj, &value, &lower, NULL,
                                      &step_increment, NULL, NULL);
          d = (rint ((value - lower) / step_increment) *
              step_increment) + lower;
          champlain_adjustment_set_value (hadj, d);
        }

      snap = TRUE;
      if (champlain_adjustment_get_elastic (vadj))
        snap = !champlain_adjustment_clamp (vadj, TRUE, n_frames, fps);

      /* Snap to the nearest step increment on vadjustment */
      if (snap)
        {
          gdouble d, value, lower, step_increment;

          champlain_adjustment_get_values (vadj, &value, &lower, NULL,
                                      &step_increment, NULL, NULL);
          d = (rint ((value - lower) / step_increment) *
              step_increment) + lower;
          champlain_adjustment_set_value (vadj, d);
        }
    }
}

static void
deceleration_completed_cb (ClutterTimeline *timeline,
                           ChamplainFingerScroll *scroll)
{
  clamp_adjustments (scroll);
  g_object_unref (timeline);
  scroll->priv->deceleration_timeline = NULL;

  g_signal_emit_by_name (scroll, "panning-completed", NULL);
}

static void
deceleration_new_frame_cb (ClutterTimeline *timeline,
                           gint frame_num,
                           ChamplainFingerScroll *scroll)
{
  ChamplainFingerScrollPrivate *priv = scroll->priv;

  if (priv->child)
    {
      gdouble    value, lower, upper, page_size;
      ChamplainAdjustment *hadjust, *vadjust;
      gint i;
      gboolean stop = TRUE;

      champlain_viewport_get_adjustments (CHAMPLAIN_VIEWPORT (priv->child),
                                       &hadjust,
                                       &vadjust);

      for (i = 0; i < clutter_timeline_get_delta (timeline) / 15; i++)
        {
          champlain_adjustment_set_value (hadjust,
                                      priv->dx +
                                        champlain_adjustment_get_value (hadjust));
          champlain_adjustment_set_value (vadjust,
                                      priv->dy +
                                        champlain_adjustment_get_value (vadjust));
          priv->dx = (priv->dx / priv->decel_rate);
          priv->dy = (priv->dy / priv->decel_rate);
        }

      /* Check if we've hit the upper or lower bounds and stop the timeline */
      champlain_adjustment_get_values (hadjust, &value, &lower, &upper,
                                   NULL, NULL, &page_size);
      if (((priv->dx > 0) && (value < upper - page_size)) ||
          ((priv->dx < 0) && (value > lower)))
        stop = FALSE;

      if (stop)
        {
          champlain_adjustment_get_values (vadjust, &value, &lower, &upper,
                                       NULL, NULL, &page_size);
          if (((priv->dy > 0) && (value < upper - page_size)) ||
              ((priv->dy < 0) && (value > lower)))
            stop = FALSE;
        }

      if (stop)
        {
          clutter_timeline_stop (timeline);
          deceleration_completed_cb (timeline, scroll);
        }
    }
}

static gboolean
button_release_event_cb (ClutterActor *actor,
                         ClutterButtonEvent *event,
                         ChamplainFingerScroll *scroll)
{
  ChamplainFingerScrollPrivate *priv = scroll->priv;
  gboolean decelerating = FALSE;
  gboolean moved = TRUE;

  if (event->button != 1)
    return FALSE;

  g_signal_handlers_disconnect_by_func (actor,
                                        motion_event_cb,
                                        scroll);
  g_signal_handlers_disconnect_by_func (actor,
                                        button_release_event_cb,
                                        scroll);

  clutter_ungrab_pointer ();

  if (priv->kinetic && priv->child)
    {
      gfloat x, y;

      if (clutter_actor_transform_stage_point (actor,
                                               event->x,
                                               event->y,
                                               &x, &y))
        {
          double frac, x_origin, y_origin;
          GTimeVal release_time, motion_time;
          ChamplainAdjustment *hadjust, *vadjust;
          glong time_diff;
          gint i;

          /* Get time delta */
          g_get_current_time (&release_time);

          /* Get average position/time of last x mouse events */
          priv->last_motion ++;
          x_origin = y_origin = 0;
          motion_time = (GTimeVal){ 0, 0 };
          for (i = 0; i < priv->last_motion; i++)
            {
              ChamplainFingerScrollMotion *motion =
                &g_array_index (priv->motion_buffer, ChamplainFingerScrollMotion, i);

              /* FIXME: This doesn't guard against overflows - Should
               *        either fix that, or calculate the correct maximum
               *        value for the buffer size
               */

              x_origin += motion->x;
              y_origin += motion->y;
              motion_time.tv_sec += motion->time.tv_sec;
              motion_time.tv_usec += motion->time.tv_usec;
            }
          x_origin /= priv->last_motion;
          y_origin /= priv->last_motion;
          motion_time.tv_sec /= priv->last_motion;
          motion_time.tv_usec /= priv->last_motion;

          if (motion_time.tv_sec == release_time.tv_sec)
            time_diff = release_time.tv_usec - motion_time.tv_usec;
          else
            time_diff = release_time.tv_usec +
                        (G_USEC_PER_SEC - motion_time.tv_usec);

          /* On a macbook that's running Ubuntu 9.04 sometimes 'time_diff' is 0
             and this causes a division by 0 when computing 'frac'. This check
             avoids this error.
           */
          if (time_diff != 0)
            {
              /* Work out the fraction of 1/60th of a second that has elapsed */
              frac = (time_diff/1000.0) / (1000.0/60.0);

              /* See how many units to move in 1/60th of a second */
              priv->dx = (x_origin - x) / frac;
              priv->dy = (y_origin - y) / frac;

              /* Get adjustments to do step-increment snapping */
              champlain_viewport_get_adjustments (CHAMPLAIN_VIEWPORT (priv->child),
                                               &hadjust,
                                               &vadjust);

              if (ABS(priv->dx) > 1 ||
                  ABS(priv->dy) > 1)
                {
                  gdouble value, lower, step_increment, d, a, x, y, n;

                  /* TODO: Convert this all to fixed point? */

                  /* We want n, where x / y    n < z,
                   * x = Distance to move per frame
                   * y = Deceleration rate
                   * z = maximum distance from target
                   *
                   * Rearrange to n = log (x / z) / log (y)
                   * To simplify, z = 1, so n = log (x) / log (y)
                   *
                   * As z = 1, this will cause stops to be slightly abrupt -
                   * add a constant 15 frames to compensate.
                   */
                  x = MAX(ABS(priv->dx), ABS(priv->dy));
                  y = priv->decel_rate;
                  n = logf (x) / logf (y) + 15.0;

                  /* Now we have n, adjust dx/dy so that we finish on a step
                   * boundary.
                   *
                   * Distance moved, using the above variable names:
                   *
                   * d = x + x/y + x/y    2 + ... + x/y    n
                   *
                   * Using geometric series,
                   *
                   * d = (1 - 1/y    (n+1))/(1 - 1/y)*x
                   *
                   * Let a = (1 - 1/y    (n+1))/(1 - 1/y),
                   *
                   * d = a * x
                   *
                   * Find d and find its nearest page boundary, then solve for x
                   *
                   * x = d / a
                   */

                  /* Get adjustments, work out y    n */
                  a = (1.0 - 1.0 / pow (y, n + 1)) / (1.0 - 1.0 / y);

                  /* Solving for dx */
                  d = a * priv->dx;
                  champlain_adjustment_get_values (hadjust, &value, &lower, NULL,
                                              &step_increment, NULL, NULL);
                  d = ((rint (((value + d) - lower) / step_increment) *
                        step_increment) + lower) - value;
                  priv->dx = (d / a);

                  /* Solving for dy */
                  d = a * (priv->dy);
                  champlain_adjustment_get_values (vadjust, &value, &lower, NULL,
                                              &step_increment, NULL, NULL);
                  d = ((rint (((value + d) - lower) / step_increment) *
                        step_increment) + lower) - value;
                  priv->dy = (d / a);

                  priv->deceleration_timeline = clutter_timeline_new ((n / 60) * 1000.0);
                }
              else
                {
                  gdouble value, lower, step_increment, d, a, y;

                  /* Start a short effects timeline to snap to the nearest step
                   * boundary (see equations above)
                   */
                  y = priv->decel_rate;
                  a = (1.0 - 1.0 / pow (y, 4 + 1)) / (1.0 - 1.0 / y);

                  champlain_adjustment_get_values (hadjust, &value, &lower, NULL,
                                              &step_increment, NULL, NULL);
                  d = ((rint ((value - lower) / step_increment) *
                        step_increment) + lower) - value;
                  priv->dx = (d / a);

                  champlain_adjustment_get_values (vadjust, &value, &lower, NULL,
                                              &step_increment, NULL, NULL);
                  d = ((rint ((value - lower) / step_increment) *
                        step_increment) + lower) - value;
                  priv->dy = (d / a);

                  priv->deceleration_timeline = clutter_timeline_new (250);
                }

              g_signal_connect (priv->deceleration_timeline, "new_frame",
                                G_CALLBACK (deceleration_new_frame_cb), scroll);
              g_signal_connect (priv->deceleration_timeline, "completed",
                                G_CALLBACK (deceleration_completed_cb), scroll);
              clutter_timeline_start (priv->deceleration_timeline);
              decelerating = TRUE;
            }
        }
    }

  /* Reset motion event buffer */
  if (priv->last_motion <= 1)
      moved = FALSE;
  priv->last_motion = 0;

  if (!decelerating)
    {
      clamp_adjustments (scroll);
      g_signal_emit_by_name (scroll, "panning-completed", NULL);
    }

  /* Pass through events to children.
   * FIXME: this probably breaks click-count.
   */
  if (!moved)
    clutter_event_put ((ClutterEvent *)event);

  return moved;
}

static gboolean
after_event_cb (ChamplainFingerScroll *scroll)
{
  /* Check the pointer grab - if something else has grabbed it - for example,
   * a scroll-bar or some such, don't do our funky stuff.
   */
  if (clutter_get_pointer_grab () != CLUTTER_ACTOR (scroll))
    {
      g_signal_handlers_disconnect_by_func (scroll,
                                            motion_event_cb,
                                            scroll);
      g_signal_handlers_disconnect_by_func (scroll,
                                            button_release_event_cb,
                                            scroll);
    }

  return FALSE;
}

static gboolean
captured_event_cb (ClutterActor     *actor,
                   ClutterEvent     *event,
                   ChamplainFingerScroll *scroll)
{
  ChamplainFingerScrollPrivate *priv = scroll->priv;

  if (event->type == CLUTTER_BUTTON_PRESS)
    {
      ChamplainFingerScrollMotion *motion;
      ClutterButtonEvent *bevent = (ClutterButtonEvent *)event;

      if (bevent->source != actor)
        return FALSE;

      /* Reset motion buffer */
      priv->last_motion = 0;
      motion = &g_array_index (priv->motion_buffer, ChamplainFingerScrollMotion, 0);

      if ((bevent->button == 1) &&
          (clutter_actor_transform_stage_point (actor,
                                           bevent->x,
                                           bevent->y,
                                           &motion->x, &motion->y)))
        {
          g_get_current_time (&motion->time);

          if (priv->deceleration_timeline)
            {
              clutter_timeline_stop (priv->deceleration_timeline);
              g_object_unref (priv->deceleration_timeline);
              priv->deceleration_timeline = NULL;
            }

          clutter_grab_pointer (actor);

          /* Add a high priority idle to check the grab after the event
           * emission is finished.
           */
          g_idle_add_full (G_PRIORITY_HIGH_IDLE,
                           (GSourceFunc)after_event_cb,
                           scroll,
                           NULL);

          g_signal_connect (actor,
                            "motion-event",
                            G_CALLBACK (motion_event_cb),
                            scroll);
          g_signal_connect (actor,
                            "button-release-event",
                            G_CALLBACK (button_release_event_cb),
                            scroll);
        }
    }

  return FALSE;
}

static void
champlain_finger_scroll_init (ChamplainFingerScroll *self)
{
  ChamplainFingerScrollPrivate *priv = self->priv = FINGER_SCROLL_PRIVATE (self);

  priv->motion_buffer = g_array_sized_new (FALSE, TRUE,
                                           sizeof (ChamplainFingerScrollMotion), 3);
  g_array_set_size (priv->motion_buffer, 3);
  priv->decel_rate = 1.1f;
  priv->child = NULL;

  clutter_actor_set_reactive (CLUTTER_ACTOR (self), TRUE);
  g_signal_connect (CLUTTER_ACTOR (self),
                    "captured-event",
                    G_CALLBACK (captured_event_cb),
                    self);


}

ClutterActor *
champlain_finger_scroll_new (gboolean kinetic)
{
  return CLUTTER_ACTOR (g_object_new (CHAMPLAIN_TYPE_FINGER_SCROLL,
                                      "mode", kinetic, NULL));
}

void
champlain_finger_scroll_stop (ChamplainFingerScroll *scroll)
{
  ChamplainFingerScrollPrivate *priv;

  g_return_if_fail (CHAMPLAIN_IS_FINGER_SCROLL (scroll));

  priv = scroll->priv;

  if (priv->deceleration_timeline)
    {
      clutter_timeline_stop (priv->deceleration_timeline);
      g_object_unref (priv->deceleration_timeline);
      priv->deceleration_timeline = NULL;
    }
}
