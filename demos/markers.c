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
#include <markers.h>

/* This event callback will never be called if you use
 * ChamplainSelectionLayer.  The selection layer uses this
 * event for the selection and do not pass the event forward.
 * For this event to be fired, you should use a simple
 * ChamplainLayer.
 */
/*
static gboolean
marker_button_release_cb (ClutterActor *actor,
    ClutterButtonEvent *event,
    ChamplainView * view)
{
  if (event->button != 1 || event->click_count > 1)
    return FALSE;

  g_print("Montreal was clicked\n");

  return TRUE;
}
*/

ChamplainLayer *
create_marker_layer (G_GNUC_UNUSED ChamplainView *view)
{
  ClutterActor *marker;
  ChamplainLayer *layer;
  ClutterActor *layer_actor;
  ClutterColor orange = { 0xf3, 0x94, 0x07, 0xbb };

  layer = champlain_layer_new_full (CHAMPLAIN_SELECTION_SINGLE);
  layer_actor = CLUTTER_ACTOR (layer);

  marker = champlain_label_new_with_text ("Montréal\n<span size=\"xx-small\">Québec</span>",
        "Serif 14", NULL, NULL);
  champlain_label_set_use_markup (CHAMPLAIN_LABEL (marker), TRUE);
  champlain_label_set_alignment (CHAMPLAIN_LABEL (marker), PANGO_ALIGN_RIGHT);
  champlain_label_set_color (CHAMPLAIN_LABEL (marker), &orange);

  champlain_marker_set_position (CHAMPLAIN_MARKER (marker),
      45.528178, -73.563788);
  champlain_layer_add_marker (layer, CHAMPLAIN_MARKER (marker));
  /*
   * This event handler will never be called anyway because this demo is using
   * a ChamplainSelectionLayer but we leave it here in the demo so that you know
   * how to have reactive markers

  clutter_actor_set_reactive (marker, TRUE);
  g_signal_connect_after (marker, "button-release-event",
    G_CALLBACK (marker_button_release_cb), view);
   */

  marker = champlain_label_new_from_file ("/usr/share/icons/gnome/24x24/emblems/emblem-generic.png", NULL);
  champlain_label_set_text (CHAMPLAIN_LABEL (marker), "New York");
  champlain_marker_set_position (CHAMPLAIN_MARKER (marker), 40.77, -73.98);
  champlain_layer_add_marker (layer, CHAMPLAIN_MARKER (marker));

  marker = champlain_label_new_from_file ("/usr/share/icons/gnome/24x24/emblems/emblem-important.png", NULL);
  champlain_marker_set_position (CHAMPLAIN_MARKER (marker), 47.130885,
      -70.764141);
  champlain_layer_add_marker (layer, CHAMPLAIN_MARKER (marker));

  marker = champlain_point_new ();
  champlain_marker_set_position (CHAMPLAIN_MARKER (marker), 45.130885,
      -65.764141);
  champlain_layer_add_marker (layer, CHAMPLAIN_MARKER (marker));

  marker = champlain_label_new_from_file ("/usr/share/icons/gnome/24x24/emblems/emblem-favorite.png", NULL);
  champlain_label_set_draw_background (CHAMPLAIN_LABEL (marker), FALSE);
  champlain_marker_set_position (CHAMPLAIN_MARKER (marker), 45.41484,
      -71.918907);
  champlain_layer_add_marker (layer, CHAMPLAIN_MARKER (marker));
  
  champlain_layer_set_all_markers_movable (layer);

  clutter_actor_show (layer_actor);
  return layer;
}
