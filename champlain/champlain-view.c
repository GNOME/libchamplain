/*
 * Copyright (C) 2008-2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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
#include "champlain-marshal.h"
#include "champlain-map-source.h"
#include "champlain-map-source-factory.h"
#include "champlain-polygon.h"
#include "champlain-private.h"
#include "champlain-tile.h"

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
  PROP_LICENSE_EXTRA,
  PROP_ZOOM_ON_DOUBLE_CLICK,
  PROP_STATE,
  PROP_SHOW_SCALE,
  PROP_SCALE_UNIT,
  PROP_MAX_SCALE_WIDTH,
};

#define PADDING 10
static guint signals[LAST_SIGNAL] = { 0, };

#define GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CHAMPLAIN_TYPE_VIEW, ChamplainViewPrivate))
#define ZOOM_LEVEL_OUT_OF_RANGE(priv, level)    (level < priv->min_zoom_level || \
                                                 level > priv->max_zoom_level || \
                                                 level < champlain_map_source_get_min_zoom_level (priv->map_source) || \
                                                 level > champlain_map_source_get_max_zoom_level (priv->map_source))

#define CHAMPLAIN_MIN_LAT -90
#define CHAMPLAIN_MAX_LAT 90
#define CHAMPLAIN_MIN_LONG -180
#define CHAMPLAIN_MAX_LONG 180

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

typedef struct {
  ChamplainView *view;
  ChamplainPolygon *polygon;
} PolygonRedrawContext;

typedef struct {
  ChamplainTile *tile;
  ChamplainMapSource *map_source;
} FillTileCallbackData;

struct _ChamplainViewPrivate
{
  ClutterActor *stage;

  ChamplainMapSourceFactory *factory; /* The map source factory */
  ChamplainMapSource *map_source; /* Current map tile source */
  ChamplainScrollMode scroll_mode;
  gint zoom_level; /* Holds the current zoom level number */
  gint min_zoom_level; /* Lowest allowed zoom level */
  gint max_zoom_level; /* Highest allowed zoom level */

  /* Represents the (lat, lon) at the center of the viewport */
  gdouble longitude;
  gdouble latitude;

  /* Hack to get smaller x,y coordinates as the clutter limit is G_MAXINT16 */
  ChamplainFloatPoint anchor;

  gdouble anchor_zoom_level; /* the zoom_level for which the current anchor has
                                been computed for */

  ClutterActor *finger_scroll; /* Contains the viewport */
  ClutterActor *viewport;  /* Contains the map_layer, license and markers */
  ClutterActor *map_layer; /* Contains tiles actors (grouped by zoom level) */
  ChamplainRectangle viewport_size;

  ClutterActor *user_layers; /* Contains the markers */

  gboolean keep_center_on_resize;

  gboolean zoom_on_double_click;

  gboolean show_license;
  ClutterActor *license_actor; /* Contains the license info */
  gchar *license_text; /* Extra license text */

  ClutterActor *scale_actor;
  gboolean show_scale;
  ChamplainUnit scale_unit;
  guint max_scale_width;

  ChamplainState state; /* View's global state */

  /* champlain_view_go_to's context, kept for stop_go_to */
  GoToContext *goto_context;

  /* notify_polygon_cb's context, kept for idle cb */
  guint polygon_redraw_id;

  /* Lines and shapes */
  ClutterActor *polygon_layer;  /* Contains the polygons */

  guint update_cb_id;
  gboolean perform_update;
  gint tiles_loading;
};

G_DEFINE_TYPE (ChamplainView, champlain_view, CLUTTER_TYPE_GROUP);

static gdouble viewport_get_current_longitude (ChamplainViewPrivate *priv);
static gdouble viewport_get_current_latitude (ChamplainViewPrivate *priv);
static gdouble viewport_get_longitude_at (ChamplainViewPrivate *priv,
    gint x);
static gdouble viewport_get_latitude_at (ChamplainViewPrivate *priv,
    gint y);
static gboolean scroll_event (ClutterActor *actor,
    ClutterScrollEvent *event,
    ChamplainView *view);
static void marker_reposition_cb (ChamplainMarker *marker,
    ChamplainView *view);
static void layer_reposition_cb (ClutterActor *layer,
    ChamplainView *view);
static gboolean marker_reposition (gpointer data);
static void resize_viewport (ChamplainView *view);
static void champlain_view_get_property (GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec);
static void champlain_view_set_property (GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec);
static void champlain_view_dispose (GObject *object);
static void champlain_view_class_init (ChamplainViewClass *champlainViewClass);
static void champlain_view_init (ChamplainView *view);
static void viewport_pos_changed_cb (GObject *gobject,
    GParamSpec *arg1,
    ChamplainView *view);
static void notify_marker_reposition_cb (ChamplainMarker *marker,
    GParamSpec *arg1,
    ChamplainView *view);
static void layer_add_marker_cb (ClutterGroup *layer,
    ChamplainMarker *marker,
    ChamplainView *view);
static void connect_marker_notify_cb (ChamplainMarker *marker,
    ChamplainView *view);
static gboolean finger_scroll_button_press_cb (ClutterActor *actor,
    ClutterButtonEvent *event,
    ChamplainView *view);
static void update_license (ChamplainView *view);
static void update_scale (ChamplainView *view);
static void view_load_visible_tiles (ChamplainView *view);
static void view_position_tile (ChamplainView* view,
    ChamplainTile* tile);
static void view_reload_tiles_cb (ChamplainMapSource *map_source,
    ChamplainView* view);
static void view_update_state (ChamplainView *view, ChamplainTile *tile);
static void view_update_anchor (ChamplainView *view,
    gint x,
    gint y);
static gboolean view_set_zoom_level_at (ChamplainView *view,
    gint zoom_level,
    gint x,
    gint y);
static void tile_state_notify (GObject *gobject,
    GParamSpec *pspec,
    gpointer data);
static void view_update_polygons (ChamplainView *view);
static gboolean finger_scroll_key_press_cb (ClutterActor *actor,
    ClutterKeyEvent *event,
    ChamplainView *view);
static void champlain_view_go_to_with_duration (ChamplainView *view,
    gdouble latitude,
    gdouble longitude,
    guint duration);
static gboolean perform_update_cb (ChamplainView *view);
static gboolean fill_tile_cb (FillTileCallbackData *data);
static void tile_destroyed_cb (GObject *gobject,
    gpointer data);

#define SCALE_HEIGHT  20
#define SCALE_PADDING 10
#define SCALE_INSIDE_PADDING 10
#define SCALE_LINE_WIDTH 2

static gdouble
viewport_get_longitude_at (ChamplainViewPrivate *priv,
    gint x)
{
  if (!priv->map_source)
    return 0.0;

  return champlain_map_source_get_longitude (priv->map_source,
      priv->zoom_level, x);
}

static gdouble
viewport_get_current_longitude (ChamplainViewPrivate *priv)
{
  return viewport_get_longitude_at (priv, priv->anchor.x +
      priv->viewport_size.x + priv->viewport_size.width / 2.0);
}

static gdouble
viewport_get_latitude_at (ChamplainViewPrivate *priv,
    gint y)
{
  if (!priv->map_source)
    return 0.0;

  return champlain_map_source_get_latitude (priv->map_source,
      priv->zoom_level, y);
}

static gdouble
viewport_get_current_latitude (ChamplainViewPrivate *priv)
{
  return viewport_get_latitude_at (priv,
      priv->anchor.y + priv->viewport_size.y +
      priv->viewport_size.height / 2.0);
}

/* Updates the internals after the viewport changed */
static void
update_viewport (ChamplainView *view,
    gfloat x,
    gfloat y)
{
  ChamplainViewPrivate *priv = view->priv;
  gfloat lat, lon;

  ChamplainFloatPoint old_anchor;

  old_anchor.x = priv->anchor.x;
  old_anchor.y = priv->anchor.y;

  view_update_anchor (view,
      x + priv->anchor.x + priv->viewport_size.width / 2.0,
      y + priv->anchor.y + priv->viewport_size.height / 2.0);

  if (priv->anchor.x - old_anchor.x != 0)
    {
      ChamplainFloatPoint diff;

      diff.x = priv->anchor.x - old_anchor.x;
      diff.y = priv->anchor.y - old_anchor.y;

      DEBUG("Relocating the viewport by %f, %f", diff.x, diff.y);

      g_signal_handlers_block_by_func (priv->viewport, G_CALLBACK (viewport_pos_changed_cb), view);
      tidy_viewport_set_origin (TIDY_VIEWPORT (priv->viewport),
          x - diff.x, y - diff.y, 0);
      g_signal_handlers_unblock_by_func (priv->viewport, G_CALLBACK (viewport_pos_changed_cb), view);
 //     return;
    }

  priv->viewport_size.x = x;
  priv->viewport_size.y = y;

  view_load_visible_tiles (view);
  marker_reposition (view);
  update_scale (view);

  view_update_polygons (view);
  lon = viewport_get_current_longitude (priv);
  lat = viewport_get_current_latitude (priv);

  priv->longitude = lon;
  priv->latitude = lat;

  if (fabs (priv->longitude - lon) > 0.001)
    g_object_notify (G_OBJECT (view), "longitude");

  if (fabs (priv->latitude - lat) > 0.001)
    g_object_notify (G_OBJECT (view), "latitude");
}

