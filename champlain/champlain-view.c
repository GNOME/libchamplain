/*
 * Copyright (C) 2008, 2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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
 * SECTION:champlain-view
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
 * an error occurs during download, an error tile will be displayed.
 *
 * The button-press-event and button-release-event signals are emitted each
 * time a mouse button is pressed on the @view.  Coordinates can be converted
 * with #champlain_view_get_coords_from_event.
 */

#include "config.h"

#include "champlain-view.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_VIEW
#include "champlain-debug.h"

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
  ANIMATION_COMPLETED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_LONGITUDE,
  PROP_LATITUDE,
  PROP_ZOOM_LEVEL,
  PROP_MIN_ZOOM_LEVEL,
  PROP_MAX_ZOOM_LEVEL,
  PROP_MAP_SOURCE,
  PROP_DECEL_RATE,
  PROP_SCROLL_MODE,
  PROP_KEEP_CENTER_ON_RESIZE,
  PROP_SHOW_LICENSE,
  PROP_ZOOM_ON_DOUBLE_CLICK,
  PROP_STATE
};

#define PADDING 10
static guint signals[LAST_SIGNAL] = { 0, };

#define GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CHAMPLAIN_TYPE_VIEW, ChamplainViewPrivate))
#define ZOOM_LEVEL_OUT_OF_RANGE(priv, level)    (level < priv->min_zoom_level || level > priv->max_zoom_level)

/* Between state values for go_to */
typedef struct {
  ChamplainView *view;
  ClutterAlpha *alpha;
  ClutterTimeline *timeline;
  gdouble to_latitude;
  gdouble to_longitude;
  gdouble from_latitude;
  gdouble from_longitude;
} GoToContext;

struct _ChamplainViewPrivate
{
  ClutterActor *stage;

  ChamplainMapSource *map_source; /* Current map tile source */
  ChamplainScrollMode scroll_mode;
  gint zoom_level; /* Holds the current zoom level number */
  gint min_zoom_level; /* Lowest allowed zoom level */
  gint max_zoom_level; /* Highest allowed zoom level */

  /* Represents the (lat, lon) at the center of the viewport */
  gdouble longitude;
  gdouble latitude;

  /* Hack to get smaller x,y coordinates as the clutter limit is G_MAXINT16 */
  ChamplainPoint anchor;
  gdouble anchor_zoom_level; /* the zoom_level for which the current anchor has
                                been computed for */

  Map *map; /* Contains the current map model */

  ClutterActor *finger_scroll; /* Contains the viewport */
  ClutterActor *viewport;  /* Contains the map_layer, license and markers */
  ClutterActor *map_layer; /* Contains tiles actors (grouped by zoom level) */
  ChamplainRectangle viewport_size;

  ClutterActor *user_layers; /* Contains the markers */

  gboolean keep_center_on_resize;

  gboolean zoom_on_double_click;

  gboolean show_license;
  ClutterActor *license_actor; /* Contains the license info */

  ChamplainState state; /* View's global state */

