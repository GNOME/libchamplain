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
#include "champlain.h"
#include "champlain-marshal.h"
#include "champlainview.h"
#include "map.h"
#include "tile.h"
#include "zoomlevel.h"

#include <clutter/clutter.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk-clutter-embed.h>
#include <math.h>
#include <tidy-adjustment.h>
#include <tidy-finger-scroll.h>
#include <tidy-scrollable.h>
#include <tidy-viewport.h>



enum
{
  /* normal signals */
  SIGNAL_TBD,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_LONGITUDE,
  PROP_LATITUDE,
  PROP_ZOOM_LEVEL,
  PROP_MAP_SOURCE,
  PROP_OFFLINE,
  PROP_DECEL_RATE,
};

static guint champlain_view_signals[LAST_SIGNAL] = { 0, };

#define CHAMPLAIN_VIEW_GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_VIEW, ChamplainViewPrivate))

#define CHAMPLAIN_PARAM_READABLE     \
        (G_PARAM_READABLE |     \
         G_PARAM_STATIC_NICK | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB)

#define CHAMPLAIN_PARAM_READWRITE    \
        (G_PARAM_READABLE | G_PARAM_WRITABLE | \
         G_PARAM_STATIC_NICK | G_PARAM_STATIC_NAME | G_PARAM_STATIC_BLURB)

struct _ChamplainViewPrivate
{
  GtkWidget *clutterEmbed;
  
  ChamplainMapSource mapSource;
  gint zoomLevel; // only used when the zoom-level property is set before map is created
  
  ClutterActor *viewport;
  ClutterActor *fingerScroll;
  GdkRectangle viewportSize;
  Map *map;

  gboolean offline;
};


G_DEFINE_TYPE (ChamplainView, champlain_view, GTK_TYPE_ALIGNMENT);

static gdouble
viewport_get_current_longitude(ChamplainViewPrivate *priv)
{
  return priv->map->x_to_longitude(priv->map, 
    priv->map->current_level->anchor.x + priv->viewportSize.x + priv->viewportSize.width / 2.0, 
    priv->map->current_level->level);
}

static gdouble
viewport_get_current_latitude(ChamplainViewPrivate *priv)
{
  return priv->map->y_to_latitude(priv->map, 
    priv->map->current_level->anchor.y + priv->viewportSize.y + priv->viewportSize.height / 2.0, 
    priv->map->current_level->level);
}

static void
resize_viewport(ChamplainView *champlainView)
{
  gdouble lower, upper;
  TidyAdjustment *hadjust, *vadjust;
  
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);
  
  if(priv->map == NULL)
    {
      priv->map = map_new(priv->mapSource);
      map_load_level(priv->map, priv->zoomLevel);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport), priv->map->current_level->group);
      
      g_object_notify(G_OBJECT(champlainView), "zoom-level");
      g_object_notify(G_OBJECT(champlainView), "map-source");
    }
  
  clutter_actor_set_size (priv->fingerScroll, priv->viewportSize.width, priv->viewportSize.height);
  
  
  tidy_scrollable_get_adjustments (TIDY_SCROLLABLE (priv->viewport), &hadjust, &vadjust);
  
  //tidy_adjustment_get_values (hadjust, NULL, &lower, &upper, NULL, NULL, NULL);
  if (priv->map->current_level->level < 8)
    {
      lower = -priv->viewportSize.width / 2;
      upper = zoom_level_get_width(priv->map->current_level) - priv->viewportSize.width / 2;
    }
  else
    {
      lower = 0;
      upper = G_MAXINT16;
    }
  g_object_set (hadjust, "lower", lower, "upper", upper,
                "step-increment", 1.0, "elastic", TRUE, NULL);
                
  //tidy_adjustment_get_values (vadjust, NULL, &lower, &upper, NULL, NULL, NULL);
  
  if (priv->map->current_level->level < 8)
    {
      lower = -priv->viewportSize.height / 2; 
      upper = zoom_level_get_height(priv->map->current_level) - priv->viewportSize.height / 2;
    }
  else
    {
      lower = 0;
      upper = G_MAXINT16;
    }
  g_object_set (vadjust, "lower", lower, "upper", upper,
                "step-increment", 1.0, "elastic", TRUE, NULL);
}