static void
panning_completed (TidyFingerScroll *scroll,
    ChamplainView *view)
{
  gfloat x, y;

  tidy_viewport_get_origin (TIDY_VIEWPORT (view->priv->viewport), &x, &y,
      NULL);

  update_viewport (view, x, y);
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
  ChamplainBaseMarker *base_marker = CHAMPLAIN_BASE_MARKER (marker);

  gint x, y;

  x = champlain_map_source_get_x (priv->map_source, priv->zoom_level,
          champlain_base_marker_get_longitude (base_marker));
  y = champlain_map_source_get_y (priv->map_source, priv->zoom_level,
          champlain_base_marker_get_latitude (base_marker));

  clutter_actor_set_position (CLUTTER_ACTOR (marker),
    x - priv->anchor.x,
    y - priv->anchor.y);
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
  clutter_container_foreach (CLUTTER_CONTAINER (view->priv->user_layers),
      CLUTTER_CALLBACK (layer_reposition_cb), view);
  return FALSE;
}

static gboolean
redraw_polygon_on_idle (PolygonRedrawContext *ctx)
{
  ChamplainViewPrivate *priv = ctx->view->priv;

  if (ctx->polygon)
    champlain_polygon_draw_polygon (ctx->polygon,
        priv->map_source, priv->zoom_level,
        priv->viewport_size.width, priv->viewport_size.height,
        priv->viewport_size.x + priv->anchor.x,
        priv->viewport_size.y + priv->anchor.y);

  priv->polygon_redraw_id = 0;
  // ctx is freed by g_idle_add_full
  return FALSE;
}

static void
notify_polygon_cb (ChamplainPolygon *polygon,
    GParamSpec *arg1,
    ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;
  PolygonRedrawContext *ctx;

  if (priv->polygon_redraw_id != 0)
    return;

  ctx = g_new0 (PolygonRedrawContext, 1);
  ctx->view = view;
  ctx->polygon = polygon;
  g_object_add_weak_pointer (G_OBJECT (polygon), (gpointer *) &ctx->polygon);

  priv->polygon_redraw_id = g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
      (GSourceFunc) redraw_polygon_on_idle, ctx, g_free);
}

