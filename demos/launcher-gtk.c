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

#include <gtk/gtk.h>

#include <champlain/champlain.h>
#include <champlain-gtk/champlain-gtk.h>
#include <clutter-gtk/clutter-gtk.h>

#include <markers.h>

#define N_COLS 2
#define COL_ID 0
#define COL_NAME 1

static ChamplainPolygon *polygon;

/*
 * Terminate the main loop.
 */
static void
on_destroy (GtkWidget *widget, gpointer data)
{
  gtk_main_quit ();
}

static void
toggle_layer (GtkToggleButton *widget,
              ClutterActor *layer)
{
  if(gtk_toggle_button_get_active(widget))
    {

      champlain_polygon_show (polygon);
      champlain_layer_animate_in_all_markers (CHAMPLAIN_LAYER (layer));
    }
  else
    {
      champlain_polygon_hide (polygon);
      champlain_layer_animate_out_all_markers (CHAMPLAIN_LAYER (layer));
    }
}

gboolean
mouse_click_cb (ClutterActor *actor, ClutterEvent *event, gpointer data)
{
    gdouble lat, lon;

    champlain_view_get_coords_from_event (CHAMPLAIN_VIEW (data), event, &lat, &lon);
    g_print ("Mouse click at: %f  %f\n", lat, lon);

    return TRUE;
}

static void
map_source_changed (GtkWidget *widget,
                    ChamplainView *view)
{
  gchar* id;
  ChamplainMapSource *source;
  GtkTreeIter iter;
  GtkTreeModel *model;

  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget), &iter))
    return;

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (widget));

  gtk_tree_model_get (model, &iter, COL_ID, &id, -1);

  ChamplainMapSourceFactory *factory = champlain_map_source_factory_dup_default ();
  source = champlain_map_source_factory_create (factory, id);

  if (source != NULL)
    {
      g_object_set (G_OBJECT (view), "map-source", source, NULL);
      g_object_unref (factory);
    }

  g_object_unref (source);
}

static void
zoom_changed (GtkSpinButton *spinbutton,
              ChamplainView *view)
{
  gint zoom = gtk_spin_button_get_value_as_int(spinbutton);
  g_object_set(G_OBJECT(view), "zoom-level", zoom, NULL);
}

static void
map_zoom_changed (ChamplainView *view,
                  GParamSpec *gobject,
                  GtkSpinButton *spinbutton)
{
  gint zoom;
  g_object_get(G_OBJECT(view), "zoom-level", &zoom, NULL);
  gtk_spin_button_set_value(spinbutton, zoom);
}

static void
view_state_changed (ChamplainView *view,
                    GParamSpec *gobject,
                    GtkImage *image)
{
  ChamplainState state;

  g_object_get (G_OBJECT (view), "state", &state, NULL);
  if (state == CHAMPLAIN_STATE_LOADING)
    {
      gtk_image_set_from_stock (image, GTK_STOCK_NETWORK, GTK_ICON_SIZE_BUTTON);
      g_print("STATE: loading\n");
    }
  else
    {
      gtk_image_clear (image);
      g_print("STATE: done\n");
    }
}

static void
zoom_in (GtkWidget *widget,
         ChamplainView *view)
{
  champlain_view_zoom_in(view);
}

static void
zoom_out (GtkWidget *widget,
          ChamplainView *view)
{
  champlain_view_zoom_out(view);
}

static void
build_combo_box (GtkComboBox *box)
{
  ChamplainMapSourceFactory *factory;
  GSList *sources, *iter;
  gint i = 0;
  GtkTreeStore *store;
  GtkTreeIter parent;
  GtkCellRenderer *cell;

  store = gtk_tree_store_new (N_COLS, G_TYPE_STRING, /* id */
      G_TYPE_STRING, /* name */
      -1);

  factory = champlain_map_source_factory_dup_default ();
  sources = champlain_map_source_factory_dup_list (factory);

  iter = sources;
  while (iter != NULL)
  {
    ChamplainMapSourceDesc *desc;

    desc = (ChamplainMapSourceDesc*) iter->data;

    gtk_tree_store_append (store, &parent, NULL );
    gtk_tree_store_set (store, &parent, COL_ID, desc->id,
        COL_NAME, desc->name, -1);

    iter = g_slist_next (iter);
  }

  g_slist_free (sources);
  g_object_unref (factory);

  gtk_combo_box_set_model (box, GTK_TREE_MODEL (store));

  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (box), cell, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (box), cell,
      "text", COL_NAME, NULL );
}

