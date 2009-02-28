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

/**
 * SECTION:champlainview
 * @short_description: A #ClutterActor to display maps
 *
 * The #ChamplainView is a ClutterActor to display maps.  It supports two modes
 * of scrolling:
 * <itemizedlist>
 *   <listitem><para>Push: the normal behavior where the maps doesn't move
 *   after the user stopped scrolling;</para></listitem>
 *   <listitem><para>Kinetic: the iPhone-like behavior where the maps
 *   decelerate after the user stopped scrolling.</para></listitem>
 * </itemizedlist>
 *
 * You can use the same #ChamplainView to display many types of maps.  In
 * Champlain they are called map sources.  You can change the #map-source
 * property at anytime to replace the current displayed map.
 *
 * The maps are downloaded from Internet from open maps sources (like
 * <ulink role="online-location"
 * url="http://www.openstreetmap.org">OpenStreetMap</ulink>).  Maps are divided
 * in tiles for each zoom level.  When a tile is requested, #ChamplainView will
 * first check if it is in cache (in the user's cache dir under champlain). If
 * an error occurs during download, an error tile will be displayed (if not in
 * offline mode).
 *
 * The button-press-event and button-release-event signals are emitted each
 * time a mouse button is pressed on the @view.  Coordinates can be converted
 * with #champlain_view_get_coords_from_event.
 */

#include "config.h"

#include "champlain-view.h"

#include "champlain.h"
#include "champlain-defines.h"
#include "champlain-enum-types.h"
#include "champlain-map.h"
#include "champlain-marshal.h"
#include "champlain-map-source.h"
#include "champlain-private.h"
#include "champlain-tile.h"
#include "champlain-zoom-level.h"

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
  PROP_SCROLL_MODE,
  PROP_KEEP_CENTER_ON_RESIZE,
  PROP_SHOW_LICENSE
};

#define PADDING 10
/*static guint signals[LAST_SIGNAL] = { 0, }; */

#define GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CHAMPLAIN_TYPE_VIEW, ChamplainViewPrivate))

struct _ChamplainViewPrivate
{
  ClutterActor *stage;

  ChamplainMapSource *map_source;
  ChamplainScrollMode scroll_mode;
  gint zoom_level; /* Holds the current zoom level number */

  /* Represents the (lat, lon) at the center of the viewport */
  gdouble longitude;
  gdouble latitude;

