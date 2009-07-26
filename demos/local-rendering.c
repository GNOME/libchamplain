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
static const char *rules[] = { "default-rules.xml", "high-contrast.xml" };

static GtkWidget *memphis_box, *memphis_net_box, *memphis_local_box;
static GtkWidget *rules_tree_view, *bg_button;

/*
 * Terminate the main loop.
 */
static void
on_destroy (GtkWidget *widget, gpointer data)
{
  gtk_main_quit ();
}

static void
load_local_map_data (ChamplainMapSource *source)
{
  ChamplainLocalMapDataSource *map_data_source;

  map_data_source = CHAMPLAIN_LOCAL_MAP_DATA_SOURCE (
    champlain_memphis_map_source_get_map_data_source (
    CHAMPLAIN_MEMPHIS_MAP_SOURCE (source)));

  champlain_local_map_data_source_load_map_data (map_data_source,
      maps[map_index]);
}

static void
load_network_map_data (ChamplainMapSource *source, ChamplainView *view)
{
  ChamplainNetworkMapDataSource *map_data_source;
  gdouble lat, lon;

  map_data_source = CHAMPLAIN_NETWORK_MAP_DATA_SOURCE (
      champlain_memphis_map_source_get_map_data_source (
      CHAMPLAIN_MEMPHIS_MAP_SOURCE (source)));

  g_object_get (G_OBJECT (view), "latitude", &lat, "longitude", &lon, NULL);

  champlain_network_map_data_source_load_map_data (map_data_source,
      lon - 0.008, lat - 0.008, lon + 0.008, lat + 0.008);
}

static void
load_rules_into_gui (ChamplainView *view)
{
  ChamplainMapSource *source;
  GList* ids, *ptr;
  GtkTreeModel *store;
  GtkTreeIter iter;
  GdkColor color;

  g_object_get (G_OBJECT (view), "map-source", &source, NULL);
  ids = champlain_memphis_map_source_get_rule_ids (
      CHAMPLAIN_MEMPHIS_MAP_SOURCE (source));

  gchar *colorstr = champlain_memphis_map_source_get_background_color (
      CHAMPLAIN_MEMPHIS_MAP_SOURCE (source));
  gdk_color_parse (colorstr, &color);
  g_free (colorstr);
  gtk_color_button_set_color (GTK_COLOR_BUTTON (bg_button), &color);

  store = gtk_tree_view_get_model (GTK_TREE_VIEW (rules_tree_view));
  gtk_list_store_clear (GTK_LIST_STORE (store));

  ptr = ids;
  while (ptr != NULL)
    {
      gtk_list_store_append (GTK_LIST_STORE (store), &iter);
      gtk_list_store_set (GTK_LIST_STORE (store), &iter, 0, ptr->data, -1);
      ptr = ptr->next;
    }

  g_list_free (ids);
}

static void
zoom_to_map_data (GtkWidget *widget, ChamplainView *view)
{
  ChamplainMemphisMapSource *source;
  ChamplainMapDataSource *data_source;
  ChamplainBoundingBox *bbox;
  gdouble lat, lon;

  g_object_get (G_OBJECT (view), "map-source", &source, NULL);
  data_source = champlain_memphis_map_source_get_map_data_source (source);
  g_object_get (G_OBJECT (data_source), "bounding-box", &bbox, NULL);
  champlain_bounding_box_get_center (bbox, &lat, &lon);

  champlain_view_center_on (CHAMPLAIN_VIEW(view), lat, lon);
  champlain_view_set_zoom_level (CHAMPLAIN_VIEW(view), 15);
}

static void
request_osm_data_cb (GtkWidget *widget, ChamplainView *view)
{
  gdouble lat, lon;
  ChamplainMapSource *source;
  g_object_get (G_OBJECT (view), "latitude", &lat, "longitude", &lon, NULL);
  g_object_get (G_OBJECT (view), "map-source", &source, NULL);

  if (g_strcmp0 (champlain_map_source_get_id (source), "memphis-network") == 0)
    {
      ChamplainNetworkMapDataSource *data_source =
          CHAMPLAIN_NETWORK_MAP_DATA_SOURCE (
          champlain_memphis_map_source_get_map_data_source (
          CHAMPLAIN_MEMPHIS_MAP_SOURCE(source)));

      champlain_network_map_data_source_load_map_data (data_source,
          lon - 0.008, lat - 0.008, lon + 0.008, lat + 0.008);
    }
}