static void 
champlain_view_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec)
{
    ChamplainView* view = CHAMPLAIN_VIEW(object);
    ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);

    switch(prop_id) 
      {
        case PROP_LONGITUDE:
          g_value_set_double(value, viewport_get_current_longitude(priv));
          break;
        case PROP_LATITUDE:
          g_value_set_double(value, viewport_get_current_latitude(priv));
          break;
        case PROP_ZOOM_LEVEL:
          {
            if (priv->map) 
              {
                g_value_set_int(value, priv->map->current_level->level);
              }
            else
              {
                g_value_set_int(value, 0);
              }
            break;
          }
        case PROP_MAP_SOURCE:
          g_value_set_int(value, priv->mapSource);
          break;
        case PROP_OFFLINE:
          g_value_set_boolean(value, priv->offline);
          break;
        case PROP_DECEL_RATE:
          {
            gdouble decel;
            g_object_get (priv->fingerScroll, "decel-rate", decel, NULL);
            g_value_set_double(value, decel);
            break;
          }
        default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      }
}

static void 
champlain_view_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec *pspec)
{
    ChamplainView* view = CHAMPLAIN_VIEW(object);
    ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);

    switch(prop_id) 
    {
      case PROP_LONGITUDE:
        {
          gdouble lon = g_value_get_double(value);
          gdouble lat = viewport_get_current_latitude(priv);
          champlain_view_center_on(view, lon, lat);
          break;
        }
      case PROP_LATITUDE:
        {
          gdouble lon = viewport_get_current_longitude(priv);
          gdouble lat = g_value_get_double(value);
          champlain_view_center_on(view, lon, lat);
          break;
        }
      case PROP_ZOOM_LEVEL:
        {
          gint level = g_value_get_int(value);
          if (priv->map) 
            {
              if (level != priv->map->current_level->level) 
                {
                  ClutterActor * group = priv->map->current_level->group;
                  gdouble lon = viewport_get_current_longitude(priv);
                  gdouble lat = viewport_get_current_latitude(priv);
                  if (map_zoom_to(priv->map, level)) 
                    {
                      resize_viewport(view);
                      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->viewport), group);
                      clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport), priv->map->current_level->group);
                      champlain_view_center_on(view, lon, lat);
                    }
                }
            }
          else 
            {
              priv->zoomLevel = level;
            }
          break;
        }
      case PROP_MAP_SOURCE:
        {
          ChamplainMapSource source = g_value_get_int(value);
          if (priv->mapSource != source)
            {
              priv->mapSource = source;
              if (priv->map) {
                gint currentLevel = priv->map->current_level->level;
                GdkPoint anchor = priv->map->current_level->anchor;
                map_free(priv->map);
                priv->map = map_new(priv->mapSource);
                
                // Keep same zoom level
                if (currentLevel > priv->map->zoom_levels)
                  currentLevel = priv->map->zoom_levels;
                  
                map_load_level(priv->map, currentLevel);
                priv->map->current_level->anchor = anchor;

                map_load_visible_tiles (priv->map, priv->viewportSize, priv->offline);
                clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport), priv->map->current_level->group);
              }
            }
          break;
        }
      case PROP_OFFLINE:
        priv->offline = g_value_get_boolean(value);
        break;
      case PROP_DECEL_RATE:
        {
          gdouble decel = g_value_get_double(value);
          g_object_set (priv->fingerScroll, "decel-rate", decel, NULL);
          break;
        }
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

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

  GObjectClass *object_class = G_OBJECT_CLASS (champlainViewClass);
  object_class->finalize = champlain_view_finalize;
  object_class->get_property = champlain_view_get_property;
  object_class->set_property = champlain_view_set_property;
  
  /**
  * ChamplainView:longitude:
  *
  * The longitude coordonate of the map
  *
  * Since: 0.1
  */
  g_object_class_install_property(object_class, PROP_LONGITUDE,
                                  g_param_spec_float("longitude",
                                                     "Longitude",
                                                     "The longitude coordonate of the map",
                                                     -180.0f,
                                                     180.0f,
                                                     0.0f,
                                                     CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainView:latitude:
  *
  * The latitude coordonate of the map
  *
  * Since: 0.1
  */
  g_object_class_install_property(object_class, PROP_LATITUDE,
                                  g_param_spec_float("latitude",
                                                     "Latitude",
                                                     "The latitude coordonate of the map",
                                                     -90.0f,
                                                     90.0f,
                                                     0.0f,
                                                     CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainView:zoom-level:
  *
  * The level of zoom of the content.
  *
  * Since: 0.1
  */
  g_object_class_install_property(object_class, PROP_ZOOM_LEVEL,
                                  g_param_spec_int("zoom-level",
                                                   "Zoom level",
                                                   "The level of zoom of the map",
                                                   0,
                                                   20,
                                                   1.0f,
                                                   CHAMPLAIN_PARAM_READWRITE));


  /**
  * ChamplainView:zoom-level:
  *
  * The level of zoom of the content.
  *
  * Since: 0.1
  */
  g_object_class_install_property(object_class, PROP_MAP_SOURCE,
                                  g_param_spec_int("map-source",
                                                   "Map source",
                                                   "The map source being displayed",
                                                   0,
                                                   CHAMPLAIN_MAP_SOURCE_COUNT,
                                                   CHAMPLAIN_MAP_SOURCE_OPENSTREETMAP,
                                                   CHAMPLAIN_PARAM_READWRITE)); 

  /**
  * ChamplainView:offline:
  *
  * If true, will fetch tiles from the Internet, otherwise, will only use cached content.
  *
  * Since: 0.2
  */
  g_object_class_install_property(object_class, PROP_OFFLINE,
                                  g_param_spec_boolean("offline",
                                                       "Offline Mode",
                                                       "If viewer is in offline mode.",
                                                       FALSE,
                                                       CHAMPLAIN_PARAM_READWRITE)); 

  /**
  * ChamplainView:offline:
  *
  * If true, will fetch tiles from the Internet, otherwise, will only use cached content.
  *
  * Since: 0.2
  */
  g_object_class_install_property (object_class,
                                   PROP_DECEL_RATE,
                                   g_param_spec_double ("decel-rate",
                                                        "Deceleration rate",
                                                        "Rate at which the view "
                                                        "will decelerate in "
                                                        "kinetic mode.",
                                                        CLUTTER_FIXED_TO_FLOAT (CFX_ONE + CFX_MIN),
                                                        CLUTTER_FIXED_TO_FLOAT (CFX_MAX),
                                                        1.1,
                                                        G_PARAM_READWRITE));
}

static void
champlain_view_init (ChamplainView *champlainView)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);

  priv->mapSource = CHAMPLAIN_MAP_SOURCE_OPENSTREETMAP;
  priv->zoomLevel = 0;
  priv->offline = FALSE;
}

static void 
viewport_x_changed_cb(GObject *gobject, GParamSpec *arg1, ChamplainView *champlainView)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);
  
  GdkPoint rect;
  tidy_viewport_get_origin(TIDY_VIEWPORT(priv->viewport), &rect.x, &rect.y, NULL);
  
  if (rect.x == priv->viewportSize.x &&
      rect.y == priv->viewportSize.y)
      return;

  priv->viewportSize.x = rect.x;
  priv->viewportSize.y = rect.y;
  
  map_load_visible_tiles (priv->map, priv->viewportSize, priv->offline);
  
  g_object_notify(G_OBJECT(champlainView), "longitude");
  g_object_notify(G_OBJECT(champlainView), "latitude");
}