  /* champlain_view_go_to's context, kept for stop_go_to */
  GoToContext *goto_context;
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
static void champlain_view_dispose (GObject *object);
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
static void view_update_state (ChamplainView *view);
static void view_update_anchor (ChamplainView *view, gint x, gint y);
static gboolean view_set_zoom_level_at (ChamplainView *view,
    gint zoom_level,
    gint x,
    gint y);

static gdouble
viewport_get_longitude_at (ChamplainViewPrivate *priv, gint x)
{
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
  ChamplainViewPrivate *priv = view->priv;

  gint zoom_level = priv->zoom_level;

  if (event->direction == CLUTTER_SCROLL_UP)
    zoom_level = priv->zoom_level + 1;
  else if (event->direction == CLUTTER_SCROLL_DOWN)
    zoom_level = priv->zoom_level - 1;

  return view_set_zoom_level_at (view, zoom_level, event->x, event->y);
}

static void
marker_reposition_cb (ChamplainMarker *marker,
    ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;
  ChamplainBaseMarkerPrivate *marker_priv = CHAMPLAIN_BASE_MARKER(marker)->priv;

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
  ChamplainViewPrivate *priv = view->priv;
  clutter_container_foreach (CLUTTER_CONTAINER (priv->user_layers),
      CLUTTER_CALLBACK (layer_reposition_cb), view);
  return FALSE;
}

static void
create_initial_map (ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;
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
  ChamplainViewPrivate *priv = view->priv;
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

  ChamplainViewPrivate *priv = view->priv;

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
      upper = champlain_zoom_level_get_width (priv->map->current_level) *
          champlain_map_source_get_tile_size (priv->map_source) -
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
      upper = champlain_zoom_level_get_height (priv->map->current_level) *
          champlain_map_source_get_tile_size (priv->map_source) -
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
  ChamplainViewPrivate *priv = view->priv;

  switch (prop_id)
    {
      case PROP_LONGITUDE:
        g_value_set_double (value, priv->longitude);
        break;
      case PROP_LATITUDE:
        g_value_set_double (value, priv->latitude);
        break;
      case PROP_ZOOM_LEVEL:
        g_value_set_int (value, priv->zoom_level);
        break;
      case PROP_MIN_ZOOM_LEVEL:
        g_value_set_int (value, priv->min_zoom_level);
        break;
      case PROP_MAX_ZOOM_LEVEL:
        g_value_set_int (value, priv->max_zoom_level);
        break;
      case PROP_MAP_SOURCE:
        g_value_set_object (value, priv->map_source);
        break;
      case PROP_SCROLL_MODE:
        g_value_set_enum (value, priv->scroll_mode);
        break;
      case PROP_DECEL_RATE:
        {
          gdouble decel = 0.0;
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
      case PROP_ZOOM_ON_DOUBLE_CLICK:
        g_value_set_boolean (value, priv->zoom_on_double_click);
        break;
      case PROP_STATE:
        g_value_set_enum (value, priv->state);
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
  ChamplainViewPrivate *priv = view->priv;

  switch (prop_id)
  {
    case PROP_LONGITUDE:
      champlain_view_center_on (view, priv->latitude,
          g_value_get_double (value));
      break;
    case PROP_LATITUDE:
      champlain_view_center_on (view, g_value_get_double (value),
          priv->longitude);
      break;
    case PROP_ZOOM_LEVEL:
      champlain_view_set_zoom_level (view, g_value_get_int (value));
      break;
    case PROP_MIN_ZOOM_LEVEL:
      champlain_view_set_min_zoom_level (view, g_value_get_int (value));
      break;
    case PROP_MAX_ZOOM_LEVEL:
      champlain_view_set_max_zoom_level (view, g_value_get_int (value));
      break;
    case PROP_MAP_SOURCE:
      champlain_view_set_map_source (view, g_value_get_object (value));
      break;
    case PROP_SCROLL_MODE:
      champlain_view_set_scroll_mode (view, g_value_get_enum (value));
      break;
    case PROP_DECEL_RATE:
      champlain_view_set_decel_rate (view, g_value_get_double (value));
      break;
    case PROP_KEEP_CENTER_ON_RESIZE:
      champlain_view_set_keep_center_on_resize (view, g_value_get_boolean (value));
      break;
    case PROP_SHOW_LICENSE:
      champlain_view_set_show_license (view, g_value_get_boolean (value));
      break;
    case PROP_ZOOM_ON_DOUBLE_CLICK:
      champlain_view_set_zoom_on_double_click (view, g_value_get_boolean (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
champlain_view_dispose (GObject *object)
{
  ChamplainView *view = CHAMPLAIN_VIEW (object);
  ChamplainViewPrivate *priv = view->priv;
  g_object_unref (priv->map_source);
  if (priv->license_actor)
    g_object_unref (priv->license_actor);
  g_object_unref (priv->finger_scroll);
  g_object_unref (priv->viewport);
  g_object_unref (priv->map_layer);
  g_object_unref (priv->user_layers);
  g_object_unref (priv->stage);

  map_free (priv->map);

  if (priv->goto_context)
    g_free (priv->goto_context);
}

static void
champlain_view_finalize (GObject *object)
{
  /*
  ChamplainView *view = CHAMPLAIN_VIEW (object);
  ChamplainViewPrivate *priv = view->priv;
  */

  G_OBJECT_CLASS (champlain_view_parent_class)->finalize (object);
}

static void
champlain_view_class_init (ChamplainViewClass *champlainViewClass)
{
  g_type_class_add_private (champlainViewClass, sizeof (ChamplainViewPrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (champlainViewClass);
  object_class->finalize = champlain_view_finalize;
  object_class->dispose = champlain_view_dispose;
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
      g_param_spec_double ("longitude",
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
      g_param_spec_double ("latitude",
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
  * ChamplainView:min-zoom-level:
  *
  * The lowest allowed level of zoom of the content.
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_MIN_ZOOM_LEVEL,
      g_param_spec_int ("min-zoom-level",
           "Min zoom level",
           "The lowest allowed level of zoom",
           0, 20, 0, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainView:max-zoom-level:
  *
  * The highest allowed level of zoom of the content.
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_MAX_ZOOM_LEVEL,
      g_param_spec_int ("max-zoom-level",
           "Max zoom level",
           "The highest allowed level of zoom",
           0, 20, 20, CHAMPLAIN_PARAM_READWRITE));

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
  * The deceleration rate for the kinetic mode. The default value is 1.1.
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

  /**
  * ChamplainView:zoom-on-double-click:
  *
  * Should the view zoom in and recenter when the user double click on the map.
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
       PROP_ZOOM_ON_DOUBLE_CLICK,
       g_param_spec_boolean ("zoom-on-double-click",
           "Zoom in on double click",
           "Zoom in and recenter on double click on the map",
           TRUE, CHAMPLAIN_PARAM_READWRITE));

  /**
  * ChamplainView:state
  *
  * The view's global state. Useful to inform using if the view is busy loading
  * tiles or not.
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
       PROP_STATE,
       g_param_spec_enum ("state",
           "View's state",
           "View's global state",
           CHAMPLAIN_TYPE_STATE,
           CHAMPLAIN_STATE_INIT,
           G_PARAM_READABLE));

  /**
  * ChamplainView::animation-completed:
  * @view: the #ChamplainView that received the signal
  *
  * The ::animation-completed signal is emitted when any animation in the view
  * ends.  This is a detailed signal.  For example, if you want to be signaled
  * only for go-to animation, you should connect to
  * "animation-completed::go-to".
  *
  * Since: 0.4
  */
  signals[ANIMATION_COMPLETED] =
      g_signal_new ("animation-completed", G_OBJECT_CLASS_TYPE (object_class),
          G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED, 0, NULL, NULL,
          g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 0);

}

static void
champlain_view_init (ChamplainView *view)
{
  ChamplainViewPrivate *priv = GET_PRIVATE (view);

  champlain_debug_set_flags (g_getenv ("CHAMPLAIN_DEBUG"));

  view->priv = priv;

  priv->map_source = g_object_ref (champlain_map_source_new_osm_mapnik ());
  priv->zoom_level = 0;
  priv->min_zoom_level = champlain_map_source_get_min_zoom_level (priv->map_source);
  priv->max_zoom_level = champlain_map_source_get_max_zoom_level (priv->map_source);
  priv->keep_center_on_resize = TRUE;
  priv->zoom_on_double_click = TRUE;
  priv->show_license = TRUE;
  priv->license_actor = NULL;
  priv->stage = g_object_ref (clutter_group_new ());
  priv->scroll_mode = CHAMPLAIN_SCROLL_MODE_PUSH;
  priv->viewport_size.x = 0;
  priv->viewport_size.y = 0;
  priv->viewport_size.width = 0;
  priv->viewport_size.height = 0;
  priv->anchor.x = 0;
  priv->anchor.y = 0;
  priv->anchor_zoom_level = 0;
  priv->state = CHAMPLAIN_STATE_INIT;
  priv->latitude = 0.0f;
  priv->longitude = 0.0f;
  priv->goto_context = NULL;

  /* Setup viewport */
  priv->viewport = g_object_ref (tidy_viewport_new ());
  g_object_set (G_OBJECT (priv->viewport), "sync-adjustments", FALSE, NULL);

  g_signal_connect (priv->viewport, "notify::x-origin",
      G_CALLBACK (viewport_x_changed_cb), view);

  /* Setup finger scroll */
  priv->finger_scroll = g_object_ref (tidy_finger_scroll_new (priv->scroll_mode));

  g_signal_connect (priv->finger_scroll, "scroll-event",
      G_CALLBACK (scroll_event), view);

  clutter_container_add_actor (CLUTTER_CONTAINER (priv->finger_scroll),
      priv->viewport);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->stage),
      priv->finger_scroll);
  clutter_container_add_actor (CLUTTER_CONTAINER (view), priv->stage);

  /* Map Layer */
  priv->map_layer = g_object_ref (clutter_group_new ());
  clutter_actor_show (priv->map_layer);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport),
      priv->map_layer);

  g_signal_connect (priv->finger_scroll, "button-press-event",
      G_CALLBACK (finger_scroll_button_press_cb), view);

  /* Setup user_layers */
  priv->user_layers = g_object_ref (clutter_group_new ());
  clutter_actor_show (priv->user_layers);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport),
      priv->user_layers);
  clutter_actor_raise (priv->user_layers, priv->map_layer);

  champlain_view_set_size(view, priv->viewport_size.width,
      priv->viewport_size.height);

  priv->state = CHAMPLAIN_STATE_DONE;
  g_object_notify (G_OBJECT (view), "state");
}

static void
viewport_x_changed_cb (GObject *gobject,
    GParamSpec *arg1,
    ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;

  ChamplainPoint rect;
  ChamplainPoint old_anchor;

  tidy_viewport_get_origin (TIDY_VIEWPORT (priv->viewport), &rect.x, &rect.y,
      NULL);

  if (rect.x == priv->viewport_size.x &&
      rect.y == priv->viewport_size.y)
      return;

  old_anchor.x = priv->anchor.x;
  old_anchor.y = priv->anchor.y;

  view_update_anchor (view,
      rect.x + priv->anchor.x + priv->viewport_size.width / 2.0,
      rect.y + priv->anchor.y + priv->viewport_size.height / 2.0);

  if (priv->anchor.x - old_anchor.x != 0)
    {
      ChamplainPoint diff;

      diff.x = priv->anchor.x - old_anchor.x;
      diff.y = priv->anchor.y - old_anchor.y;

      DEBUG("Relocating the viewport by %d, %d", diff.x, diff.y);
      tidy_viewport_set_origin (TIDY_VIEWPORT (priv->viewport),
          rect.x - diff.x, rect.y - diff.y, 0);
      return;
    }

  priv->viewport_size.x = rect.x;
  priv->viewport_size.y = rect.y;

  view_load_visible_tiles (view);
  view_tiles_reposition (view);
  marker_reposition (view);

  priv->longitude = viewport_get_current_longitude (priv);
  priv->latitude = viewport_get_current_latitude (priv);

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

  ChamplainViewPrivate *priv = view->priv;

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
  ChamplainViewPrivate *priv = view->priv;

  if (priv->license_actor)
  {
    g_object_unref (priv->license_actor);
    clutter_container_remove_actor (CLUTTER_CONTAINER (priv->stage),
        priv->license_actor);
    priv->license_actor = NULL;
  }

  if (priv->show_license)
    {
      priv->license_actor = g_object_ref (clutter_label_new_with_text ("sans 8",
          champlain_map_source_get_license (priv->map_source)));
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
  ChamplainViewPrivate *priv = view->priv;

  if (priv->zoom_on_double_click && event->button == 1 && event->click_count == 2)
    {
      return view_set_zoom_level_at (view, priv->zoom_level + 1, event->x, event->y);
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
champlain_view_new (void)
{
  return g_object_new (CHAMPLAIN_TYPE_VIEW, NULL);
}

static void
view_update_anchor (ChamplainView *view,
    gint x,
    gint y)
{
  ChamplainViewPrivate *priv = view->priv;
  gboolean need_anchor = FALSE;
  gboolean need_update = FALSE;

  if (priv->zoom_level >= 8)
    need_anchor = TRUE;

  if (priv->anchor_zoom_level != priv->zoom_level ||
      x - priv->anchor.x + priv->viewport_size.width >= G_MAXINT16 ||
      y - priv->anchor.y + priv->viewport_size.height >= G_MAXINT16)
    need_update = TRUE;

  if (need_anchor && need_update)
    {
      gdouble max;

      priv->anchor.x = x - G_MAXINT16 / 2;
      priv->anchor.y = y - G_MAXINT16 / 2;

      if ( priv->anchor.x < 0 )
        priv->anchor.x = 0;
      if ( priv->anchor.y < 0 )
        priv->anchor.y = 0;

      max = champlain_zoom_level_get_width (priv->map->current_level) *
          champlain_map_source_get_tile_size (priv->map_source) -
          (G_MAXINT16 / 2);
      if (priv->anchor.x > max)
        priv->anchor.x = max;
      if (priv->anchor.y > max)
        priv->anchor.y = max;

      priv->anchor_zoom_level = priv->zoom_level;
    }

  if (need_anchor == FALSE)
    {
      priv->anchor.x = 0;
      priv->anchor.y = 0;
      priv->anchor_zoom_level = priv->zoom_level;
    }
  DEBUG ("New Anchor (%d, %d) for (%d, %d)", priv->anchor.x, priv->anchor.y, x, y);
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
  ChamplainViewPrivate *priv = view->priv;

  priv->longitude = longitude;
  priv->latitude = latitude;

  if (!priv->map)
    return;

  x = champlain_map_source_get_x (priv->map_source, priv->zoom_level, longitude);
  y = champlain_map_source_get_y (priv->map_source, priv->zoom_level, latitude);

  view_update_anchor (view, x, y);

  x -= priv->anchor.x;
  y -= priv->anchor.y;

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

static void
timeline_new_frame (ClutterTimeline *timeline,
    gint frame_num,
    GoToContext *ctx)
{
  gdouble alpha;
  gdouble lat;
  gdouble lon;

  alpha = (double) clutter_alpha_get_alpha (ctx->alpha) / CLUTTER_ALPHA_MAX_ALPHA;
  lat = ctx->to_latitude - ctx->from_latitude;
  lon = ctx->to_longitude - ctx->from_longitude;

  champlain_view_center_on (ctx->view,
      ctx->from_latitude + alpha * lat,
      ctx->from_longitude + alpha * lon);
}

static void
timeline_completed (ClutterTimeline *timeline,
                    ChamplainView *view)
{
  champlain_view_stop_go_to (view);
}

/**
 * champlain_view_stop_go_to:
 * @view: a #ChamplainView
 *
 * Stop the go to animation.  The view will stay where it was when the
 * animation was stopped.
 *
 * Since: 0.4
 */
void
champlain_view_stop_go_to (ChamplainView *view)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = view->priv;

  if (priv->goto_context == NULL)
    return;

  clutter_timeline_stop (priv->goto_context->timeline);

  g_object_unref (priv->goto_context->timeline);
  g_object_unref (priv->goto_context->alpha);

  g_signal_emit_by_name (view, "animation-completed::go-to", NULL);

  g_free (priv->goto_context);
  priv->goto_context = NULL;
}

/**
 * champlain_view_go_to:
 * @view: a #ChamplainView
 * @latitude: the longitude to center the map at
 * @longitude: the longitude to center the map at
 *
 * Move from the current position to these coordinates. All tiles in the
 * intermediate view WILL be loaded!
 *
 * Since: 0.4
 */
void
champlain_view_go_to (ChamplainView *view,
    gdouble latitude,
    gdouble longitude)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  gint duration;
  GoToContext *ctx;

  ChamplainViewPrivate *priv = view->priv;

  champlain_view_stop_go_to (view);

  ctx = g_new0 (GoToContext, 1);
  ctx->from_latitude = priv->latitude;
  ctx->from_longitude = priv->longitude;
  ctx->to_latitude = latitude;
  ctx->to_longitude = longitude;
  ctx->view = view;

  /* We keep a reference for stop */
  priv->goto_context = ctx;

  /* A ClutterTimeline will be responsible for the animation,
   * at each frame, the current position will be computer and set
   * using champlain_view_center_on.  Timelines skip frames if the
   * computer is not fast enough, so we just need to set the duration.
   *
   * To have a nice animation, the duration should be longer if the zoom level
   * is higher and if the points are far away
   */
  duration = 500 * priv->zoom_level / 2.0;
  ctx->timeline = clutter_timeline_new_for_duration (duration);
  ctx->alpha = clutter_alpha_new_full (ctx->timeline, CLUTTER_ALPHA_SINE_INC, NULL,
      NULL);

  g_signal_connect (ctx->timeline, "new-frame", G_CALLBACK (timeline_new_frame),
      ctx);
  g_signal_connect (ctx->timeline, "completed", G_CALLBACK (timeline_completed),
      view);

  clutter_timeline_start (ctx->timeline);
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

  ChamplainViewPrivate *priv = view->priv;

  champlain_view_set_zoom_level (view, priv->zoom_level + 1);
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

  ChamplainViewPrivate *priv = view->priv;

  champlain_view_set_zoom_level (view, priv->zoom_level - 1);
}

/**
 * champlain_view_set_zoom_level:
 * @view: a #ChamplainView
 * @zoom_level: a gint
 *
 * Changes the current zoom level
 *
 * Since: 0.4
 */
void
champlain_view_set_zoom_level (ChamplainView *view,
    gint zoom_level)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = view->priv;
  gdouble longitude;
  gdouble latitude;

  if (priv->map == NULL)
    return;

  if (zoom_level == priv->zoom_level || ZOOM_LEVEL_OUT_OF_RANGE(priv, zoom_level))
    return;

  champlain_view_stop_go_to (view);

  ClutterActor *group = champlain_zoom_level_get_actor (priv->map->current_level);
  if (!map_zoom_to (priv->map, priv->map_source, zoom_level))
    return;

  priv->zoom_level = zoom_level;
  /* Fix to bug 575133: keep the lat,lon as it gets set to a wrong value
   * when resizing the viewport, when passing from zoom_level 7 to 6
   * (or more precisely when anchor is set to 0).
   */
  longitude = priv->longitude;
  latitude = priv->latitude;
  resize_viewport (view);

  ClutterActor *new_group = champlain_zoom_level_get_actor (priv->map->current_level);
  clutter_container_remove_actor (CLUTTER_CONTAINER (priv->map_layer), group);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer), new_group);
  champlain_view_center_on (view, latitude, longitude);

  g_object_notify (G_OBJECT (view), "zoom-level");
}

/**
 * champlain_view_set_min_zoom_level:
 * @view: a #ChamplainView
 * @min_zoom_level: a gint
 *
 * Changes the lowest allowed zoom level
 *
 * Since: 0.4
 */
void
champlain_view_set_min_zoom_level (ChamplainView *view,
    gint min_zoom_level)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = view->priv;

  if (priv->min_zoom_level == min_zoom_level ||
      min_zoom_level > priv->max_zoom_level ||
      min_zoom_level < champlain_map_source_get_min_zoom_level (priv->map_source))
    return;

  priv->min_zoom_level = min_zoom_level;

  if (priv->zoom_level < min_zoom_level)
    champlain_view_set_zoom_level (view, min_zoom_level);
}

/**
 * champlain_view_set_max_zoom_level:
 * @view: a #ChamplainView
 * @max_zoom_level: a gint
 *
 * Changes the highest allowed zoom level
 *
 * Since: 0.4
 */
void
champlain_view_set_max_zoom_level (ChamplainView *view,
    gint max_zoom_level)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = view->priv;

  if (priv->max_zoom_level == max_zoom_level ||
      max_zoom_level < priv->min_zoom_level ||
      max_zoom_level > champlain_map_source_get_max_zoom_level (priv->map_source))
    return;

  priv->max_zoom_level = max_zoom_level;

  if (priv->zoom_level > max_zoom_level)
    champlain_view_set_zoom_level (view, max_zoom_level);
}

/**
 * champlain_view_add_layer:
 * @view: a #ChamplainView
 * @layer: a #ChamplainLayer
 *
 * Adds a new layer to the view
 *
 * Since: 0.2
 */
void
champlain_view_add_layer (ChamplainView *view,
    ChamplainLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));
  g_return_if_fail (CLUTTER_IS_ACTOR (layer));

