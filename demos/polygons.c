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

#include <champlain/champlain.h>

#define PADDING 10

static gboolean
zoom_in (G_GNUC_UNUSED ClutterActor *actor,
    G_GNUC_UNUSED ClutterButtonEvent *event,
    ChamplainView *view)
{
  champlain_view_zoom_in (view);
  return TRUE;
}


static gboolean
zoom_out (G_GNUC_UNUSED ClutterActor *actor,
    G_GNUC_UNUSED ClutterButtonEvent *event,
    ChamplainView *view)
{
  champlain_view_zoom_out (view);
  return TRUE;
}


static ClutterActor *
make_button (char *text)
{
  ClutterActor *button, *button_bg, *button_text;
  ClutterColor white = { 0xff, 0xff, 0xff, 0xff };
  ClutterColor black = { 0x00, 0x00, 0x00, 0xff };
  gfloat width, height;

  button = clutter_group_new ();

  button_bg = clutter_rectangle_new_with_color (&white);
  clutter_container_add_actor (CLUTTER_CONTAINER (button), button_bg);
  clutter_actor_set_opacity (button_bg, 0xcc);

  button_text = clutter_text_new_full ("Sans 10", text, &black);
  clutter_container_add_actor (CLUTTER_CONTAINER (button), button_text);
  clutter_actor_get_size (button_text, &width, &height);

  clutter_actor_set_size (button_bg, width + PADDING * 2, height + PADDING * 2);
  clutter_actor_set_position (button_bg, 0, 0);
  clutter_actor_set_position (button_text, PADDING, PADDING);

  return button;
}


static void
append_point (ChamplainPathLayer *layer, gdouble lon, gdouble lat)
{
  ChamplainCoordinate *coord;  
  
  coord = champlain_coordinate_new_full (lon, lat);
  champlain_path_layer_add_node (layer, CHAMPLAIN_LOCATION (coord));
}


int
main (int argc,
    char *argv[])
{
  ClutterActor *actor, *stage, *buttons, *button;
  ChamplainPathLayer *layer;
  gfloat width, total_width = 0;;

  g_thread_init (NULL);
  if (clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
    return 1;

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 800, 600);

  /* Create the map view */
  actor = champlain_view_new ();
  clutter_actor_set_size (CLUTTER_ACTOR (actor), 800, 600);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), actor);

  /* Create the buttons */
  buttons = clutter_group_new ();
  clutter_actor_set_position (buttons, PADDING, PADDING);

  button = make_button ("Zoom in");
  clutter_container_add_actor (CLUTTER_CONTAINER (buttons), button);
  clutter_actor_set_reactive (button, TRUE);
  clutter_actor_get_size (button, &width, NULL);
  total_width += width + PADDING;
  g_signal_connect (button, "button-release-event",
      G_CALLBACK (zoom_in),
      actor);

  button = make_button ("Zoom out");
  clutter_container_add_actor (CLUTTER_CONTAINER (buttons), button);
  clutter_actor_set_reactive (button, TRUE);
  clutter_actor_set_position (button, total_width, 0);
  clutter_actor_get_size (button, &width, NULL);
  g_signal_connect (button, "button-release-event",
      G_CALLBACK (zoom_out),
      actor);

  clutter_container_add_actor (CLUTTER_CONTAINER (stage), buttons);

  /* draw a line */
  layer = champlain_path_layer_new ();
  /* Cheap approx of Highway 10 */
  append_point (layer, 45.4104, -73.2846);
  append_point (layer, 45.4178, -73.2239);
  append_point (layer, 45.4176, -73.2181);
  append_point (layer, 45.4151, -73.2126);
  append_point (layer, 45.4016, -73.1926);
  append_point (layer, 45.3994, -73.1877);
  append_point (layer, 45.4000, -73.1815);
  append_point (layer, 45.4151, -73.1218);
  champlain_path_layer_set_stroke_width (layer, 5.0);
  champlain_view_add_layer (CHAMPLAIN_VIEW (actor), CHAMPLAIN_LAYER (layer));

  /* draw a path */
  layer = champlain_path_layer_new ();
  append_point (layer, 45.1386, -73.9196);
  append_point (layer, 45.1229, -73.8991);
  append_point (layer, 45.0946, -73.9531);
  append_point (layer, 45.1085, -73.9714);
  append_point (layer, 45.1104, -73.9761);
  g_object_set (layer, "closed", TRUE, NULL);
  g_object_set (layer, "fill", TRUE, NULL);
  champlain_path_layer_set_visible (layer, TRUE);
  champlain_view_add_layer (CHAMPLAIN_VIEW (actor), CHAMPLAIN_LAYER (layer));

  /* Finish initialising the map view */
  g_object_set (G_OBJECT (actor), "zoom-level", 8,
      "kinetic-mode", TRUE, NULL);
  champlain_view_center_on (CHAMPLAIN_VIEW (actor), 45.466, -73.75);

  clutter_actor_show (stage);
  clutter_main ();

  clutter_actor_destroy (actor);
  return 0;
}