  /* Hack to get smaller x,y coordinates as the clutter limit is G_MAXINT16 */
  ChamplainPoint anchor;

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

static gdouble viewport_get_current_longitude (ChamplainViewPrivate *priv);
static gdouble viewport_get_current_latitude (ChamplainViewPrivate *priv);
static gdouble viewport_get_longitude_at (ChamplainViewPrivate *priv, gint x);
static gdouble viewport_get_latitude_at (ChamplainViewPrivate *priv, gint y);
static gboolean scroll_event (ClutterActor *actor, ClutterScrollEvent *event,
    ChamplainView *view);
static void marker_reposition_cb (ChamplainMarker *marker, ChamplainView *view);
static void layer_reposition_cb (ClutterActor *layer, ChamplainView *view);
static gboolean marker_reposition (gpointer data);
static void create_initial_map (ChamplainView *view);
static void resize_viewport (ChamplainView *view);
static void champlain_view_get_property (GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec);
static void champlain_view_set_property (GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec);
static void champlain_view_finalize (GObject *object);
static void champlain_view_class_init (ChamplainViewClass *champlainViewClass);
static void champlain_view_init (ChamplainView *view);
static void viewport_x_changed_cb (GObject *gobject, GParamSpec *arg1,
    ChamplainView *view);
static void notify_marker_reposition_cb (ChamplainMarker *marker,
    GParamSpec *arg1, ChamplainView *view);
static void layer_add_marker_cb (ClutterGroup *layer, ChamplainMarker *marker,
    ChamplainView *view);
static void connect_marker_notify_cb (ChamplainMarker *marker,
    ChamplainView *view);
static gboolean finger_scroll_button_press_cb (ClutterActor *actor,
    ClutterButtonEvent *event, ChamplainView *view);
static void update_license (ChamplainView *view);
static void license_set_position (ChamplainView *view);
static void view_load_visible_tiles (ChamplainView *view);
static void view_position_tile (ChamplainView* view, ChamplainTile* tile);
static void view_tiles_reposition (ChamplainView* view);

static gdouble
viewport_get_longitude_at (ChamplainViewPrivate *priv, gint x)
{
  gint level;

  if (!priv->map_source)
    return 0.0;

  return champlain_map_source_get_longitude (priv->map_source,
      priv->zoom_level, x);
}

static gdouble
viewport_get_current_longitude (ChamplainViewPrivate *priv)
{
  if (!priv->map)
    return 0.0;

  return viewport_get_longitude_at (priv, priv->anchor.x +
      priv->viewport_size.x + priv->viewport_size.width / 2.0);
}

static gdouble
viewport_get_latitude_at (ChamplainViewPrivate *priv, gint y)
{
  gint level;

  if (!priv->map_source)
    return 0.0;

  return champlain_map_source_get_latitude (priv->map_source,
      priv->zoom_level, y);
}

static gdouble
viewport_get_current_latitude (ChamplainViewPrivate *priv)
{
  if (!priv->map)
    return 0.0;

  return viewport_get_latitude_at (priv,
      priv->anchor.y + priv->viewport_size.y +
      priv->viewport_size.height / 2.0);
}

static gboolean
scroll_event (ClutterActor *actor,
              ClutterScrollEvent *event,
              ChamplainView *view)
{
  ChamplainViewPrivate *priv = GET_PRIVATE (view);

  ClutterActor *group, *new_group;
  gboolean success = FALSE;
  gdouble lon, lat;
  gint x_diff, y_diff;
  gint actor_x, actor_y;
  gint rel_x, rel_y;

  group = champlain_zoom_level_get_actor (priv->map->current_level);
  clutter_actor_get_transformed_position (priv->finger_scroll, &actor_x, &actor_y);
  rel_x = event->x - actor_x;
  rel_y = event->y - actor_y;

  /* Keep the lon, lat where the mouse is */
  lon = viewport_get_longitude_at (priv,
    priv->viewport_size.x + rel_x + priv->anchor.x);
  lat = viewport_get_latitude_at (priv,
    priv->viewport_size.y + rel_y + priv->anchor.y);

  /* How far was it from the center of the viewport (in px) */
  x_diff = priv->viewport_size.width / 2 - rel_x;
  y_diff = priv->viewport_size.height / 2 - rel_y;

  if (event->direction == CLUTTER_SCROLL_UP)
    success = map_zoom_in (priv->map, priv->map_source);
  else if (event->direction == CLUTTER_SCROLL_DOWN)
    success = map_zoom_out (priv->map, priv->map_source);

  if (success)
    {
      gint x2, y2;
      gdouble lat2, lon2;

      priv->zoom_level = champlain_zoom_level_get_zoom_level (priv->map->current_level);
      new_group = champlain_zoom_level_get_actor (priv->map->current_level);

      /* Get the new x,y in the new zoom level */
      x2 = champlain_map_source_get_x (priv->map_source, priv->zoom_level, lon);
      y2 = champlain_map_source_get_y (priv->map_source, priv->zoom_level, lat);
      /* Get the new lon,lat of these new x,y minus the distance from the
       * viewport center */
      lon2 = champlain_map_source_get_longitude (priv->map_source,
          priv->zoom_level, x2 + x_diff);
      lat2 = champlain_map_source_get_latitude (priv->map_source,
          priv->zoom_level, y2 + y_diff);

      resize_viewport (view);
      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->map_layer),
          group);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer),
          new_group);
      champlain_view_center_on (view, lat2, lon2);

      g_object_notify (G_OBJECT (view), "zoom-level");
    }

  return success;
}

static void
marker_reposition_cb (ChamplainMarker *marker,
                      ChamplainView *view)
{
  ChamplainViewPrivate *priv = GET_PRIVATE (view);
  ChamplainMarkerPrivate *marker_priv = CHAMPLAIN_MARKER_GET_PRIVATE (marker);

  gint level;
  gint x, y;

  if (priv->map)
    {
      x = champlain_map_source_get_x (priv->map_source, priv->zoom_level, marker_priv->lon);
      y = champlain_map_source_get_y (priv->map_source, priv->zoom_level, marker_priv->lat);

      clutter_actor_set_position (CLUTTER_ACTOR (marker),
        x - priv->anchor.x,
        y - priv->anchor.y);
    }
}

static void
notify_marker_reposition_cb (ChamplainMarker *marker,
                             GParamSpec *arg1,
                             ChamplainView *view)
{
  marker_reposition_cb (marker, view);
}