  ChamplainViewPrivate *priv = view->priv;
  clutter_container_add (CLUTTER_CONTAINER (priv->user_layers),
      CLUTTER_ACTOR (layer), NULL);
  clutter_actor_raise_top (CLUTTER_ACTOR (layer));

  if (priv->map)
    g_idle_add (marker_reposition, view);

  g_signal_connect_after (layer, "actor-added",
      G_CALLBACK (layer_add_marker_cb), view);

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

  ChamplainViewPrivate *priv = view->priv;
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
  ChamplainViewPrivate *priv = view->priv;
  ChamplainRectangle viewport = priv->viewport_size;

  viewport.x += priv->anchor.x;
  viewport.y += priv->anchor.y;

  map_load_visible_tiles (priv->map, view, priv->map_source, viewport);
  view_update_state (view);
}

static void
view_position_tile (ChamplainView* view,
    ChamplainTile* tile)
{
  ChamplainViewPrivate *priv = view->priv;

  ClutterActor *actor;
  gint x;
  gint y;
  guint size;

  actor = champlain_tile_get_actor (tile);
  x = champlain_tile_get_x (tile);
  y = champlain_tile_get_y (tile);
  size = champlain_tile_get_size (tile);

  clutter_actor_set_position (actor,
    (x * size) - priv->anchor.x,
    (y * size) - priv->anchor.y);
}

