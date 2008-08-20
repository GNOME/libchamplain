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

#include "champlain.h"
#include "champlain_defines.h"
#include "champlainview.h"
#include "tile.h"
#include "map.h"
#include "zoomlevel.h"
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

struct _ChamplainViewPrivate
{
  GtkWidget *clutterEmbed;
  ClutterActor *viewport;
  ClutterActor *fingerScroll;
  ChamplainRect viewportSize;
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

void viewport_x_changed_cb(GObject    *gobject,
                           GParamSpec *arg1,
                           ChamplainView *champlainView)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);
  
  ChamplainRect rect;
  tidy_viewport_get_origin(TIDY_VIEWPORT(priv->viewport), &rect.x, &rect.y, NULL);
  if (rect.x < 0 || rect.y < 0)
      return;
  if (rect.x == priv->viewportSize.x &&
      rect.y == priv->viewportSize.y &&
      rect.width == priv->viewportSize.width &&
      rect.height == priv->viewportSize.height)
      return;
  priv->viewportSize.x = rect.x;
  priv->viewportSize.y = rect.y;
  
  map_load_visible_tiles (priv->map, priv->viewportSize);
}

static void
resize_viewport(ChamplainView *champlainView)
{
  gdouble lower, upper;
  TidyAdjustment *hadjust, *vadjust;
  
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);
  
  clutter_actor_set_size (priv->fingerScroll, priv->viewportSize.width, priv->viewportSize.height);
  
  g_object_set (G_OBJECT (priv->viewport), "sync-adjustments", FALSE, NULL);
  
  tidy_scrollable_get_adjustments (TIDY_SCROLLABLE (priv->viewport), &hadjust, &vadjust);
  
  tidy_adjustment_get_values (hadjust, NULL, &lower, &upper, NULL, NULL, NULL);
  lower = 0;
  upper = zoom_level_get_width(priv->map->current_level) - priv->viewportSize.width; 
  g_object_set (hadjust, "lower", lower, "upper", upper,
                "step-increment", 1.0, "elastic", TRUE, NULL);
                
  tidy_adjustment_get_values (vadjust, NULL, &lower, &upper, NULL, NULL, NULL);
  lower = 0;
  upper = zoom_level_get_height(priv->map->current_level) - priv->viewportSize.height;
  g_object_set (vadjust, "lower", lower, "upper", upper,
                "step-increment", 1.0, "elastic", TRUE, NULL);
  
  g_print("%d, %d, %d\n", zoom_level_get_width(priv->map->current_level), zoom_level_get_height(priv->map->current_level), sizeof(guint));
}

static void
view_size_allocated_cb (GtkWidget *view, GtkAllocation *allocation, ChamplainView *champlainView) 
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);
  
  priv->viewportSize.width = allocation->width;
  priv->viewportSize.height = allocation->height;
  
  resize_viewport(champlainView);
  map_load_visible_tiles (priv->map, priv->viewportSize);
}
                          
GtkWidget *
champlain_view_new ()
{
  ClutterColor stage_color = { 0x34, 0x39, 0x39, 0xff };
  ChamplainView *view;
  ClutterActor *stage; 
  
  view = CHAMPLAIN_VIEW (g_object_new (CHAMPLAIN_TYPE_VIEW, NULL));
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);
  
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
  
  g_signal_connect (priv->viewport,
                    "notify::x-origin",
                    G_CALLBACK (viewport_x_changed_cb),
                    view);

  // Setup finger scroll
  priv->fingerScroll = tidy_finger_scroll_new(TIDY_FINGER_SCROLL_MODE_KINETIC);
  g_object_set (priv->fingerScroll, "decel-rate", 1.25, NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->fingerScroll), priv->viewport);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), priv->fingerScroll);

  priv->map = map_new(CHAMPLAIN_MAP_SOURCE_OPENSTREETMAP);//OPENSTREETMAP
  map_load_level(priv->map, 2);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport), priv->map->current_level->group);
  return GTK_WIDGET (view);
}

// FIXME: Animate this.  Can be done in Tidy-Adjustment (like for elastic effect)
void
champlain_view_center_on (ChamplainView *champlainView, gdouble longitude, gdouble latitude)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);

  gdouble x, y;
  x = priv->map->longitude_to_x(priv->map, longitude, priv->map->current_level->level);
  y = priv->map->latitude_to_y(priv->map, latitude, priv->map->current_level->level);

  tidy_viewport_set_origin(TIDY_VIEWPORT(priv->viewport), x - priv->viewportSize.width/2.0, y - priv->viewportSize.height/2.0, 0);
}

void
champlain_view_zoom_in (ChamplainView *champlainView)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);
  ClutterActor * group = priv->map->current_level->group;
  if(map_zoom_in(priv->map)) 
    {
      gint level = priv->map->current_level->level;
      g_print("Zoom: %d\n", level);
      gdouble lon = priv->map->x_to_longitude(priv->map, priv->viewportSize.x + priv->viewportSize.width/2.0, level);
      gdouble lat = priv->map->y_to_latitude(priv->map, priv->viewportSize.y + priv->viewportSize.height/2.0, level);
      level++;
      gdouble x = priv->map->longitude_to_x(priv->map, lon, level);
      gdouble y = priv->map->latitude_to_y(priv->map, lat, level);
      
      resize_viewport(champlainView);
      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->viewport), group);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport), priv->map->current_level->group);
      
      tidy_viewport_set_origin(TIDY_VIEWPORT(priv->viewport), x - priv->viewportSize.width/2.0, y - priv->viewportSize.height/2.0, 0);
    }
}

void
champlain_view_zoom_out (ChamplainView *champlainView)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);
  ClutterActor * group = priv->map->current_level->group;
  if(map_zoom_out(priv->map)) 
    {
      gint level = priv->map->current_level->level;
      g_print("Zoom: %d\n", level);
      gdouble lon = priv->map->x_to_longitude(priv->map, priv->viewportSize.x + priv->viewportSize.width/2.0, level);
      gdouble lat = priv->map->y_to_latitude(priv->map, priv->viewportSize.y + priv->viewportSize.height/2.0, level);
      level--;
      gdouble x = priv->map->longitude_to_x(priv->map, lon, level);
      gdouble y = priv->map->latitude_to_y(priv->map, lat, level);
      
      resize_viewport(champlainView);
      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->viewport), group);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport), priv->map->current_level->group);
      
      tidy_viewport_set_origin(TIDY_VIEWPORT(priv->viewport), x - priv->viewportSize.width/2.0, y - priv->viewportSize.height/2.0, 0);
    }
}