static void
layer_add_marker_cb (ClutterGroup *layer,
                     ChamplainMarker *marker,
                     ChamplainView *view)
{
  g_signal_connect (marker, "notify::longitude",
      G_CALLBACK (notify_marker_reposition_cb), view);

  g_idle_add (marker_reposition, view);
}

static void
connect_marker_notify_cb (ChamplainMarker *marker,
                          ChamplainView *view)
{
  g_signal_connect (marker, "notify::longitude",
      G_CALLBACK (notify_marker_reposition_cb), view);
}

static void
layer_reposition_cb (ClutterActor *layer,
                     ChamplainView *view)
{
  clutter_container_foreach (CLUTTER_CONTAINER (layer),
      CLUTTER_CALLBACK (marker_reposition_cb), view);
}

static gboolean
marker_reposition (gpointer data)
{
  ChamplainView *view = CHAMPLAIN_VIEW (data);
  ChamplainViewPrivate *priv = GET_PRIVATE (view);
  clutter_container_foreach (CLUTTER_CONTAINER (priv->user_layers),
      CLUTTER_CALLBACK (layer_reposition_cb), view);
  return FALSE;
}

static void
create_initial_map (ChamplainView *view)
{
  ChamplainViewPrivate *priv = GET_PRIVATE (view);
  ClutterActor *group;

  priv->map = map_new ();
  map_load_level (priv->map, priv->map_source, priv->zoom_level);
  group = champlain_zoom_level_get_actor (priv->map->current_level);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer), group);

  g_idle_add (marker_reposition, view);
  view_tiles_reposition (view);
  update_license (view);

  g_object_notify (G_OBJECT (view), "zoom-level");
  g_object_notify (G_OBJECT (view), "map-source");
}

static void
license_set_position (ChamplainView *view)
{
  ChamplainViewPrivate *priv = GET_PRIVATE (view);
  guint width, height;

  if (!priv->license_actor)
    return;

  clutter_actor_get_size (priv->license_actor, &width, &height);
  clutter_actor_set_position (priv->license_actor, priv->viewport_size.width -
      PADDING - width, priv->viewport_size.height - PADDING - height);
}

static void
resize_viewport (ChamplainView *view)
{
  gdouble lower, upper;
  gboolean center = FALSE;
  TidyAdjustment *hadjust, *vadjust;
  gint level;

  ChamplainViewPrivate *priv = GET_PRIVATE (view);

  if (!priv->map)
    {
      create_initial_map (view);
      center = TRUE;
    }

  clutter_actor_set_size (priv->finger_scroll, priv->viewport_size.width,
      priv->viewport_size.height);

  tidy_scrollable_get_adjustments (TIDY_SCROLLABLE (priv->viewport), &hadjust,
      &vadjust);

  if (priv->zoom_level < 8)
    {
      lower = -priv->viewport_size.width / 2.0;
      upper = champlain_zoom_level_get_width (priv->map->current_level)*256 - //XXX
          priv->viewport_size.width / 2.0;
    }
  else
    {
      lower = 0;
      upper = G_MAXINT16;
    }
  g_object_set (hadjust, "lower", lower, "upper", upper,
      "page-size", 1.0, "step-increment", 1.0, "elastic", TRUE, NULL);

  if (priv->zoom_level < 8)
    {
      lower = -priv->viewport_size.height / 2.0;
      upper = champlain_zoom_level_get_height (priv->map->current_level)*256 - //XXX
          priv->viewport_size.height / 2.0;
    }
  else
    {
      lower = 0;
      upper = G_MAXINT16;
    }
  g_object_set (vadjust, "lower", lower, "upper", upper,
      "page-size", 1.0, "step-increment", 1.0, "elastic", TRUE, NULL);

  if (center)
    {
      champlain_view_center_on (view, priv->latitude, priv->longitude);
    }
}