static void
view_tiles_reposition (ChamplainView* view)
{
  ChamplainViewPrivate *priv = view->priv;
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
  if (animate)
    {
      etemplate = clutter_effect_template_new_for_duration (750, CLUTTER_ALPHA_SINE_INC);
      clutter_actor_set_opacity(actor, 0);
      clutter_effect_fade (etemplate, actor, 255, NULL, NULL);
    }

  clutter_container_add (CLUTTER_CONTAINER (champlain_zoom_level_get_actor (level)), actor, NULL);
  clutter_actor_show (actor);

  view_position_tile (view, tile);
  view_update_state (view);
}

static void
view_update_state (ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;
  ChamplainState new_state = CHAMPLAIN_STATE_DONE;
  gint i;

  for (i = 0; i < champlain_zoom_level_tile_count (priv->map->current_level); i++)
    {
      ChamplainTile *tile = champlain_zoom_level_get_nth_tile (priv->map->current_level, i);
      if (champlain_tile_get_state (tile) == CHAMPLAIN_STATE_LOADING)
        new_state = CHAMPLAIN_STATE_LOADING;
    }

  if (priv->state != new_state)
    {
      priv->state = new_state;
      g_object_notify (G_OBJECT (view), "state");
    }
}

/**
 * champlain_view_set_map_source:
 * @view: a #ChamplainView
 * @source: a #ChamplainMapSource
 *
 * Changes the currently used map source.  #g_object_unref will be called on
 * the previous one.
 *
 * Since: 0.4
 */
