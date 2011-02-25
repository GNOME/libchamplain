/* champlain-viewport.c: Viewport actor
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <clutter/clutter.h>

#include "champlain-viewport.h"
#include "champlain-private.h"


G_DEFINE_TYPE (ChamplainViewport, champlain_viewport, CLUTTER_TYPE_ACTOR)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_VIEWPORT, \
  ChamplainViewportPrivate))

struct _ChamplainViewportPrivate
{
  gfloat x;
  gfloat y;

  ChamplainAdjustment *hadjustment;
  ChamplainAdjustment *vadjustment;

  gboolean sync_adjustments;
  ClutterActor *child;
  ClutterActor *content_group;
};

enum
{
  PROP_0,

  PROP_X_ORIGIN,
  PROP_Y_ORIGIN,
  PROP_HADJUST,
  PROP_VADJUST,
  PROP_SYNC_ADJUST,
};

static void
champlain_viewport_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  ChamplainAdjustment *adjustment;

  ChamplainViewportPrivate *priv = CHAMPLAIN_VIEWPORT (object)->priv;

  switch (prop_id)
    {
    case PROP_X_ORIGIN:
      g_value_set_int (value, priv->x);
      break;

    case PROP_Y_ORIGIN:
      g_value_set_int (value, priv->y);
      break;

    case PROP_HADJUST :
      champlain_viewport_get_adjustments (CHAMPLAIN_VIEWPORT (object), &adjustment, NULL);
      g_value_set_object (value, adjustment);
      break;

    case PROP_VADJUST :
      champlain_viewport_get_adjustments (CHAMPLAIN_VIEWPORT (object), NULL, &adjustment);
      g_value_set_object (value, adjustment);
      break;

    case PROP_SYNC_ADJUST :
      g_value_set_boolean (value, priv->sync_adjustments);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
champlain_viewport_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  ChamplainViewport *viewport = CHAMPLAIN_VIEWPORT (object);
  ChamplainViewportPrivate *priv = viewport->priv;

  switch (prop_id)
    {
    case PROP_X_ORIGIN:
      champlain_viewport_set_origin (viewport,
                                 g_value_get_int (value),
                                 priv->y);
      break;

    case PROP_Y_ORIGIN:
      champlain_viewport_set_origin (viewport,
                                 priv->x,
                                 g_value_get_int (value));
      break;

    case PROP_HADJUST :
      champlain_viewport_set_adjustments (CHAMPLAIN_VIEWPORT (object),
                                  g_value_get_object (value),
                                  priv->vadjustment);
      break;

    case PROP_VADJUST :
      champlain_viewport_set_adjustments (CHAMPLAIN_VIEWPORT (object),
                                  priv->hadjustment,
                                  g_value_get_object (value));
      break;

    case PROP_SYNC_ADJUST :
      priv->sync_adjustments = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

void
champlain_viewport_stop (ChamplainViewport *viewport)
{
  ChamplainViewportPrivate *priv = CHAMPLAIN_VIEWPORT (viewport)->priv;

  champlain_adjustment_interpolate_stop (priv->hadjustment);
  champlain_adjustment_interpolate_stop (priv->vadjustment);
}

static void
champlain_viewport_dispose (GObject *gobject)
{
  ChamplainViewportPrivate *priv = CHAMPLAIN_VIEWPORT (gobject)->priv;

  if (priv->hadjustment)
    {
      champlain_adjustment_interpolate_stop (priv->hadjustment);
      g_object_unref (priv->hadjustment);
      priv->hadjustment = NULL;
    }

  if (priv->vadjustment)
    {
      champlain_adjustment_interpolate_stop (priv->vadjustment);
      g_object_unref (priv->vadjustment);
      priv->vadjustment = NULL;
    }

  if (priv->content_group)
    {
      clutter_actor_destroy (priv->content_group);
      priv->content_group = NULL;
    }

  G_OBJECT_CLASS (champlain_viewport_parent_class)->dispose (gobject);
}


static void
paint (ClutterActor *self)
{
  ChamplainViewportPrivate *priv = GET_PRIVATE (self);
  
  clutter_actor_paint (priv->content_group);
}


static void
pick (ClutterActor *self, 
    const ClutterColor *color)
{
  ChamplainViewportPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_viewport_parent_class)->pick (self, color);

  clutter_actor_paint (CLUTTER_ACTOR (priv->content_group));
}


static void
get_preferred_width (ClutterActor *self,
    gfloat for_height,
    gfloat *min_width_p,
    gfloat *natural_width_p)
{
  ChamplainViewportPrivate *priv = GET_PRIVATE (self);

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
  ChamplainViewportPrivate *priv = GET_PRIVATE (self);

  clutter_actor_get_preferred_height (CLUTTER_ACTOR (priv->content_group),
      for_width,
      min_height_p,
      natural_height_p);
}


static void
map (ClutterActor *self)
{
  ChamplainViewportPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_viewport_parent_class)->map (self);

  clutter_actor_map (CLUTTER_ACTOR (priv->content_group));
}


static void
unmap (ClutterActor *self)
{
  ChamplainViewportPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_viewport_parent_class)->unmap (self);

  clutter_actor_unmap (CLUTTER_ACTOR (priv->content_group));
}


static void
allocate (ClutterActor          *self,
                        const ClutterActorBox *box,
                        ClutterAllocationFlags flags)
{
  ClutterActorBox child_box;
  CoglFixed prev_value;

  ChamplainViewportPrivate *priv = CHAMPLAIN_VIEWPORT (self)->priv;

  /* Chain up */
  CLUTTER_ACTOR_CLASS (champlain_viewport_parent_class)->
    allocate (self, box, flags);

  /* Refresh adjustments */
  if (priv->sync_adjustments)
    {
      if (priv->hadjustment)
        {
          g_object_set (G_OBJECT (priv->hadjustment),
                       "lower", 0.0,
                       "upper", (box->x2 - box->x1),
                       NULL);

          /* Make sure value is clamped */
          prev_value = champlain_adjustment_get_value (priv->hadjustment);
          champlain_adjustment_set_value (priv->hadjustment, prev_value);
        }

      if (priv->vadjustment)
        {
          g_object_set (G_OBJECT (priv->vadjustment),
                       "lower", 0.0,
                       "upper", (box->y2 - box->y1),
                       NULL);

          prev_value = champlain_adjustment_get_value (priv->vadjustment);
          champlain_adjustment_set_value (priv->vadjustment, prev_value);
        }
    }

  child_box.x1 = 0;
  child_box.x2 = box->x2 - box->x1;
  child_box.y1 = 0;
  child_box.y2 = box->y2 - box->y1;

  clutter_actor_allocate (CLUTTER_ACTOR (priv->content_group), &child_box, flags);
}

