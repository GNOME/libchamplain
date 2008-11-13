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

#include "config.h"

#include "champlaindefines.h"
#include "champlainprivate.h"
#include "champlain.h"
#include "champlain-marshal.h"
#include "champlainview.h"
#include "map.h"
#include "tile.h"
#include "zoomlevel.h"

#include <clutter/clutter.h>
#include <glib.h>
#include <glib-object.h>
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
  PROP_KEEP_CENTER_ON_RESIZE,
  PROP_SHOW_LICENSE
};

// static guint champlain_view_signals[LAST_SIGNAL] = { 0, };

#define CHAMPLAIN_VIEW_GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_VIEW, ChamplainViewPrivate))

struct _ChamplainViewPrivate
{
  ClutterActor *stage;

  ChamplainMapSource map_source;
  gint zoom_level; // only used when the zoom-level property is set before map is created

  // Represents the (lat, lon) at the center of the viewport
  gdouble longitude;
  gdouble latitude;

  ClutterActor *map_layer;
  ClutterActor *viewport;
  ClutterActor *finger_scroll;
  ChamplainRectangle viewport_size;
  ClutterActor *license_actor;

  ClutterActor *user_layers;

  Map *map;

  gboolean offline;
  gboolean keep_center_on_resize;
  gboolean show_license;
};

G_DEFINE_TYPE (ChamplainView, champlain_view, CLUTTER_TYPE_GROUP);

static gdouble viewport_get_current_longitude(ChamplainViewPrivate *priv);
static gdouble viewport_get_current_latitude(ChamplainViewPrivate *priv);
static gdouble viewport_get_longitude_at(ChamplainViewPrivate *priv, gint x);
static gdouble viewport_get_latitude_at(ChamplainViewPrivate *priv, gint y);
static gboolean scroll_event (ClutterActor *actor, ClutterScrollEvent *event, ChamplainView *view);
static void marker_reposition_cb (ChamplainMarker *marker, ChamplainView *view);
static void layer_reposition_cb (ClutterActor *layer, ChamplainView *view);
static void marker_reposition (ChamplainView *view);
static void create_initial_map(ChamplainView *view);
static void resize_viewport(ChamplainView *view);
static void champlain_view_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void champlain_view_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void champlain_view_finalize (GObject *object);
static void champlain_view_class_init (ChamplainViewClass *champlainViewClass);
static void champlain_view_init (ChamplainView *view);
static void viewport_x_changed_cb(GObject *gobject, GParamSpec *arg1, ChamplainView *view);
static void notify_marker_reposition_cb(ChamplainMarker *marker, GParamSpec *arg1, ChamplainView *view);
static void layer_add_marker_cb (ClutterGroup *layer, ChamplainMarker *marker, ChamplainView *view);
static void connect_marker_notify_cb (ChamplainMarker *marker, ChamplainView *view);
static gboolean finger_scroll_clicked (ClutterActor *actor, ClutterButtonEvent *event, ChamplainView *view);

static gdouble
viewport_get_longitude_at(ChamplainViewPrivate *priv, gint x)
{
  if (!priv->map)
    return 0.0;

  return priv->map->x_to_longitude(priv->map, x, priv->map->current_level->level);
}

static gdouble
viewport_get_current_longitude(ChamplainViewPrivate *priv)
{
  if (!priv->map)
    return 0.0;

  return viewport_get_longitude_at(priv,
    priv->map->current_level->anchor.x + priv->viewport_size.x + priv->viewport_size.width / 2.0);
}

static gdouble
viewport_get_latitude_at(ChamplainViewPrivate *priv, gint y)
{
  if (!priv->map)
    return 0.0;

  return priv->map->y_to_latitude(priv->map, y, priv->map->current_level->level);
}

static gdouble
viewport_get_current_latitude(ChamplainViewPrivate *priv)
{
  if (!priv->map)
    return 0.0;

  return viewport_get_latitude_at(priv,
    priv->map->current_level->anchor.y + priv->viewport_size.y + priv->viewport_size.height / 2.0);
}

