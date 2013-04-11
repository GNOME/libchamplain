/*
 * Copyright (C) 2011-2012 Jiri Techet <techet@gmail.com>
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
 * @short_description: #ChamplainCustomMarker A marker implementing the
 * #ClutterContainer interface.
 *
 * A marker implementing the #ClutterContainer interface. You can insert
 * your custom actors into the container. Don't forget to set the anchor
 * position in the marker using #clutter_actor_set_anchor_point.
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
  ClutterContainer *dummy;
};

static void clutter_container_iface_init (ClutterContainerIface *iface);


G_DEFINE_TYPE_WITH_CODE (ChamplainCustomMarker, champlain_custom_marker, CHAMPLAIN_TYPE_MARKER,
    G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER, clutter_container_iface_init));


#define GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CHAMPLAIN_TYPE_CUSTOM_MARKER, ChamplainCustomMarkerPrivate))


static void
add_actor (ClutterContainer *container,
    ClutterActor *actor)
{
  clutter_actor_add_child (CLUTTER_ACTOR (container), actor);
}


static void
remove_actor (ClutterContainer *container,
    ClutterActor *actor)
{
  clutter_actor_remove_child (CLUTTER_ACTOR (container), actor);
}


static void
foreach_actor (ClutterContainer *container,
    ClutterCallback callback,
    gpointer user_data)
{
  ClutterActor *child;

  for (child = clutter_actor_get_first_child (CLUTTER_ACTOR (container)); 
       child != NULL; 
       child = clutter_actor_get_next_sibling (child))
    {
      callback (child, user_data);
    }
}


static void
raise_actor (ClutterContainer *container,
    ClutterActor *actor,
    ClutterActor *sibling)
{
  clutter_actor_set_child_above_sibling (CLUTTER_ACTOR (container), actor, sibling);
}


static void
lower_actor (ClutterContainer *container,
    ClutterActor *actor,
    ClutterActor *sibling)
{
  clutter_actor_set_child_below_sibling (CLUTTER_ACTOR (container), actor, sibling);
}


static void
sort_depth_order (ClutterContainer *container)
{
  /* NOOP */
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
champlain_custom_marker_dispose (GObject *object)
{
/*  ChamplainCustomMarkerPrivate *priv = CHAMPLAIN_CUSTOM_MARKER (object)->priv; */

  G_OBJECT_CLASS (champlain_custom_marker_parent_class)->dispose (object);
}


static void
champlain_custom_marker_finalize (GObject *object)
{
/*  ChamplainCustomMarkerPrivate *priv = CHAMPLAIN_CUSTOM_MARKER (object)->priv; */

  G_OBJECT_CLASS (champlain_custom_marker_parent_class)->finalize (object);
}


static void
champlain_custom_marker_class_init (ChamplainCustomMarkerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainCustomMarkerPrivate));

  object_class->finalize = champlain_custom_marker_finalize;
  object_class->dispose = champlain_custom_marker_dispose;
}


static void
champlain_custom_marker_init (ChamplainCustomMarker *custom_marker)
{
  ChamplainCustomMarkerPrivate *priv = GET_PRIVATE (custom_marker);

  custom_marker->priv = priv;
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