static void
champlain_viewport_class_init (ChamplainViewportClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainViewportPrivate));

  gobject_class->get_property = champlain_viewport_get_property;
  gobject_class->set_property = champlain_viewport_set_property;
  gobject_class->dispose = champlain_viewport_dispose;

  actor_class->get_preferred_width = get_preferred_width;
  actor_class->get_preferred_height = get_preferred_height;
  actor_class->allocate = allocate;
  actor_class->paint = paint;
  actor_class->pick = pick;
  actor_class->map = map;
  actor_class->unmap = unmap;

  g_object_class_install_property (gobject_class,
                                   PROP_X_ORIGIN,
                                   g_param_spec_int ("x-origin",
                                                     "X Origin",
                                                     "Origin's X coordinate in pixels",
                                                     -G_MAXINT, G_MAXINT,
                                                     0,
                                                     G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_Y_ORIGIN,
                                   g_param_spec_int ("y-origin",
                                                     "Y Origin",
                                                     "Origin's Y coordinate in pixels",
                                                     -G_MAXINT, G_MAXINT,
                                                     0,
                                                     G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_SYNC_ADJUST,
                                   g_param_spec_boolean ("sync-adjustments",
                                                         "Synchronise "
                                                         "adjustments",
                                                         "Whether to "
                                                         "synchronise "
                                                         "adjustments with "
                                                         "viewport size",
                                                         TRUE,
                                                         G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_HADJUST,
                                   g_param_spec_object ("hadjustment",
                                                        "ChamplainAdjustment",
                                                        "Horizontal adjustment",
                                                        CHAMPLAIN_TYPE_ADJUSTMENT,
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_VADJUST,
                                   g_param_spec_object ("vadjustment",
                                                        "ChamplainAdjustment",
                                                        "Vertical adjustment",
                                                        CHAMPLAIN_TYPE_ADJUSTMENT,
                                                        G_PARAM_READWRITE));

}

static void
hadjustment_value_notify_cb (ChamplainAdjustment *adjustment,
                             GParamSpec     *pspec,
                             ChamplainViewport   *viewport)
{
  ChamplainViewportPrivate *priv = viewport->priv;
  gdouble value;

  value = champlain_adjustment_get_value (adjustment);

  champlain_viewport_set_origin (viewport,
                             value,
                             priv->y);
}

static void
vadjustment_value_notify_cb (ChamplainAdjustment *adjustment, GParamSpec *arg1,
                             ChamplainViewport *viewport)
{
  ChamplainViewportPrivate *priv = viewport->priv;
  gdouble value;

  value = champlain_adjustment_get_value (adjustment);

  champlain_viewport_set_origin (viewport,
                             priv->x,
                             value);
}

void
champlain_viewport_set_adjustments (ChamplainViewport *viewport,
                            ChamplainAdjustment *hadjustment,
                            ChamplainAdjustment *vadjustment)
{
  ChamplainViewportPrivate *priv = CHAMPLAIN_VIEWPORT (viewport)->priv;

  if (hadjustment != priv->hadjustment)
    {
      if (priv->hadjustment)
        {
          g_signal_handlers_disconnect_by_func (priv->hadjustment,
                                                hadjustment_value_notify_cb,
                                                viewport);
          g_object_unref (priv->hadjustment);
        }

      if (hadjustment)
        {
          g_object_ref (hadjustment);
          g_signal_connect (hadjustment, "notify::value",
                            G_CALLBACK (hadjustment_value_notify_cb),
                            viewport);
        }

      priv->hadjustment = hadjustment;
    }

  if (vadjustment != priv->vadjustment)
    {
      if (priv->vadjustment)
        {
          g_signal_handlers_disconnect_by_func (priv->vadjustment,
                                                vadjustment_value_notify_cb,
                                                viewport);
          g_object_unref (priv->vadjustment);
        }

      if (vadjustment)
        {
          g_object_ref (vadjustment);
          g_signal_connect (vadjustment, "notify::value",
                            G_CALLBACK (vadjustment_value_notify_cb),
                            viewport);
        }

      priv->vadjustment = vadjustment;
    }
}

void
champlain_viewport_get_adjustments (ChamplainViewport *viewport,
                            ChamplainAdjustment **hadjustment,
                            ChamplainAdjustment **vadjustment)
{
  ChamplainViewportPrivate *priv;

  g_return_if_fail (CHAMPLAIN_IS_VIEWPORT (viewport));

  priv = ((ChamplainViewport *)viewport)->priv;

  if (hadjustment)
    {
      if (priv->hadjustment)
        *hadjustment = priv->hadjustment;
      else
        {
          ChamplainAdjustment *adjustment;
          guint width, stage_width, increment;

          width = clutter_actor_get_width (CLUTTER_ACTOR(viewport));
          stage_width = clutter_actor_get_width (clutter_stage_get_default ());
          increment = MAX (1, MIN(stage_width, width));

          adjustment = champlain_adjustment_new (priv->x,
                                            0,
                                            width,
                                            1,
                                            increment,
                                            increment);
          champlain_viewport_set_adjustments (viewport,
                                      adjustment,
                                      priv->vadjustment);
          *hadjustment = adjustment;
        }
    }

  if (vadjustment)
    {
      if (priv->vadjustment)
        *vadjustment = priv->vadjustment;
      else
        {
          ChamplainAdjustment *adjustment;
          guint height, stage_height, increment;

          height = clutter_actor_get_height (CLUTTER_ACTOR(viewport));
          stage_height = clutter_actor_get_height (clutter_stage_get_default ());
          increment = MAX (1, MIN(stage_height, height));

          adjustment = champlain_adjustment_new (priv->y,
                                            0,
                                            height,
                                            1,
                                            increment,
                                            increment);
          champlain_viewport_set_adjustments (viewport,
                                      priv->hadjustment,
                                      adjustment);
          *vadjustment = adjustment;
        }
    }
}


static void
clip_notify_cb (ClutterActor *actor,
                GParamSpec   *pspec,
                ChamplainViewport *self)
{
  gfloat width, height;
  ChamplainViewportPrivate *priv = self->priv;

  if (!priv->sync_adjustments)
    return;

  if (!clutter_actor_has_clip (actor))
    {
      if (priv->hadjustment)
        g_object_set (priv->hadjustment, "page-size", (gdouble)1.0, NULL);
      if (priv->vadjustment)
        g_object_set (priv->vadjustment, "page-size", (gdouble)1.0, NULL);
      return;
    }

  clutter_actor_get_clip (actor, NULL, NULL, &width, &height);

  if (priv->hadjustment)
    g_object_set (priv->hadjustment, "page-size", (gdouble)width, NULL);

  if (priv->vadjustment)
    g_object_set (priv->vadjustment, "page-size", (gdouble)height, NULL);
}

static void
champlain_viewport_init (ChamplainViewport *self)
{
  self->priv = GET_PRIVATE (self);

  self->priv->sync_adjustments = TRUE;

  self->priv->child = NULL;
  self->priv->content_group = clutter_group_new ();
  clutter_actor_set_parent (CLUTTER_ACTOR (self->priv->content_group), CLUTTER_ACTOR (self));
  clutter_actor_queue_relayout (CLUTTER_ACTOR (self));

  g_signal_connect (self, "notify::clip",
                    G_CALLBACK (clip_notify_cb), self);
}

ClutterActor *
champlain_viewport_new (void)
{
  return g_object_new (CHAMPLAIN_TYPE_VIEWPORT, NULL);
}

void
champlain_viewport_set_origin (ChamplainViewport *viewport,
                          float x,
                          float y)
{
  ChamplainViewportPrivate *priv;

  g_return_if_fail (CHAMPLAIN_IS_VIEWPORT (viewport));

  priv = viewport->priv;

  g_object_freeze_notify (G_OBJECT (viewport));

  if (x != priv->x)
    {
      priv->x = x;
      g_object_notify (G_OBJECT (viewport), "x-origin");

      if (priv->hadjustment)
        champlain_adjustment_set_value (priv->hadjustment,
                                    x);
    }

  if (y != priv->y)
    {
      priv->y = y;
      g_object_notify (G_OBJECT (viewport), "y-origin");

      if (priv->vadjustment)
        champlain_adjustment_set_value (priv->vadjustment,
                                    y);
    }

  g_object_thaw_notify (G_OBJECT (viewport));
  
  if (priv->child)
    clutter_actor_set_position (priv->child, -x, -y);

  clutter_actor_queue_redraw (CLUTTER_ACTOR (viewport));
}

void
champlain_viewport_get_origin (ChamplainViewport *viewport,
                          float *x,
                          float *y)
{
  ChamplainViewportPrivate *priv;

  g_return_if_fail (CHAMPLAIN_IS_VIEWPORT (viewport));

  priv = viewport->priv;

  if (x)
    *x = priv->x;

  if (y)
    *y = priv->y;
}


void 
champlain_viewport_set_child (ChamplainViewport *viewport, ClutterActor *child)
{
  ChamplainViewportPrivate *priv = viewport->priv;
  
  if (priv->child)
    clutter_container_remove_actor (CLUTTER_CONTAINER (priv->content_group), priv->child);
    
  priv->child = child;
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->content_group), child);  
}
