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

#include "config.h"

#include "champlain_defines.h"
#include "champlain_widget.h"

#include <stdio.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk-clutter-embed.h>

enum {
    /* normal signals */
    TBD,
    LAST_SIGNAL
};

enum {
    PROP_0,

    PROP_TBD
};

static guint champlain_widget_signals[LAST_SIGNAL] = { 0, };

#define CHAMPLAIN_WIDGET_GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_WIDGET, ChamplainWidgetPrivate))

struct _ChamplainWidgetPrivate {
    GtkWidget* clutterEmbed;
};

G_DEFINE_TYPE(ChamplainWidget, champlain_widget, GTK_TYPE_ALIGNMENT)


static void 
champlain_widget_class_init(ChamplainWidgetClass* champlainWidgetClass)
{
/*    GtkWidgetClass* widgetClass = GTK_WIDGET_CLASS(champlainWidget);
    widgetClass->realize = champlain_widget_realize;
    widgetClass->expose_event = champlain_widget_expose_event;
    widgetClass->key_press_event = champlain_widget_key_press_event;
    widgetClass->key_release_event = champlain_widget_key_release_event;
    widgetClass->button_press_event = champlain_widget_button_press_event;
    widgetClass->button_release_event = champlain_widget_button_release_event;
    widgetClass->motion_notify_event = champlain_widget_motion_event;
    widgetClass->scroll_event = champlain_widget_scroll_event;
    widgetClass->size_allocate = champlain_widget_size_allocate;
    widgetClass->popup_menu = champlain_widget_popup_menu_handler;
    widgetClass->focus_in_event = champlain_widget_focus_in_event;
    widgetClass->focus_out_event = champlain_widget_focus_out_event;
    widgetClass->get_accessible = champlain_widget_get_accessible;
*/
	g_type_class_add_private(champlainWidgetClass, sizeof(ChamplainWidgetPrivate));

}

static void champlain_widget_init(ChamplainWidget* champlainWidget)
{
}

GtkWidget* champlain_widget_new()
{
	ChamplainWidget* widget = CHAMPLAIN_WIDGET(g_object_new(CHAMPLAIN_TYPE_WIDGET, NULL));
	ChamplainWidgetPrivate* priv = CHAMPLAIN_WIDGET_GET_PRIVATE(widget);
	
	priv->clutterEmbed = gtk_clutter_embed_new();
	gtk_container_add(GTK_CONTAINER(widget), priv->clutterEmbed);
	gtk_widget_show_all(GTK_WIDGET(widget));
	
    return GTK_WIDGET(widget);
}
