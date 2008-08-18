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
#include "champlain_view.h"
#include "map_tile.h"
#include "map.h"
#include "map_zoom_level.h"
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

static guint champlain_view_signals[LAST_SIGNAL] = { 0, };

#define CHAMPLAIN_VIEW_GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_VIEW, ChamplainViewPrivate))

typedef struct
{
  /* Units to store the origin of a click when scrolling */
  ClutterUnit x;
  ClutterUnit y;
} ChamplainPoint;

struct _ChamplainViewPrivate
{
  GtkWidget *clutterEmbed;
  ClutterActor *viewport;
  ChamplainPoint viewportSize;
  ClutterActor *fingerScroll;
  
  Map *map;
};


G_DEFINE_TYPE (ChamplainView, champlain_view, GTK_TYPE_ALIGNMENT);

static void
champlain_view_finalize (GObject * object)
{
  ChamplainView *view = CHAMPLAIN_VIEW (object);
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);

  G_OBJECT_CLASS (champlain_view_parent_class)->finalize (object);
}

static void
champlain_view_class_init (ChamplainViewClass *champlainViewClass)
{
  g_type_class_add_private (champlainViewClass, sizeof (ChamplainViewPrivate));

  GObjectClass *objectClass = G_OBJECT_CLASS (champlainViewClass);
  objectClass->finalize = champlain_view_finalize;
}

static void
champlain_view_init (ChamplainView *champlainView)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);
}

static void
view_size_allocated_cb (GtkWidget *view, GtkAllocation *allocation, ChamplainView *champlainView) 
{                
  gdouble lower, upper;
  TidyAdjustment *hadjust, *vadjust;
  
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);
  priv->viewportSize.x = allocation->width;
  priv->viewportSize.y = allocation->height;
  clutter_actor_set_size (priv->fingerScroll, priv->viewportSize.x, priv->viewportSize.y);
  
  g_object_set (G_OBJECT (priv->viewport), "sync-adjustments", FALSE, NULL);
  
  tidy_scrollable_get_adjustments (TIDY_SCROLLABLE (priv->viewport), &hadjust, &vadjust);
  
  tidy_adjustment_get_values (hadjust, NULL, &lower, &upper, NULL, NULL, NULL);
  lower = 0;
  upper = map_zoom_level_get_width(priv->map->current_level) - priv->viewportSize.x; 
  g_object_set (hadjust, "lower", lower, "upper", upper,
                "step-increment", 1.0, "elastic", TRUE, NULL);
                
  tidy_adjustment_get_values (vadjust, NULL, &lower, &upper, NULL, NULL, NULL);
  lower = 0;
  upper = map_zoom_level_get_height(priv->map->current_level) - priv->viewportSize.y;
  g_object_set (vadjust, "lower", lower, "upper", upper,
                "step-increment", 1.0, "elastic", TRUE, NULL);
                
}
                          
GtkWidget *
champlain_view_new ()
{
  ClutterColor stage_color = { 0x34, 0x39, 0x39, 0xff };
  ChamplainView *view;
  ClutterActor *stage; 
  
  view = CHAMPLAIN_VIEW (g_object_new (CHAMPLAIN_TYPE_VIEW, NULL));
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);
  
  priv->viewportSize.x = 640;
  priv->viewportSize.y = 480;
	
  priv->clutterEmbed = gtk_clutter_embed_new ();
  g_signal_connect (priv->clutterEmbed,
                    "size-allocate",
                    G_CALLBACK (view_size_allocated_cb),
                    view);

	// Setup stage
  stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (priv->clutterEmbed));
  
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
  gtk_container_add (GTK_CONTAINER (view), priv->clutterEmbed);
  
  // Setup viewport
  priv->viewport = tidy_viewport_new ();
  ClutterActor* group = clutter_group_new();
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport), group);
                
  // Setup finger scroll
  priv->fingerScroll = tidy_finger_scroll_new(TIDY_FINGER_SCROLL_MODE_KINETIC);
  g_object_set (priv->fingerScroll, "decel-rate", 1.25, NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->fingerScroll), priv->viewport);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), priv->fingerScroll);
    
	priv->map = map_new(CHAMPLAIN_MAP_SOURCE_OPENSTREETMAP);//OPENSTREETMAP
	map_load(priv->map, 4);
  clutter_container_add_actor (CLUTTER_CONTAINER (group), priv->map->current_level->group);
  
  return GTK_WIDGET (view);
}