void
champlain_view_set_map_source (ChamplainView *view,
    ChamplainMapSource *source)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view) &&
      CHAMPLAIN_IS_MAP_SOURCE (source));

  ClutterActor *group;

  ChamplainViewPrivate *priv = view->priv;

  if (priv->map_source == source)
    return;

  g_object_unref (priv->map_source);
  priv->map_source = g_object_ref (source);

  priv->min_zoom_level = champlain_map_source_get_min_zoom_level (priv->map_source);
  priv->max_zoom_level = champlain_map_source_get_max_zoom_level (priv->map_source);

  if (priv->map == NULL)
    return;

  group = champlain_zoom_level_get_actor (priv->map->current_level);
  clutter_container_remove_actor (CLUTTER_CONTAINER (priv->map_layer), group);

  map_free (priv->map);
  priv->map = map_new ();

  /* Keep same zoom level if the new map supports it */
  if (priv->zoom_level > priv->max_zoom_level)
    {
      priv->zoom_level = priv->max_zoom_level;
      g_object_notify (G_OBJECT (view), "zoom-level");
    }
  else if (priv->zoom_level < priv->min_zoom_level)
    {
      priv->zoom_level = priv->min_zoom_level;
      g_object_notify (G_OBJECT (view), "zoom-level");
    }

  map_load_level (priv->map, priv->map_source, priv->zoom_level);
  group = champlain_zoom_level_get_actor (priv->map->current_level);

  view_load_visible_tiles (view);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer), group);

  update_license (view);
  g_idle_add (marker_reposition, view);
  view_tiles_reposition (view);
  champlain_view_center_on (view, priv->latitude, priv->longitude);
}