static gboolean
scroll_event (ClutterActor *actor, ClutterScrollEvent *event, ChamplainView *view)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);
  ClutterActor *group = priv->map->current_level->group;
  gboolean success = FALSE;

  // Keep the lon, lat where the mouse is
  gdouble lon = viewport_get_longitude_at(priv,
    priv->viewport_size.x + event->x + priv->map->current_level->anchor.x);
  gdouble lat = viewport_get_latitude_at(priv, 
    priv->viewport_size.y + event->y + priv->map->current_level->anchor.y);

  // How far was it from the center of the viewport (in px)
  gint x_diff = priv->viewport_size.width / 2 - event->x;
  gint y_diff = priv->viewport_size.height / 2 - event->y;

  if (event->direction == CLUTTER_SCROLL_UP)
    success = map_zoom_in(priv->map);
  else if (event->direction == CLUTTER_SCROLL_DOWN) 
    success = map_zoom_out(priv->map);

  if (success)
    {
      // Get the new x,y in the new zoom level
      gint x2 = priv->map->longitude_to_x(priv->map, lon, priv->map->current_level->level);
      gint y2 = priv->map->latitude_to_y(priv->map, lat, priv->map->current_level->level);
      // Get the new lon,lat of these new x,y minus the distance from the viewport center
      gdouble lon2 = priv->map->x_to_longitude(priv->map, x2 + x_diff, priv->map->current_level->level);
      gdouble lat2 = priv->map->y_to_latitude(priv->map, y2 + y_diff, priv->map->current_level->level);

      resize_viewport(view);
      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->map_layer), group);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer), priv->map->current_level->group);
      champlain_view_center_on(view, lat2, lon2);

      g_object_notify(G_OBJECT(view), "zoom-level");
    }
  
  return success;
}

static void
marker_reposition_cb (ChamplainMarker *marker, ChamplainView *view)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);
  ChamplainMarkerPrivate *marker_priv = CHAMPLAIN_MARKER_GET_PRIVATE (marker);

  gint x, y;

  if(priv->map)
    {
      x = priv->map->longitude_to_x(priv->map, marker_priv->lon, priv->map->current_level->level);
      y = priv->map->latitude_to_y(priv->map, marker_priv->lat, priv->map->current_level->level);

      clutter_actor_set_position(CLUTTER_ACTOR(marker),
        x - priv->map->current_level->anchor.x,
        y - priv->map->current_level->anchor.y);
    }
}

static void
notify_marker_reposition_cb(ChamplainMarker *marker, GParamSpec *arg1, ChamplainView *view)
{
  marker_reposition_cb(marker, view);
}

static void
layer_add_marker_cb (ClutterGroup *layer, ChamplainMarker *marker, ChamplainView *view)
{
  g_signal_connect (marker,
                    "notify::longitude",
                    G_CALLBACK (notify_marker_reposition_cb),
                    view);
  marker_reposition(view);
}

static void
connect_marker_notify_cb (ChamplainMarker *marker, ChamplainView *view)
{
  g_signal_connect (marker,
                    "notify::longitude",
                    G_CALLBACK (notify_marker_reposition_cb),
                    view);
}

static void
layer_reposition_cb (ClutterActor *layer, ChamplainView *view)
{
  clutter_container_foreach(CLUTTER_CONTAINER(layer), CLUTTER_CALLBACK(marker_reposition_cb), view);
}

static void
marker_reposition (ChamplainView *view)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);
  clutter_container_foreach(CLUTTER_CONTAINER(priv->user_layers), CLUTTER_CALLBACK(layer_reposition_cb), view);
}

static void
create_initial_map(ChamplainView *view)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);
  priv->map = map_new(priv->map_source);
  map_load_level(priv->map, priv->zoom_level);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer), priv->map->current_level->group);

  marker_reposition(view);

  g_object_notify(G_OBJECT(view), "zoom-level");
  g_object_notify(G_OBJECT(view), "map-source");
}

static void
resize_viewport(ChamplainView *view)
{
  gdouble lower, upper;
  gboolean center = FALSE;
  TidyAdjustment *hadjust, *vadjust;

  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);

  if(!priv->map)
    {
      create_initial_map(view);
      center = TRUE;
    }

  clutter_actor_set_size (priv->finger_scroll, priv->viewport_size.width, priv->viewport_size.height);


  tidy_scrollable_get_adjustments (TIDY_SCROLLABLE (priv->viewport), &hadjust, &vadjust);

  if (priv->map->current_level->level < 8)
    {
      lower = -priv->viewport_size.width / 2;
      upper = zoom_level_get_width(priv->map->current_level) - priv->viewport_size.width / 2;
    }
  else
    {
      lower = 0;
      upper = G_MAXINT16;
    }
  g_object_set (hadjust, "lower", lower, "upper", upper,
                "step-increment", 1.0, "elastic", TRUE, NULL);

  if (priv->map->current_level->level < 8)
    {
      lower = -priv->viewport_size.height / 2;
      upper = zoom_level_get_height(priv->map->current_level) - priv->viewport_size.height / 2;
    }
  else
    {
      lower = 0;
      upper = G_MAXINT16;
    }
  g_object_set (vadjust, "lower", lower, "upper", upper,
                "step-increment", 1.0, "elastic", TRUE, NULL);

  if (center)
    {
      champlain_view_center_on(view, priv->latitude, priv->longitude);
    }
}

