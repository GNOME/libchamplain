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

#include "config.h"

#include "champlainlayer.h"
#include "champlaindefines.h"

#include <clutter/clutter.h>
#include <glib.h>

/* This callback serves to keep the markers ordered by their latitude.
 * Markers that are up north on the map should be lowered in the list so that
 * they are drawn the first. This is to make the illusion of a semi-3d plane
 * where the most north you are, the farther you are.
 */
static void
layer_add_cb (ClutterGroup *layer, ClutterActor *marker, gpointer data)
{

  GList* markers = clutter_container_get_children (CLUTTER_CONTAINER(layer));
  gint size, i;
  gdouble y, tmp_y, low_y;
  ChamplainMarker *lowest = NULL;

  size = g_list_length (markers);
  g_object_get(G_OBJECT(marker), "latitude", &y, NULL);
  y = 90 - y;
  low_y = G_MAXDOUBLE;

  for (i = 0; i < size; i++)
    {
      ChamplainMarker *prev_marker = (ChamplainMarker*) g_list_nth_data (markers, i);
      g_object_get(G_OBJECT(prev_marker), "latitude", &tmp_y, NULL);
      tmp_y = 90 - tmp_y;

      if (prev_marker == (ChamplainMarker*) marker)
        continue;

      if (y < tmp_y && tmp_y < low_y)
        {
          lowest = prev_marker;
          low_y = tmp_y;
        }
    }

  if (lowest)
    clutter_container_lower_child(CLUTTER_CONTAINER(layer), CLUTTER_ACTOR(marker), CLUTTER_ACTOR(lowest));
}


/**
 * champlain_layer_new:
 *
 * Returns a new #ChamplainLayer ready to be used as a #ClutterContainer for the markers.
 *
 * Since: 0.2.2
 */
ChamplainLayer *
champlain_layer_new ()
{
  ClutterActor *layer;

  layer = clutter_group_new();
  g_signal_connect_after(G_OBJECT(layer), "actor-added", G_CALLBACK(layer_add_cb), NULL);

  return layer;
}