/**
* champlain_view_set_decel_rate:
* @view: a #ChamplainView
* @rate: a #gdouble between 0.0 and 2.0
*
* The deceleration rate for the kinetic mode.
*
* Since: 0.4
*/
void
champlain_view_set_decel_rate (ChamplainView *view,
    gdouble rate)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view) &&
      rate > 2.0 &&
      rate < 0);

  ChamplainViewPrivate *priv = view->priv;

  g_object_set (priv->finger_scroll, "decel-rate", rate, NULL);
}

/**
* champlain_view_set_scroll_mode:
* @view: a #ChamplainView
* @mode: a #ChamplainScrollMode value
*
* Determines the way the view reacts to scroll events.
*
* Since: 0.4
*/
void
champlain_view_set_scroll_mode (ChamplainView *view,
    ChamplainScrollMode mode)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = view->priv;

  priv->scroll_mode = mode;

  g_object_set (G_OBJECT (priv->finger_scroll), "mode",
      priv->scroll_mode, NULL);
}

/**
* champlain_view_set_keep_center_on_resize:
* @view: a #ChamplainView
* @value: a #gboolean
*
* Keep the current centered position when resizing the view.
*
* Since: 0.4
*/
void
champlain_view_set_keep_center_on_resize (ChamplainView *view,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = view->priv;

  priv->keep_center_on_resize = value;
}