static void
champlain_view_get_property (GObject *object,
                             guint prop_id,
                             GValue *value,
                             GParamSpec *pspec)
{
  ChamplainView *view = CHAMPLAIN_VIEW (object);
  ChamplainViewPrivate *priv = GET_PRIVATE (view);

  switch (prop_id)
    {
      case PROP_LONGITUDE:
        g_value_set_double (value, viewport_get_current_longitude (priv));
        break;
      case PROP_LATITUDE:
        g_value_set_double (value, viewport_get_current_latitude (priv));
        break;
      case PROP_ZOOM_LEVEL:
        g_value_set_int (value, priv->zoom_level);
        break;
      case PROP_MAP_SOURCE:
        g_value_set_object (value, priv->map_source);
        break;
      case PROP_SCROLL_MODE:
        g_value_set_enum (value, priv->scroll_mode);
        break;
      case PROP_OFFLINE:
        g_value_set_boolean (value, priv->offline);
        break;
      case PROP_DECEL_RATE:
        {
          gdouble decel;
          g_object_get (priv->finger_scroll, "decel-rate", decel, NULL);
          g_value_set_double (value, decel);
          break;
        }
      case PROP_KEEP_CENTER_ON_RESIZE:
        g_value_set_boolean (value, priv->keep_center_on_resize);
        break;
      case PROP_SHOW_LICENSE:
        g_value_set_boolean (value, priv->show_license);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
champlain_view_set_property (GObject *object,
                             guint prop_id,
                             const GValue *value,
                             GParamSpec *pspec)
{
  ChamplainView *view = CHAMPLAIN_VIEW (object);
  ChamplainViewPrivate *priv = GET_PRIVATE (view);

  switch (prop_id)
  {
    case PROP_LONGITUDE:
      {
        champlain_view_center_on (view, priv->latitude,
            g_value_get_double (value));
        break;
      }
    case PROP_LATITUDE:
      {
        champlain_view_center_on (view, g_value_get_double (value),
            priv->longitude);
        break;
      }
    case PROP_ZOOM_LEVEL:
      {
        gint level = g_value_get_int (value);
        if (priv->map)
          {
            if (level != priv->zoom_level)
              {
                ClutterActor *group = champlain_zoom_level_get_actor (priv->map->current_level);
                if (map_zoom_to (priv->map, priv->map_source, level))
                  {
                    priv->zoom_level = level;
                    ClutterActor *new_group = champlain_zoom_level_get_actor (priv->map->current_level);
                    resize_viewport (view);
                    clutter_container_remove_actor (
                        CLUTTER_CONTAINER (priv->map_layer), group);
                    clutter_container_add_actor (
                        CLUTTER_CONTAINER (priv->map_layer), new_group);
                    champlain_view_center_on (view, priv->latitude,
                        priv->longitude);
                  }
              }
          }
        break;
      }
    case PROP_MAP_SOURCE:
      {
        ChamplainMapSource *source = g_value_get_object (value);

        if (priv->map_source != source)
          {
            g_object_unref (priv->map_source);
            priv->map_source = g_object_ref (source);
            if (priv->map)
              {
                ClutterActor *group;

                group = champlain_zoom_level_get_actor (priv->map->current_level);
                clutter_container_remove_actor (CLUTTER_CONTAINER (priv->map_layer),
                    group);

                map_free (priv->map);
                priv->map = map_new ();

                /* Keep same zoom level if the new map supports it */
                if (priv->zoom_level > champlain_map_source_get_max_zoom_level (priv->map_source))
                  {
                    priv->zoom_level = champlain_map_source_get_max_zoom_level (priv->map_source);
                    g_object_notify (G_OBJECT (view), "zoom-level");
                  }

                map_load_level (priv->map, priv->map_source, priv->zoom_level);
                group = champlain_zoom_level_get_actor (priv->map->current_level);

                view_load_visible_tiles (view);
                clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer),
                    group);

                update_license (view);
                g_idle_add (marker_reposition, view);
                view_tiles_reposition (view);
                champlain_view_center_on (view, priv->latitude, priv->longitude);
              }
          }
        break;
      }
    case PROP_SCROLL_MODE:
      priv->scroll_mode = g_value_get_enum (value);
      g_object_set (G_OBJECT (priv->finger_scroll), "mode",
          priv->scroll_mode, NULL);
      break;
    case PROP_OFFLINE:
      priv->offline = g_value_get_boolean (value);
      break;
    case PROP_DECEL_RATE:
      {
        gdouble decel = g_value_get_double (value);
        g_object_set (priv->finger_scroll, "decel-rate", decel, NULL);
        break;
      }
    case PROP_KEEP_CENTER_ON_RESIZE:
      priv->keep_center_on_resize = g_value_get_boolean (value);
      break;
    case PROP_SHOW_LICENSE:
      priv->show_license = g_value_get_boolean (value);
      update_license (view);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
champlain_view_finalize (GObject *object)
{
  /*
  ChamplainView *view = CHAMPLAIN_VIEW (object);
  ChamplainViewPrivate *priv = GET_PRIVATE (view);
  */

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
  g_object_class_install_property (object_class,
      PROP_LONGITUDE,
      g_param_spec_float ("longitude",
         "Longitude",
         "The longitude coordonate of the map",
         -180.0f, 180.0f, 0.0f, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainView:latitude:
  *
  * The latitude coordonate of the map
  *
  * Since: 0.1
  */
  g_object_class_install_property (object_class,
      PROP_LATITUDE,
      g_param_spec_float ("latitude",
           "Latitude",
           "The latitude coordonate of the map",
           -90.0f, 90.0f, 0.0f, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainView:zoom-level:
  *
  * The level of zoom of the content.
  *
  * Since: 0.1
  */
  g_object_class_install_property (object_class,
      PROP_ZOOM_LEVEL,
      g_param_spec_int ("zoom-level",
           "Zoom level",
           "The level of zoom of the map",
           0, 20, 3, CHAMPLAIN_PARAM_READWRITE));


  /**
  * ChamplainView:map-source:
  *
  * The #ChamplainMapSource being displayed
  *
  * Since: 0.2
  */
  g_object_class_install_property (object_class,
      PROP_MAP_SOURCE,
      g_param_spec_object ("map-source",
           "Map source",
           "The map source being displayed",
           CHAMPLAIN_TYPE_MAP_SOURCE,
           CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainView:offline:
  *
  * If true, will fetch tiles from the Internet, otherwise, will only use
  * cached content.
  *
  * Since: 0.2
  */
  g_object_class_install_property (object_class,
      PROP_OFFLINE,
      g_param_spec_boolean ("offline",
           "Offline Mode",
           "If viewer is in offline mode.",
           FALSE, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainView:scroll-mode:
  *
  * Determines the way the view reacts to scroll events.
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_SCROLL_MODE,
      g_param_spec_enum ("scroll-mode",
           "Scroll Mode",
           "Determines the way the view reacts to scroll events.",
           CHAMPLAIN_TYPE_SCROLL_MODE,
           CHAMPLAIN_SCROLL_MODE_KINETIC,
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
            "Rate at which the view will decelerate in kinetic mode.",
            1.0, 2.0, 1.1, CHAMPLAIN_PARAM_READWRITE));
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
           TRUE, CHAMPLAIN_PARAM_READWRITE));
  /**
  * ChamplainView:show-license:
  *
  * Show the license on the map view.  The license information should always be
  * available in a way or another in your application.  You can have it in
  * About, or on the map.
  *
  * Since: 0.2.8
  */
  g_object_class_install_property (object_class,
       PROP_SHOW_LICENSE,
       g_param_spec_boolean ("show-license",
           "Show the map data license",
           "Show the map data license on the map view",
           TRUE, CHAMPLAIN_PARAM_READWRITE));
}

static void
champlain_view_init (ChamplainView *view)
{
  ChamplainViewPrivate *priv = GET_PRIVATE (view);

  champlain_debug_set_flags (g_getenv ("CHAMPLAIN_DEBUG"));

  priv->map_source = champlain_map_source_new_osm_mapnik ();
  priv->zoom_level = 0;
  priv->offline = FALSE;
  priv->keep_center_on_resize = TRUE;
  priv->show_license = TRUE;
  priv->license_actor = NULL;
  priv->stage = clutter_group_new ();
  priv->scroll_mode = CHAMPLAIN_SCROLL_MODE_PUSH;
  priv->viewport_size.x = 0;
  priv->viewport_size.y = 0;
  priv->viewport_size.width = 0;
  priv->viewport_size.height = 0;
  priv->anchor.x = 0;
  priv->anchor.y = 0;


  /* Setup viewport */
  priv->viewport = tidy_viewport_new ();
  g_object_set (G_OBJECT (priv->viewport), "sync-adjustments", FALSE, NULL);

  g_signal_connect (priv->viewport,
                    "notify::x-origin",
                    G_CALLBACK (viewport_x_changed_cb),
                    view);

  /* Setup finger scroll */
  priv->finger_scroll = tidy_finger_scroll_new (priv->scroll_mode);

  g_signal_connect (priv->finger_scroll,
                    "scroll-event",
                    G_CALLBACK (scroll_event),
                    view);

  clutter_container_add_actor (CLUTTER_CONTAINER (priv->finger_scroll),
      priv->viewport);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->stage),
      priv->finger_scroll);
  clutter_container_add_actor (CLUTTER_CONTAINER (view), priv->stage);

  /* Map Layer */
  priv->map_layer = clutter_group_new ();
  clutter_actor_show (priv->map_layer);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport),
      priv->map_layer);

  g_signal_connect (priv->finger_scroll,
                    "button-press-event",
                    G_CALLBACK (finger_scroll_button_press_cb),
                    view);

  /* Setup user_layers */
  priv->user_layers = clutter_group_new ();
  clutter_actor_show (priv->user_layers);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport),
      priv->user_layers);
  clutter_actor_raise (priv->user_layers, priv->map_layer);
}

static void
viewport_x_changed_cb (GObject *gobject,
                       GParamSpec *arg1,
                       ChamplainView *view)
{
  ChamplainViewPrivate *priv = GET_PRIVATE (view);

  ChamplainPoint rect;
  tidy_viewport_get_origin (TIDY_VIEWPORT (priv->viewport), &rect.x, &rect.y,
      NULL);

  if (rect.x == priv->viewport_size.x &&
      rect.y == priv->viewport_size.y)
      return;

  priv->viewport_size.x = rect.x;
  priv->viewport_size.y = rect.y;

  view_load_visible_tiles (view);
  view_tiles_reposition (view);

  g_object_notify (G_OBJECT (view), "longitude");
  g_object_notify (G_OBJECT (view), "latitude");
}

//FIXME: move to an handler of actor size change
void
champlain_view_set_size (ChamplainView *view,
                         guint width,
                         guint height)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = GET_PRIVATE (view);

  priv->viewport_size.width = width;
  priv->viewport_size.height = height;

  license_set_position (view);
  resize_viewport (view);

  if (priv->keep_center_on_resize)
    champlain_view_center_on (view, priv->latitude, priv->longitude);
  else
    view_load_visible_tiles (view);
}

