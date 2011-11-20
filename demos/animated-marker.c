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
#include <math.h>

#define MARKER_SIZE 10

/* The marker is drawn with cairo.  It is composed of 1 static filled circle
 * and 1 stroked circle animated as an echo.
 */
static ClutterActor *
create_marker ()
{
  ClutterActor *marker;
  ClutterActor *bg;
  ClutterTimeline *timeline;
  cairo_t *cr;

  /* Create the marker */
  marker = champlain_custom_marker_new ();

  /* Static filled circle ----------------------------------------------- */
  bg = clutter_cairo_texture_new (MARKER_SIZE, MARKER_SIZE);
  cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (bg));

  cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
  cairo_paint(cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

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

  /* Echo circle -------------------------------------------------------- */
  bg = clutter_cairo_texture_new (2 * MARKER_SIZE, 2 * MARKER_SIZE);
  cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (bg));

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

  /* Animate the echo circle */
  timeline = clutter_timeline_new (1000);
  clutter_timeline_set_loop (timeline, TRUE);
  clutter_actor_set_opacity (CLUTTER_ACTOR (bg), 255);
  clutter_actor_set_scale (CLUTTER_ACTOR (bg), 0.5, 0.5);
  clutter_actor_animate_with_timeline (CLUTTER_ACTOR (bg),
      CLUTTER_EASE_OUT_SINE, 
      timeline, 
      "opacity", 0, 
      "scale-x", 2.0, 
      "scale-y", 2.0, 
      NULL);

  clutter_timeline_start (timeline);

  return marker;
}


double lat = 45.466;
double lon = -73.75;

typedef struct
{
  ChamplainView *view;
  ChamplainMarker *marker;
} GpsCallbackData;

static gboolean
gps_callback (GpsCallbackData *data)
{
  lat += 0.005;
  lon += 0.005;
  champlain_view_center_on (data->view, lat, lon);
  champlain_location_set_location (CHAMPLAIN_LOCATION (data->marker), lat, lon);
  return TRUE;
}


int
main (int argc, char *argv[])
{
  ClutterActor *actor, *marker, *stage;
  ChamplainMarkerLayer *layer;
  GpsCallbackData callback_data;

  if (clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
    return 1;

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 800, 600);

  /* Create the map view */
  actor = champlain_view_new ();
  clutter_actor_set_size (CLUTTER_ACTOR (actor), 800, 600);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), actor);

  /* Create the marker layer */
  layer = champlain_marker_layer_new_full (CHAMPLAIN_SELECTION_SINGLE);
  clutter_actor_show (CLUTTER_ACTOR (layer));
  champlain_view_add_layer (CHAMPLAIN_VIEW (actor), CHAMPLAIN_LAYER (layer));

  /* Create a marker */
  marker = create_marker ();
  champlain_marker_layer_add_marker (layer, CHAMPLAIN_MARKER (marker));

  /* Finish initialising the map view */
  g_object_set (G_OBJECT (actor), "zoom-level", 12,
      "kinetic-mode", TRUE, NULL);
  champlain_view_center_on (CHAMPLAIN_VIEW (actor), lat, lon);

  /* Create callback that updates the map periodically */
  callback_data.view = CHAMPLAIN_VIEW (actor);
  callback_data.marker = CHAMPLAIN_MARKER (marker);

  g_timeout_add (1000, (GSourceFunc) gps_callback, &callback_data);

  clutter_actor_show (stage);
  clutter_main ();

  clutter_actor_destroy (actor);
  return 0;
}
