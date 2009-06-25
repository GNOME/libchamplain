/*
 * Copyright (C) 2008 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
 * Copyright (C) 2009 Simon Wenner <simon@wenner.ch>
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
#include <clutter-gtk/gtk-clutter-embed.h>

#define N_COLS 2
#define COL_ID 0
#define COL_NAME 1

guint map_index = 0;
static const char *maps[] = { "schaffhausen.osm", "las_palmas.osm" };
static const double coords[][3] = { {47.696303, 8.634481, 14},
                                    {28.13476, -15.43814, 15} };

static const char *rules[] = { "default-rules.xml", "high-contrast.xml" };

/*
 * Terminate the main loop.
 */
static void
on_destroy (GtkWidget *widget, gpointer data)
{
  gtk_main_quit ();
}

static void
load_map_data (ChamplainMapSource *source, ChamplainView *view)
{
  ChamplainLocalMapDataSource *data;

  data = CHAMPLAIN_LOCAL_MAP_DATA_SOURCE (
    champlain_memphis_map_source_get_map_data_source (
    CHAMPLAIN_MEMPHIS_MAP_SOURCE (source)));

  gchar * tmp = g_strdup (maps[map_index]); // FIXME
  champlain_local_map_data_source_load_map_data (data, tmp);
  g_free (tmp);
  g_object_set (G_OBJECT (view), "map-source", source, NULL);
}

static void
zoom_to_map_data (GtkWidget *widget, ChamplainView *view)
{
  champlain_view_center_on (CHAMPLAIN_VIEW(view), coords[map_index][0],
      coords[map_index][1]);
  champlain_view_set_zoom_level (CHAMPLAIN_VIEW(view), coords[map_index][2]);
}

static void
map_source_changed (GtkWidget *widget, ChamplainView *view)
{
  gchar* id;
  ChamplainMapSource *source;
  ChamplainLocalMapDataSource *data;
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
      if (g_strcmp0 (id, "memphis-local") == 0)
        load_map_data (source, view);
      else
        g_object_set (G_OBJECT (view), "map-source", source, NULL);

      g_object_unref (source);
    }

  g_object_unref (factory);
}

static void
map_data_changed (GtkWidget *widget, ChamplainView *view)
{
  gint index;
  ChamplainMapSource *source;
  GtkTreeIter iter;
  GtkTreeModel *model;

  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget), &iter))
    return;

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (widget));
  gtk_tree_model_get (model, &iter, 1, &index, -1);
  
  map_index = index;

  g_object_get (G_OBJECT (view), "map-source", &source, NULL);
  if (g_strcmp0 (champlain_map_source_get_id (source), "memphis-local") == 0)
    load_map_data (source, view);
}

static void
rules_changed (GtkWidget *widget, ChamplainView *view)
{
  gchar* file;
  ChamplainMapSource *source;
  GtkTreeIter iter;
  GtkTreeModel *model;

  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget), &iter))
    return;

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (widget));
  gtk_tree_model_get (model, &iter, 0, &file, -1);

  g_object_get (G_OBJECT (view), "map-source", &source, NULL);
  if (g_strcmp0 (champlain_map_source_get_id (source), "memphis-local") == 0)
    champlain_memphis_map_source_load_rules (
        CHAMPLAIN_MEMPHIS_MAP_SOURCE(source),
        file);
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
build_source_combo_box (GtkComboBox *box)
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

static void
build_data_combo_box (GtkComboBox *box)
{
  GtkTreeStore *store;
  GtkTreeIter parent;
  GtkCellRenderer *cell;

  store = gtk_tree_store_new (2, G_TYPE_STRING, /* file name */
      G_TYPE_INT, /* index */ -1);

  gtk_tree_store_append (store, &parent, NULL);
  gtk_tree_store_set (store, &parent, 0, maps[0],
      1, 0, -1);

  gtk_tree_store_append (store, &parent, NULL);
  gtk_tree_store_set (store, &parent, 0, maps[1],
      1, 1, -1);

  gtk_combo_box_set_model (box, GTK_TREE_MODEL (store));

  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (box), cell, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (box), cell,
      "text", 0, NULL );
}

static void
build_rules_combo_box (GtkComboBox *box)
{
  GtkTreeStore *store;
  GtkTreeIter parent;
  GtkCellRenderer *cell;

  store = gtk_tree_store_new (1, G_TYPE_STRING, /* file name */ -1);
  gtk_tree_store_append (store, &parent, NULL);
  gtk_tree_store_set (store, &parent, 0, rules[0], -1);

  gtk_tree_store_append (store, &parent, NULL);
  gtk_tree_store_set (store, &parent, 0, rules[1], -1);

  gtk_combo_box_set_model (box, GTK_TREE_MODEL (store));

  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (box), cell, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (box), cell,
      "text", 0, NULL );
}

