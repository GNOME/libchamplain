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
#include "champlain-marshal.h"

#include <stdio.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk-clutter-embed.h>
#include <clutter/clutter.h>

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
    GtkAdjustment* horizontalAdjustment;
    GtkAdjustment* verticalAdjustment;
    GdkPoint scrollOffset;    
};

G_DEFINE_TYPE(ChamplainWidget, champlain_widget, GTK_TYPE_ALIGNMENT)

               
static void
champlain_widget_adjustement_changed(GtkAdjustment* adjustment, gpointer champlainWidget)
{
	ChamplainWidgetPrivate* priv = CHAMPLAIN_WIDGET_GET_PRIVATE(champlainWidget);
	
    if (adjustment == priv->horizontalAdjustment)
        priv->scrollOffset.x = (int)gtk_adjustment_get_value(adjustment);
    else if (adjustment == priv->verticalAdjustment)
        priv->scrollOffset.y = (int)gtk_adjustment_get_value(adjustment);

	// Check if the offset is empty
	
	
}            

static void 
champlain_widget_set_scroll_adjustments(ChamplainWidget      *champlainWidget,
                                        GtkAdjustment        *hadjustment,
                                        GtkAdjustment        *vadjustment)
{
	ChamplainWidgetPrivate* priv = CHAMPLAIN_WIDGET_GET_PRIVATE(champlainWidget);
	
    if (priv->horizontalAdjustment) {
		g_signal_handlers_disconnect_by_func(G_OBJECT(priv->horizontalAdjustment), (gpointer)champlain_widget_adjustement_changed, champlainWidget);
		g_signal_handlers_disconnect_by_func(G_OBJECT(priv->verticalAdjustment), (gpointer)champlain_widget_adjustement_changed, champlainWidget);
		
		g_object_unref(priv->horizontalAdjustment);
		g_object_unref(priv->verticalAdjustment);
	}
	
	priv->horizontalAdjustment = hadjustment;
	priv->verticalAdjustment = vadjustment;
	
	if (hadjustment) {
		g_object_ref_sink(priv->horizontalAdjustment);
		g_object_ref_sink(priv->verticalAdjustment);
		
		gdouble val = gtk_adjustment_get_value(hadjustment);
		g_print("value: %f \n", val);
		val = gtk_adjustment_get_value(vadjustment);
		g_print("value: %f \n", val);
    	// Connect the signals
	
		g_object_set(G_OBJECT(priv->horizontalAdjustment), "lower", 0.0, NULL);
		g_object_set(G_OBJECT(priv->horizontalAdjustment), "upper", 100.0, NULL);
		g_object_set(G_OBJECT(priv->horizontalAdjustment), "page-size", 20.0, NULL);
		g_object_set(G_OBJECT(priv->horizontalAdjustment), "step-increment", 5.0, NULL);
		g_object_set(G_OBJECT(priv->horizontalAdjustment), "page-increment", 15.0, NULL);
	}
	
    
}   

static void champlain_widget_finalize(GObject* object)
{
    ChamplainWidget* widget = CHAMPLAIN_WIDGET(object);
	ChamplainWidgetPrivate* priv = CHAMPLAIN_WIDGET_GET_PRIVATE(widget);

    if (priv->horizontalAdjustment) {
        g_object_unref(priv->horizontalAdjustment);
       	g_signal_handlers_disconnect_by_func(G_OBJECT(priv->horizontalAdjustment), (gpointer)champlain_widget_adjustement_changed, widget);
	}
	
    if (priv->verticalAdjustment) {
        g_object_unref(priv->verticalAdjustment);
        g_signal_handlers_disconnect_by_func(G_OBJECT(priv->verticalAdjustment), (gpointer)champlain_widget_adjustement_changed, widget);
    }

    G_OBJECT_CLASS(champlain_widget_parent_class)->finalize(object);
}
                                        
