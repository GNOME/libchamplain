/* tidy-scroll-view.h: Container with scroll-bars
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

#include "tidy-scroll-view.h"
#include "tidy-marshal.h"
#include "tidy-scrollable.h"
#include <clutter/clutter.h>

static void clutter_container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (TidyScrollView, tidy_scroll_view, CLUTTER_TYPE_ACTOR,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init))

#define SCROLL_VIEW_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                TIDY_TYPE_SCROLL_VIEW, \
                                TidyScrollViewPrivate))

struct _TidyScrollViewPrivate
{
  ClutterActor   *child;
};

enum {
  PROP_0,
  PROP_CHILD,
};

static void
tidy_scroll_view_get_property (GObject *object, guint property_id,
                                 GValue *value, GParamSpec *pspec)
{
  TidyScrollViewPrivate *priv = ((TidyScrollView *)object)->priv;

  switch (property_id)
    {
    case PROP_CHILD :
      g_value_set_object (value, priv->child);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
tidy_scroll_view_set_property (GObject *object, guint property_id,
                                 const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
tidy_scroll_view_dispose (GObject *object)
{
  TidyScrollViewPrivate *priv = TIDY_SCROLL_VIEW (object)->priv;

  if (priv->child)
    clutter_container_remove_actor (CLUTTER_CONTAINER (object), priv->child);

  G_OBJECT_CLASS (tidy_scroll_view_parent_class)->dispose (object);
}

static void
tidy_scroll_view_finalize (GObject *object)
{
  G_OBJECT_CLASS (tidy_scroll_view_parent_class)->finalize (object);
}

static void
tidy_scroll_view_paint (ClutterActor *actor)
{
  TidyScrollViewPrivate *priv = TIDY_SCROLL_VIEW (actor)->priv;

  if (priv->child && CLUTTER_ACTOR_IS_VISIBLE (priv->child))
    clutter_actor_paint (priv->child);
}

static void
tidy_scroll_view_pick (ClutterActor *actor, const ClutterColor *color)
{
  /* Chain up so we get a bounding box pained (if we are reactive) */
  CLUTTER_ACTOR_CLASS (tidy_scroll_view_parent_class)->pick (actor, color);

  /* Trigger pick on children */
  tidy_scroll_view_paint (actor);
}

static void
tidy_scroll_view_get_preferred_width (ClutterActor *actor,
                                      gfloat   for_height,
                                      gfloat  *min_width_p,
                                      gfloat  *natural_width_p)
{

  TidyScrollViewPrivate *priv = TIDY_SCROLL_VIEW (actor)->priv;

  if (!priv->child)
    return;


  /* Our natural width is the natural width of the child */
  clutter_actor_get_preferred_width (priv->child,
                                     for_height,
                                     NULL,
                                     natural_width_p);

}

static void
tidy_scroll_view_get_preferred_height (ClutterActor *actor,
                                       gfloat   for_width,
                                       gfloat  *min_height_p,
                                       gfloat  *natural_height_p)
{

  TidyScrollViewPrivate *priv = TIDY_SCROLL_VIEW (actor)->priv;

  if (!priv->child)
    return;


  /* Our natural height is the natural height of the child */
  clutter_actor_get_preferred_height (priv->child,
                                      for_width,
                                      NULL,
                                      natural_height_p);
}

static void
tidy_scroll_view_allocate (ClutterActor          *actor,
                           const ClutterActorBox *box,
                           ClutterAllocationFlags flags)
{
  ClutterActorBox child_box;

  TidyScrollViewPrivate *priv = TIDY_SCROLL_VIEW (actor)->priv;

  /* Chain up */
  CLUTTER_ACTOR_CLASS (tidy_scroll_view_parent_class)->
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
tidy_scroll_view_class_init (TidyScrollViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TidyScrollViewPrivate));

  object_class->get_property = tidy_scroll_view_get_property;
  object_class->set_property = tidy_scroll_view_set_property;
  object_class->dispose= tidy_scroll_view_dispose;
  object_class->finalize = tidy_scroll_view_finalize;

  actor_class->paint = tidy_scroll_view_paint;
  actor_class->pick = tidy_scroll_view_pick;
  actor_class->get_preferred_width = tidy_scroll_view_get_preferred_width;
  actor_class->get_preferred_height = tidy_scroll_view_get_preferred_height;
  actor_class->allocate = tidy_scroll_view_allocate;

  g_object_class_install_property (object_class,
                                   PROP_CHILD,
                                   g_param_spec_object ("child",
                                                        "ClutterActor",
                                                        "Child actor",
                                                        CLUTTER_TYPE_ACTOR,
                                                        G_PARAM_READABLE));
}