static void
resize_viewport (ChamplainView *view)
{
  gdouble lower, upper;
  gint i;
  TidyAdjustment *hadjust, *vadjust;

  ChamplainViewPrivate *priv = view->priv;

  tidy_scrollable_get_adjustments (TIDY_SCROLLABLE (priv->viewport), &hadjust,
      &vadjust);

  if (priv->zoom_level < 8)
    {
      lower = -priv->viewport_size.width / 2.0;
      upper = champlain_map_source_get_column_count (priv->map_source, priv->zoom_level) *
          champlain_map_source_get_tile_size (priv->map_source) -
          priv->viewport_size.width / 2.0;
    }
  else
    {
      lower = 0;
      upper = G_MAXINT16;
    }

  /* block emmision of signal by priv->viewport with viewport_pos_changed_cb()
     callback - the signal can be emitted by updating TidyAdjustment, but
     calling the callback now would be a disaster since we don't have updated
     anchor yet*/
  g_signal_handlers_block_by_func (priv->viewport, G_CALLBACK (viewport_pos_changed_cb), view);

  g_object_set (hadjust, "lower", lower, "upper", upper,
      "page-size", 1.0, "step-increment", 1.0, "elastic", TRUE, NULL);

  if (priv->zoom_level < 8)
    {
      lower = -priv->viewport_size.height / 2.0;
      upper = champlain_map_source_get_row_count (priv->map_source, priv->zoom_level) *
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

  /* no more updates of TidyAdjustment, we can unblock the signal again */
  g_signal_handlers_unblock_by_func (priv->viewport, G_CALLBACK (viewport_pos_changed_cb), view);

  /* Resize polygon actors */
  if (priv->viewport_size.width == 0 ||
      priv->viewport_size.height == 0)
    return;

  for (i = 0; i < clutter_group_get_n_children (CLUTTER_GROUP (priv->polygon_layer)); i++)
    {
      ChamplainPolygon *polygon = CHAMPLAIN_POLYGON (clutter_group_get_nth_child (CLUTTER_GROUP (priv->polygon_layer), i));

      clutter_actor_set_position (CLUTTER_ACTOR (polygon), 0, 0);
      champlain_polygon_draw_polygon (polygon, priv->map_source, priv->zoom_level,
                                      priv->viewport_size.width, priv->viewport_size.height,
                                      priv->viewport_size.x + priv->anchor.x, priv->viewport_size.y + priv->anchor.y);
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
        g_value_set_double (value,
            CLAMP (priv->longitude, CHAMPLAIN_MIN_LONG, CHAMPLAIN_MAX_LONG));
        break;
      case PROP_LATITUDE:
        g_value_set_double (value,
            CLAMP (priv->latitude, CHAMPLAIN_MIN_LAT, CHAMPLAIN_MAX_LAT));
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
      case PROP_SHOW_SCALE:
        g_value_set_boolean (value, priv->show_scale);
        break;
      case PROP_MAX_SCALE_WIDTH:
        g_value_set_uint (value, priv->max_scale_width);
        break;
      case PROP_SCALE_UNIT:
        g_value_set_enum (value, priv->scale_unit);
        break;
      case PROP_DECEL_RATE:
        {
          gdouble decel = 0.0;
          g_object_get (priv->finger_scroll, "decel-rate", &decel, NULL);
          g_value_set_double (value, decel);
          break;
        }
      case PROP_KEEP_CENTER_ON_RESIZE:
        g_value_set_boolean (value, priv->keep_center_on_resize);
        break;
      case PROP_SHOW_LICENSE:
        g_value_set_boolean (value, priv->show_license);
        break;
      case PROP_LICENSE_EXTRA:
        g_value_set_string (value, priv->license_text);
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
    case PROP_SHOW_SCALE:
      champlain_view_set_show_scale (view, g_value_get_boolean (value));
      break;
    case PROP_MAX_SCALE_WIDTH:
      champlain_view_set_max_scale_width (view, g_value_get_uint (value));
      break;
    case PROP_SCALE_UNIT:
      champlain_view_set_scale_unit (view, g_value_get_enum (value));
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
    case PROP_LICENSE_EXTRA:
      champlain_view_set_license_text (view, g_value_get_string (value));
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

  g_source_remove (priv->update_cb_id);

  if (priv->factory != NULL)
    {
      g_object_unref (priv->factory);
      priv->factory = NULL;
    }

  if (priv->map_source != NULL)
    {
      g_object_unref (priv->map_source);
      priv->map_source = NULL;
    }

  if (priv->stage != NULL)
    {
      g_object_unref (priv->stage);
      priv->stage = NULL;
    }

  if (priv->license_actor != NULL)
    {
      g_object_unref (priv->license_actor);
      priv->license_actor = NULL;
    }

  if (priv->scale_actor != NULL)
    {
      g_object_unref (priv->scale_actor);
      priv->scale_actor = NULL;
    }

  if (priv->finger_scroll != NULL)
    {
      tidy_finger_scroll_stop (TIDY_FINGER_SCROLL (priv->finger_scroll));
      g_object_unref (priv->finger_scroll);
      priv->finger_scroll = NULL;
    }

  if (priv->viewport != NULL)
    {
      tidy_viewport_stop (TIDY_VIEWPORT (priv->viewport));
      g_object_unref (priv->viewport);
      priv->viewport = NULL;
    }

  if (priv->map_layer != NULL)
    {
      g_object_unref (priv->map_layer);
      priv->map_layer = NULL;
    }

  if (priv->user_layers != NULL)
    {
      g_object_unref (priv->user_layers);
      priv->user_layers = NULL;
    }

  if (priv->polygon_layer != NULL)
    {
      g_object_unref (CLUTTER_ACTOR (priv->polygon_layer));
      priv->polygon_layer = NULL;
    }

  if (priv->goto_context != NULL)
    champlain_view_stop_go_to (view);

  G_OBJECT_CLASS (champlain_view_parent_class)->dispose (object);
}

static gboolean
_update_idle_cb (ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;

  clutter_actor_set_size (priv->finger_scroll,
                          priv->viewport_size.width,
                          priv->viewport_size.height);

  resize_viewport (view);

  clutter_actor_set_position (priv->license_actor,
      priv->viewport_size.width - PADDING,
      priv->viewport_size.height - PADDING);
  clutter_actor_set_position (priv->scale_actor,
      SCALE_PADDING,
      priv->viewport_size.height - SCALE_HEIGHT - SCALE_PADDING);

  if (priv->keep_center_on_resize)
    champlain_view_center_on (view, priv->latitude, priv->longitude);
  else
    view_load_visible_tiles (view);

  return FALSE;
}

static void
champlain_view_allocate (ClutterActor *actor,
    const ClutterActorBox *box,
    ClutterAllocationFlags flags)
{
  ChamplainView *view = CHAMPLAIN_VIEW (actor);
  ChamplainViewPrivate *priv = view->priv;
  guint width, height;

  /* Chain up */
  CLUTTER_ACTOR_CLASS (champlain_view_parent_class)->allocate (actor, box, flags);

  width = box->x2 - box->x1;
  height = box->y2 - box->y1;

  if (priv->viewport_size.width == width && priv->viewport_size.height == height)
    return;

  priv->viewport_size.width = width;
  priv->viewport_size.height = height;

  g_idle_add_full (G_PRIORITY_HIGH_IDLE,
                   (GSourceFunc)_update_idle_cb,
                   g_object_ref (view),
                   (GDestroyNotify)g_object_unref);
}

static void
champlain_view_realize (ClutterActor *actor)
{
  ChamplainView *view = CHAMPLAIN_VIEW (actor);
  ChamplainViewPrivate *priv = view->priv;

  /*
   We should be calling this but it segfaults
   CLUTTER_ACTOR_CLASS (champlain_view_parent_class)->realize (actor);
   ClutterStage uses clutter_actor_realize.
   */
  clutter_actor_realize (actor);

  /* Setup the viewport according to the zoom level */
  //resize_viewport (view);

  g_object_notify (G_OBJECT (view), "zoom-level");
  g_object_notify (G_OBJECT (view), "map-source");

  /* this call will launch the tiles loading */
  champlain_view_center_on (view, priv->latitude, priv->longitude);

  update_scale (view);
  update_license (view);
}

/* These return fixed sizes because either a.) We expect the user to size
 * explicitly with clutter_actor_get_size or b.) place it in a container that
 * allocates it whatever it wants.
 */
static void
champlain_view_get_preferred_width (ClutterActor *actor,
    gfloat for_height,
    gfloat *min_width,
    gfloat *nat_width)
{
  ChamplainView *view = CHAMPLAIN_VIEW (actor);
  gint width = champlain_map_source_get_tile_size (view->priv->map_source);

  if (min_width)
    *min_width = 1;

  if (nat_width)
    *nat_width = width;
}

static void
champlain_view_get_preferred_height (ClutterActor *actor,
    gfloat for_width,
    gfloat *min_height,
    gfloat *nat_height)
{
  ChamplainView *view = CHAMPLAIN_VIEW (actor);
  gint height = champlain_map_source_get_tile_size (view->priv->map_source);

  if (min_height)
    *min_height = 1;

  if (nat_height)
    *nat_height = height;
}

static void
champlain_view_class_init (ChamplainViewClass *champlainViewClass)
{
  g_type_class_add_private (champlainViewClass, sizeof (ChamplainViewPrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (champlainViewClass);
  object_class->dispose = champlain_view_dispose;
  object_class->get_property = champlain_view_get_property;
  object_class->set_property = champlain_view_set_property;

  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (champlainViewClass);
  actor_class->allocate = champlain_view_allocate;
  actor_class->get_preferred_width = champlain_view_get_preferred_width;
  actor_class->get_preferred_height = champlain_view_get_preferred_height;
  actor_class->realize = champlain_view_realize;

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
            1.0001, 2.0, 1.1, CHAMPLAIN_PARAM_READWRITE));

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
  * ChamplainView:license-text:
  *
  * Sets additional text to be displayed in the license area.  The map's
  * license will be added below it. Your text can have multiple line, just use
  * "\n" in between.
  *
  * Since: 0.4.3
  */
  g_object_class_install_property (object_class,
       PROP_LICENSE_EXTRA,
       g_param_spec_string ("license-text",
           "Additional license",
           "Additional license text",
           "",
           CHAMPLAIN_PARAM_READWRITE));

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
           CHAMPLAIN_STATE_NONE,
           G_PARAM_READABLE));

  /**
  * ChamplainView:show-scale:
  *
  * Display the map scale.
  *
  * Since: 0.4.3
  */
  g_object_class_install_property (object_class,
       PROP_SHOW_SCALE,
       g_param_spec_boolean ("show-scale",
           "Show the map scale",
           "Show the map scale "
           "on the screen",
           FALSE,
           G_PARAM_READWRITE));

  /**
  * ChamplainView:max-scale-width:
  *
  * The size of the map scale on screen in pixels.
  *
  * Since: 0.4.3
  */
  g_object_class_install_property (object_class,
       PROP_MAX_SCALE_WIDTH,
       g_param_spec_uint ("max-scale-width",
           "The width of the scale",
           "The max width of the scale"
           "on screen",
           1,
           2000,
           100,
           G_PARAM_READWRITE));

  /**
  * ChamplainView:scale-unit:
  *
  * The scale's units.
  *
  * Since: 0.4.3
  */
  g_object_class_install_property (object_class,
       PROP_SCALE_UNIT,
       g_param_spec_enum ("scale-unit",
           "The scale's unit",
           "The map scale's unit",
           CHAMPLAIN_TYPE_UNIT,
           CHAMPLAIN_UNIT_KM,
           G_PARAM_READWRITE));

  /**
  * ChamplainView::animation-completed:
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
create_license (ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;

  if (priv->license_actor)
    {
      g_object_unref (priv->license_actor);
      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->stage), priv->license_actor);
    }

  priv->license_actor = g_object_ref (clutter_text_new ());
  clutter_text_set_font_name (CLUTTER_TEXT (priv->license_actor), "sans 8");
  clutter_text_set_line_alignment (CLUTTER_TEXT (priv->license_actor), PANGO_ALIGN_RIGHT);
  clutter_actor_set_opacity (priv->license_actor, 128);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->stage),
      priv->license_actor);
  clutter_actor_set_anchor_point_from_gravity (priv->license_actor, CLUTTER_GRAVITY_SOUTH_EAST);
  clutter_actor_raise_top (priv->license_actor);
}

static gboolean
button_release_cb (ClutterActor *actor,
    ClutterEvent *event,
    ChamplainView *view)
{
  guint i;
  gboolean found = FALSE;
  ChamplainViewPrivate *priv = view->priv;

  if (clutter_event_get_button (event) != 1)
    return FALSE;

  for (i = 0; i < clutter_group_get_n_children (CLUTTER_GROUP (priv->user_layers)); i++)
    {
      ChamplainSelectionLayer *layer = CHAMPLAIN_SELECTION_LAYER (clutter_group_get_nth_child (CLUTTER_GROUP (priv->user_layers), i));

      if (layer && champlain_selection_layer_count_selected_markers (layer ) > 0)
        {
          champlain_selection_layer_unselect_all (layer);
          found = TRUE;
        }
    }

  return found;
}

static void
update_scale (ChamplainView *view)
{
  static gfloat previous_m_per_pixel = 0.0;
  static gint previous_zoom_level = 0.0;

  gboolean is_small_unit = TRUE;  /* indicates if using meters */
  ClutterActor *text, *line;
  gfloat width;
  ChamplainViewPrivate *priv = view->priv;
  gfloat m_per_pixel;
  gfloat scale_width = priv->max_scale_width;
  gchar *label;
  cairo_t *cr;
  gfloat base;
  gfloat factor;
  gboolean final_unit = FALSE;

  if (priv->show_scale)
    {
      clutter_actor_show (priv->scale_actor);
    }
  else
    {
      clutter_actor_hide (priv->scale_actor);
      return;
    }

  m_per_pixel = champlain_map_source_get_meters_per_pixel (priv->map_source,
      priv->zoom_level, priv->latitude, priv->longitude);

  /* Don't redraw too often, 1 meters difference is a good value
   * since at low levels the value changes alot, and not at high levels */
  if (fabs (m_per_pixel - previous_m_per_pixel) < 10 &&
      previous_zoom_level == priv->zoom_level)
    return;

  previous_m_per_pixel = m_per_pixel;
  previous_zoom_level = priv->zoom_level;

  if (priv->scale_unit == CHAMPLAIN_UNIT_MILES)
    m_per_pixel *= 3.28; /* m_per_pixel is now in ft */

  /* This loop will find the pretty value to display on the scale.
   * It will be run once for metric units, and twice for imperials
   * so that both feet and miles have pretty numbers.
   */
  do
    {
      /* Keep the previous power of 10 */
      base = floor (log (m_per_pixel * scale_width) / log (10));
      base = pow (10, base);

      /* How many times can it be fitted in our max scale width */
      g_assert (base > 0);
      g_assert (m_per_pixel * scale_width / base > 0);
      scale_width /= m_per_pixel * scale_width / base;
      g_assert (scale_width > 0);
      factor = floor (priv->max_scale_width / scale_width);
      base *= factor;
      scale_width *= factor;

      if (priv->scale_unit == CHAMPLAIN_UNIT_KM)
        {
          if (base / 1000.0 >= 1)
            {
              base /= 1000.0; /* base is now in km */
              is_small_unit = FALSE;
            }
          final_unit = TRUE; /* Don't need to recompute */
        }
      else if (priv->scale_unit == CHAMPLAIN_UNIT_MILES)
        {
          if (is_small_unit && base / 5280.0 >= 1)
            {
              m_per_pixel /= 5280.0; /* m_per_pixel is now in miles */
              is_small_unit = FALSE;
              /* we need to recompute the base because 1000 ft != 1 mile */
            }
          else
            final_unit = TRUE;
        }
    }
  while (!final_unit);

  text = clutter_container_find_child_by_name (CLUTTER_CONTAINER (priv->scale_actor), "scale-far-label");
  label = g_strdup_printf ("%g", base);
  /* Get only digits width for centering */
  clutter_text_set_text (CLUTTER_TEXT (text), label);
  g_free (label);
  clutter_actor_get_size (text, &width, NULL);
  /* actual label with unit */
  label = g_strdup_printf ("%g %s", base,
      priv->scale_unit == CHAMPLAIN_UNIT_KM ?
      (is_small_unit ? "m": "km"):
      (is_small_unit ? "ft": "miles")
      );
  clutter_text_set_text (CLUTTER_TEXT (text), label);
  g_free (label);
  clutter_actor_set_position (text, (scale_width - width / 2) + SCALE_INSIDE_PADDING, - SCALE_INSIDE_PADDING);

  text = clutter_container_find_child_by_name (CLUTTER_CONTAINER (priv->scale_actor), "scale-mid-label");
  label = g_strdup_printf ("%g", base / 2.0);
  clutter_text_set_text (CLUTTER_TEXT (text), label);
  clutter_actor_get_size (text, &width, NULL);
  clutter_actor_set_position (text, (scale_width - width) / 2 + SCALE_INSIDE_PADDING, - SCALE_INSIDE_PADDING);
  g_free (label);

  /* Draw the line */
  line = clutter_container_find_child_by_name (CLUTTER_CONTAINER (priv->scale_actor), "scale-line");
  clutter_cairo_texture_clear (CLUTTER_CAIRO_TEXTURE (line));
  cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (line));

  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_width (cr, SCALE_LINE_WIDTH);

  /* First tick */
  cairo_move_to (cr, SCALE_INSIDE_PADDING, SCALE_HEIGHT / 4);
  cairo_line_to (cr, SCALE_INSIDE_PADDING, SCALE_HEIGHT / 2 );
  cairo_stroke (cr);

  /* Line */
  cairo_move_to (cr, SCALE_INSIDE_PADDING, SCALE_HEIGHT / 2);
  cairo_line_to (cr, scale_width + SCALE_INSIDE_PADDING, SCALE_HEIGHT / 2);
  cairo_stroke (cr);

  /* Middle tick */
  cairo_move_to (cr, scale_width / 2 + SCALE_INSIDE_PADDING, SCALE_HEIGHT / 4);
  cairo_line_to (cr, scale_width / 2 + SCALE_INSIDE_PADDING, SCALE_HEIGHT / 2);
  cairo_stroke (cr);

  /* Last tick */
  cairo_move_to (cr, scale_width + SCALE_INSIDE_PADDING, SCALE_HEIGHT / 4);
  cairo_line_to (cr, scale_width + SCALE_INSIDE_PADDING, SCALE_HEIGHT / 2);
  cairo_stroke (cr);

  cairo_destroy (cr);
}