void
bg_color_set_cb (GtkColorButton *widget, ChamplainView *view)
{
  GdkColor color;
  ChamplainMapSource *source;

  gtk_color_button_get_color (widget, &color);

  g_object_get (G_OBJECT (view), "map-source", &source, NULL);
  if (strncmp (champlain_map_source_get_id (source), "memphis", 7) == 0)
    {
      char *str = gdk_color_to_string (&color);
      champlain_memphis_map_source_set_background_color (
          CHAMPLAIN_MEMPHIS_MAP_SOURCE (source),
          str);
      g_free (str);
      champlain_memphis_map_source_delete_session_cache (
          CHAMPLAIN_MEMPHIS_MAP_SOURCE (source));
    }
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
        {
          load_local_map_data (source);
          gtk_widget_hide_all (memphis_box);
          gtk_widget_set_no_show_all (memphis_box, FALSE);
          gtk_widget_set_no_show_all (memphis_local_box, FALSE);
          gtk_widget_set_no_show_all (memphis_net_box, TRUE);
          gtk_widget_show_all (memphis_box);
        }
      else if (g_strcmp0 (id, "memphis-network") == 0)
        {
          load_network_map_data (source, view);
          gtk_widget_hide_all (memphis_box);
          gtk_widget_set_no_show_all (memphis_box, FALSE);
          gtk_widget_set_no_show_all (memphis_local_box, TRUE);
          gtk_widget_set_no_show_all (memphis_net_box, FALSE);
          gtk_widget_show_all (memphis_box);
        }
      else
        {
          gtk_widget_hide_all (memphis_box);
          gtk_widget_set_no_show_all (memphis_box, TRUE);
        }

      g_object_set (G_OBJECT (view), "map-source", source, NULL);
      g_object_unref (source);
      if (strncmp (id, "memphis", 7) == 0)
        load_rules_into_gui (view);
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
    load_local_map_data (source);
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
  if (strncmp (champlain_map_source_get_id (source), "memphis", 7) == 0)
    {
      champlain_memphis_map_source_load_rules (
          CHAMPLAIN_MEMPHIS_MAP_SOURCE(source),
          file);
      load_rules_into_gui (view);
    }
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
  memphis_box = gtk_vbox_new (FALSE, 10);
  gtk_widget_set_no_show_all (memphis_box, TRUE);
  memphis_net_box = gtk_hbox_new (FALSE, 10);
  gtk_widget_set_no_show_all (memphis_net_box, TRUE);
  memphis_local_box = gtk_hbox_new (FALSE, 10);
  gtk_widget_set_no_show_all (memphis_local_box, TRUE);

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

  /* Memphis options */
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), "<b>Memphis Rendering Options</b>");
  gtk_box_pack_start (GTK_BOX (memphis_box), label, FALSE, FALSE, 0);

  /* local source panel */
  button = gtk_combo_box_new ();
  build_data_combo_box (GTK_COMBO_BOX (button));
  gtk_combo_box_set_active (GTK_COMBO_BOX (button), 0);
  g_signal_connect (button, "changed", G_CALLBACK (map_data_changed), view);
  gtk_box_pack_start (GTK_BOX (memphis_local_box), button, FALSE, FALSE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_ZOOM_FIT);
  g_signal_connect (button, "clicked", G_CALLBACK (zoom_to_map_data), view);
  gtk_container_add (GTK_CONTAINER (memphis_local_box), button);

  gtk_box_pack_start (GTK_BOX (memphis_box), memphis_local_box, FALSE, FALSE, 0);

  /* network source panel */
  button = gtk_button_new_with_label ("Request OSM data");
  g_signal_connect (button, "clicked", G_CALLBACK (request_osm_data_cb), view);
  gtk_box_pack_start (GTK_BOX (memphis_net_box), button, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (memphis_box), memphis_net_box, FALSE, FALSE, 0);

  /* rules chooser */
  button = gtk_combo_box_new ();
  build_rules_combo_box (GTK_COMBO_BOX (button));
  gtk_combo_box_set_active (GTK_COMBO_BOX (button), 0);
  g_signal_connect (button, "changed", G_CALLBACK (rules_changed), view);
  gtk_box_pack_start (GTK_BOX (memphis_box), button, FALSE, FALSE, 0);

  /* bg chooser */
  bbox =  gtk_hbox_new (FALSE, 10);

  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), "Background color");
  gtk_box_pack_start (GTK_BOX (bbox), label, FALSE, FALSE, 0);
  
  bg_button = gtk_color_button_new ();
  gtk_color_button_set_title (GTK_COLOR_BUTTON (bg_button), "Background");
  g_signal_connect (bg_button, "color-set", G_CALLBACK (bg_color_set_cb), view);
  gtk_box_pack_start (GTK_BOX (bbox), bg_button, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (memphis_box), bbox, FALSE, FALSE, 0);

  /* rules list */
  label = gtk_label_new ("Rules");
  bbox =  gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (bbox), label, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (memphis_box), bbox, FALSE, FALSE, 0);

  GtkListStore *store;
  GtkWidget *tree_view, *scrolled;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  store = gtk_list_store_new (1, G_TYPE_STRING);

  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL(store));
  rules_tree_view = tree_view;
  g_object_unref (store);
  renderer = gtk_cell_renderer_text_new ();

  column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 0, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scrolled), tree_view);

  gtk_box_pack_start (GTK_BOX (memphis_box), scrolled, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (menubox), memphis_box, TRUE, TRUE, 0);

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

