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

#include <champlain/champlain.h>
#include <clutter-cairo/clutter-cairo.h>
#include <math.h>

#define MARKER_SIZE 10

/* The marker is drawn with cairo.  It is composed of 1 static filled circle
 * and 1 stroked circle animated as an echo.
 */
static ClutterActor*
create_marker ()
{
  ClutterActor *marker;
  ClutterColor orange = { 0xf3, 0x94, 0x07, 0xbb };
  ClutterColor white = { 0xff, 0xff, 0xff, 0xff };
  ClutterActor *actor, *bg;
  ClutterTimeline *timeline;
  ClutterBehaviour *behaviour;
  ClutterAlpha *alpha;
  cairo_t *cr;

  /* Create the marker */
  marker = champlain_marker_new ();

  /* Static filled cercle ----------------------------------------------- */
  bg = clutter_cairo_new (MARKER_SIZE, MARKER_SIZE);
  cr = clutter_cairo_create (CLUTTER_CAIRO (bg));

  /* Draw the circle */
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_arc (cr, MARKER_SIZE / 2.0, MARKER_SIZE / 2.0, MARKER_SIZE / 2.0, 0, 2 * M_PI);
  cairo_close_path (cr);

  /* Fill the circle */
  cairo_set_source_rgba (cr, 0.1, 0.1, 0.9, 1.0);
  cairo_fill (cr);

  cairo_destroy (cr);

  /* Add the circle to the marker */
  clutter_container_add_actor (CLUTTER_CONTAINER (marker), bg);
  clutter_actor_set_anchor_point_from_gravity (bg, CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_position (bg, 0, 0);

  /* Echo cercle -------------------------------------------------------- */
  bg = clutter_cairo_new (2 * MARKER_SIZE, 2 * MARKER_SIZE);
  cr = clutter_cairo_create (CLUTTER_CAIRO (bg));

  /* Draw the circle */
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_arc (cr, MARKER_SIZE, MARKER_SIZE, 0.9 * MARKER_SIZE, 0, 2 * M_PI);
  cairo_close_path (cr);

  /* Stroke the circle */
  cairo_set_line_width (cr, 2.0);
  cairo_set_source_rgba (cr, 0.1, 0.1, 0.7, 1.0);
  cairo_stroke (cr);

  cairo_destroy (cr);

  /* Add the circle to the marker */
  clutter_container_add_actor (CLUTTER_CONTAINER (marker), bg);
  clutter_actor_lower_bottom (bg); /* Ensure it is under the previous circle */
  clutter_actor_set_position (bg, 0, 0);
  clutter_actor_set_anchor_point_from_gravity (bg, CLUTTER_GRAVITY_CENTER);

  /* Animate the echo cercle */
  timeline = clutter_timeline_new_for_duration (1000);
  clutter_timeline_set_loop (timeline, TRUE);
  alpha = clutter_alpha_new_full (timeline, CLUTTER_ALPHA_SINE_INC, NULL, g_object_unref);

  behaviour = clutter_behaviour_scale_new (alpha, 0.5, 0.5, 2.0, 2.0);
  clutter_behaviour_apply (behaviour, bg);

  behaviour = clutter_behaviour_opacity_new (alpha, 255, 0);
  clutter_behaviour_apply (behaviour, bg);

  clutter_timeline_start (timeline);

  /* Sets marker position on the map */
  champlain_marker_set_position (CHAMPLAIN_MARKER (marker),
      45.528178, -73.563788);

  return marker;
}

int
main (int argc, char *argv[])
{
  ClutterActor* actor, *layer, *marker, *stage;

  g_thread_init (NULL);
  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 800, 600);

  /* Create the map view */
  actor = champlain_view_new ();
  champlain_view_set_size (CHAMPLAIN_VIEW (actor), 800, 600);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), actor);

  /* Create the marker layer */
  layer = champlain_layer_new ();
  clutter_actor_show (layer);
  champlain_view_add_layer (CHAMPLAIN_VIEW (actor), layer);

  /* Create a marker */
  marker = create_marker ();
  clutter_container_add (CLUTTER_CONTAINER (layer), marker, NULL);

  /* Finish initialising the map view */
  g_object_set (G_OBJECT (actor), "zoom-level", 5,
      "scroll-mode", CHAMPLAIN_SCROLL_MODE_KINETIC, NULL);
  champlain_view_center_on (CHAMPLAIN_VIEW (actor), 45.466, -73.75);

  clutter_actor_show (stage);
  clutter_main ();

  return 0;
}