static void
create_scale (ChamplainView *view)
{
  ClutterActor *scale, *text;
  gfloat width;
  ChamplainViewPrivate *priv = view->priv;

  if (priv->scale_actor)
    {
      g_object_unref (priv->scale_actor);
      clutter_container_remove_actor (CLUTTER_CONTAINER (priv->stage), priv->scale_actor);
    }

  priv->scale_actor = g_object_ref (clutter_group_new());
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->stage), priv->scale_actor);

  scale = clutter_cairo_texture_new (priv->max_scale_width + 2 * SCALE_INSIDE_PADDING, SCALE_HEIGHT + 2 * SCALE_INSIDE_PADDING);
  clutter_actor_set_name (scale, "scale-line");

  text = clutter_text_new_with_text ("Sans 9", "X km");
  clutter_actor_set_name (text, "scale-far-label");
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->scale_actor), text);

  text = clutter_text_new_with_text ("Sans 9", "X km");
  clutter_actor_set_name (text, "scale-mid-label");
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->scale_actor), text);

  text = clutter_text_new_with_text ("Sans 9", "0");
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->scale_actor), text);
  clutter_actor_get_size (text, &width, NULL);
  clutter_actor_set_position (text, SCALE_INSIDE_PADDING - width / 2, - SCALE_INSIDE_PADDING);

  clutter_container_add_actor (CLUTTER_CONTAINER (priv->scale_actor), scale);
  clutter_actor_set_position (priv->scale_actor, SCALE_PADDING - SCALE_INSIDE_PADDING,
    priv->viewport_size.height - SCALE_HEIGHT - SCALE_PADDING - SCALE_INSIDE_PADDING);

  clutter_actor_set_opacity (priv->scale_actor, 200);
  clutter_actor_raise_top (priv->scale_actor);
}