static void
update_license (ChamplainView *view)
{
  ChamplainViewPrivate *priv = GET_PRIVATE (view);

  if (priv->license_actor)
    clutter_container_remove_actor (CLUTTER_CONTAINER (priv->stage),
        priv->license_actor);

  if (priv->show_license)
    {
      priv->license_actor = clutter_label_new_with_text ( "sans 8",
          ""); //XXX: champlain_map_source_get_license (priv->map_source));
      clutter_actor_set_opacity (priv->license_actor, 128);
      clutter_actor_show (priv->license_actor);

      clutter_container_add_actor (CLUTTER_CONTAINER (priv->stage),
          priv->license_actor);
      clutter_actor_raise_top (priv->license_actor);
      license_set_position (view);
    }
}

static gboolean
finger_scroll_button_press_cb (ClutterActor *actor,
                               ClutterButtonEvent *event,
                               ChamplainView *view)
{
  ChamplainViewPrivate *priv = GET_PRIVATE (view);

  if (event->button == 1 && event->click_count == 2)
    {
      gint actor_x, actor_y;
      gint rel_x, rel_y;

      clutter_actor_get_transformed_position (priv->finger_scroll, &actor_x, &actor_y);
      rel_x = event->x - actor_x;
      rel_y = event->y - actor_y;

      ClutterActor *group = champlain_zoom_level_get_actor (priv->map->current_level);
      /* Keep the lon, lat where the mouse is */
      gdouble lon = viewport_get_longitude_at (priv,
        priv->viewport_size.x + rel_x + priv->anchor.x);
      gdouble lat = viewport_get_latitude_at (priv,
        priv->viewport_size.y + rel_y + priv->anchor.y);

      /* How far was it from the center of the viewport (in px) */
      gint x_diff = priv->viewport_size.width / 2 - rel_x;
      gint y_diff = priv->viewport_size.height / 2 - rel_y;

      if (map_zoom_in (priv->map, priv->map_source))
        {
          gint x2, y2;
          gdouble lat2, lon2;

          priv->zoom_level++;
          ClutterActor *new_group = champlain_zoom_level_get_actor (priv->map->current_level);

          /* Get the new x,y in the new zoom level */
          x2 = champlain_map_source_get_x (priv->map_source, priv->zoom_level, lon);
          y2 = champlain_map_source_get_y (priv->map_source, priv->zoom_level, lat);
          /* Get the new lon,lat of these new x,y minus the distance from the
           * viewport center */
          lon2 = champlain_map_source_get_longitude (priv->map_source,
              priv->zoom_level, x2 + x_diff);
          lat2 = champlain_map_source_get_latitude (priv->map_source,
              priv->zoom_level, y2 + y_diff);

          resize_viewport (view);
          clutter_container_remove_actor (CLUTTER_CONTAINER (priv->map_layer),
              group);
          clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer),
              new_group);
          champlain_view_center_on (view, lat2, lon2);

          g_object_notify (G_OBJECT (view), "zoom-level");
        }
      champlain_view_center_on (view, lat, lon);
      return TRUE;
    }
  return FALSE; /* Propagate the event */
}

