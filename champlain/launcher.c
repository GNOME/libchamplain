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

#include <config.h>

#include <champlain.h>

static ClutterActor*
create_marker_layer ()
{
  ClutterActor *layer, *marker;
  
  layer = champlain_layer_new();
  
  ClutterColor orange = { 0xf3, 0x94, 0x07, 0xbb };
  ClutterColor white = { 0xff, 0xff, 0xff, 0xff };
  marker = champlain_marker_new_with_label("Montréal", "Airmole 14", NULL, NULL);
  champlain_marker_set_position(CHAMPLAIN_MARKER(marker), 45.528178, -73.563788);
  clutter_container_add(CLUTTER_CONTAINER(layer), marker, NULL);
  
  marker = champlain_marker_new_with_label("New York", "Sans 25", &white, NULL);
  champlain_marker_set_position(CHAMPLAIN_MARKER(marker), 40.77, -73.98);
  clutter_container_add(CLUTTER_CONTAINER(layer), marker, NULL);
  
  marker = champlain_marker_new_with_label("Saint-Tite-des-Caps", "Serif 12", NULL, &orange);
  champlain_marker_set_position(CHAMPLAIN_MARKER(marker), 47.130885, -70.764141);
  clutter_container_add(CLUTTER_CONTAINER(layer), marker, NULL);
  
  clutter_actor_show(layer);
  return layer;
}

int
main (int argc, char *argv[])
{
  ClutterActor* actor, *layer, *stage;
  
  g_thread_init (NULL);
  clutter_init (&argc, &argv);
  
  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 800, 600);
  
  actor = champlain_view_new (CHAMPLAIN_VIEW_MODE_KINETIC);
  
  champlain_view_set_size (actor, 800, 600);
  
  g_object_set(G_OBJECT(actor), "zoom-level", 5, NULL);
  layer = create_marker_layer();
  champlain_view_add_layer(actor, layer);

  clutter_container_add_actor (CLUTTER_CONTAINER (stage), actor);
  champlain_view_center_on(CHAMPLAIN_VIEW(actor), 45.466, -73.75);
  
  clutter_actor_show (stage);
  clutter_main ();

  return 0;
}