static void
champlain_view_init (ChamplainView *view)
{
  ChamplainViewPrivate *priv = GET_PRIVATE (view);
  ChamplainMapSource *source;

  champlain_debug_set_flags (g_getenv ("CHAMPLAIN_DEBUG"));

  view->priv = priv;

  priv->factory = champlain_map_source_factory_dup_default ();
  source = champlain_map_source_factory_create_cached_source (priv->factory, CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK);

  priv->map_source = CHAMPLAIN_MAP_SOURCE(source);

  priv->zoom_level = 0;
  priv->min_zoom_level = champlain_map_source_get_min_zoom_level (priv->map_source);
  priv->max_zoom_level = champlain_map_source_get_max_zoom_level (priv->map_source);
  priv->keep_center_on_resize = TRUE;
  priv->zoom_on_double_click = TRUE;
  priv->show_license = TRUE;
  priv->license_actor = NULL;
  priv->license_text = NULL;
  priv->stage = NULL;
  priv->scroll_mode = CHAMPLAIN_SCROLL_MODE_PUSH;
  priv->viewport_size.x = 0;
  priv->viewport_size.y = 0;
  priv->viewport_size.width = 0;
  priv->viewport_size.height = 0;
  priv->anchor.x = 0;
  priv->anchor.y = 0;
  priv->anchor_zoom_level = 0;
  priv->state = CHAMPLAIN_STATE_NONE;
  priv->latitude = 0.0f;
  priv->longitude = 0.0f;
  priv->goto_context = NULL;
  priv->polygon_redraw_id = 0;
  priv->show_scale = FALSE;
  priv->scale_unit = CHAMPLAIN_UNIT_KM;
  priv->max_scale_width = 100;
  priv->perform_update = TRUE;
  priv->tiles_loading = 0;

  /* Setup map layer */
  priv->map_layer = g_object_ref (clutter_group_new ());
  clutter_actor_show (priv->map_layer);

  /* Setup polygon layer */
  priv->polygon_layer = g_object_ref (clutter_group_new ());
  clutter_actor_show (priv->polygon_layer);

  /* Setup user_layers */
  priv->user_layers = g_object_ref (clutter_group_new ());
  clutter_actor_show (priv->user_layers);

  /* Setup viewport */
  priv->viewport = g_object_ref (tidy_viewport_new ());
  g_object_set (G_OBJECT (priv->viewport), "sync-adjustments", FALSE, NULL);

  g_signal_connect (priv->viewport, "notify::x-origin",
      G_CALLBACK (viewport_pos_changed_cb), view);
  g_signal_connect (priv->viewport, "notify::y-origin",
      G_CALLBACK (viewport_pos_changed_cb), view);

  clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport),
      priv->map_layer);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport),
      priv->polygon_layer);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->viewport),
      priv->user_layers);

  clutter_actor_raise (priv->polygon_layer, priv->map_layer);
  clutter_actor_raise (priv->user_layers, priv->map_layer);

  /* Setup finger scroll */
  priv->finger_scroll = g_object_ref (tidy_finger_scroll_new (priv->scroll_mode));

  g_signal_connect (priv->finger_scroll, "scroll-event",
      G_CALLBACK (scroll_event), view);
  g_signal_connect (priv->finger_scroll, "panning-completed",
      G_CALLBACK (panning_completed), view);
  g_signal_connect (priv->finger_scroll, "button-press-event",
      G_CALLBACK (finger_scroll_button_press_cb), view);
  g_signal_connect_after (priv->finger_scroll, "button-release-event",
      G_CALLBACK (button_release_cb), view);

  clutter_stage_set_key_focus (CLUTTER_STAGE (clutter_stage_get_default()),
      priv->finger_scroll);
  g_signal_connect (priv->finger_scroll, "key-press-event",
      G_CALLBACK (finger_scroll_key_press_cb), view);

  clutter_container_add_actor (CLUTTER_CONTAINER (priv->finger_scroll),
      priv->viewport);

  /* Setup stage */
  priv->stage = g_object_ref (clutter_group_new ());

  clutter_container_add_actor (CLUTTER_CONTAINER (priv->stage),
      priv->finger_scroll);

  clutter_container_add_actor (CLUTTER_CONTAINER (view), priv->stage);

  resize_viewport (view);

  /* Setup scale */
  create_scale (view);

  /* Setup license */
  create_license (view);

  priv->update_cb_id = g_timeout_add (250, (GSourceFunc) perform_update_cb, view);

  priv->state = CHAMPLAIN_STATE_DONE;
  g_object_notify (G_OBJECT (view), "state");

  g_signal_connect (priv->map_source, "reload-tiles",
    G_CALLBACK (view_reload_tiles_cb), view);
}

static gboolean
perform_update_cb (ChamplainView *view)
{
  view->priv->perform_update = TRUE;
  return TRUE;
}

static void
viewport_pos_changed_cb (GObject *gobject,
    GParamSpec *arg1,
    ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;

  gfloat x, y;

  tidy_viewport_get_origin (TIDY_VIEWPORT (priv->viewport), &x, &y,
      NULL);

  if (fabs (x - priv->viewport_size.x) > 100 ||
      fabs (y - priv->viewport_size.y) > 100 ||
      priv->perform_update)
    {
      priv->perform_update = FALSE;

      update_viewport (view, x, y);
    }
}

/**
 * champlain_view_set_size:
 * @view: a #ChamplainView
 * @width: the width in pixels
 * @height: the height in pixels
 *
 * Sets the size of the view.  This function is deprecated and should not be used in new code
 * Use #clutter_actor_set_size instead.
 *
 * Since: 0.1
 */
//FIXME: move to an handler of actor size change
void
champlain_view_set_size (ChamplainView *view,
    guint width,
    guint height)
{
  clutter_actor_set_size (CLUTTER_ACTOR (view), width, height);
}

static void
update_license (ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;
  gchar *license;

  if (priv->license_text)
    license = g_strjoin ("\n",
        priv->license_text,
        champlain_map_source_get_license (priv->map_source),
        NULL);
  else
    license = g_strdup (champlain_map_source_get_license (priv->map_source));

  clutter_text_set_text (CLUTTER_TEXT (priv->license_actor), license);

  if (priv->show_license)
    clutter_actor_show (priv->license_actor);
  else
    clutter_actor_hide (priv->license_actor);

  g_free (license);
}

static gboolean
finger_scroll_button_press_cb (ClutterActor *actor,
    ClutterButtonEvent *event,
    ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;

  if (priv->zoom_on_double_click && event->button == 1 && event->click_count == 2)
    return view_set_zoom_level_at (view, priv->zoom_level + 1, event->x, event->y);

  return FALSE; /* Propagate the event */
}

static void
scroll_to (ChamplainView *view,
    gint x,
    gint y)
{
  ChamplainViewPrivate *priv = view->priv;
  gfloat lat, lon;

  lat = champlain_map_source_get_latitude (priv->map_source, priv->zoom_level, y);
  lon = champlain_map_source_get_longitude (priv->map_source, priv->zoom_level, x);

  if (priv->scroll_mode == CHAMPLAIN_SCROLL_MODE_KINETIC)
    champlain_view_go_to_with_duration (view, lat, lon, 300);
  else if (priv->scroll_mode == CHAMPLAIN_SCROLL_MODE_PUSH)
    champlain_view_center_on (view, lat, lon);
}

/* These functions should be exposed in the next API break */
static void
champlain_view_scroll_left (ChamplainView* view)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  gint x, y;
  ChamplainViewPrivate *priv = view->priv;

  x = champlain_map_source_get_x (priv->map_source, priv->zoom_level, priv->longitude);
  y = champlain_map_source_get_y (priv->map_source, priv->zoom_level, priv->latitude);

  x -= priv->viewport_size.width / 4.0;

  scroll_to (view, x, y);
}

static void
champlain_view_scroll_right (ChamplainView* view)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  gint x, y;
  ChamplainViewPrivate *priv = view->priv;

  x = champlain_map_source_get_x (priv->map_source, priv->zoom_level, priv->longitude);
  y = champlain_map_source_get_y (priv->map_source, priv->zoom_level, priv->latitude);

  x += priv->viewport_size.width / 4.0;

  scroll_to (view, x, y);
}

static void
champlain_view_scroll_up (ChamplainView* view)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  gint x, y;
  ChamplainViewPrivate *priv = view->priv;

  x = champlain_map_source_get_x (priv->map_source, priv->zoom_level, priv->longitude);
  y = champlain_map_source_get_y (priv->map_source, priv->zoom_level, priv->latitude);

  y -= priv->viewport_size.width / 4.0;

  scroll_to (view, x, y);
}

static void
champlain_view_scroll_down (ChamplainView* view)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  gint x, y;
  ChamplainViewPrivate *priv = view->priv;

  x = champlain_map_source_get_x (priv->map_source, priv->zoom_level, priv->longitude);
  y = champlain_map_source_get_y (priv->map_source, priv->zoom_level, priv->latitude);

  y += priv->viewport_size.width / 4.0;

  scroll_to (view, x, y);
}


static gboolean
finger_scroll_key_press_cb (ClutterActor *actor,
    ClutterKeyEvent *event,
    ChamplainView *view)
{
  switch (event->keyval)
  {
    case 65361: // Left
      champlain_view_scroll_left (view);
      return TRUE;
      break;
    case 65362: // Up
      if (event->modifier_state & CLUTTER_CONTROL_MASK)
        champlain_view_zoom_in (view);
      else
        champlain_view_scroll_up (view);
      return TRUE;
      break;
    case 65363: // Right
      champlain_view_scroll_right (view);
      return TRUE;
      break;
    case 65364: // Down
      if (event->modifier_state & CLUTTER_CONTROL_MASK)
        champlain_view_zoom_out (view);
      else
        champlain_view_scroll_down (view);
      return TRUE;
      break;
    default:
      return FALSE; /* Propagate the event */
  }
  return FALSE; /* Propagate the event */
}