static void
view_size_allocated_cb (GtkWidget *view, GtkAllocation *allocation, ChamplainView *champlainView) 
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);
  
  priv->viewportSize.width = allocation->width;
  priv->viewportSize.height = allocation->height;
  
  resize_viewport(champlainView);
  map_load_visible_tiles (priv->map, priv->viewportSize, priv->offline);
}

/**
 * champlain_view_new:
 *
 * Returns a new #ChamplainWidget ready to be used as a #GtkWidget.
 *
 * Since: 0.1
 */
GtkWidget *
champlain_view_new (ChamplainViewMode mode)
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
  g_object_set (G_OBJECT (priv->viewport), "sync-adjustments", FALSE, NULL);
  
  g_signal_connect (priv->viewport,
                    "notify::x-origin",
                    G_CALLBACK (viewport_x_changed_cb),
                    view);

  // Setup finger scroll
  TidyFingerScrollMode tidy_mode;
  switch(mode)
    {
      case CHAMPLAIN_VIEW_MODE_PUSH:
        tidy_mode = TIDY_FINGER_SCROLL_MODE_PUSH;
        break;
      case CHAMPLAIN_VIEW_MODE_KINETIC:
        tidy_mode = TIDY_FINGER_SCROLL_MODE_KINETIC;
        break;
    }

  priv->fingerScroll = tidy_finger_scroll_new(mode);

  clutter_container_add_actor (CLUTTER_CONTAINER (priv->fingerScroll), priv->viewport);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), priv->fingerScroll);

  
  return GTK_WIDGET (view);
}