/**
 * champlain_view_new:
 * Returns a new #ChamplainView ready to be used as a #ClutterActor.
 *
 * Since: 0.4
 */
ClutterActor *
champlain_view_new ()
{
  return g_object_new (CHAMPLAIN_TYPE_VIEW, NULL);
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
void
champlain_view_center_on (ChamplainView *view,
                          gdouble latitude,
                          gdouble longitude)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  gint x, y;
  guint i;
  ChamplainViewPrivate *priv = GET_PRIVATE (view);

  priv->longitude = longitude;
  priv->latitude = latitude;

  if (!priv->map)
    return;

  x = champlain_map_source_get_x (priv->map_source, priv->zoom_level, longitude);
  y = champlain_map_source_get_y (priv->map_source, priv->zoom_level, latitude);

  if (priv->zoom_level >= 8)
    {
      gdouble max;

      priv->anchor.x = x - G_MAXINT16 / 2;
      priv->anchor.y = y - G_MAXINT16 / 2;

      if ( priv->anchor.x < 0 )
        priv->anchor.x = 0;
      if ( priv->anchor.y < 0 )
        priv->anchor.y = 0;

      max = champlain_zoom_level_get_width (priv->map->current_level)*256 - //XXX
          (G_MAXINT16 / 2);
      if (priv->anchor.x > max)
        priv->anchor.x = max;
      if (priv->anchor.y > max)
        priv->anchor.y = max;

      x -= priv->anchor.x;
      y -= priv->anchor.y;
    }
  else
    {
      priv->anchor.x = 0;
      priv->anchor.y = 0;
    }

  tidy_viewport_set_origin (TIDY_VIEWPORT (priv->viewport),
    x - priv->viewport_size.width / 2.0,
    y - priv->viewport_size.height / 2.0,
    0);

  g_object_notify (G_OBJECT (view), "longitude");
  g_object_notify (G_OBJECT (view), "latitude");

  view_load_visible_tiles (view);
  view_tiles_reposition (view);
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
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = GET_PRIVATE (view);
  ClutterActor *group = champlain_zoom_level_get_actor (priv->map->current_level);

  if (map_zoom_in (priv->map, priv->map_source))
    {
      priv->zoom_level++;
      resize_viewport (view);
      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->map_layer),
          group);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer),
          champlain_zoom_level_get_actor (priv->map->current_level));
      champlain_view_center_on (view, priv->latitude, priv->longitude);

      g_object_notify (G_OBJECT (view), "zoom-level");
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
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = GET_PRIVATE (view);
  ClutterActor *group = champlain_zoom_level_get_actor (priv->map->current_level);

  if (map_zoom_out (priv->map, priv->map_source))
    {
      priv->zoom_level--;
      resize_viewport (view);
      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->map_layer),
          group);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer),
          champlain_zoom_level_get_actor (priv->map->current_level));
      champlain_view_center_on (view, priv->latitude, priv->longitude);

      g_object_notify (G_OBJECT (view), "zoom-level");
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
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));
  g_return_if_fail (CLUTTER_IS_ACTOR (layer));

  ChamplainViewPrivate *priv = GET_PRIVATE (view);
  clutter_container_add (CLUTTER_CONTAINER (priv->user_layers), layer, NULL);
  clutter_actor_raise_top (layer);

  if (priv->map)
    g_idle_add (marker_reposition, view);

  g_signal_connect_after (layer,
                    "actor-added",
                    G_CALLBACK (layer_add_marker_cb),
                    view);

  clutter_container_foreach (CLUTTER_CONTAINER (layer),
      CLUTTER_CALLBACK (connect_marker_notify_cb), view);
}