/**
 * champlain_view_new:
 *
 * Creates an instance of #ChamplainView.
 *
 * Returns: a new #ChamplainView ready to be used as a #ClutterActor.
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
    gint x,  /* Absolute x */
    gint y)  /* Absolute y */
{
  ChamplainViewPrivate *priv = view->priv;
  gboolean need_anchor = FALSE;
  gboolean need_update = FALSE;

  if (priv->zoom_level >= 8)
    need_anchor = TRUE;

  /* update anchor one viewport size before reaching the margin to be sure */
  if (priv->anchor_zoom_level != priv->zoom_level ||
      x - priv->anchor.x - priv->viewport_size.width / 2 >= G_MAXINT16 - 2 * priv->viewport_size.width ||
      y - priv->anchor.y - priv->viewport_size.height / 2 >= G_MAXINT16 - 2 * priv->viewport_size.height ||
      x - priv->anchor.x - priv->viewport_size.width / 2 <= 0 + priv->viewport_size.width ||
      y - priv->anchor.y - priv->viewport_size.height / 2 <= 0 + priv->viewport_size.height )
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

      max = champlain_map_source_get_row_count (priv->map_source, priv->zoom_level) *
          champlain_map_source_get_tile_size (priv->map_source) -
          (G_MAXINT16 / 2);
      if (priv->anchor.x > max)
        priv->anchor.x = max;
      if (priv->anchor.y > max)
        priv->anchor.y = max;

      priv->anchor_zoom_level = priv->zoom_level;
      DEBUG ("New Anchor (%f, %f) at (%d, %d)", priv->anchor.x, priv->anchor.y, x, y);
    }

  if (!need_anchor)
    {
      priv->anchor.x = 0;
      priv->anchor.y = 0;
      priv->anchor_zoom_level = priv->zoom_level;
      DEBUG ("Clear Anchor at (%d, %d)", x, y);
    }
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

  priv->longitude = CLAMP (longitude, CHAMPLAIN_MIN_LONG, CHAMPLAIN_MAX_LONG);
  priv->latitude = CLAMP (latitude, CHAMPLAIN_MIN_LAT, CHAMPLAIN_MAX_LAT);

  x = champlain_map_source_get_x (priv->map_source, priv->zoom_level, longitude);
  y = champlain_map_source_get_y (priv->map_source, priv->zoom_level, latitude);

  DEBUG ("Centering on %f, %f (%d, %d)", latitude, longitude, x, y);

  view_update_anchor (view, x, y);

  priv->viewport_size.x = x - priv->anchor.x - priv->viewport_size.width / 2.0;
  priv->viewport_size.y = y - priv->anchor.y - priv->viewport_size.height / 2.0;

  g_signal_handlers_block_by_func (priv->viewport, G_CALLBACK (viewport_pos_changed_cb), view);
  tidy_viewport_set_origin (TIDY_VIEWPORT (priv->viewport),
    priv->viewport_size.x,
    priv->viewport_size.y,
    0);
  g_signal_handlers_unblock_by_func (priv->viewport, G_CALLBACK (viewport_pos_changed_cb), view);

  g_object_notify (G_OBJECT (view), "longitude");
  g_object_notify (G_OBJECT (view), "latitude");

  view_load_visible_tiles (view);
  view_update_polygons (view);
  update_scale (view);
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

  alpha = clutter_alpha_get_alpha (ctx->alpha);
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
  guint duration;

  duration = 500 * view->priv->zoom_level / 2.0;
  champlain_view_go_to_with_duration (view, latitude, longitude, duration);
}

/* FIXME: make public after API freeze */
static void
champlain_view_go_to_with_duration (ChamplainView *view,
    gdouble latitude,
    gdouble longitude,
    guint duration) /* In ms */
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  if (duration == 0)
    {
      champlain_view_center_on (view, latitude, longitude);
      return;
    }

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
  ctx->timeline = clutter_timeline_new (duration);
  ctx->alpha = clutter_alpha_new_full (ctx->timeline, CLUTTER_EASE_IN_OUT_CIRC);

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

  champlain_view_set_zoom_level (view, view->priv->zoom_level + 1);
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

  champlain_view_set_zoom_level (view, view->priv->zoom_level - 1);
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

  if (zoom_level == priv->zoom_level || ZOOM_LEVEL_OUT_OF_RANGE(priv, zoom_level))
    return;

  champlain_view_stop_go_to (view);

  priv->zoom_level = zoom_level;

  DEBUG ("Zooming to %d", zoom_level);

  resize_viewport (view);

  champlain_view_center_on (view, priv->latitude, priv->longitude);

  g_object_notify (G_OBJECT (view), "zoom-level");
}

/**
 * champlain_view_set_min_zoom_level:
 * @view: a #ChamplainView
 * @zoom_level: a gint
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
 * @zoom_level: a gint
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
  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));

  clutter_container_add_actor (CLUTTER_CONTAINER (view->priv->user_layers),
      CLUTTER_ACTOR (layer));
  clutter_actor_raise_top (CLUTTER_ACTOR (layer));

  g_idle_add (marker_reposition, view);

  g_signal_connect_after (layer, "actor-added",
      G_CALLBACK (layer_add_marker_cb), view);

  clutter_container_foreach (CLUTTER_CONTAINER (layer),
      CLUTTER_CALLBACK (connect_marker_notify_cb), view);
}

/**
 * champlain_view_remove_layer:
 * @view: a #ChamplainView
 * @layer: a #ChamplainLayer
 *
 * Removes the layer from the view
 *
 * Since: 0.4.1
 */
void
champlain_view_remove_layer (ChamplainView *view,
    ChamplainLayer *layer)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));
  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));

  g_signal_handlers_disconnect_by_func (layer,
      G_CALLBACK (layer_add_marker_cb), view);
  clutter_container_remove_actor (CLUTTER_CONTAINER (view->priv->user_layers),
      CLUTTER_ACTOR (layer));
}

/**
 * champlain_view_get_coords_from_event:
 * @view: a #ChamplainView
 * @event: a #ClutterEvent
 * @lat: a variable where to put the latitude of the event
 * @lon: a variable where to put the longitude of the event
 *
 * Gets coordinates from button-press-event and button-release-event signals.
 *
 * Returns: the latitude, longitude coordinates for the given ClutterEvent.
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

  guint x, y;

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

  return champlain_view_get_coords_at (view, x, y, latitude, longitude);
}

/**
 * champlain_view_get_coords_at:
 * @view: a #ChamplainView
 * @x: the x position in the view
 * @y: the y position in the view
 * @lat: a variable where to put the latitude of the event
 * @lon: a variable where to put the longitude of the event
 *
 * Gets latitude and longitude for the given x, y position in
 * the view. Use if you get coordinates from GtkEvents for example.
 *
 * Returns: TRUE when successful.
 *
 * Since: 0.4
 */