int
main (int argc,
      char *argv[])
{
  GtkWidget *window;
  GtkWidget *widget, *vbox, *bbox, *button, *viewport;
  ChamplainView *view;
  ChamplainLayer *layer;

  g_thread_init (NULL);
  gtk_clutter_init (&argc, &argv);

  /* create the main, top level, window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  /* give the window a 10px wide border */
  gtk_container_set_border_width (GTK_CONTAINER (window), 10);

  /* give it the title */
  gtk_window_set_title (GTK_WINDOW (window), "libchamplain Gtk+ demo");

  /* Connect the destroy event of the window with our on_destroy function
   * When the window is about to be destroyed we get a notificaiton and
   * stop the main GTK loop
   */
  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (on_destroy),
      NULL);

  vbox = gtk_vbox_new(FALSE, 10);

  widget = gtk_champlain_embed_new ();
  view = gtk_champlain_embed_get_view (GTK_CHAMPLAIN_EMBED (widget));
  clutter_actor_set_reactive (CLUTTER_ACTOR (view), TRUE);
  g_signal_connect (view, "button-release-event", G_CALLBACK (mouse_click_cb), view);


  g_object_set (G_OBJECT (view),
      "scroll-mode", CHAMPLAIN_SCROLL_MODE_KINETIC,
      "zoom-level", 5,
      "license-text", "Don't eat cereals with orange juice\nIt tastes bad",
      "show-scale", TRUE,
      NULL);
  champlain_view_center_on(CHAMPLAIN_VIEW(view), 45.466, -73.75);

  layer = create_marker_layer (view);
  champlain_view_add_layer(view, layer);
  champlain_layer_hide_all_markers (CHAMPLAIN_LAYER (layer));

  polygon = champlain_polygon_new ();
  /* Cheap approx of Highway 10 */
  champlain_polygon_append_point (polygon, 45.4095, -73.3197);
  champlain_polygon_append_point (polygon, 45.4104, -73.2846);
  champlain_polygon_append_point (polygon, 45.4178, -73.2239);
  champlain_polygon_append_point (polygon, 45.4176, -73.2181);
  champlain_polygon_append_point (polygon, 45.4151, -73.2126);
  champlain_polygon_append_point (polygon, 45.4016, -73.1926);
  champlain_polygon_append_point (polygon, 45.3994, -73.1877);
  champlain_polygon_append_point (polygon, 45.4000, -73.1815);
  champlain_polygon_append_point (polygon, 45.4151, -73.1218);
  champlain_polygon_set_stroke_width (polygon, 5.0);
  g_object_set (G_OBJECT (polygon),
      "mark-points", TRUE,
      NULL);
  champlain_view_add_polygon (CHAMPLAIN_VIEW (view), polygon);
  champlain_polygon_hide (polygon);

  gtk_widget_set_size_request (widget, 640, 480);

  bbox =  gtk_hbox_new (FALSE, 10);
  button = gtk_button_new_from_stock (GTK_STOCK_ZOOM_IN);
  g_signal_connect (button, "clicked", G_CALLBACK (zoom_in), view);
  gtk_container_add (GTK_CONTAINER (bbox), button);

  button = gtk_button_new_from_stock (GTK_STOCK_ZOOM_OUT);
  g_signal_connect (button, "clicked", G_CALLBACK (zoom_out), view);
  gtk_container_add (GTK_CONTAINER (bbox), button);

  button = gtk_toggle_button_new_with_label  ("Markers");
  g_signal_connect (button, "toggled", G_CALLBACK (toggle_layer), layer);
  gtk_container_add (GTK_CONTAINER (bbox), button);

  button = gtk_combo_box_new ();
  build_combo_box (GTK_COMBO_BOX (button));
  gtk_combo_box_set_active (GTK_COMBO_BOX (button), 0);
  g_signal_connect (button, "changed", G_CALLBACK (map_source_changed), view);
  gtk_container_add (GTK_CONTAINER (bbox), button);

  button = gtk_spin_button_new_with_range(0, 20, 1);
  g_signal_connect (button, "changed", G_CALLBACK (zoom_changed), view);
  g_signal_connect (view, "notify::zoom-level", G_CALLBACK (map_zoom_changed),
      button);
  gtk_container_add (GTK_CONTAINER (bbox), button);

  button = gtk_image_new ();
  g_signal_connect (view, "notify::state", G_CALLBACK (view_state_changed),
      button);
  gtk_box_pack_end (GTK_BOX (bbox), button, FALSE, FALSE, 0);

  viewport = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (viewport), widget);

  gtk_box_pack_start (GTK_BOX (vbox), bbox, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (vbox), viewport);

  /* and insert it into the main window  */
  gtk_container_add (GTK_CONTAINER (window), vbox);

  /* make sure that everything, window and label, are visible */
  gtk_widget_show_all (window);
  /* start the main loop */
  gtk_main ();

  return 0;
}

