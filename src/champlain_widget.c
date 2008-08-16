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
#include "champlain_map_tile.h"
#include "champlain_map.h"
#include "champlain_widget.h"
#include "champlain-marshal.h"

#include <tidy-finger-scroll.h>
#include <tidy-scrollable.h>
#include <tidy-viewport.h>
#include <tidy-adjustment.h>
#include <math.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk-clutter-embed.h>
#include <clutter/clutter.h>

enum
{
  /* normal signals */
  TBD,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_TBD
};

static guint champlain_widget_signals[LAST_SIGNAL] = { 0, };

#define CHAMPLAIN_WIDGET_GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_WIDGET, ChamplainWidgetPrivate))

typedef struct
{
  /* Units to store the origin of a click when scrolling */
  ClutterUnit x;
  ClutterUnit y;
} ChamplainPoint;

struct _ChamplainWidgetPrivate
{
  GtkWidget *clutterEmbed;
  ClutterActor *viewport;
  ChamplainPoint viewportSize;
  ClutterActor *fingerScroll;
  
  ChamplainMap *map;
};


G_DEFINE_TYPE (ChamplainWidget, champlain_widget, GTK_TYPE_ALIGNMENT);



static void
champlain_widget_finalize (GObject * object)
{
  ChamplainWidget *widget = CHAMPLAIN_WIDGET (object);
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (widget);

  G_OBJECT_CLASS (champlain_widget_parent_class)->finalize (object);
}

static void
champlain_widget_class_init (ChamplainWidgetClass *champlainWidgetClass)
{
  g_type_class_add_private (champlainWidgetClass, sizeof (ChamplainWidgetPrivate));

  GObjectClass *objectClass = G_OBJECT_CLASS (champlainWidgetClass);
  objectClass->finalize = champlain_widget_finalize;
}

static void
champlain_widget_init (ChamplainWidget *champlainWidget)
{
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (champlainWidget);
}

static void
widget_size_allocated_cb (GtkWidget *widget, GtkAllocation *allocation, ChamplainWidget *champlainWidget) 
{                
  gdouble lower, upper;
  TidyAdjustment *hadjust, *vadjust;
  
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (champlainWidget);
  priv->viewportSize.x = allocation->width;
  priv->viewportSize.y = allocation->height;
  clutter_actor_set_size (priv->fingerScroll, priv->viewportSize.x, priv->viewportSize.y);
  
  g_object_set (G_OBJECT (priv->viewport), "sync-adjustments", FALSE, NULL);
  
  tidy_scrollable_get_adjustments (TIDY_SCROLLABLE (priv->viewport), &hadjust, &vadjust);
  
  tidy_adjustment_get_values (hadjust, NULL, &lower, &upper, NULL, NULL, NULL);
  lower = 0;
  upper = champlain_map_zoom_level_get_width(priv->map->current_level) - priv->viewportSize.x; // Map's width - Viewport width
  g_object_set (hadjust, "lower", lower, "upper", upper,
                "step-increment", 1.0, "elastic", TRUE, NULL);
                
  tidy_adjustment_get_values (vadjust, NULL, &lower, &upper, NULL, NULL, NULL);
  lower = 0;
  upper = champlain_map_zoom_level_get_height(priv->map->current_level) - priv->viewportSize.y;
  g_object_set (vadjust, "lower", lower, "upper", upper,
                "step-increment", 1.0, "elastic", TRUE, NULL);
}
                          
GtkWidget *
champlain_widget_new ()
{
  ClutterColor stage_color = { 0x34, 0x39, 0x39, 0xff };
  ChamplainWidget *widget, *stage; 
  
  widget = CHAMPLAIN_WIDGET (g_object_new (CHAMPLAIN_TYPE_WIDGET, NULL));
  ChamplainWidgetPrivate *priv = CHAMPLAIN_WIDGET_GET_PRIVATE (widget);
  
  priv->viewportSize.x = 640;
  priv->viewportSize.y = 480;
	
  priv->clutterEmbed = gtk_clutter_embed_new ();
  g_signal_connect (priv->clutterEmbed,
                    "size-allocate",
                    G_CALLBACK (widget_size_allocated_cb),
                    widget);

	// Setup stage
  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->clutterEmbed));
  
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  gtk_container_add (GTK_CONTAINER (widget), priv->clutterEmbed);
  
  // Setup viewport
  priv->viewport = tidy_viewport_new ();
  ClutterActor* group = clutter_group_new();
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport), group);
                
  // Setup finger scroll
  priv->fingerScroll = tidy_finger_scroll_new(TIDY_FINGER_SCROLL_MODE_KINETIC);
  g_object_set (priv->fingerScroll, "decel-rate", 1.25, NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->fingerScroll), priv->viewport);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), priv->fingerScroll);
  
  
	priv->map = champlain_map_new(CHAMPLAIN_MAP_SOURCE_DEBUG);
	champlain_map_load(priv->map, 1);
  clutter_container_add_actor (CLUTTER_CONTAINER (group), priv->map->current_level->group);
  
  return GTK_WIDGET (widget);
}