/**
* champlain_view_set_show_license:
* @view: a #ChamplainView
* @value: a #gboolean
*
* Show the license on the map view.  The license information should always be
* available in a way or another in your application.  You can have it in
* About, or on the map.
*
* Since: 0.4
*/
void
champlain_view_set_show_license (ChamplainView *view,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = view->priv;

  priv->show_license = value;
  update_license (view);
}

/**
* champlain_view_set_zoom_on_double_click:
* @view: a #ChamplainView
* @value: a #gboolean
*
* Should the view zoom in and recenter when the user double click on the map.
*
* Since: 0.4
*/
void
champlain_view_set_zoom_on_double_click (ChamplainView *view,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = view->priv;

  priv->zoom_on_double_click = value;
}

/**
 * champlain_view_ensure_visible:
 * @view: a #ChamplainView
 * @lat1: the latitude of position 1
 * @lon1: the longitude of position 1
 * @lat2: the latitude of position 2
 * @lon2: the longitude of position 2
 *
 * Changes the map's zoom level and center to make sure the two given
 * positions are visible
 *
 * Since: 0.4
 */
void
champlain_view_ensure_visible (ChamplainView *view,
    gdouble lat1,
    gdouble lon1,
    gdouble lat2,
    gdouble lon2,
    gboolean animate)
{
  ChamplainViewPrivate *priv = view->priv;
  gint zoom_level = priv->zoom_level;
  gdouble width, height;
  gdouble min_lat,min_lon,max_lat,max_lon;
  gboolean good_size = FALSE;

  /*We first sort the lat,lon in order to have min and max */
  if (lat1 < lat2)
    {
      min_lat = lat1;
      max_lat = lat2;
    }
  else
    {
      max_lat = lat1;
      min_lat = lat2;
    }

  if (lon1 < lon2)
    {
      min_lon = lon1;
      max_lon = lon2;
    }
  else
    {
      max_lon = lon1;
      min_lon = lon2;
    }

  width = max_lon - min_lon;
  height = max_lat - min_lat;
  width *= 1.1;
  height *= 1.1;

  DEBUG("Zone to expose (%f, %f) to (%f, %f)", min_lat, min_lon, max_lat, max_lon);
  do
    {
      gint min_x, min_y, max_x, max_y;
      min_x = champlain_map_source_get_x (priv->map_source, zoom_level, min_lon);
      min_y = champlain_map_source_get_y (priv->map_source, zoom_level, min_lat);

      max_x = champlain_map_source_get_x (priv->map_source, zoom_level, max_lon);
      max_y = champlain_map_source_get_y (priv->map_source, zoom_level, max_lat);

      if (min_y - max_y <= priv->viewport_size.height &&
          max_x - min_x <= priv->viewport_size.width)
        good_size = TRUE;
      else
        zoom_level--;

      if (zoom_level <= priv->min_zoom_level)
        break;
    }
  while (good_size == FALSE);

  if (good_size == FALSE)
    {
      zoom_level = priv->min_zoom_level;
      min_lat = min_lon = width = height = 0;
    }

  DEBUG ("Ideal zoom level is %d", zoom_level);
  champlain_view_set_zoom_level (view, zoom_level);
  if (animate)
    champlain_view_go_to (view, min_lat + height / 2.0, min_lon + width / 2.0);
  else
    champlain_view_center_on (view, min_lat + height / 2.0, min_lon + width / 2.0);
}