static void
tidy_scroll_view_init (TidyScrollView *self)
{
  self->priv = SCROLL_VIEW_PRIVATE (self);
}

static void
tidy_scroll_view_add_actor (ClutterContainer *container,
                            ClutterActor     *actor)
{
  TidyScrollView *self = TIDY_SCROLL_VIEW (container);
  TidyScrollViewPrivate *priv = self->priv;

  if (priv->child)
    {
      g_warning ("Attempting to add an actor of type %s to "
                 "a TidyScrollView that already contains "
                 "an actor of type %s.",
                 g_type_name (G_OBJECT_TYPE (actor)),
                 g_type_name (G_OBJECT_TYPE (priv->child)));
    }
  else
    {
      if (TIDY_IS_SCROLLABLE(actor))
        {
          priv->child = actor;
          clutter_actor_set_parent (actor, CLUTTER_ACTOR (container));

          /* Notify that child has been set */
          g_signal_emit_by_name (container, "actor-added", priv->child);
          g_object_notify (G_OBJECT (container), "child");

          clutter_actor_queue_relayout (CLUTTER_ACTOR (container));
        }
      else
        {
          g_warning ("Attempting to add an actor to "
                     "a TidyScrollView, but the actor does "
                     "not implement TidyScrollable.");
        }
    }
}

static void
tidy_scroll_view_remove_actor (ClutterContainer *container,
                               ClutterActor     *actor)
{
  TidyScrollViewPrivate *priv = TIDY_SCROLL_VIEW (container)->priv;

  if (actor == priv->child)
    {
      g_object_ref (priv->child);


      clutter_actor_unparent (priv->child);

      g_signal_emit_by_name (container, "actor-removed", priv->child);

      g_object_unref (priv->child);
      priv->child = NULL;

      g_object_notify (G_OBJECT (container), "child");

      if (CLUTTER_ACTOR_IS_VISIBLE (container))
        clutter_actor_queue_relayout (CLUTTER_ACTOR (container));
    }
}

static void
tidy_scroll_view_foreach (ClutterContainer *container,
                          ClutterCallback   callback,
                          gpointer          callback_data)
{
  TidyScrollViewPrivate *priv = TIDY_SCROLL_VIEW (container)->priv;

  if (priv->child)
    callback (priv->child, callback_data);
}

static void
tidy_scroll_view_lower (ClutterContainer *container,
                        ClutterActor     *actor,
                        ClutterActor     *sibling)
{
  /* single child */
}

static void
tidy_scroll_view_raise (ClutterContainer *container,
                        ClutterActor     *actor,
                        ClutterActor     *sibling)
{
  /* single child */
}

static void
tidy_scroll_view_sort_depth_order (ClutterContainer *container)
{
  /* single child */
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = tidy_scroll_view_add_actor;
  iface->remove = tidy_scroll_view_remove_actor;
  iface->foreach = tidy_scroll_view_foreach;
  iface->lower = tidy_scroll_view_lower;
  iface->raise = tidy_scroll_view_raise;
  iface->sort_depth_order = tidy_scroll_view_sort_depth_order;
}

ClutterActor *
tidy_scroll_view_new (void)
{
  return CLUTTER_ACTOR (g_object_new (TIDY_TYPE_SCROLL_VIEW, NULL));
}

ClutterActor *
tidy_scroll_view_get_child (TidyScrollView *scroll)
{
  TidyScrollViewPrivate *priv;

  g_return_val_if_fail (TIDY_IS_SCROLL_VIEW (scroll), NULL);

  priv = scroll->priv;

  return priv->child;
}
