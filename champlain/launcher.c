/*
 * Copyright (C) 2008 Pierre-Luc Beaudoin <pierre-luc@squidy.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>

#include <gtk/gtk.h>

#include <champlain.h>

#define OSM_MAP "Open Street Map"
#define OAM_MAP "Open Arial Map"
#define MFF_MAP "Maps for free - Relief"

/*
 * Terminate the main loop.
 */
static void
on_destroy (GtkWidget * widget, gpointer data)
{
  gtk_main_quit ();
}

static void
go_to_montreal (GtkWidget * widget, ChamplainView* view)
{
  champlain_view_center_on(view, -73.75, 45.466);
}

static void
map_source_changed (GtkWidget * widget, ChamplainView* view)
{
  gchar* selection = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));
  if (g_strcmp0(selection, OSM_MAP) == 0)
    {
      g_object_set(G_OBJECT(view), "map-source", CHAMPLAIN_MAP_SOURCE_OPENSTREETMAP, NULL);
    }
  else if (g_strcmp0(selection, OAM_MAP) == 0)
    {
      g_object_set(G_OBJECT(view), "map-source", CHAMPLAIN_MAP_SOURCE_OPENARIALMAP, NULL);
    }
  else if (g_strcmp0(selection, MFF_MAP) == 0)
    {
      g_object_set(G_OBJECT(view), "map-source", CHAMPLAIN_MAP_SOURCE_MAPSFORFREE_RELIEF, NULL);
    }
}

static void 
zoom_changed (GtkSpinButton *spinbutton, ChamplainView* view)
{
  gint zoom = gtk_spin_button_get_value_as_int(spinbutton);
  g_object_set(G_OBJECT(view), "zoom-level", zoom, NULL);
}

static void 
map_zoom_changed (ChamplainView* view, GParamSpec *gobject, GtkSpinButton *spinbutton)
{
  gint zoom;
  g_object_get(G_OBJECT(view), "zoom-level", &zoom, NULL);
  gtk_spin_button_set_value(spinbutton, zoom);
}

static void
zoom_in (GtkWidget * widget, ChamplainView* view)
{
  champlain_view_zoom_in(view);
}

static void
zoom_out (GtkWidget * widget, ChamplainView* view)
{
  champlain_view_zoom_out(view);
}

int
main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *widget, *vbox, *bbox, *button, *viewport;
  GtkWidget *scrolled;
  
  g_thread_init (NULL);
  gtk_clutter_init (&argc, &argv);

  /* create the main, top level, window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  /* give the window a 10px wide border */
  gtk_container_set_border_width (GTK_CONTAINER (window), 10);

  /* give it the title */
  gtk_window_set_title (GTK_WINDOW (window), PACKAGE " " VERSION);

  /* Connect the destroy event of the window with our on_destroy function
   * When the window is about to be destroyed we get a notificaiton and
   * stop the main GTK loop
   */
  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (on_destroy), NULL);

  vbox = gtk_vbox_new(FALSE, 10);
  
  widget = champlain_view_new ();
  g_object_set(G_OBJECT(widget), "zoom-level", 5, NULL);
  
  gtk_widget_set_size_request(widget, 640, 480);
  
  bbox =  gtk_hbox_new (FALSE, 10);
  button = gtk_button_new_from_stock (GTK_STOCK_ZOOM_IN);
  g_signal_connect (button,
                    "clicked",
                    G_CALLBACK (zoom_in),
                    widget);
  gtk_container_add (GTK_CONTAINER (bbox), button);
  button = gtk_button_new_from_stock (GTK_STOCK_ZOOM_OUT);
  g_signal_connect (button,
                    "clicked",
                    G_CALLBACK (zoom_out),
                    widget);
  gtk_container_add (GTK_CONTAINER (bbox), button);
  button = gtk_button_new_with_label ("Montréal");
  g_signal_connect (button,
                    "clicked",
                    G_CALLBACK (go_to_montreal),
                    widget);
  gtk_container_add (GTK_CONTAINER (bbox), button);
  
  button = gtk_combo_box_new_text();
  gtk_combo_box_append_text(GTK_COMBO_BOX(button), OSM_MAP);
  gtk_combo_box_append_text(GTK_COMBO_BOX(button), OAM_MAP);
  gtk_combo_box_append_text(GTK_COMBO_BOX(button), MFF_MAP);
  gtk_combo_box_set_active(GTK_COMBO_BOX(button), 0);
  g_signal_connect (button,
                    "changed",
                    G_CALLBACK (map_source_changed),
                    widget);
  gtk_container_add (GTK_CONTAINER (bbox), button);
  
  button = gtk_spin_button_new_with_range(0, 20, 1);
  g_signal_connect (button,
                    "changed",
                    G_CALLBACK (zoom_changed),
                    widget);
  g_signal_connect (widget,
                    "notify::zoom-level",
                    G_CALLBACK (map_zoom_changed),
                    button);
  gtk_container_add (GTK_CONTAINER (bbox), button);
  
  viewport = gtk_viewport_new (NULL, NULL);
  gtk_viewport_set_shadow_type (GTK_VIEWPORT(viewport), GTK_SHADOW_ETCHED_IN);
  gtk_container_add (GTK_CONTAINER (viewport), widget);
  
  gtk_box_pack_start (GTK_BOX (vbox), bbox, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (vbox), viewport);

  /* and insert it into the main window  */
  gtk_container_add (GTK_CONTAINER (window), vbox);

  /* make sure that everything, window and label, are visible */
  gtk_widget_show_all (window);
  champlain_view_center_on(CHAMPLAIN_VIEW(widget), -73.75, 45.466);
  /* start the main loop */
  gtk_main ();

  return 0;
}
