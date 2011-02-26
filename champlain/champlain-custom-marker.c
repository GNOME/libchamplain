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
 * SECTION:champlain-custom-marker
 * @short_description: #ChamplainCustomMarker is a marker implementing the
 * #ClutterContainer interface. 
 * 
 * #ChamplainCustomMarker is a marker implementing the #ClutterContainer 
 * interface. You can insert your custom actors into the container. Don't forget 
 * to set the anchor position in the marker using #clutter_actor_set_anchor_point.
 */

#include "config.h"

#include "champlain.h"
#include "champlain-defines.h"
#include "champlain-marshal.h"
#include "champlain-private.h"

#include <clutter/clutter.h>
#include <glib.h>
#include <glib-object.h>


enum
{
  /* normal signals */
  LAST_SIGNAL
};

enum
{
  PROP_0,
};

/* static guint champlain_custom_marker_signals[LAST_SIGNAL] = { 0, }; */

struct _ChamplainCustomMarkerPrivate
{
  ClutterContainer *content_group;
};

static void
clutter_container_iface_init (ClutterContainerIface *iface);


G_DEFINE_TYPE_WITH_CODE (ChamplainCustomMarker, champlain_custom_marker, CHAMPLAIN_TYPE_MARKER,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                clutter_container_iface_init));


#define GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CHAMPLAIN_TYPE_CUSTOM_MARKER, ChamplainCustomMarkerPrivate))



static void
add_actor (ClutterContainer *container,
    ClutterActor *actor)
{
  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (container);
  
  clutter_container_add_actor (priv->content_group, actor);  
}

static void
remove_actor (ClutterContainer *container,
    ClutterActor *actor)
{
  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (container);
  
  clutter_container_remove_actor (priv->content_group, actor);  
}

static void
foreach_actor (ClutterContainer *container,
    ClutterCallback callback,
    gpointer user_data)
{
  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (container);
  
  clutter_container_foreach (priv->content_group, callback, user_data);  
}

static void
raise_actor (ClutterContainer *container,
    ClutterActor *actor,
    ClutterActor *sibling)
{
  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (container);
  
  clutter_container_raise_child (priv->content_group, actor, sibling);  
  clutter_actor_queue_redraw (CLUTTER_ACTOR (container));
}

static void
lower_actor (ClutterContainer *container,
    ClutterActor *actor,
    ClutterActor *sibling)
{
  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (container);
  
  clutter_container_lower_child (priv->content_group, actor, sibling);  
  clutter_actor_queue_redraw (CLUTTER_ACTOR (container));
}

static void
sort_depth_order (ClutterContainer *container)
{
  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (container);
  
  clutter_container_sort_depth_order (priv->content_group);  
  clutter_actor_queue_redraw (CLUTTER_ACTOR (container));
}

static void
clutter_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = add_actor;
  iface->remove = remove_actor;
  iface->foreach = foreach_actor;
  iface->raise = raise_actor;
  iface->lower = lower_actor;
  iface->sort_depth_order = sort_depth_order;
}


static void
paint (ClutterActor *actor)
{
  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (actor);

  clutter_actor_paint (CLUTTER_ACTOR (priv->content_group));
}

static void
pick (ClutterActor       *actor,
                         const ClutterColor *color)
{
  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (actor);

  CLUTTER_ACTOR_CLASS (champlain_custom_marker_parent_class)->pick (actor, color);

  clutter_actor_paint (CLUTTER_ACTOR (priv->content_group));
}

static void
get_preferred_width (ClutterActor *actor,
                                        gfloat        for_height,
                                        gfloat       *min_width,
                                        gfloat       *natural_width)
{
  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (actor);

  clutter_actor_get_preferred_width (CLUTTER_ACTOR (priv->content_group),
      for_height,
      min_width,
      natural_width);
}

static void
get_preferred_height (ClutterActor *actor,
                                         gfloat        for_width,
                                         gfloat       *min_height,
                                         gfloat       *natural_height)
{
  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (actor);

  clutter_actor_get_preferred_height (CLUTTER_ACTOR (priv->content_group),
      for_width,
      min_height,
      natural_height);
}

static void
allocate (ClutterActor *actor,
    const ClutterActorBox *box,
    ClutterAllocationFlags flags)
{
  ClutterActorBox child_box;

  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (actor);

  CLUTTER_ACTOR_CLASS (champlain_custom_marker_parent_class)->allocate (actor, box, flags);

  child_box.x1 = 0;
  child_box.x2 = box->x2 - box->x1;
  child_box.y1 = 0;
  child_box.y2 = box->y2 - box->y1;

  clutter_actor_allocate (CLUTTER_ACTOR (priv->content_group), &child_box, flags);
}


static void
map (ClutterActor *self)
{
  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_custom_marker_parent_class)->map (self);

  clutter_actor_map (CLUTTER_ACTOR (priv->content_group));
}


static void
unmap (ClutterActor *self)
{
  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (self);

  CLUTTER_ACTOR_CLASS (champlain_custom_marker_parent_class)->unmap (self);

  clutter_actor_unmap (CLUTTER_ACTOR (priv->content_group));
}


static void
champlain_custom_marker_dispose (GObject *object)
{
  ChamplainCustomMarkerPrivate *priv = CHAMPLAIN_CUSTOM_MARKER (object)->priv;
  
  if (priv->content_group)
    {
      clutter_actor_unparent (CLUTTER_ACTOR (priv->content_group));
      priv->content_group = NULL;
    }

  G_OBJECT_CLASS (champlain_custom_marker_parent_class)->dispose (object);
}


static void
champlain_custom_marker_finalize (GObject *object)
{
//  ChamplainCustomMarkerPrivate *priv = CHAMPLAIN_CUSTOM_MARKER (object)->priv;

  G_OBJECT_CLASS (champlain_custom_marker_parent_class)->finalize (object);
}


static void
champlain_custom_marker_class_init (ChamplainCustomMarkerClass *klass)
{
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  g_type_class_add_private (klass, sizeof (ChamplainCustomMarkerPrivate));

  object_class->finalize = champlain_custom_marker_finalize;
  object_class->dispose = champlain_custom_marker_dispose;

  actor_class->get_preferred_width = get_preferred_width;
  actor_class->get_preferred_height = get_preferred_height;
  actor_class->allocate = allocate;
  actor_class->paint = paint;
  actor_class->pick = pick;
  actor_class->map = map;
  actor_class->unmap = unmap;
}


static void
champlain_custom_marker_init (ChamplainCustomMarker *custom_marker)
{
  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (custom_marker);

  custom_marker->priv = priv;
  priv->content_group = CLUTTER_CONTAINER (clutter_group_new ());
  clutter_actor_set_parent (CLUTTER_ACTOR (priv->content_group), CLUTTER_ACTOR (custom_marker));
  clutter_actor_queue_relayout (CLUTTER_ACTOR (custom_marker));
}


/**
 * champlain_custom_marker_new:
 * 
 * Creates an instance of #ChamplainCustomMarker.
 *
 * Returns: a new #ChamplainCustomMarker.
 *
 * Since: 0.10
 */
ClutterActor *
champlain_custom_marker_new (void)
{
  return CLUTTER_ACTOR (g_object_new (CHAMPLAIN_TYPE_CUSTOM_MARKER, NULL));
}
