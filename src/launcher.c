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

/*
 * Terminate the main loop.
 */
static void
on_destroy (GtkWidget * widget, gpointer data)
{
    gtk_main_quit ();
}

int
main (int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *widget;

    gtk_init (&argc, &argv);

    /* create the main, top level, window */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    /* give the window a 20px wide border */
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    /* give it the title */
    gtk_window_set_title (GTK_WINDOW (window), PACKAGE " " VERSION);

    /* open it a bit wider so that both the label and title show up */
    gtk_window_set_default_size (GTK_WINDOW (window), 200, 50);


    /* Connect the destroy event of the window with our on_destroy function
     * When the window is about to be destroyed we get a notificaiton and
     * stop the main GTK loop
     */
    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (on_destroy), NULL);

    /* Create the "Hello, World" label  */
    widget = champlain_widget_new ();

    /* and insert it into the main window  */
    gtk_container_add (GTK_CONTAINER (window), widget);

    /* make sure that everything, window and label, are visible */
    gtk_widget_show_all (window);

    /* start the main loop */
    gtk_main ();

    return 0;
}