/**
 * champlain_view_get_coords_from_event:
 * @view: a #ChamplainView
 * @event: a #ClutterEvent
 * @latitude: a variable where to put the latitude of the event
 * @longitude: a variable where to put the longitude of the event
 *
 * Returns a new #ChamplainView ready to be used as a #ClutterActor.
 *
 * Since: 0.2.8
 */
gboolean
champlain_view_get_coords_from_event (ChamplainView *view,
                                      ClutterEvent *event,
                                      gdouble *latitude,
                                      gdouble *longitude)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), FALSE);
  /* Apparently there isn a more precise test */
  g_return_val_if_fail (event, FALSE);

  ChamplainViewPrivate *priv = GET_PRIVATE (view);
  guint x, y;
  gint actor_x, actor_y;
  gint rel_x, rel_y;

  clutter_actor_get_transformed_position (priv->finger_scroll, &actor_x, &actor_y);

  switch (clutter_event_type (event))
    {
      case CLUTTER_BUTTON_PRESS:
      case CLUTTER_BUTTON_RELEASE:
        {
          ClutterButtonEvent *e = (ClutterButtonEvent*) event;
          x = e->x;
          y = e->y;
        }
        break;
      case CLUTTER_SCROLL:
        {
          ClutterScrollEvent *e = (ClutterScrollEvent*) event;
          x = e->x;
          y = e->y;
        }
        break;
      case CLUTTER_MOTION:
        {
          ClutterMotionEvent *e = (ClutterMotionEvent*) event;
          x = e->x;
          y = e->y;
        }
        break;
      case CLUTTER_ENTER:
      case CLUTTER_LEAVE:
        {
          ClutterCrossingEvent *e = (ClutterCrossingEvent*) event;
          x = e->x;
          y = e->y;
        }
        break;
      default:
        return FALSE;
    }

  rel_x = x - actor_x;
  rel_y = y - actor_y;

  if (latitude)
    *latitude = viewport_get_latitude_at (priv,
        priv->viewport_size.y + rel_y + priv->anchor.y);
  if (longitude)
    *longitude = viewport_get_longitude_at (priv,
        priv->viewport_size.x + rel_x + priv->anchor.x);

  return TRUE;
}