/**
 * champlain_view_ensure_markers_visible:
 * @view: a #ChamplainView
 * @markers: a NULL terminated array of #ChamplainMarkers
 *
 * Changes the map's zoom level and center to make sure those markers are
 * visible.
 *
 * FIXME: This doesn't take into account the marker's actor size yet
 *
 * Since: 0.4
 */
void
champlain_view_ensure_markers_visible (ChamplainView *view,
    ChamplainBaseMarker *markers[],
    gboolean animate)
{
  gdouble min_lat, min_lon, max_lat, max_lon;
  ChamplainBaseMarker *marker = NULL;
  gint i = 0;

  min_lat = min_lon = 200;
  max_lat = max_lon = -200;

  marker = markers[i];
  while (marker != NULL)
    {
      gdouble lat, lon;
      g_object_get (G_OBJECT (marker), "latitude", &lat, "longitude", &lon,
          NULL);

      if (lon < min_lon)
        min_lon = lon;

      if (lat < min_lat)
        min_lat = lat;

      if (lon > max_lon)
        max_lon = lon;

      if (lat > max_lat)
        max_lat = lat;

      marker = markers[i++];
    }
  champlain_view_ensure_visible (view, min_lat, min_lon, max_lat, max_lon, animate);
}

/* Sets the zoom level, leaving the (x, y) at the exact same point in the view */
static gboolean
view_set_zoom_level_at (ChamplainView *view,
    gint zoom_level,
    gint x,
    gint y)
{
  ChamplainViewPrivate *priv = view->priv;

  ClutterActor *group, *new_group;
  gdouble lon, lat;
  gint x_diff, y_diff;
  gint actor_x, actor_y;
  gint rel_x, rel_y;
  gint x2, y2;
  gdouble lat2, lon2;

  if (zoom_level == priv->zoom_level || ZOOM_LEVEL_OUT_OF_RANGE(priv, zoom_level))
    return FALSE;

  champlain_view_stop_go_to (view);

  group = champlain_zoom_level_get_actor (priv->map->current_level);
  clutter_actor_get_transformed_position (priv->finger_scroll, &actor_x, &actor_y);
  rel_x = x - actor_x;
  rel_y = y - actor_y;

  /* Keep the lon, lat where the mouse is */
  lon = viewport_get_longitude_at (priv,
    priv->viewport_size.x + rel_x + priv->anchor.x);
  lat = viewport_get_latitude_at (priv,
    priv->viewport_size.y + rel_y + priv->anchor.y);

  /* How far was it from the center of the viewport (in px) */
  x_diff = priv->viewport_size.width / 2 - rel_x;
  y_diff = priv->viewport_size.height / 2 - rel_y;

  if (!map_zoom_to (priv->map, priv->map_source, zoom_level))
    return FALSE;

  priv->zoom_level = zoom_level;
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
  return TRUE;
}