static gboolean
delete_window (GtkWidget *widget,
    GdkEvent  *event,
    gpointer   user_data)
{
  gtk_main_quit ();
}

int
main (int argc,
      char *argv[])
{
  GtkWidget *window;
  GtkWidget *widget, *hbox, *bbox, *menubox, *button, *viewport, *label;
  ChamplainView *view;

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

  hbox = gtk_hbox_new (FALSE, 10);
  menubox = gtk_vbox_new (FALSE, 10);

  widget = gtk_champlain_embed_new ();
  view = gtk_champlain_embed_get_view (GTK_CHAMPLAIN_EMBED (widget));

  g_object_set (G_OBJECT (view), "scroll-mode", CHAMPLAIN_SCROLL_MODE_KINETIC,
      "zoom-level", 9, NULL);

  gtk_widget_set_size_request (widget, 640, 480);

  /* first line of buttons */
  bbox =  gtk_hbox_new (FALSE, 10);
  button = gtk_button_new_from_stock (GTK_STOCK_ZOOM_IN);
  g_signal_connect (button, "clicked", G_CALLBACK (zoom_in), view);
  gtk_container_add (GTK_CONTAINER (bbox), button);

  button = gtk_button_new_from_stock (GTK_STOCK_ZOOM_OUT);
  g_signal_connect (button, "clicked", G_CALLBACK (zoom_out), view);
  gtk_container_add (GTK_CONTAINER (bbox), button);

  button = gtk_spin_button_new_with_range(0, 20, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (button),
      champlain_view_get_zoom_level (view));
  g_signal_connect (button, "changed", G_CALLBACK (zoom_changed), view);
  g_signal_connect (view, "notify::zoom-level", G_CALLBACK (map_zoom_changed),
      button);
  gtk_container_add (GTK_CONTAINER (bbox), button);

  gtk_box_pack_start (GTK_BOX (menubox), bbox, FALSE, FALSE, 0);

  /* map source combo box */
  button = gtk_combo_box_new ();
  build_source_combo_box (GTK_COMBO_BOX (button));
  gtk_combo_box_set_active (GTK_COMBO_BOX (button), 0);
  g_signal_connect (button, "changed", G_CALLBACK (map_source_changed), view);
  gtk_box_pack_start (GTK_BOX (menubox), button, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), "<b>Memphis Rendering Options</b>");
  gtk_box_pack_start (GTK_BOX (menubox), label, FALSE, FALSE, 0);

  bbox =  gtk_hbox_new (FALSE, 10);

  button = gtk_combo_box_new ();
  build_data_combo_box (GTK_COMBO_BOX (button));
  gtk_combo_box_set_active (GTK_COMBO_BOX (button), 0);
  g_signal_connect (button, "changed", G_CALLBACK (map_data_changed), view);
  gtk_box_pack_start (GTK_BOX (bbox), button, FALSE, FALSE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_ZOOM_FIT);
  g_signal_connect (button, "clicked", G_CALLBACK (zoom_to_map_data), view);
  gtk_container_add (GTK_CONTAINER (bbox), button);

  gtk_box_pack_start (GTK_BOX (menubox), bbox, FALSE, FALSE, 0);

  button = gtk_combo_box_new ();
  build_rules_combo_box (GTK_COMBO_BOX (button));
  gtk_combo_box_set_active (GTK_COMBO_BOX (button), 0);
  g_signal_connect (button, "changed", G_CALLBACK (rules_changed), view);
  gtk_box_pack_start (GTK_BOX (menubox), button, FALSE, FALSE, 0);
  
  /* viewport */
  viewport = gtk_viewport_new (NULL, NULL);
  gtk_viewport_set_shadow_type (GTK_VIEWPORT(viewport), GTK_SHADOW_ETCHED_IN);
  gtk_container_add (GTK_CONTAINER (viewport), widget);

  gtk_box_pack_end (GTK_BOX (hbox), menubox, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (hbox), viewport);

  /* and insert it into the main window  */
  gtk_container_add (GTK_CONTAINER (window), hbox);

  /* make sure that everything, window and label, are visible */
  gtk_widget_show_all (window);
  champlain_view_center_on (CHAMPLAIN_VIEW(view), 28.13476, -15.43814);
  /* start the main loop */
  gtk_main ();

  return 0;
}