gboolean champlain_view_get_coords_at (ChamplainView *view,
    guint x,
    guint y,
    gdouble *latitude,
    gdouble *longitude)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), FALSE);
  ChamplainViewPrivate *priv = view->priv;
  gfloat actor_x, actor_y;
  gdouble rel_x, rel_y;

  clutter_actor_get_transformed_position (priv->finger_scroll, &actor_x, &actor_y);

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
  gint size;
  GList *children;
  gint x_count, y_count, x_first, y_first, x_end, y_end, max_x_end, max_y_end;
  gboolean *tile_map;
  gint arm_size, arm_max, turn;
  gint dirs[5] = {0, 1, 0, -1, 0};
  int i, x, y;

  size = champlain_map_source_get_tile_size (priv->map_source);

  viewport.x += priv->anchor.x;
  viewport.y += priv->anchor.y;

  if (viewport.x < 0)
    viewport.x = 0;
  if (viewport.y < 0)
    viewport.y = 0;

  x_count = ceil((float)viewport.width / size) + 1;
  y_count = ceil((float)viewport.height / size) + 1;

  x_first = viewport.x / size;
  y_first = viewport.y / size;

  x_end = x_first + x_count;
  y_end = y_first + y_count;

  max_x_end = champlain_map_source_get_row_count (priv->map_source, priv->zoom_level);
  max_y_end = champlain_map_source_get_column_count (priv->map_source, priv->zoom_level);

  if (x_end > max_x_end)
    {
      x_end = max_x_end;
      x_count = x_end - x_first;
    }
  if (y_end > max_y_end)
    {
      y_end = max_y_end;
      y_count = y_end - y_first;
    }

  DEBUG ("Range %d, %d to %d, %d", x_first, y_first, x_end, y_end);

  tile_map = g_new0 (gboolean, x_count * y_count);

  // Get rid of old tiles first
  children = clutter_container_get_children (CLUTTER_CONTAINER (priv->map_layer));
  for ( ; children != NULL; children = g_list_next (children))
    {
      ChamplainTile *tile = CHAMPLAIN_TILE (children->data);

      gint tile_x = champlain_tile_get_x (tile);
      gint tile_y = champlain_tile_get_y (tile);
      guint zoom_level = champlain_tile_get_zoom_level (tile);

      if (tile_x < x_first || tile_x >= x_end ||
          tile_y < y_first || tile_y >= y_end ||
          zoom_level != priv->zoom_level)
        clutter_container_remove_actor (CLUTTER_CONTAINER (priv->map_layer), CLUTTER_ACTOR (tile));
      else
        {
          tile_map[(tile_y - y_first) * x_count + (tile_x - x_first)] = TRUE;
          view_position_tile (view, tile);
        }
    }

  g_list_free (children);

  //Load new tiles if needed
  x = x_first + x_count / 2 - 1;
  y = y_first + y_count / 2 - 1;
  arm_max = MAX(x_count, y_count) + 2;
  arm_size = 1;

  for (turn = 0; arm_size < arm_max; turn++)
    {
      for (i = 0; i < arm_size; i++)
        {
          if (y >= y_first && y < y_end && x >= x_first && x < x_end &&
              !tile_map[(y - y_first) * x_count + (x - x_first)])
            {
              ChamplainTile *tile;
              FillTileCallbackData *data;

              DEBUG ("Loading tile %d, %d, %d", priv->zoom_level, x, y);
              tile = champlain_tile_new ();
              g_object_set (G_OBJECT (tile), "x", x, "y", y,
                            "zoom-level", priv->zoom_level,
                            "size", size, NULL);
              g_signal_connect (tile, "notify::state", G_CALLBACK (tile_state_notify), view);
              clutter_container_add_actor (CLUTTER_CONTAINER (priv->map_layer), CLUTTER_ACTOR (tile));
              view_position_tile (view, tile);

              /* updates champlain_view state automatically as
                 notify::state signal is connected  */
              champlain_tile_set_state (tile, CHAMPLAIN_STATE_LOADING);

              data = g_new (FillTileCallbackData, 1);
              data->tile = tile;
              data->map_source = priv->map_source;

              g_signal_connect (tile, "destroy", G_CALLBACK (tile_destroyed_cb), view);

              g_object_add_weak_pointer (G_OBJECT (tile), (gpointer*)&data->tile);
              g_object_ref (priv->map_source);

              /* set priority high, otherwise tiles will be loaded after panning is done */
              g_idle_add_full (G_PRIORITY_HIGH_IDLE, (GSourceFunc) fill_tile_cb, data, NULL);
            }

          x += dirs[turn % 4 + 1];
          y += dirs[turn % 4];
        }

      if (turn % 2 == 1)
        arm_size++;
    }

  g_free (tile_map);
}

static gboolean
fill_tile_cb (FillTileCallbackData *data)
{
  ChamplainTile *tile = data->tile;
  ChamplainMapSource *map_source = data->map_source;

  if (data->tile)
    {
      g_object_remove_weak_pointer (G_OBJECT (data->tile), (gpointer*)&data->tile);
      champlain_map_source_fill_tile (map_source, tile);
    }

  g_free (data);
  g_object_unref (map_source);

  return FALSE;
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

  actor = CLUTTER_ACTOR (tile);

  x = champlain_tile_get_x (tile);
  y = champlain_tile_get_y (tile);
  size = champlain_tile_get_size (tile);

  clutter_actor_set_position (actor,
    (x * size) - priv->anchor.x,
    (y * size) - priv->anchor.y);
}

static void
view_reload_tiles_cb (ChamplainMapSource *map_source,
    ChamplainView* view)
{
  clutter_group_remove_all (CLUTTER_GROUP (view->priv->map_layer));

  view_load_visible_tiles (view);
}

static void
tile_destroyed_cb (GObject *gobject,
    gpointer data)
{
  ChamplainView *view = CHAMPLAIN_VIEW (data);
  ChamplainTile *tile = CHAMPLAIN_TILE (gobject);
  ChamplainViewPrivate *priv = view->priv;

  if (champlain_tile_get_state (tile) == CHAMPLAIN_STATE_LOADING)
    {
      priv->tiles_loading--;
      if (priv->tiles_loading == 0)
        {
          priv->state = CHAMPLAIN_STATE_DONE;
          g_object_notify (G_OBJECT (view), "state");
        }
    }
}

static void
tile_state_notify (GObject *gobject,
    GParamSpec *pspec,
    gpointer data)
{
  view_update_state (CHAMPLAIN_VIEW (data), CHAMPLAIN_TILE (gobject));
}

static void
view_update_state (ChamplainView *view, ChamplainTile *tile)
{
  ChamplainState tile_state = champlain_tile_get_state (tile);
  ChamplainViewPrivate *priv = view->priv;

  if (tile_state == CHAMPLAIN_STATE_LOADING)
    {
      if (priv->tiles_loading == 0)
        {
          priv->state = CHAMPLAIN_STATE_LOADING;
          g_object_notify (G_OBJECT (view), "state");
        }
      priv->tiles_loading++;
    }
  else if (tile_state == CHAMPLAIN_STATE_DONE)
    {
      priv->tiles_loading--;
      if (priv->tiles_loading == 0)
        {
          priv->state = CHAMPLAIN_STATE_DONE;
          g_object_notify (G_OBJECT (view), "state");
        }
    }
}

/**
 * champlain_view_set_map_source:
 * @view: a #ChamplainView
 * @map_source: a #ChamplainMapSource
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

  ChamplainViewPrivate *priv = view->priv;

  if (priv->map_source == source)
    return;

  g_object_unref (priv->map_source);
  priv->map_source = g_object_ref_sink (source);

  priv->min_zoom_level = champlain_map_source_get_min_zoom_level (priv->map_source);
  priv->max_zoom_level = champlain_map_source_get_max_zoom_level (priv->map_source);

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

  clutter_group_remove_all (CLUTTER_GROUP (priv->map_layer));

  update_license (view);
  champlain_view_center_on (view, priv->latitude, priv->longitude);

  g_signal_connect (priv->map_source, "reload-tiles",
    G_CALLBACK (view_reload_tiles_cb), view);

  g_object_notify (G_OBJECT (view), "map-source");
}

/**
* champlain_view_set_decel_rate:
* @view: a #ChamplainView
* @rate: a #gdouble between 1.001 and 2.0
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
      rate < 2.0 &&
      rate > 1.0001);

  g_object_set (view->priv->finger_scroll, "decel-rate", rate, NULL);
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

  view->priv->keep_center_on_resize = value;
}

/**
* champlain_view_set_license_text:
* @view: a #ChamplainView
* @text: a license
*
* Show the additional license text on the map view.  The text will preceed the
* map's licence when displayed. Use "\n" to separate the lines.
*
* Since: 0.4.3
*/
void
champlain_view_set_license_text (ChamplainView *view,
    const gchar *text)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = view->priv;

  if (priv->license_text)
    g_free (priv->license_text);

  priv->license_text = g_strdup (text);
  update_license (view);
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

  view->priv->show_license = value;
  update_license (view);
}

/**
* champlain_view_set_show_scale:
* @view: a #ChamplainView
* @value: a #gboolean
*
* Show the scale on the map view.
*
* Since: 0.4.3
*/
void
champlain_view_set_show_scale (ChamplainView *view,
    gboolean value)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  view->priv->show_scale = value;
  update_scale (view);
}

/**
* champlain_view_set_max_scale_width:
* @view: a #ChamplainView
* @value: a #guint in pixels
*
* Sets the maximum width of the scale on the screen in pixels
*
* Since: 0.4.3
*/
void
champlain_view_set_max_scale_width (ChamplainView *view,
    guint value)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  view->priv->max_scale_width = value;
  create_scale (view);
  update_scale (view);
}

/**
* champlain_view_set_scale_unit:
* @view: a #ChamplainView
* @unit: a #ChamplainUnit
*
* Sets the scales unit.
*
* Since: 0.4.3
*/
void
champlain_view_set_scale_unit (ChamplainView *view,
    ChamplainUnit unit)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  view->priv->scale_unit = unit;
  update_scale (view);
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

  view->priv->zoom_on_double_click = value;
}