static void 
champlain_widget_class_init(ChamplainWidgetClass* champlainWidgetClass)
{
	g_type_class_add_private(champlainWidgetClass, sizeof(ChamplainWidgetPrivate));
	
    /*
     * make us scrollable (e.g. addable to a GtkScrolledWindow)
     */
    champlainWidgetClass->set_scroll_adjustments = champlain_widget_set_scroll_adjustments;
    GTK_WIDGET_CLASS(champlainWidgetClass)->set_scroll_adjustments_signal = g_signal_new("set-scroll-adjustments",
            G_TYPE_FROM_CLASS(champlainWidgetClass),
            (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
            G_STRUCT_OFFSET(ChamplainWidgetClass, set_scroll_adjustments),
            NULL, NULL,
            champlain_marshal_VOID__OBJECT_OBJECT,
            G_TYPE_NONE, 2,
            GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);
            
    GObjectClass* objectClass = G_OBJECT_CLASS(champlainWidgetClass);
    objectClass->finalize = champlain_widget_finalize;
}

static void champlain_widget_init(ChamplainWidget* champlainWidget)
{
	ChamplainWidgetPrivate* priv = CHAMPLAIN_WIDGET_GET_PRIVATE(champlainWidget);

    priv->horizontalAdjustment = GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 0.0, 100.0, 10.0, 50.0, 20.0));
    priv->verticalAdjustment = GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 0.0, 100.0, 10.0, 50.0, 20.0));
    
    g_object_ref_sink(priv->horizontalAdjustment);
    g_object_ref_sink(priv->verticalAdjustment);
}

static void champlain_widget_load_map(ChamplainWidget* champlainWidget)
{
	ChamplainWidgetPrivate* priv = CHAMPLAIN_WIDGET_GET_PRIVATE(champlainWidget);
	
	ClutterActor* stage = gtk_clutter_embed_get_stage(GTK_CLUTTER_EMBED(priv->clutterEmbed));
	
	ClutterColor white;
	clutter_color_parse("white", &white);
	ClutterColor blue;
	clutter_color_parse("blue", &blue);
	
	ClutterActor * rect = clutter_rectangle_new_with_color(&blue);
	clutter_container_add(CLUTTER_CONTAINER(stage), rect, NULL);
  	clutter_actor_set_position (rect, 100, 100);
  	clutter_actor_set_size (rect, 100, 100);
	clutter_actor_show(rect);
	
	rect = clutter_rectangle_new_with_color(&white);
	clutter_container_add(CLUTTER_CONTAINER(stage), rect, NULL);
  	clutter_actor_set_position (rect, 100, 200);
  	clutter_actor_set_size (rect, 100, 100);
	clutter_actor_show(rect);
	
	rect = clutter_rectangle_new_with_color(&blue);
	clutter_container_add(CLUTTER_CONTAINER(stage), rect, NULL);
  	clutter_actor_set_position (rect, 200, 200);
  	clutter_actor_set_size (rect, 100, 100);
	clutter_actor_show(rect);
	
	rect = clutter_rectangle_new_with_color(&white);
	clutter_container_add(CLUTTER_CONTAINER(stage), rect, NULL);
  	clutter_actor_set_position (rect, 200, 100);
  	clutter_actor_set_size (rect, 100, 100);
	clutter_actor_show(rect);
	
}

GtkWidget* champlain_widget_new()
{
	ChamplainWidget* widget = CHAMPLAIN_WIDGET(g_object_new(CHAMPLAIN_TYPE_WIDGET, NULL));
	ChamplainWidgetPrivate* priv = CHAMPLAIN_WIDGET_GET_PRIVATE(widget);
	
	priv->clutterEmbed = gtk_clutter_embed_new();
	ClutterActor* stage = gtk_clutter_embed_get_stage(GTK_CLUTTER_EMBED(priv->clutterEmbed));
	
	ClutterColor black;
	clutter_color_parse("black", &black);
	clutter_stage_set_color(CLUTTER_STAGE(stage), &black);
	gtk_container_add(GTK_CONTAINER(widget), priv->clutterEmbed);
	
	champlain_widget_load_map(widget);
	
    return GTK_WIDGET(widget);
}