static void
champlain_view_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  ChamplainView *view = CHAMPLAIN_VIEW(object);
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
        g_value_set_int(value, priv->map_source);
        break;
      case PROP_OFFLINE:
        g_value_set_boolean(value, priv->offline);
        break;
      case PROP_DECEL_RATE:
        {
          gdouble decel;
          g_object_get (priv->finger_scroll, "decel-rate", decel, NULL);
          g_value_set_double(value, decel);
          break;
        }
      case PROP_KEEP_CENTER_ON_RESIZE:
        g_value_set_boolean(value, priv->keep_center_on_resize);
        break;
      case PROP_SHOW_LICENSE:
        g_value_set_boolean(value, priv->show_license);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
champlain_view_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  ChamplainView *view = CHAMPLAIN_VIEW(object);
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);

  switch(prop_id)
  {
    case PROP_LONGITUDE:
      {
        champlain_view_center_on (view, priv->latitude, g_value_get_double (value));
        break;
      }
    case PROP_LATITUDE:
      {
        champlain_view_center_on (view, g_value_get_double (value), priv->longitude);
        break;
      }
    case PROP_ZOOM_LEVEL:
      {
        gint level = g_value_get_int(value);
        if (priv->map)
          {
            if (level != priv->map->current_level->level)
              {
                ClutterActor *group = priv->map->current_level->group;
                if (map_zoom_to(priv->map, level))
                  {
                    resize_viewport(view);
                    clutter_container_remove_actor (CLUTTER_CONTAINER (priv->map_layer), group);
                    clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer), priv->map->current_level->group);
                    champlain_view_center_on(view, priv->latitude, priv->longitude);
                  }
              }
          }
        else
          {
            priv->zoom_level = level;
          }
        break;
      }
    case PROP_MAP_SOURCE:
      {
        ChamplainMapSource source = g_value_get_int(value);
        if (priv->map_source != source)
          {
            priv->map_source = source;
            if (priv->map) {
              guint currentLevel = priv->map->current_level->level;
              map_free(priv->map);
              priv->map = map_new(priv->map_source);

              // Keep same zoom level if the new map supports it
              if (currentLevel > priv->map->zoom_levels)
                {
                  currentLevel = priv->map->zoom_levels;
                  g_object_notify(G_OBJECT(view), "zoom-level");
                }

              map_load_level(priv->map, currentLevel);

              map_load_visible_tiles (priv->map, priv->viewport_size, priv->offline);
              clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer), priv->map->current_level->group);

              marker_reposition(view);
              champlain_view_center_on(view, priv->latitude, priv->longitude);
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
        g_object_set (priv->finger_scroll, "decel-rate", decel, NULL);
        break;
      }
    case PROP_KEEP_CENTER_ON_RESIZE:
      priv->keep_center_on_resize = g_value_get_boolean(value);
      break;
    case PROP_SHOW_LICENSE:
      priv->show_license = g_value_get_boolean(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}

static void
champlain_view_finalize (GObject *object)
{
  //ChamplainView *view = CHAMPLAIN_VIEW (object);
  //ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);

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
                                                   3,
                                                   CHAMPLAIN_PARAM_READWRITE));


  /**
  * ChamplainView:map-source:
  *
  * The #ChamplainMapSource being displayed
  *
  * Since: 0.2
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
  * ChamplainView:decel-rate:
  *
  * The deceleration rate for the kinetic mode.
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
                                                        1.0,
                                                        2.0,
                                                        1.1,
                                                        CHAMPLAIN_PARAM_READWRITE));
  /**
  * ChamplainView:keep-center-on-resize:
  *
  * Keep the current centered position when resizing the view.
  *
  * Since: 0.2.7
  */
  g_object_class_install_property (object_class,
                                   PROP_KEEP_CENTER_ON_RESIZE,
                                   g_param_spec_boolean ("keep-center-on-resize",
                                                         "Keep center on resize",
                                                         "Keep the current centered position "
                                                         "upon resizing",
                                                         TRUE,
                                                         CHAMPLAIN_PARAM_READWRITE));
  /**
  * ChamplainView:show-license:
  *
  * Show the license on the map view.  The license information should always be available 
  * in a way or another in your application.  You can have it in About, or on the map. 
  *
  * Since: 0.2.8
  */
  g_object_class_install_property (object_class,
                                   PROP_SHOW_LICENSE,
                                   g_param_spec_boolean ("show-license",
                                                         "Show the map data license",
                                                         "Show the map data license on the map view",
                                                         TRUE,
                                                         CHAMPLAIN_PARAM_READWRITE));
}