/**
 * champlain_view_ensure_visible:
 * @view: a #ChamplainView
 * @lat1: the latitude of position 1
 * @lon1: the longitude of position 1
 * @lat2: the latitude of position 2
 * @lon2: the longitude of position 2
 * @animate: a #gboolean
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
        {
          zoom_level = priv->min_zoom_level;
          min_lat = min_lon = width = height = 0;
          break;
        }
    }
  while (!good_size);

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
 * @markers: a NULL terminated array of #ChamplainMarker elements
 * @animate: a #gboolean
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

  gdouble lon, lat;
  gint x_diff, y_diff;
  gfloat actor_x, actor_y;
  gdouble rel_x, rel_y;
  gfloat x2, y2;
  gdouble lat2, lon2;

  if (zoom_level == priv->zoom_level || ZOOM_LEVEL_OUT_OF_RANGE(priv, zoom_level))
    return FALSE;

  champlain_view_stop_go_to (view);

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

  priv->zoom_level = zoom_level;

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
  champlain_view_center_on (view, lat2, lon2);

  g_object_notify (G_OBJECT (view), "zoom-level");
  return TRUE;
}

/**
 * champlain_view_get_zoom_level:
 * @view: The view
 *
 * Gets the view's current zoom level.
 *
 * Returns: the view's current zoom level.
 *
 * Since: 0.4
 */
gint
champlain_view_get_zoom_level (ChamplainView *view)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), 0);

  return view->priv->zoom_level;
}

/**
 * champlain_view_get_min_zoom_level:
 * @view: The view
 *
 * Gets the view's minimal allowed zoom level.
 *
 * Returns: the view's minimal allowed zoom level.
 *
 * Since: 0.4
 */
gint
champlain_view_get_min_zoom_level (ChamplainView *view)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), 0);

  return view->priv->min_zoom_level;
}

/**
 * champlain_view_get_max_zoom_level:
 * @view: The view
 *
 * Gets the view's maximal allowed zoom level.
 *
 * Returns: the view's maximal allowed zoom level.
 *
 * Since: 0.4
 */
gint
champlain_view_get_max_zoom_level (ChamplainView *view)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), 0);

  return view->priv->max_zoom_level;
}

/**
 * champlain_view_get_map_source:
 * @view: The view
 *
 * Gets the view's current map source.
 *
 * Returns: the view's current map source. If you need to keep a reference to the
 * map source then you have to call #g_object_ref.
 *
 * Since: 0.4
 */
ChamplainMapSource*
champlain_view_get_map_source (ChamplainView *view)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), NULL);

  return view->priv->map_source;
}

/**
 * champlain_view_get_decel_rate:
 * @view: The view
 *
 * Gets the view's deceleration rate.
 *
 * Returns: the view's deceleration rate.
 *
 * Since: 0.4
 */
gdouble
champlain_view_get_decel_rate (ChamplainView *view)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), 0.0);

  gdouble decel = 0.0;
  g_object_get (view->priv->finger_scroll, "decel-rate", &decel, NULL);
  return decel;
}

/**
 * champlain_view_get_scroll_mode:
 * @view: The view
 *
 * Gets the view's scroll mode behaviour.
 *
 * Returns: the view's scroll mode behaviour.
 *
 * Since: 0.4
 */
ChamplainScrollMode
champlain_view_get_scroll_mode (ChamplainView *view)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), CHAMPLAIN_SCROLL_MODE_PUSH);

  return view->priv->scroll_mode;
}

/**
 * champlain_view_get_keep_center_on_resize:
 * @view: The view
 *
 * Checks whether to keep the center on resize
 *
 * Returns: TRUE if the view keeps the center on resize, FALSE otherwise.
 *
 * Since: 0.4
 */
gboolean
champlain_view_get_keep_center_on_resize (ChamplainView *view)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), FALSE);

  return view->priv->keep_center_on_resize;
}

/**
 * champlain_view_get_show_license:
 * @view: The view
 *
 * Checks whether the view displays the license.
 *
 * Returns: TRUE if the view displays the license, FALSE otherwise.
 *
 * Since: 0.4
 */
gboolean
champlain_view_get_show_license (ChamplainView *view)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), FALSE);

  return view->priv->show_license;
}

/**
 * champlain_view_get_license_text:
 * @view: The view
 *
 * Gets the additional license text.
 *
 * Returns: the additional license text
 *
 * Since: 0.4.3
 */
const gchar *
champlain_view_get_license_text (ChamplainView *view)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), FALSE);

  return view->priv->license_text;
}


/**
 * champlain_view_get_show_scale:
 * @view: The view
 *
 * Checks whether the view displays the scale.
 *
 * Returns: TRUE if the view displays the scale, FALSE otherwise.
 *
 * Since: 0.4.3
 */
gboolean
champlain_view_get_show_scale (ChamplainView *view)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), FALSE);

  return view->priv->show_scale;
}

/**
 * champlain_view_get_max_scale_width:
 * @view: The view
 *
 * Gets the maximal scale width.
 *
 * Returns: The max scale width in pixels.
 *
 * Since: 0.4.3
 */
guint
champlain_view_get_max_scale_width (ChamplainView *view)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), FALSE);

  return view->priv->max_scale_width;
}

/**
 * champlain_view_get_scale_unit:
 * @view: The view
 *
 * Gets the unit used by the scale.
 *
 * Returns: The unit used by the scale
 *
 * Since: 0.4.3
 */
ChamplainUnit
champlain_view_get_scale_unit (ChamplainView *view)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), FALSE);

  return view->priv->scale_unit;
}

/**
 * champlain_view_get_zoom_on_double_click:
 * @view: The view
 *
 * Checks whether the view zooms on double click.
 *
 * Returns: TRUE if the view zooms on double click, FALSE otherwise.
 *
 * Since: 0.4
 */
gboolean
champlain_view_get_zoom_on_double_click (ChamplainView *view)
{
  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), FALSE);

  return view->priv->zoom_on_double_click;
}

static void
view_update_polygons (ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;
  ClutterGroup *polygon_layer_group = CLUTTER_GROUP (priv->polygon_layer);
  guint polygon_num, i;
  gfloat x, y;

  polygon_num = clutter_group_get_n_children (polygon_layer_group);

  if (polygon_num == 0)
    return;

  for (i = 0; i < polygon_num; i++)
    {
      ChamplainPolygon *polygon = CHAMPLAIN_POLYGON (clutter_group_get_nth_child (polygon_layer_group, i));

      champlain_polygon_draw_polygon (polygon, priv->map_source, priv->zoom_level,
                                      priv->viewport_size.width, priv->viewport_size.height,
                                      priv->viewport_size.x + priv->anchor.x, priv->viewport_size.y + priv->anchor.y);
    }

  /* Position the layer in the viewport */
  x = priv->viewport_size.x;
  y = priv->viewport_size.y;

  clutter_actor_set_position (priv->polygon_layer, x, y);
}

/**
 * champlain_view_add_polygon:
 * @view: a #ChamplainView
 * @polygon: a #ChamplainPolygon
 *
 * Adds a #ChamplainPolygon to the #ChamplainView
 *
 * Since: 0.4
 */
void
champlain_view_add_polygon (ChamplainView *view,
    ChamplainPolygon *polygon)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));
  g_return_if_fail (CHAMPLAIN_IS_POLYGON (polygon));

  ChamplainViewPrivate *priv = view->priv;

  g_signal_connect (polygon, "notify",
      G_CALLBACK (notify_polygon_cb), view);

//  if (priv->viewport_size.width == 0 ||
//      priv->viewport_size.height == 0)
//    return;

  clutter_actor_set_position (CLUTTER_ACTOR (polygon), 0, 0);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->polygon_layer),
      CLUTTER_ACTOR (polygon));
}

/**
 * champlain_view_remove_polygon:
 * @view: a #ChamplainView
 * @polygon: a #ChamplainPolygon
 *
 * Removes a #ChamplainPolygon from #ChamplainView
 *
 * Since: 0.4
 */
void
champlain_view_remove_polygon (ChamplainView *view,
    ChamplainPolygon *polygon)
{
  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));
  g_return_if_fail (CHAMPLAIN_IS_POLYGON (polygon));

  clutter_container_remove_actor (CLUTTER_CONTAINER (view->priv->polygon_layer),
      CLUTTER_ACTOR (polygon));
}