static void
view_load_visible_tiles (ChamplainView *view)
{
  ChamplainViewPrivate *priv = GET_PRIVATE (view);
  ChamplainRectangle viewport = priv->viewport_size;

  viewport.x += priv->anchor.x;
  viewport.y += priv->anchor.y;

  map_load_visible_tiles (priv->map, view, priv->map_source, viewport, priv->offline);
}

static void
view_position_tile (ChamplainView* view, ChamplainTile* tile)
{
  ChamplainViewPrivate *priv = GET_PRIVATE (view);

  ClutterActor *actor;
  gint x;
  gint y;
  guint size;

  g_object_get (G_OBJECT (tile), "actor", &actor,
      "x", &x, "y", &y,
      "size", &size, NULL);

  clutter_actor_set_position (actor,
    (x * size) - priv->anchor.x,
    (y * size) - priv->anchor.y);
}

static void
view_tiles_reposition (ChamplainView* view)
{
  ChamplainViewPrivate *priv = GET_PRIVATE (view);
  gint i;

  for (i = 0; i < champlain_zoom_level_tile_count (priv->map->current_level); i++)
    {
      ChamplainTile *tile = champlain_zoom_level_get_nth_tile (priv->map->current_level, i);
      if (champlain_tile_get_state (tile) == CHAMPLAIN_STATE_DONE)
        view_position_tile (view, tile);
    }
}

void
champlain_view_tile_ready (ChamplainView *view,
                           ChamplainZoomLevel *level,
                           ChamplainTile *tile,
                           gboolean animate)
{
  ClutterActor *actor;
  ClutterEffectTemplate *etemplate;

  actor = champlain_tile_get_actor (tile);
  clutter_actor_show (actor);
  if (animate)
    {
      etemplate = clutter_effect_template_new_for_duration (750, CLUTTER_ALPHA_SINE_INC);
      clutter_actor_set_opacity(actor, 0);
      clutter_effect_fade (etemplate, actor, 255, NULL, NULL);
    }

  clutter_container_add (CLUTTER_CONTAINER (champlain_zoom_level_get_actor (level)), actor, NULL);
  view_position_tile (view, tile);
}