/**
 * champlain_view_center_on:
 * @view: a #ChamplainView
 * @longitude: the longitude to center the map at
 * @latitude: the longitude to center the map at
 *
 * Centers the map on these coordinates.
 *
 * Since: 0.1
 */
// FIXME: Animate this.  Can be done in Tidy-Adjustment (like for elastic effect)
void
champlain_view_center_on (ChamplainView *champlainView, gdouble longitude, gdouble latitude)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);

  gdouble x, y;
  x = priv->map->longitude_to_x(priv->map, longitude, priv->map->current_level->level);
  y = priv->map->latitude_to_y(priv->map, latitude, priv->map->current_level->level);
  GdkPoint* anchor = &priv->map->current_level->anchor;

  if (priv->map->current_level->level >= 8) {
    anchor->x = x - G_MAXINT16 / 2;
    anchor->y = y - G_MAXINT16 / 2;

    if ( anchor->x < 0 )
      anchor->x = 0;
    if ( anchor->y < 0 )
      anchor->y = 0;

    gdouble max = zoom_level_get_width(priv->map->current_level) - (G_MAXINT16 / 2);
    if (anchor->x > max)
      anchor->x = max;
    if (anchor->y > max)
      anchor->y = max;
    
    x -= anchor->x;
    y -= anchor->y;
  } else {
    anchor->x = 0;
    anchor->y = 0;
  }

  //FIXME: Inform tiles that there is a new anchor
  int i;
  for (i = 0; i < priv->map->current_level->tiles->len; i++)
    {
      Tile* tile = g_ptr_array_index(priv->map->current_level->tiles, i);
      if (!tile->loading)
        tile_set_position(priv->map, tile);
    }

  tidy_viewport_set_origin(TIDY_VIEWPORT(priv->viewport),
    x - priv->viewportSize.width / 2.0,
    y - priv->viewportSize.height / 2.0,
    0);
  
  g_object_notify(G_OBJECT(champlainView), "longitude");
  g_object_notify(G_OBJECT(champlainView), "latitude");
  
  map_load_visible_tiles (priv->map, priv->viewportSize, priv->offline);
}

/**
 * champlain_view_zoom_in:
 * @view: a #ChamplainView
 *
 * Zoom in the map by one level.
 *
 * Since: 0.1
 */
void
champlain_view_zoom_in (ChamplainView *champlainView)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);
  ClutterActor * group = priv->map->current_level->group;
  gdouble lon = viewport_get_current_longitude(priv);
  gdouble lat = viewport_get_current_latitude(priv);
  if(map_zoom_in(priv->map)) 
    {
      resize_viewport(champlainView);
      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->viewport), group);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport), priv->map->current_level->group);
      champlain_view_center_on(champlainView, lon, lat);
      
      g_object_notify(G_OBJECT(champlainView), "zoom-level");
    }
}

/**
 * champlain_view_zoom_out:
 * @view: a #ChamplainView
 *
 * Zoom out the map by one level.
 *
 * Since: 0.1
 */
void
champlain_view_zoom_out (ChamplainView *champlainView)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (champlainView);
  ClutterActor * group = priv->map->current_level->group;
  gdouble lon = viewport_get_current_longitude(priv);
  gdouble lat = viewport_get_current_latitude(priv);
  if(map_zoom_out(priv->map))
    {
      resize_viewport(champlainView);
      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->viewport), group);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport), priv->map->current_level->group);
      champlain_view_center_on(champlainView, lon, lat);
      
      g_object_notify(G_OBJECT(champlainView), "zoom-level");
    }
}