static void
champlain_view_init (ChamplainView *view)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);

  priv->map_source = CHAMPLAIN_MAP_SOURCE_OPENSTREETMAP;
  priv->zoom_level = 3;
  priv->offline = FALSE;
  priv->keep_center_on_resize = TRUE;
}

static void
viewport_x_changed_cb(GObject *gobject, GParamSpec *arg1, ChamplainView *view)
{
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);

  ChamplainPoint rect;
  tidy_viewport_get_origin(TIDY_VIEWPORT(priv->viewport), &rect.x, &rect.y, NULL);

  if (rect.x == priv->viewport_size.x &&
      rect.y == priv->viewport_size.y)
      return;

  priv->viewport_size.x = rect.x;
  priv->viewport_size.y = rect.y;

  map_load_visible_tiles (priv->map, priv->viewport_size, priv->offline);

  g_object_notify(G_OBJECT(view), "longitude");
  g_object_notify(G_OBJECT(view), "latitude");
}

void
champlain_view_set_size (ChamplainView *view, guint width, guint height)
{
  g_return_if_fail(CHAMPLAIN_IS_VIEW(view));

  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);
  
  priv->viewport_size.width = width;
  priv->viewport_size.height = height;

  resize_viewport(view);
  if (priv->keep_center_on_resize)
    champlain_view_center_on(view, priv->latitude, priv->longitude);
  else
    map_load_visible_tiles (priv->map, priv->viewport_size, priv->offline);
}

static gboolean
finger_scroll_clicked (ClutterActor *actor,
                  ClutterButtonEvent *event,
                  ChamplainView *view)
{	
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);

  if (event->button == 1 && event->click_count == 2)
    {
      ClutterActor *group = priv->map->current_level->group;
      // Keep the lon, lat where the mouse is
      gdouble lon = viewport_get_longitude_at(priv,
        priv->viewport_size.x + event->x + priv->map->current_level->anchor.x);
      gdouble lat = viewport_get_latitude_at(priv, 
        priv->viewport_size.y + event->y + priv->map->current_level->anchor.y);
      
      // How far was it from the center of the viewport (in px)
      gint x_diff = priv->viewport_size.width / 2 - event->x;
      gint y_diff = priv->viewport_size.height / 2 - event->y;

      if (map_zoom_in (priv->map))
        {
          // Get the new x,y in the new zoom level
          gint x2 = priv->map->longitude_to_x(priv->map, lon, priv->map->current_level->level);
          gint y2 = priv->map->latitude_to_y(priv->map, lat, priv->map->current_level->level);
          // Get the new lon,lat of these new x,y minus the distance from the viewport center
          gdouble lon2 = priv->map->x_to_longitude(priv->map, x2 + x_diff, priv->map->current_level->level);
          gdouble lat2 = priv->map->y_to_latitude(priv->map, y2 + y_diff, priv->map->current_level->level);

          resize_viewport(view);
          clutter_container_remove_actor (CLUTTER_CONTAINER (priv->map_layer), group);
          clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer), priv->map->current_level->group);
          champlain_view_center_on(view, lat2, lon2);

          g_object_notify(G_OBJECT(view), "zoom-level");
        }
      champlain_view_center_on (view, lat, lon);
      return TRUE;
    }
  return FALSE; // Propagate the event
}

/**
 * champlain_view_new:
 * @mode: a #ChamplainViewMode, the scrolling mode
 * Returns a new #ChamplainView ready to be used as a #ClutterActor.
 *
 * Since: 0.1
 */
ClutterActor *
champlain_view_new (ChamplainViewMode mode)
{
  ChamplainView *view;

  view = CHAMPLAIN_VIEW (g_object_new (CHAMPLAIN_TYPE_VIEW, NULL));
  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);

  priv->stage = clutter_group_new ();
  

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

  priv->finger_scroll = tidy_finger_scroll_new(mode);

  g_signal_connect (priv->finger_scroll,
                    "scroll-event",
                    G_CALLBACK (scroll_event),
                    view);
                    
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->finger_scroll), priv->viewport);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->stage), priv->finger_scroll);
  clutter_container_add_actor (CLUTTER_CONTAINER (view), priv->stage);

  // Map Layer
  priv->map_layer = clutter_group_new();
  clutter_actor_show(priv->map_layer);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport), priv->map_layer);

  g_signal_connect (priv->finger_scroll,
                    "button-press-event",
                    G_CALLBACK (finger_scroll_clicked),
                    view);
 
  // License layer 
  priv->license_actor = clutter_group_new();
  clutter_actor_show(priv->license_actor);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->stage), priv->license_actor);
  clutter_actor_raise_top (priv->license_actor);

  // Setup user_layers
  priv->user_layers = clutter_group_new();
  clutter_actor_show(priv->user_layers);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport), priv->user_layers);
  clutter_actor_raise(priv->user_layers, priv->map_layer);

  return CLUTTER_ACTOR (view);
}

/**
 * champlain_view_center_on:
 * @view: a #ChamplainView
 * @latitude: the longitude to center the map at
 * @longitude: the longitude to center the map at
 *
 * Centers the map on these coordinates.
 *
 * Since: 0.1
 */
// FIXME: Animate this.  Can be done in Tidy-Adjustment (like for elastic effect)
void
champlain_view_center_on (ChamplainView *view, gdouble latitude, gdouble longitude)
{
  g_return_if_fail(CHAMPLAIN_IS_VIEW(view));

  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);

  priv->longitude = longitude;
  priv->latitude = latitude;

  if(!priv->map)
    return;

  gdouble x, y;
  x = priv->map->longitude_to_x(priv->map, longitude, priv->map->current_level->level);
  y = priv->map->latitude_to_y(priv->map, latitude, priv->map->current_level->level);
  ChamplainPoint* anchor = &priv->map->current_level->anchor;

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

  guint i;
  for (i = 0; i < priv->map->current_level->tiles->len; i++)
    {
      Tile *tile = g_ptr_array_index(priv->map->current_level->tiles, i);
      if (!tile->loading)
        tile_set_position(priv->map, tile);
    }

  tidy_viewport_set_origin(TIDY_VIEWPORT(priv->viewport),
    x - priv->viewport_size.width / 2.0,
    y - priv->viewport_size.height / 2.0,
    0);

  g_object_notify(G_OBJECT(view), "longitude");
  g_object_notify(G_OBJECT(view), "latitude");

  map_load_visible_tiles (priv->map, priv->viewport_size, priv->offline);
  marker_reposition (view);
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
champlain_view_zoom_in (ChamplainView *view)
{
  g_return_if_fail(CHAMPLAIN_IS_VIEW(view));

  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);
  ClutterActor *group = priv->map->current_level->group;

  if(map_zoom_in(priv->map))
    {
      resize_viewport(view);
      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->map_layer), group);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer), priv->map->current_level->group);
      champlain_view_center_on(view, priv->latitude, priv->longitude);

      g_object_notify(G_OBJECT(view), "zoom-level");
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
champlain_view_zoom_out (ChamplainView *view)
{
  g_return_if_fail(CHAMPLAIN_IS_VIEW(view));

  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);
  ClutterActor *group = priv->map->current_level->group;

  if(map_zoom_out(priv->map))
    {
      resize_viewport(view);
      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->map_layer), group);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer), priv->map->current_level->group);
      champlain_view_center_on(view, priv->latitude, priv->longitude);

      g_object_notify(G_OBJECT(view), "zoom-level");
    }
}

/**
 * champlain_view_add_layer:
 * @view: a #ChamplainView
 * @layer: a #ClutterActor
 *
 * Adds a new layer to the view
 *
 * Since: 0.2
 */
void
champlain_view_add_layer (ChamplainView *view, ClutterActor *layer)
{
  g_return_if_fail(CHAMPLAIN_IS_VIEW(view));
  g_return_if_fail(CLUTTER_IS_ACTOR(layer));

  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW_GET_PRIVATE (view);
  clutter_container_add(CLUTTER_CONTAINER(priv->user_layers), layer, NULL);
  clutter_actor_raise_top(layer);

  if(priv->map)
    marker_reposition(view);

  g_signal_connect_after (layer,
                    "add",
                    G_CALLBACK (layer_add_marker_cb),
                    view);

  clutter_container_foreach(CLUTTER_CONTAINER(layer), CLUTTER_CALLBACK(connect_marker_notify_cb), view);
}
