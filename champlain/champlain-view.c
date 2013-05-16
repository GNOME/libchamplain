/*
 * Copyright (C) 2008-2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
 * Copyright (C) 2010-2013 Jiri Techet <techet@gmail.com>
 * Copyright (C) 2012 Collabora Ltd. <http://www.collabora.co.uk/>
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
 *   <listitem><para>Push: the normal behavior where the maps don't move
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
 * time a mouse button is pressed and released on the @view.
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
#include "champlain-private.h"
#include "champlain-tile.h"
#include "champlain-license.h"

#include <clutter/clutter.h>
#include <glib.h>
#include <glib-object.h>
#include <math.h>
#include <champlain-kinetic-scroll-view.h>
#include <champlain-viewport.h>
#include <champlain-adjustment.h>

/* #define VIEW_LOG */
#ifdef VIEW_LOG
#define DEBUG_LOG() g_print ("%s\n", __FUNCTION__);
#else
#define DEBUG_LOG()
#endif

enum
{
  /* normal signals */
  ANIMATION_COMPLETED,
  LAYER_RELOCATED,
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
  PROP_DECELERATION,
  PROP_KINETIC_MODE,
  PROP_KEEP_CENTER_ON_RESIZE,
  PROP_ZOOM_ON_DOUBLE_CLICK,
  PROP_ANIMATE_ZOOM,
  PROP_STATE,
  PROP_BACKGROUND_PATTERN,
};

#define PADDING 10
static guint signals[LAST_SIGNAL] = { 0, };

#define GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CHAMPLAIN_TYPE_VIEW, ChamplainViewPrivate))
  
#define ZOOM_LEVEL_OUT_OF_RANGE(priv, level) \
  (level < priv->min_zoom_level || \
           level > priv->max_zoom_level || \
   level < champlain_map_source_get_min_zoom_level (priv->map_source) || \
           level > champlain_map_source_get_max_zoom_level (priv->map_source))

/* Between state values for go_to */
typedef struct
{
  ChamplainView *view;
  ClutterTimeline *timeline;
  gdouble to_latitude;
  gdouble to_longitude;
  gdouble from_latitude;
  gdouble from_longitude;
} GoToContext;


typedef struct
{
  ChamplainTile *tile;
  ChamplainMapSource *map_source;
} FillTileCallbackData;


struct _ChamplainViewPrivate
{
                                /* ChamplainView */
  ClutterActor *kinetic_scroll;     /* kinetic_scroll */
  ClutterActor *viewport;               /* viewport */
                                            /* viewport_container */  
  ClutterActor *background_layer;               /* background_layer */
  ClutterActor *zoom_layer;                     /* zoom_layer */
  ClutterActor *map_layer;                      /* map_layer */
  ClutterActor *user_layers;                    /* user_layers */
  ClutterActor *zoom_overlay_actor; /* zoom_overlay_actor */
  ClutterActor *license_actor;      /* license_actor */

  ClutterContent *background_content; 

  gdouble viewport_x;
  gdouble viewport_y;
  gint viewport_width;
  gint viewport_height;

  ChamplainMapSource *map_source; /* Current map tile source */

  guint zoom_level; /* Holds the current zoom level number */
  guint min_zoom_level; /* Lowest allowed zoom level */
  guint max_zoom_level; /* Highest allowed zoom level */

  /* Represents the (lat, lon) at the center of the viewport */
  gdouble longitude;
  gdouble latitude;
  gboolean location_updated;

  gint bg_offset_x;
  gint bg_offset_y;

  gboolean keep_center_on_resize;
  gboolean zoom_on_double_click;
  gboolean animate_zoom;

  gboolean kinetic_mode;

  ChamplainState state; /* View's global state */

  /* champlain_view_go_to's context, kept for stop_go_to */
  GoToContext *goto_context;

  gint tiles_loading;
  
  guint redraw_timeout;
  
  gboolean animating_zoom;
  guint anim_start_zoom_level;
  gdouble zoom_actor_viewport_x;
  gdouble zoom_actor_viewport_y;
  guint zoom_actor_timeout;
};

G_DEFINE_TYPE (ChamplainView, champlain_view, CLUTTER_TYPE_ACTOR);


static gboolean scroll_event (ClutterActor *actor,
    ClutterScrollEvent *event,
    ChamplainView *view);
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
static gboolean kinetic_scroll_button_press_cb (ClutterActor *actor,
    ClutterButtonEvent *event,
    ChamplainView *view);
static void load_visible_tiles (ChamplainView *view,
    gboolean relocate);
static gboolean view_set_zoom_level_at (ChamplainView *view,
    guint zoom_level,
    gboolean use_event_coord,
    gint x,
    gint y);
static void tile_state_notify (ChamplainTile *tile,
    G_GNUC_UNUSED GParamSpec *pspec,
    ChamplainView *view);
static gboolean kinetic_scroll_key_press_cb (ClutterActor *actor,
    ClutterKeyEvent *event,
    ChamplainView *view);
static void champlain_view_go_to_with_duration (ChamplainView *view,
    gdouble latitude,
    gdouble longitude,
    guint duration);
static gboolean fill_tile_cb (FillTileCallbackData *data);
static gboolean redraw_timeout_cb(gpointer view);
static void remove_all_tiles (ChamplainView *view);


static void
update_coords (ChamplainView *view,
    gdouble x,
    gdouble y,
    gboolean notify)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;

  priv->viewport_x = x;
  priv->viewport_y = y;
  priv->longitude = champlain_map_source_get_longitude (priv->map_source,
        priv->zoom_level,
        x + priv->viewport_width / 2.0);
  priv->latitude = champlain_map_source_get_latitude (priv->map_source,
        priv->zoom_level,
        y + priv->viewport_height / 2.0);
  
  if (notify)
    {
      g_object_notify (G_OBJECT (view), "longitude");
      g_object_notify (G_OBJECT (view), "latitude");
    }
}


static void
position_viewport (ChamplainView *view,
    gdouble x,
    gdouble y)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;
  gint old_bg_offset_x = 0, old_bg_offset_y = 0;
  gfloat bg_width, bg_height;

  /* remember the relative offset of the background tile */
  if (priv->background_content)
    {
      clutter_content_get_preferred_size (priv->background_content, &bg_width, &bg_height);
      old_bg_offset_x = ((gint)priv->viewport_x + priv->bg_offset_x) % (gint)bg_width;
      old_bg_offset_y = ((gint)priv->viewport_y + priv->bg_offset_y) % (gint)bg_height;
    }
    
  /* notify about latitude and longitude change only after the viewport position is set */
  g_object_freeze_notify (G_OBJECT (view));
  
  update_coords (view, x, y, TRUE);

  /* compute the new relative offset of the background tile */
  if (priv->background_content)
    {
      gint new_bg_offset_x = (gint)priv->viewport_x % (gint)bg_width;
      gint new_bg_offset_y = (gint)priv->viewport_y % (gint)bg_height;
      priv->bg_offset_x = (old_bg_offset_x - new_bg_offset_x) % (gint)bg_width;
      priv->bg_offset_y = (old_bg_offset_y - new_bg_offset_y) % (gint)bg_height;
      if (priv->bg_offset_x < 0)
        priv->bg_offset_x += bg_width;
      if (priv->bg_offset_y < 0)
        priv->bg_offset_y += bg_height;
    }

  /* we know about the change already - don't send the notifications again */
  g_signal_handlers_block_by_func (priv->viewport, G_CALLBACK (viewport_pos_changed_cb), view);
  champlain_viewport_set_origin (CHAMPLAIN_VIEWPORT (priv->viewport),
      (gint)priv->viewport_x,
      (gint)priv->viewport_y);
  g_signal_handlers_unblock_by_func (priv->viewport, G_CALLBACK (viewport_pos_changed_cb), view);

  g_object_thaw_notify (G_OBJECT (view));
}


static void
view_relocated_cb (G_GNUC_UNUSED ChamplainViewport *viewport,
    ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;
  
  clutter_actor_destroy_all_children (priv->zoom_layer);
  load_visible_tiles (view, TRUE);
  g_signal_emit_by_name (view, "layer-relocated", NULL);
}


static void
panning_completed (G_GNUC_UNUSED ChamplainKineticScrollView *scroll,
    ChamplainView *view)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;
  gdouble x, y;
  
  if (priv->redraw_timeout != 0)
    {
      g_source_remove (priv->redraw_timeout);
      priv->redraw_timeout = 0;
    }

  champlain_viewport_get_origin (CHAMPLAIN_VIEWPORT (priv->viewport), &x, &y);

  update_coords (view, x, y, TRUE);
  load_visible_tiles (view, FALSE);
}


static gboolean
scroll_event (G_GNUC_UNUSED ClutterActor *actor,
    ClutterScrollEvent *event,
    ChamplainView *view)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;

  guint zoom_level = priv->zoom_level;

  if (event->direction == CLUTTER_SCROLL_UP)
    zoom_level = priv->zoom_level + 1;
  else if (event->direction == CLUTTER_SCROLL_DOWN)
    zoom_level = priv->zoom_level - 1;
    
  return view_set_zoom_level_at (view, zoom_level, TRUE, event->x, event->y);
}


static void
resize_viewport (ChamplainView *view)
{
  DEBUG_LOG ()

  gdouble lower_x = 0;
  gdouble lower_y = 0;
  gdouble upper_x = G_MAXINT16;
  gdouble upper_y = G_MAXINT16;
  ChamplainAdjustment *hadjust, *vadjust;

  ChamplainViewPrivate *priv = view->priv;

  champlain_viewport_get_adjustments (CHAMPLAIN_VIEWPORT (priv->viewport), &hadjust,
      &vadjust);

  gint map_width = champlain_map_source_get_column_count (priv->map_source, priv->zoom_level) *
    champlain_map_source_get_tile_size (priv->map_source);
  gint map_height = champlain_map_source_get_row_count (priv->map_source, priv->zoom_level) *
    champlain_map_source_get_tile_size (priv->map_source);
  
  lower_x = MIN (-priv->viewport_width / 2, -priv->viewport_width + map_width / 2);
  lower_y = MIN (-priv->viewport_height / 2, -priv->viewport_height + map_height / 2);
  upper_x = MAX (map_width - priv->viewport_width / 2, map_width / 2);
  upper_y = MAX (map_height - priv->viewport_height / 2, map_height / 2);

  /* we don't want to get notified about the position change now */
  g_signal_handlers_block_by_func (priv->viewport, G_CALLBACK (viewport_pos_changed_cb), view);
  champlain_adjustment_set_values (hadjust, champlain_adjustment_get_value (hadjust), lower_x, upper_x, 1.0);
  champlain_adjustment_set_values (vadjust, champlain_adjustment_get_value (vadjust), lower_y, upper_y, 1.0);
  g_signal_handlers_unblock_by_func (priv->viewport, G_CALLBACK (viewport_pos_changed_cb), view);
}


static void
champlain_view_get_property (GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
  DEBUG_LOG ()

  ChamplainView *view = CHAMPLAIN_VIEW (object);
  ChamplainViewPrivate *priv = view->priv;

  switch (prop_id)
    {
    case PROP_LONGITUDE:
      g_value_set_double (value,
          CLAMP (priv->longitude, CHAMPLAIN_MIN_LONGITUDE, CHAMPLAIN_MAX_LONGITUDE));
      break;

    case PROP_LATITUDE:
      g_value_set_double (value,
          CLAMP (priv->latitude, CHAMPLAIN_MIN_LATITUDE, CHAMPLAIN_MAX_LATITUDE));
      break;

    case PROP_ZOOM_LEVEL:
      g_value_set_uint (value, priv->zoom_level);
      break;

    case PROP_MIN_ZOOM_LEVEL:
      g_value_set_uint (value, priv->min_zoom_level);
      break;

    case PROP_MAX_ZOOM_LEVEL:
      g_value_set_uint (value, priv->max_zoom_level);
      break;

    case PROP_MAP_SOURCE:
      g_value_set_object (value, priv->map_source);
      break;

    case PROP_KINETIC_MODE:
      g_value_set_boolean (value, priv->kinetic_mode);
      break;

    case PROP_DECELERATION:
      {
        gdouble decel = 0.0;
        g_object_get (priv->kinetic_scroll, "deceleration", &decel, NULL);
        g_value_set_double (value, decel);
        break;
      }

    case PROP_KEEP_CENTER_ON_RESIZE:
      g_value_set_boolean (value, priv->keep_center_on_resize);
      break;

    case PROP_ZOOM_ON_DOUBLE_CLICK:
      g_value_set_boolean (value, priv->zoom_on_double_click);
      break;

    case PROP_ANIMATE_ZOOM:
      g_value_set_boolean (value, priv->animate_zoom);
      break;

    case PROP_STATE:
      g_value_set_enum (value, priv->state);
      break;

    case PROP_BACKGROUND_PATTERN:
      g_value_set_object (value, priv->background_content);
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
  DEBUG_LOG ()

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
      champlain_view_set_zoom_level (view, g_value_get_uint (value));
      break;

    case PROP_MIN_ZOOM_LEVEL:
      champlain_view_set_min_zoom_level (view, g_value_get_uint (value));
      break;

    case PROP_MAX_ZOOM_LEVEL:
      champlain_view_set_max_zoom_level (view, g_value_get_uint (value));
      break;

    case PROP_MAP_SOURCE:
      champlain_view_set_map_source (view, g_value_get_object (value));
      break;

    case PROP_KINETIC_MODE:
      champlain_view_set_kinetic_mode (view, g_value_get_boolean (value));
      break;

    case PROP_DECELERATION:
      champlain_view_set_deceleration (view, g_value_get_double (value));
      break;

    case PROP_KEEP_CENTER_ON_RESIZE:
      champlain_view_set_keep_center_on_resize (view, g_value_get_boolean (value));
      break;

    case PROP_ZOOM_ON_DOUBLE_CLICK:
      champlain_view_set_zoom_on_double_click (view, g_value_get_boolean (value));
      break;

    case PROP_ANIMATE_ZOOM:
      champlain_view_set_animate_zoom (view, g_value_get_boolean (value));
      break;
      
    case PROP_BACKGROUND_PATTERN:
      champlain_view_set_background_pattern (view, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
champlain_view_dispose (GObject *object)
{
  DEBUG_LOG ()
  
  ChamplainView *view = CHAMPLAIN_VIEW (object);
  ChamplainViewPrivate *priv = view->priv;

  if (priv->goto_context != NULL)
    champlain_view_stop_go_to (view);

  if (priv->kinetic_scroll != NULL)
    {
      champlain_kinetic_scroll_view_stop (CHAMPLAIN_KINETIC_SCROLL_VIEW (priv->kinetic_scroll));
      priv->kinetic_scroll = NULL;
    }

  if (priv->viewport != NULL)
    {
      champlain_viewport_stop (CHAMPLAIN_VIEWPORT (priv->viewport));
      priv->viewport = NULL;
    }

  if (priv->map_source != NULL)
    {
      g_object_unref (priv->map_source);
      priv->map_source = NULL;
    }

  if (priv->background_content)
    {
      g_object_unref (priv->background_content);
      priv->background_content = NULL;
    }
    
  if (priv->redraw_timeout != 0)
    {
      g_source_remove (priv->redraw_timeout);
      priv->redraw_timeout = 0;
    }
    
  if (priv->redraw_timeout != 0)
    {
      g_source_remove (priv->zoom_actor_timeout);
      priv->zoom_actor_timeout = 0;
    }

  priv->map_layer = NULL;
  priv->license_actor = NULL;
  priv->user_layers = NULL;
  priv->zoom_layer = NULL;

  G_OBJECT_CLASS (champlain_view_parent_class)->dispose (object);
}


static void
champlain_view_finalize (GObject *object)
{
  DEBUG_LOG ()

/*  ChamplainViewPrivate *priv = CHAMPLAIN_VIEW (object)->priv; */

  G_OBJECT_CLASS (champlain_view_parent_class)->finalize (object);
}


/* These return fixed sizes because either a.) We expect the user to size
 * explicitly with clutter_actor_get_size or b.) place it in a container that
 * allocates it whatever it wants.
 */
static void
champlain_view_get_preferred_width (ClutterActor *actor,
    G_GNUC_UNUSED gfloat for_height,
    gfloat *min_width,
    gfloat *nat_width)
{
  DEBUG_LOG ()

  ChamplainView *view = CHAMPLAIN_VIEW (actor);
  gint width = champlain_map_source_get_tile_size (view->priv->map_source);

  if (min_width)
    *min_width = 1;

  if (nat_width)
    *nat_width = width;
}


static void
champlain_view_get_preferred_height (ClutterActor *actor,
    G_GNUC_UNUSED gfloat for_width,
    gfloat *min_height,
    gfloat *nat_height)
{
  DEBUG_LOG ()

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
  DEBUG_LOG ()

  g_type_class_add_private (champlainViewClass, sizeof (ChamplainViewPrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (champlainViewClass);
  object_class->dispose = champlain_view_dispose;
  object_class->finalize = champlain_view_finalize;
  object_class->get_property = champlain_view_get_property;
  object_class->set_property = champlain_view_set_property;

  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (champlainViewClass);
  actor_class->get_preferred_width = champlain_view_get_preferred_width;
  actor_class->get_preferred_height = champlain_view_get_preferred_height;

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
  g_object_class_install_property (object_class,
      PROP_LATITUDE,
      g_param_spec_double ("latitude",
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
  g_object_class_install_property (object_class,
      PROP_ZOOM_LEVEL,
      g_param_spec_uint ("zoom-level",
          "Zoom level",
          "The level of zoom of the map",
          0, 
          20, 
          3, 
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainView:min-zoom-level:
   *
   * The lowest allowed level of zoom of the content.
   *
   * Since: 0.4
   */
  g_object_class_install_property (object_class,
      PROP_MIN_ZOOM_LEVEL,
      g_param_spec_uint ("min-zoom-level",
          "Min zoom level",
          "The lowest allowed level of zoom",
          0, 
          20, 
          0, 
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainView:max-zoom-level:
   *
   * The highest allowed level of zoom of the content.
   *
   * Since: 0.4
   */
  g_object_class_install_property (object_class,
      PROP_MAX_ZOOM_LEVEL,
      g_param_spec_uint ("max-zoom-level",
          "Max zoom level",
          "The highest allowed level of zoom",
          0, 
          20, 
          20, 
          CHAMPLAIN_PARAM_READWRITE));

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
   * ChamplainView:kinetic-mode:
   *
   * Determines whether the view should use kinetic mode.
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_KINETIC_MODE,
      g_param_spec_boolean ("kinetic-mode",
          "Kinetic Mode",
          "Determines whether the view should use kinetic mode.",
          TRUE, 
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainView:deceleration:
   *
   * The deceleration rate for the kinetic mode. The default value is 1.1.
   *
   * Since: 0.10
   */
  g_object_class_install_property (object_class,
      PROP_DECELERATION,
      g_param_spec_double ("deceleration",
          "Deceleration rate",
          "Rate at which the view will decelerate in kinetic mode.",
          1.0001, 
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
          "Keep the current centered position upon resizing",
          TRUE, 
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
          TRUE, 
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainView:animate-zoom:
   *
   * Animate zoom change when zooming in/out.
   *
   * Since: 0.12
   */
  g_object_class_install_property (object_class,
      PROP_ANIMATE_ZOOM,
      g_param_spec_boolean ("animate-zoom",
          "Animate zoom level change",
          "Animate zoom change when zooming in/out",
          TRUE, 
          CHAMPLAIN_PARAM_READWRITE));

  /**
   * ChamplainView:state:
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
   * ChamplainView:background-pattern:
   *
   * The pattern displayed in the background of the map.
   *
   * Since: 0.12.4
   */
  g_object_class_install_property (object_class,
      PROP_BACKGROUND_PATTERN,
      g_param_spec_object ("background-pattern",
          "Background pattern",
          "The tile's background pattern",
          CLUTTER_TYPE_ACTOR,
          G_PARAM_READWRITE));

  /**
   * ChamplainView::animation-completed:
   *
   * The #ChamplainView::animation-completed signal is emitted when any animation in the view
   * ends.  This is a detailed signal.  For example, if you want to be signaled
   * only for go-to animation, you should connect to
   * "animation-completed::go-to".
   *
   * Since: 0.4
   */
  signals[ANIMATION_COMPLETED] =
    g_signal_new ("animation-completed", 
        G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED, 
        0, NULL, NULL,
        g_cclosure_marshal_VOID__OBJECT, 
        G_TYPE_NONE, 
        0);

  /**
   * ChamplainView::layer-relocated:
   *
   * Indicates that the layers have been "relocated". In practice this means that
   * every layer should connect to this signal and redraw itself when the signal is
   * emitted. Layer relocation happens when zooming in/out and when panning for more
   * than MAX_INT pixels.
   *
   * Since: 0.10
   */
  signals[LAYER_RELOCATED] =
    g_signal_new ("layer-relocated", 
        G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_LAST, 
        0, NULL, NULL,
        g_cclosure_marshal_VOID__VOID, 
        G_TYPE_NONE, 
        0);
}


static void
champlain_view_realized_cb (ChamplainView *view,
    G_GNUC_UNUSED GParamSpec *pspec)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;
  
  if (!CLUTTER_ACTOR_IS_REALIZED (view))
    return;

  clutter_actor_grab_key_focus (priv->kinetic_scroll);

  resize_viewport (view);
  champlain_view_center_on (view, priv->latitude, priv->longitude);

  g_object_notify (G_OBJECT (view), "zoom-level");
  g_object_notify (G_OBJECT (view), "map-source");
  g_signal_emit_by_name (view, "layer-relocated", NULL);
}


static gboolean
_update_idle_cb (ChamplainView *view)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;
  
  if (!priv->kinetic_scroll)
    return FALSE;

  clutter_actor_set_size (priv->kinetic_scroll,
      priv->viewport_width,
      priv->viewport_height);

  resize_viewport (view);

  if (priv->keep_center_on_resize)
    champlain_view_center_on (view, priv->latitude, priv->longitude);
  else
    load_visible_tiles (view, FALSE);

  return FALSE;
}


static void
view_size_changed_cb (ChamplainView *view,
    G_GNUC_UNUSED GParamSpec *pspec)
{
  ChamplainViewPrivate *priv = GET_PRIVATE (view);
  gint width, height;
  
  width = clutter_actor_get_width (CLUTTER_ACTOR (view));
  height = clutter_actor_get_height (CLUTTER_ACTOR (view));
  
  if (priv->viewport_width != width || priv->viewport_height != height)
    {
      g_idle_add_full (CLUTTER_PRIORITY_REDRAW,
          (GSourceFunc) _update_idle_cb,
          g_object_ref (view),
          (GDestroyNotify) g_object_unref);
    }
    
  priv->viewport_width = width;
  priv->viewport_height = height;
}


static void
champlain_view_init (ChamplainView *view)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = GET_PRIVATE (view);
  ChamplainMapSourceFactory *factory;
  ChamplainMapSource *source;
  ClutterActor *viewport_container;
  ClutterLayoutManager *layout;
  ClutterColor color = { 0xf1, 0xee, 0xe8, 0xff };

  champlain_debug_set_flags (g_getenv ("CHAMPLAIN_DEBUG"));

  view->priv = priv;

  factory = champlain_map_source_factory_dup_default ();
  source = champlain_map_source_factory_create_cached_source (factory, CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK);

  priv->map_source = CHAMPLAIN_MAP_SOURCE (source);

  priv->zoom_level = 0;
  priv->min_zoom_level = champlain_map_source_get_min_zoom_level (priv->map_source);
  priv->max_zoom_level = champlain_map_source_get_max_zoom_level (priv->map_source);
  priv->keep_center_on_resize = TRUE;
  priv->zoom_on_double_click = TRUE;
  priv->animate_zoom = TRUE;
  priv->license_actor = NULL;
  priv->kinetic_mode = TRUE;
  priv->viewport_x = 0;
  priv->viewport_y = 0;
  priv->viewport_width = 0;
  priv->viewport_height = 0;
  priv->state = CHAMPLAIN_STATE_NONE;
  priv->latitude = 0.0;
  priv->longitude = 0.0;
  priv->goto_context = NULL;
  priv->tiles_loading = 0;
  priv->animating_zoom = FALSE;
  priv->background_content = NULL;
  priv->zoom_overlay_actor = NULL;
  priv->bg_offset_x = 0;
  priv->bg_offset_y = 0;
  priv->location_updated = FALSE;
  priv->redraw_timeout = 0;
  priv->zoom_actor_timeout = 0;

  clutter_actor_set_background_color (CLUTTER_ACTOR (view), &color);

  g_signal_connect (view, "notify::width", G_CALLBACK (view_size_changed_cb), NULL);
  g_signal_connect (view, "notify::height", G_CALLBACK (view_size_changed_cb), NULL);

  g_signal_connect (view, "notify::realized", G_CALLBACK (champlain_view_realized_cb), NULL);

  layout = clutter_bin_layout_new (CLUTTER_BIN_ALIGNMENT_FIXED,
                                   CLUTTER_BIN_ALIGNMENT_FIXED);
  clutter_actor_set_layout_manager (CLUTTER_ACTOR (view), layout);

  /* Setup viewport layers*/
  priv->background_layer = clutter_actor_new ();
  priv->zoom_layer = clutter_actor_new ();
  priv->map_layer = clutter_actor_new ();
  priv->user_layers = clutter_actor_new ();

  viewport_container = clutter_actor_new ();
  clutter_actor_add_child (viewport_container, priv->background_layer);
  clutter_actor_add_child (viewport_container, priv->zoom_layer);
  clutter_actor_add_child (viewport_container, priv->map_layer);
  clutter_actor_add_child (viewport_container, priv->user_layers);

  /* Setup viewport */
  priv->viewport = champlain_viewport_new ();
  champlain_viewport_set_child (CHAMPLAIN_VIEWPORT (priv->viewport), viewport_container);
  g_signal_connect (priv->viewport, "relocated", G_CALLBACK (view_relocated_cb), view);

  g_signal_connect (priv->viewport, "notify::x-origin",
      G_CALLBACK (viewport_pos_changed_cb), view);
  g_signal_connect (priv->viewport, "notify::y-origin",
      G_CALLBACK (viewport_pos_changed_cb), view);

  /* Setup kinetic scroll */
  priv->kinetic_scroll = champlain_kinetic_scroll_view_new (FALSE, CHAMPLAIN_VIEWPORT (priv->viewport));

  g_signal_connect (priv->kinetic_scroll, "scroll-event",
      G_CALLBACK (scroll_event), view);
  g_signal_connect (priv->kinetic_scroll, "panning-completed",
      G_CALLBACK (panning_completed), view);
  g_signal_connect (priv->kinetic_scroll, "button-press-event",
      G_CALLBACK (kinetic_scroll_button_press_cb), view);
  g_signal_connect (priv->kinetic_scroll, "key-press-event",
      G_CALLBACK (kinetic_scroll_key_press_cb), view);

  /* Setup stage */
  clutter_actor_add_child (CLUTTER_ACTOR (view), priv->kinetic_scroll);
  priv->zoom_overlay_actor = clutter_actor_new ();
  clutter_actor_add_child (CLUTTER_ACTOR (view), priv->zoom_overlay_actor);

  /* Setup license */
  priv->license_actor = champlain_license_new ();
  champlain_license_connect_view (CHAMPLAIN_LICENSE (priv->license_actor), view);
  clutter_actor_set_x_expand (priv->license_actor, TRUE);
  clutter_actor_set_y_expand (priv->license_actor, TRUE);
  clutter_actor_set_x_align (priv->license_actor, CLUTTER_ACTOR_ALIGN_END);
  clutter_actor_set_y_align (priv->license_actor, CLUTTER_ACTOR_ALIGN_END);
  clutter_actor_add_child (CLUTTER_ACTOR (view), priv->license_actor);
  
  priv->state = CHAMPLAIN_STATE_DONE;
  g_object_notify (G_OBJECT (view), "state");
}


static gboolean
redraw_timeout_cb (gpointer data)
{
  DEBUG_LOG ()

  ChamplainView *view = data;
  ChamplainViewPrivate *priv = view->priv;
  gdouble x, y;

  champlain_viewport_get_origin (CHAMPLAIN_VIEWPORT (priv->viewport), &x, &y);

  if (priv->location_updated || (gint)ABS (x - priv->viewport_x) > 0 || (gint)ABS (y - priv->viewport_y) > 0)
    {
      update_coords (view, x, y, TRUE);
      load_visible_tiles (view, FALSE);
      priv->location_updated = FALSE;
    }

  return TRUE;
}


static void
viewport_pos_changed_cb (G_GNUC_UNUSED GObject *gobject,
    G_GNUC_UNUSED GParamSpec *arg1,
    ChamplainView *view)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;
  gdouble x, y;

  if (priv->redraw_timeout == 0)
    priv->redraw_timeout = g_timeout_add (350, redraw_timeout_cb, view);

  champlain_viewport_get_origin (CHAMPLAIN_VIEWPORT (priv->viewport), &x, &y);

  if (ABS (x - priv->viewport_x) > 100 || ABS (y - priv->viewport_y) > 100)
    {
      update_coords (view, x, y, FALSE);
      load_visible_tiles (view, FALSE);
      priv->location_updated = TRUE;
    }
}


static gboolean
kinetic_scroll_button_press_cb (G_GNUC_UNUSED ClutterActor *actor,
    ClutterButtonEvent *event,
    ChamplainView *view)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;

  if (priv->zoom_on_double_click && event->button == 1 && event->click_count == 2)
    return view_set_zoom_level_at (view, priv->zoom_level + 1, TRUE, event->x, event->y);

  return FALSE; /* Propagate the event */
}


static void
champlain_view_scroll (ChamplainView *view,
    gint deltax,
    gint deltay)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;
  gdouble lat, lon;
  gint x, y;

  x = priv->viewport_x + priv->viewport_width / 2.0 + deltax;
  y = priv->viewport_y + priv->viewport_height / 2.0 + deltay;

  lat = champlain_map_source_get_latitude (priv->map_source, priv->zoom_level, y);
  lon = champlain_map_source_get_longitude (priv->map_source, priv->zoom_level, x);

  if (priv->kinetic_mode)
    champlain_view_go_to_with_duration (view, lat, lon, 300);
  else
    champlain_view_center_on (view, lat, lon);
}


static gboolean
kinetic_scroll_key_press_cb (G_GNUC_UNUSED ClutterActor *actor,
    ClutterKeyEvent *event,
    ChamplainView *view)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;

  switch (event->keyval)
    {
    case 65361: /* Left */
      champlain_view_scroll (view, -priv->viewport_width / 4.0, 0);
      return TRUE;
      break;

    case 65362: /* Up */
      if (event->modifier_state & CLUTTER_CONTROL_MASK)
        champlain_view_zoom_in (view);
      else
        champlain_view_scroll (view, 0, -priv->viewport_width / 4.0);
      return TRUE;
      break;

    case 65363: /* Right */
      champlain_view_scroll (view, priv->viewport_width / 4.0, 0);
      return TRUE;
      break;

    case 65364: /* Down */
      if (event->modifier_state & CLUTTER_CONTROL_MASK)
        champlain_view_zoom_out (view);
      else
        champlain_view_scroll (view, 0, priv->viewport_width / 4.0);
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
  DEBUG_LOG ()

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
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  gdouble x, y;
  ChamplainViewPrivate *priv = view->priv;

  longitude = CLAMP (longitude, CHAMPLAIN_MIN_LONGITUDE, CHAMPLAIN_MAX_LONGITUDE);
  latitude = CLAMP (latitude, CHAMPLAIN_MIN_LATITUDE, CHAMPLAIN_MAX_LATITUDE);

  x = champlain_map_source_get_x (priv->map_source, priv->zoom_level, longitude) - priv->viewport_width / 2.0;
  y = champlain_map_source_get_y (priv->map_source, priv->zoom_level, latitude) - priv->viewport_height / 2.0;

  DEBUG ("Centering on %f, %f (%g, %g)", latitude, longitude, x, y);

  position_viewport (view, x, y);
  load_visible_tiles (view, FALSE);
}


static void
timeline_new_frame (G_GNUC_UNUSED ClutterTimeline *timeline,
    G_GNUC_UNUSED gint frame_num,
    GoToContext *ctx)
{
  DEBUG_LOG ()

  gdouble alpha;
  gdouble lat;
  gdouble lon;

  alpha = clutter_timeline_get_progress (timeline);
  lat = ctx->to_latitude - ctx->from_latitude;
  lon = ctx->to_longitude - ctx->from_longitude;

  champlain_view_center_on (ctx->view,
      ctx->from_latitude + alpha * lat,
      ctx->from_longitude + alpha * lon);
}


static void
timeline_completed (G_GNUC_UNUSED ClutterTimeline *timeline,
    ChamplainView *view)
{
  DEBUG_LOG ()

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
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = view->priv;

  if (priv->goto_context == NULL)
    return;

  clutter_timeline_stop (priv->goto_context->timeline);

  g_object_unref (priv->goto_context->timeline);

  g_slice_free (GoToContext, priv->goto_context);
  priv->goto_context = NULL;

  g_signal_emit_by_name (view, "animation-completed::go-to", NULL);
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
  DEBUG_LOG ()

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
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  if (duration == 0)
    {
      champlain_view_center_on (view, latitude, longitude);
      return;
    }

  GoToContext *ctx;

  ChamplainViewPrivate *priv = view->priv;

  champlain_view_stop_go_to (view);

  ctx = g_slice_new (GoToContext);
  ctx->from_latitude = priv->latitude;
  ctx->from_longitude = priv->longitude;
  ctx->to_latitude = CLAMP (latitude, CHAMPLAIN_MIN_LATITUDE, CHAMPLAIN_MAX_LATITUDE);;
  ctx->to_longitude = CLAMP (longitude, CHAMPLAIN_MIN_LONGITUDE, CHAMPLAIN_MAX_LONGITUDE);

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
  clutter_timeline_set_progress_mode (ctx->timeline, CLUTTER_EASE_IN_OUT_CIRC);

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
  DEBUG_LOG ()

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
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  champlain_view_set_zoom_level (view, view->priv->zoom_level - 1);
}


/**
 * champlain_view_set_zoom_level:
 * @view: a #ChamplainView
 * @zoom_level: the level of zoom, a guint between 1 and 20
 *
 * Changes the current level of zoom
 *
 * Since: 0.4
 */
void
champlain_view_set_zoom_level (ChamplainView *view,
    guint zoom_level)
{
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  view_set_zoom_level_at (view, zoom_level, FALSE, 0, 0);
}


/**
 * champlain_view_set_min_zoom_level:
 * @view: a #ChamplainView
 * @zoom_level: the level of zoom
 *
 * Changes the lowest allowed level of zoom
 *
 * Since: 0.4
 */
void
champlain_view_set_min_zoom_level (ChamplainView *view,
    guint min_zoom_level)
{
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = view->priv;

  if (priv->min_zoom_level == min_zoom_level ||
      min_zoom_level > priv->max_zoom_level ||
      min_zoom_level < champlain_map_source_get_min_zoom_level (priv->map_source))
    return;

  priv->min_zoom_level = min_zoom_level;
  g_object_notify (G_OBJECT (view), "min-zoom-level");

  if (priv->zoom_level < min_zoom_level)
    champlain_view_set_zoom_level (view, min_zoom_level);
}


/**
 * champlain_view_set_max_zoom_level:
 * @view: a #ChamplainView
 * @zoom_level: the level of zoom
 *
 * Changes the highest allowed level of zoom
 *
 * Since: 0.4
 */
void
champlain_view_set_max_zoom_level (ChamplainView *view,
    guint max_zoom_level)
{
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = view->priv;

  if (priv->max_zoom_level == max_zoom_level ||
      max_zoom_level < priv->min_zoom_level ||
      max_zoom_level > champlain_map_source_get_max_zoom_level (priv->map_source))
    return;

  priv->max_zoom_level = max_zoom_level;
  g_object_notify (G_OBJECT (view), "max-zoom-level");

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
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));
  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));

  clutter_actor_add_child (view->priv->user_layers, CLUTTER_ACTOR (layer));
  champlain_layer_set_view (layer, view);
  clutter_actor_set_child_above_sibling (view->priv->user_layers, CLUTTER_ACTOR (layer), NULL);
}


/**
 * champlain_view_remove_layer:
 * @view: a #ChamplainView
 * @layer: a #ChamplainLayer
 *
 * Removes the given layer from the view
 *
 * Since: 0.4.1
 */
void
champlain_view_remove_layer (ChamplainView *view,
    ChamplainLayer *layer)
{
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));
  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));

  champlain_layer_set_view (layer, NULL);

  clutter_actor_remove_child (view->priv->user_layers, CLUTTER_ACTOR (layer));
}


/**
 * champlain_view_x_to_longitude:
 * @view: a #ChamplainView
 * @x: x coordinate of the view
 *
 * Converts the view's x coordinate to longitude.
 *
 * Returns: the longitude
 *
 * Since: 0.10
 */
gdouble
champlain_view_x_to_longitude (ChamplainView *view,
    gdouble x)
{
  ChamplainViewPrivate *priv = view->priv;
  gdouble longitude;

  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), 0.0);

  longitude = champlain_map_source_get_longitude (priv->map_source,
        priv->zoom_level,
        x + priv->viewport_x);

  return longitude;
}


/**
 * champlain_view_y_to_latitude:
 * @view: a #ChamplainView
 * @y: y coordinate of the view
 *
 * Converts the view's y coordinate to latitude.
 *
 * Returns: the latitude
 *
 * Since: 0.10
 */
gdouble
champlain_view_y_to_latitude (ChamplainView *view,
    gdouble y)
{
  ChamplainViewPrivate *priv = view->priv;
  gdouble latitude;

  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), 0.0);

  latitude = champlain_map_source_get_latitude (priv->map_source,
        priv->zoom_level,
        y + priv->viewport_y);

  return latitude;
}


/**
 * champlain_view_longitude_to_x:
 * @view: a #ChamplainView
 * @longitude: the longitude
 *
 * Converts the longitude to view's x coordinate.
 *
 * Returns: the x coordinate
 *
 * Since: 0.10
 */
gdouble
champlain_view_longitude_to_x (ChamplainView *view,
    gdouble longitude)
{
  ChamplainViewPrivate *priv = view->priv;
  gdouble x;

  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), 0);

  x = champlain_map_source_get_x (priv->map_source, priv->zoom_level, longitude);

  return x - priv->viewport_x;
}


/**
 * champlain_view_latitude_to_y:
 * @view: a #ChamplainView
 * @latitude: the latitude
 *
 * Converts the latitude to view's y coordinate.
 *
 * Returns: the y coordinate
 *
 * Since: 0.10
 */
gdouble
champlain_view_latitude_to_y (ChamplainView *view,
    gdouble latitude)
{
  ChamplainViewPrivate *priv = view->priv;
  gdouble y;

  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), 0);

  y = champlain_map_source_get_y (priv->map_source, priv->zoom_level, latitude);

  return y - priv->viewport_y;
}


/**
 * champlain_view_get_viewport_origin:
 * @view: a #ChamplainView
 * @x: (out): the x coordinate of the viewport
 * @y: (out): the y coordinate of the viewport
 *
 * Gets the x and y coordinate of the viewport in respect to the layer origin.
 *
 * Since: 0.10
 */
void
champlain_view_get_viewport_origin (ChamplainView *view,
    gint *x,
    gint *y)
{
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));
  ChamplainViewPrivate *priv = view->priv;
  gint anchor_x, anchor_y;
  
  champlain_viewport_get_anchor (CHAMPLAIN_VIEWPORT (priv->viewport), &anchor_x, &anchor_y);

  if (x)
    *x = priv->viewport_x - anchor_x;

  if (y)
    *y = priv->viewport_y - anchor_y;
}


static void
fill_background_tiles (ChamplainView *view)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;
  ClutterActorIter iter;
  ClutterActor *child;
  gint x_count, y_count, x_first, y_first;
  gint x, y;
  gfloat width, height;
  gboolean have_children = TRUE;

  clutter_content_get_preferred_size (priv->background_content, &width, &height);

  x_count = ceil ((gfloat) priv->viewport_width / width) + 3;
  y_count = ceil ((gfloat) priv->viewport_height / height) + 3;

  x_first = (gint)priv->viewport_x / width - 1;
  y_first = (gint)priv->viewport_y / height - 1;
  
  clutter_actor_iter_init (&iter, priv->background_layer);

  for (x = x_first; x < x_first + x_count; ++x)
    {
      for (y = y_first; y < y_first + y_count; ++y)
        {
          if (!have_children || !clutter_actor_iter_next (&iter, &child))
            {
              have_children = FALSE;
              child = clutter_actor_new ();
              clutter_actor_set_size (child, width, height);
              clutter_actor_set_content (child, priv->background_content);
              clutter_actor_add_child (priv->background_layer, child);
            }
          champlain_viewport_set_actor_position (CHAMPLAIN_VIEWPORT (priv->viewport),
              child,
              (x * width) - priv->bg_offset_x,
              (y * height) - priv->bg_offset_y);
          child = clutter_actor_get_next_sibling (child);
        }
    }
  
  if (have_children)
    {
      while (clutter_actor_iter_next (&iter, &child))
          clutter_actor_iter_destroy (&iter);
    }
}


static void
load_visible_tiles (ChamplainView *view,
    gboolean relocate)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;
  ClutterActorIter iter;
  gint size;
  ClutterActor *child;
  gint x_count, y_count, x_first, y_first, x_end, y_end, max_x_end, max_y_end;
  gboolean *tile_map;
  gint arm_size, arm_max, turn;
  gint dirs[5] = { 0, 1, 0, -1, 0 };
  int i, x, y;

  size = champlain_map_source_get_tile_size (priv->map_source);

  max_x_end = champlain_map_source_get_column_count (priv->map_source, priv->zoom_level);
  max_y_end = champlain_map_source_get_row_count (priv->map_source, priv->zoom_level);

  x_count = ceil ((gfloat) priv->viewport_width / size) + 1;
  y_count = ceil ((gfloat) priv->viewport_height / size) + 1;

  x_first = CLAMP (priv->viewport_x / size, 0, max_x_end);
  y_first = CLAMP (priv->viewport_y / size, 0, max_y_end);

  x_end = x_first + x_count;
  y_end = y_first + y_count;

  x_end = CLAMP (x_end, x_first, max_x_end);
  y_end = CLAMP (y_end, y_first, max_y_end);

  x_count = x_end - x_first;
  y_count = y_end - y_first;

  DEBUG ("Range %d, %d to %d, %d", x_first, y_first, x_end, y_end);

  tile_map = g_slice_alloc0 (sizeof (gboolean) * x_count * y_count);

  /* fill background tiles */
  if (priv->background_content != NULL)
      fill_background_tiles (view);

  /* Get rid of old tiles first */
  clutter_actor_iter_init (&iter, priv->map_layer);
  while (clutter_actor_iter_next (&iter, &child))
    {
      ChamplainTile *tile = CHAMPLAIN_TILE (child);

      gint tile_x = champlain_tile_get_x (tile);
      gint tile_y = champlain_tile_get_y (tile);

      if (tile_x < x_first || tile_x >= x_end ||
          tile_y < y_first || tile_y >= y_end)
        {
          /* inform map source to terminate loading the tile */
          champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);
          clutter_actor_iter_destroy (&iter);
        }
      else
        {
          tile_map[(tile_y - y_first) * x_count + (tile_x - x_first)] = TRUE;
          if (relocate)
            champlain_viewport_set_actor_position (CHAMPLAIN_VIEWPORT (priv->viewport), CLUTTER_ACTOR (tile), tile_x * size, tile_y * size);
        }
    }
  
  /* Load new tiles if needed */
  x = x_first + x_count / 2 - 1;
  y = y_first + y_count / 2 - 1;
  arm_max = MAX (x_count, y_count) + 2;
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
              champlain_tile_set_x (tile, x);
              champlain_tile_set_y (tile, y);
              champlain_tile_set_zoom_level (tile, priv->zoom_level);
              champlain_tile_set_size (tile, size);
                  
              g_signal_connect (tile, "notify::state", G_CALLBACK (tile_state_notify), view);
              clutter_actor_add_child (priv->map_layer, CLUTTER_ACTOR (tile));
              champlain_viewport_set_actor_position (CHAMPLAIN_VIEWPORT (priv->viewport), CLUTTER_ACTOR (tile), x * size, y * size);

              /* updates champlain_view state automatically as
                 notify::state signal is connected  */
              champlain_tile_set_state (tile, CHAMPLAIN_STATE_LOADING);

              data = g_slice_new (FillTileCallbackData);
              data->tile = tile;
              data->map_source = priv->map_source;

              g_object_ref (tile);
              g_object_ref (priv->map_source);

              g_idle_add_full (CLUTTER_PRIORITY_REDRAW, (GSourceFunc) fill_tile_cb, data, NULL);
            }

          x += dirs[turn % 4 + 1];
          y += dirs[turn % 4];
        }

      if (turn % 2 == 1)
        arm_size++;
    }

  g_slice_free1 (sizeof (gboolean) * x_count * y_count, tile_map);
}


static gboolean
fill_tile_cb (FillTileCallbackData *data)
{
  DEBUG_LOG ()

  ChamplainTile *tile = data->tile;
  ChamplainMapSource *map_source = data->map_source;

  champlain_map_source_fill_tile (map_source, tile);

  g_slice_free (FillTileCallbackData, data);
  g_object_unref (map_source);
  g_object_unref (tile);

  return FALSE;
}


static void
remove_all_tiles (ChamplainView *view)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;
  ClutterActorIter iter;
  ClutterActor *child;

  clutter_actor_destroy_all_children (priv->zoom_layer);

  clutter_actor_iter_init (&iter, priv->map_layer);
  while (clutter_actor_iter_next (&iter, &child))
    champlain_tile_set_state (CHAMPLAIN_TILE (child), CHAMPLAIN_STATE_DONE);

  clutter_actor_destroy_all_children (priv->map_layer);
}


/**
 * champlain_view_reload_tiles:
 * @view: a #ChamplainView
 *
 * Reloads all visible tiles.
 *
 * Since: 0.8
 */
void
champlain_view_reload_tiles (ChamplainView *view)
{
  DEBUG_LOG ()

  remove_all_tiles (view);

  load_visible_tiles (view, FALSE);
}


static gboolean
remove_zoom_actor_cb (ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;
  
  clutter_actor_destroy_all_children (priv->zoom_layer);
  priv->zoom_actor_timeout = 0;
  return FALSE;
}


static void
tile_state_notify (ChamplainTile *tile,
    G_GNUC_UNUSED GParamSpec *pspec,
    ChamplainView *view)
{
  DEBUG_LOG ()

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
      if (priv->tiles_loading > 0)
        priv->tiles_loading--;
      if (priv->tiles_loading == 0)
        {
          priv->state = CHAMPLAIN_STATE_DONE;
          g_object_notify (G_OBJECT (view), "state");
          if (clutter_actor_get_n_children (priv->zoom_layer) > 0)
            priv->zoom_actor_timeout = g_timeout_add_seconds_full (CLUTTER_PRIORITY_REDRAW, 1, (GSourceFunc) remove_zoom_actor_cb, view, NULL);
        }
    }
}


/**
 * champlain_view_set_map_source:
 * @view: a #ChamplainView
 * @map_source: a #ChamplainMapSource
 *
 * Changes the currently used map source. #g_object_unref() will be called on
 * the previous one.
 *
 * Since: 0.4
 */
void
champlain_view_set_map_source (ChamplainView *view,
    ChamplainMapSource *source)
{
  DEBUG_LOG ()

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
    
  champlain_view_reload_tiles (view);

  g_object_notify (G_OBJECT (view), "map-source");
}


/**
 * champlain_view_set_deceleration:
 * @view: a #ChamplainView
 * @rate: a #gdouble between 1.001 and 2.0
 *
 * The deceleration rate for the kinetic mode.
 *
 * Since: 0.4
 */
void
champlain_view_set_deceleration (ChamplainView *view,
    gdouble rate)
{
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view) &&
      rate < 2.0 && rate > 1.0001);

  g_object_set (view->priv->kinetic_scroll, "decel-rate", rate, NULL);
  g_object_notify (G_OBJECT (view), "deceleration");
}


/**
 * champlain_view_set_kinetic_mode:
 * @view: a #ChamplainView
 * @kinetic: TRUE for kinetic mode, FALSE for push mode
 *
 * Determines the way the view reacts to scroll events.
 *
 * Since: 0.10
 */
void
champlain_view_set_kinetic_mode (ChamplainView *view,
    gboolean kinetic)
{
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = view->priv;

  priv->kinetic_mode = kinetic;
  g_object_set (view->priv->kinetic_scroll, "mode", kinetic, NULL);
  g_object_notify (G_OBJECT (view), "kinetic-mode");
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
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  view->priv->keep_center_on_resize = value;
  g_object_notify (G_OBJECT (view), "keep-center-on-resize");
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
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  view->priv->zoom_on_double_click = value;
  g_object_notify (G_OBJECT (view), "zoom-on-double-click");
}


/**
 * champlain_view_set_animate_zoom:
 * @view: a #ChamplainView
 * @value: a #gboolean
 *
 * Should the view animate zoom level changes.
 *
 * Since: 0.12
 */
void
champlain_view_set_animate_zoom (ChamplainView *view,
    gboolean value)
{
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  view->priv->animate_zoom = value;
  g_object_notify (G_OBJECT (view), "animate-zoom");
}


/**
 * champlain_view_ensure_visible:
 * @view: a #ChamplainView
 * @bbox: bounding box of the area that should be visible
 * @animate: TRUE to perform animation, FALSE otherwise
 *
 * Changes the map's zoom level and center to make sure the given area
 * is visible
 *
 * Since: 0.10
 */
void
champlain_view_ensure_visible (ChamplainView *view,
    ChamplainBoundingBox *bbox,
    gboolean animate)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;
  guint zoom_level = priv->zoom_level;
  gboolean good_size = FALSE;
  gdouble lat, lon;

  if (!champlain_bounding_box_is_valid (bbox))
    return;

  champlain_bounding_box_get_center (bbox, &lat, &lon);

  DEBUG ("Zone to expose (%f, %f) to (%f, %f)", bbox->bottom, bbox->left, bbox->top, bbox->right);
  do
    {
      gint min_x, min_y, max_x, max_y;

      min_x = champlain_map_source_get_x (priv->map_source, zoom_level, bbox->left);
      min_y = champlain_map_source_get_y (priv->map_source, zoom_level, bbox->bottom);
      max_x = champlain_map_source_get_x (priv->map_source, zoom_level, bbox->right);
      max_y = champlain_map_source_get_y (priv->map_source, zoom_level, bbox->top);

      if (min_y - max_y <= priv->viewport_height &&
          max_x - min_x <= priv->viewport_width)
        good_size = TRUE;
      else
        zoom_level--;

      if (zoom_level <= priv->min_zoom_level)
        {
          zoom_level = priv->min_zoom_level;
          good_size = TRUE;
        }
    } while (!good_size);

  DEBUG ("Ideal zoom level is %d", zoom_level);
  champlain_view_set_zoom_level (view, zoom_level);
  if (animate)
    champlain_view_go_to (view, lat, lon);
  else
    champlain_view_center_on (view, lat, lon);
}


/**
 * champlain_view_ensure_layers_visible:
 * @view: a #ChamplainView
 * @animate: TRUE to perform animation, FALSE otherwise
 *
 * Changes the map's zoom level and center to make sure that the bounding
 * boxes of all inserted layers are visible.
 *
 * Since: 0.10
 */
void
champlain_view_ensure_layers_visible (ChamplainView *view,
    gboolean animate)
{
  DEBUG_LOG ()

  ClutterActorIter iter;
  ClutterActor *child;
  ChamplainBoundingBox *bbox;

  bbox = champlain_bounding_box_new ();

  clutter_actor_iter_init (&iter, view->priv->user_layers);
  while (clutter_actor_iter_next (&iter, &child))
    {
      ChamplainLayer *layer = CHAMPLAIN_LAYER (child);
      ChamplainBoundingBox *other;

      other = champlain_layer_get_bounding_box (layer);
      champlain_bounding_box_compose (bbox, other);
      champlain_bounding_box_free (other);
    }

  champlain_view_ensure_visible (view, bbox, animate);

  champlain_bounding_box_free (bbox);
}


/**
 * champlain_view_set_background_pattern:
 * @view: a #ChamplainView
 * @background: The background texture
 *
 * Sets the background texture displayed behind the map. Setting the background
 * pattern affects performence slightly - use reasonably large patterns for
 * better performance.
 *
 * Since: 0.12.4
 */
void 
champlain_view_set_background_pattern (ChamplainView *view,
    ClutterContent *background)
{
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  ChamplainViewPrivate *priv = view->priv;
  
  if (priv->background_content)
    g_object_unref (priv->background_content);
    
  priv->background_content = g_object_ref_sink (background);
  clutter_actor_destroy_all_children (priv->background_layer);
}


/**
 * champlain_view_get_background_pattern:
 * @view: a #ChamplainView
 *
 * Gets the current background texture displayed behind the map.
 *
 * Returns: (transfer none): The texture.
 * 
 * Since: 0.12.4
 */
ClutterContent *
champlain_view_get_background_pattern (ChamplainView *view)
{
  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), NULL);

  ChamplainViewPrivate *priv = view->priv;

  return priv->background_content;
}


static void
position_zoom_actor (ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;
  gdouble x, y;
  gdouble deltazoom;
  
  clutter_actor_destroy_all_children (priv->zoom_layer);
  if (priv->zoom_actor_timeout != 0)
    {
      g_source_remove (priv->zoom_actor_timeout);
      priv->zoom_actor_timeout = 0;
    }

  ClutterActor *zoom_actor = clutter_actor_get_first_child (priv->zoom_overlay_actor);
  clutter_actor_set_pivot_point (zoom_actor, 0.0, 0.0);
  
  g_object_ref (zoom_actor);
  clutter_actor_remove_child(priv->zoom_overlay_actor, zoom_actor);
  clutter_actor_add_child (priv->zoom_layer, zoom_actor);
  g_object_unref (zoom_actor);

  deltazoom = pow (2, (gdouble)priv->zoom_level - (gdouble)priv->anim_start_zoom_level);
  x = priv->zoom_actor_viewport_x * deltazoom;
  y = priv->zoom_actor_viewport_y * deltazoom;

  champlain_viewport_set_actor_position (CHAMPLAIN_VIEWPORT (priv->viewport), zoom_actor, x, y);
}


static void
zoom_animation_completed (ClutterActor *actor,
    const gchar *transition_name,
    gboolean is_finished,
    ChamplainView *view)
{
  ChamplainViewPrivate *priv = view->priv;

  priv->animating_zoom = FALSE;
  position_zoom_actor (view);  
  clutter_actor_show (priv->user_layers);

  g_signal_handlers_disconnect_by_func (actor, zoom_animation_completed, view);
}


static void
show_zoom_actor (ChamplainView *view, 
    guint zoom_level, 
    gdouble x, 
    gdouble y)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;
  ClutterActor *zoom_actor = NULL;
  gdouble deltazoom;
  
  if (!priv->animating_zoom)
    {
      ClutterActorIter iter;
      ClutterActor *child;
      gint size;
      gint x_first, y_first;
      gdouble zoom_actor_width, zoom_actor_height;
      gdouble max_x_end, max_y_end;
      gdouble deltax, deltay;

      size = champlain_map_source_get_tile_size (priv->map_source);

      max_x_end = champlain_map_source_get_column_count (priv->map_source, priv->zoom_level);
      max_y_end = champlain_map_source_get_row_count (priv->map_source, priv->zoom_level);

      x_first = CLAMP (priv->viewport_x / size, 0, max_x_end);
      y_first = CLAMP (priv->viewport_y / size, 0, max_y_end);

      clutter_actor_destroy_all_children (priv->zoom_overlay_actor);
      zoom_actor = clutter_actor_new ();
      clutter_actor_add_child (priv->zoom_overlay_actor, zoom_actor);
      
      deltax = priv->viewport_x - x_first * size;
      deltay = priv->viewport_y - y_first * size;

      priv->anim_start_zoom_level = priv->zoom_level;
      priv->zoom_actor_viewport_x = priv->viewport_x - deltax;
      priv->zoom_actor_viewport_y = priv->viewport_y - deltay;

      clutter_actor_iter_init (&iter, priv->map_layer);
      while (clutter_actor_iter_next (&iter, &child))
        {
          ChamplainTile *tile = CHAMPLAIN_TILE (child);
          gint tile_x = champlain_tile_get_x (tile);
          gint tile_y = champlain_tile_get_y (tile);

          champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);
          
          g_object_ref (CLUTTER_ACTOR (tile));
          clutter_actor_iter_remove (&iter);
          clutter_actor_add_child (zoom_actor, CLUTTER_ACTOR (tile));
          g_object_unref (CLUTTER_ACTOR (tile));
          
          clutter_actor_set_position (CLUTTER_ACTOR (tile), (tile_x - x_first) * size, (tile_y - y_first) * size);
        }
      
      zoom_actor_width = clutter_actor_get_width (zoom_actor);
      zoom_actor_height = clutter_actor_get_height (zoom_actor);

      clutter_actor_set_pivot_point (zoom_actor, (x + deltax) / zoom_actor_width, (y + deltay) / zoom_actor_height);
      clutter_actor_set_position (zoom_actor, -deltax, -deltay);
    }
  else
    zoom_actor = clutter_actor_get_first_child (priv->zoom_overlay_actor);

  deltazoom = pow (2.0, (gdouble)zoom_level - priv->anim_start_zoom_level);

  if (priv->animate_zoom)
    {
      clutter_actor_set_opacity (priv->map_layer, 0);

      clutter_actor_destroy_all_children (priv->zoom_layer);

      clutter_actor_save_easing_state (zoom_actor);
      clutter_actor_set_easing_mode (zoom_actor, CLUTTER_EASE_IN_OUT_QUAD);
      clutter_actor_set_easing_duration (zoom_actor, 350);
      clutter_actor_set_scale (zoom_actor, deltazoom, deltazoom);
      clutter_actor_restore_easing_state (zoom_actor);

      clutter_actor_save_easing_state (priv->map_layer);
      clutter_actor_set_easing_mode (priv->map_layer, CLUTTER_EASE_IN_EXPO);
      clutter_actor_set_easing_duration (priv->map_layer, 350);
      clutter_actor_set_opacity (priv->map_layer, 255);
      clutter_actor_restore_easing_state (priv->map_layer);
        
      if (!priv->animating_zoom)
        {
          clutter_actor_hide (priv->user_layers);
          g_signal_connect (zoom_actor, "transition-stopped::scale-x", G_CALLBACK (zoom_animation_completed), view);
        }
        
      priv->animating_zoom = TRUE;
    }
  else
    clutter_actor_set_scale (zoom_actor, deltazoom, deltazoom);
}


/* Sets the zoom level, leaving the (x, y) at the exact same point in the view */
static gboolean
view_set_zoom_level_at (ChamplainView *view,
    guint zoom_level,
    gboolean use_event_coord,
    gint x,
    gint y)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;
  gdouble new_x, new_y;
  gdouble offset_x = x;
  gdouble offset_y = y;
  gdouble deltazoom;

  if (zoom_level == priv->zoom_level || ZOOM_LEVEL_OUT_OF_RANGE (priv, zoom_level))
    return FALSE;
    
  champlain_view_stop_go_to (view);
    
  if (!use_event_coord)
    {
      offset_x = priv->viewport_width / 2.0;
      offset_y = priv->viewport_height / 2.0;
    }

  /* don't do anything when view not yet realized */
  if (CLUTTER_ACTOR_IS_REALIZED (view))
    show_zoom_actor (view, zoom_level, offset_x, offset_y);

  deltazoom = pow (2, (gdouble)zoom_level - (gdouble)priv->zoom_level);

  new_x = (priv->viewport_x + offset_x) * deltazoom - offset_x;
  new_y = (priv->viewport_y + offset_y) * deltazoom - offset_y;
  
  priv->zoom_level = zoom_level;

  if (CLUTTER_ACTOR_IS_REALIZED (view))
    {
      resize_viewport (view);
      remove_all_tiles (view);
      position_viewport (view, new_x, new_y);
      load_visible_tiles (view, FALSE);

      if (!priv->animate_zoom)
        position_zoom_actor (view);
    }

  g_object_notify (G_OBJECT (view), "zoom-level");
  return TRUE;
}


/**
 * champlain_view_get_zoom_level:
 * @view: a #ChamplainView
 *
 * Gets the view's current zoom level.
 *
 * Returns: the view's current zoom level.
 *
 * Since: 0.4
 */
guint
champlain_view_get_zoom_level (ChamplainView *view)
{
  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), 0);

  return view->priv->zoom_level;
}


/**
 * champlain_view_get_min_zoom_level:
 * @view: a #ChamplainView
 *
 * Gets the view's minimal allowed zoom level.
 *
 * Returns: the view's minimal allowed zoom level.
 *
 * Since: 0.4
 */
guint
champlain_view_get_min_zoom_level (ChamplainView *view)
{
  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), 0);

  return view->priv->min_zoom_level;
}


/**
 * champlain_view_get_max_zoom_level:
 * @view: a #ChamplainView
 *
 * Gets the view's maximum allowed zoom level.
 *
 * Returns: the view's maximum allowed zoom level.
 *
 * Since: 0.4
 */
guint
champlain_view_get_max_zoom_level (ChamplainView *view)
{
  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), 0);

  return view->priv->max_zoom_level;
}


/**
 * champlain_view_get_map_source:
 * @view: a #ChamplainView
 *
 * Gets the view's current map source.
 *
 * Returns: (transfer none): the view's current map source. If you need to keep a reference to the
 * map source then you have to call #g_object_ref().
 *
 * Since: 0.4
 */
ChamplainMapSource *
champlain_view_get_map_source (ChamplainView *view)
{
  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), NULL);

  return view->priv->map_source;
}


/**
 * champlain_view_get_deceleration:
 * @view: a #ChamplainView
 *
 * Gets the view's deceleration rate.
 *
 * Returns: the view's deceleration rate.
 *
 * Since: 0.4
 */
gdouble
champlain_view_get_deceleration (ChamplainView *view)
{
  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), 0.0);

  gdouble decel = 0.0;
  g_object_get (view->priv->kinetic_scroll, "decel-rate", &decel, NULL);
  return decel;
}


/**
 * champlain_view_get_kinetic_mode:
 * @view: a #ChamplainView
 *
 * Gets the view's scroll mode behaviour.
 *
 * Returns: TRUE for kinetic mode, FALSE for push mode.
 *
 * Since: 0.10
 */
gboolean
champlain_view_get_kinetic_mode (ChamplainView *view)
{
  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), FALSE);

  return view->priv->kinetic_mode;
}


/**
 * champlain_view_get_keep_center_on_resize:
 * @view: a #ChamplainView
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
  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), FALSE);

  return view->priv->keep_center_on_resize;
}


/**
 * champlain_view_get_zoom_on_double_click:
 * @view: a #ChamplainView
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
  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), FALSE);

  return view->priv->zoom_on_double_click;
}


/**
 * champlain_view_get_animate_zoom:
 * @view: a #ChamplainView
 *
 * Checks whether the view animates zoom level changes.
 *
 * Returns: TRUE if the view animates zooms, FALSE otherwise.
 *
 * Since: 0.12
 */
gboolean
champlain_view_get_animate_zoom (ChamplainView *view)
{
  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), FALSE);

  return view->priv->animate_zoom;
}


static ClutterActorAlign
bin_alignment_to_actor_align (ClutterBinAlignment alignment)
{
    switch (alignment)
      {
        case CLUTTER_BIN_ALIGNMENT_FILL:
            return CLUTTER_ACTOR_ALIGN_FILL;
        case CLUTTER_BIN_ALIGNMENT_START:
            return CLUTTER_ACTOR_ALIGN_START;
        case CLUTTER_BIN_ALIGNMENT_END:
            return CLUTTER_ACTOR_ALIGN_END;
        case CLUTTER_BIN_ALIGNMENT_CENTER:
            return CLUTTER_ACTOR_ALIGN_CENTER;
        default:
            return CLUTTER_ACTOR_ALIGN_START;
      }
}


/**
 * champlain_view_bin_layout_add:
 * @view: a #ChamplainView
 * @child: The child to be inserted
 * @x_align: x alignment
 * @y_align: y alignment
 *
 * This function inserts a custom actor to the undrelying #ClutterBinLayout
 * manager. The inserted actors appear on top of the map. See clutter_bin_layout_add()
 * for reference.
 *
 * Since: 0.10
 *
 * Deprecated: 0.12.4: Use #ClutterActorAlign and the #ClutterActor
 * API instead.
 */
void
champlain_view_bin_layout_add (ChamplainView *view,
    ClutterActor *child,
    ClutterBinAlignment x_align,
    ClutterBinAlignment y_align)
{
  DEBUG_LOG ()

  g_return_if_fail (CHAMPLAIN_IS_VIEW (view));

  clutter_actor_set_x_expand (child, TRUE);
  clutter_actor_set_y_expand (child, TRUE);
  clutter_actor_set_x_align (child, bin_alignment_to_actor_align (x_align));
  clutter_actor_set_y_align (child, bin_alignment_to_actor_align (y_align));
  clutter_actor_add_child (CLUTTER_ACTOR (view), child);
}


/**
 * champlain_view_get_license_actor:
 * @view: a #ChamplainView
 *
 * Returns the #ChamplainLicense actor which is inserted by default into the
 * layout manager. It can be manipulated using standard #ClutterActor methods
 * (hidden and so on).
 *
 * Returns: (transfer none): the license actor
 *
 * Since: 0.10
 */
ChamplainLicense *
champlain_view_get_license_actor (ChamplainView *view)
{
  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), NULL);

  return CHAMPLAIN_LICENSE (view->priv->license_actor);
}


/**
 * champlain_view_get_center_latitude:
 * @view: a #ChamplainView
 *
 * Gets the latitude of the view's center.
 *
 * Returns: the latitude.
 *
 * Since: 0.10
 */
gdouble
champlain_view_get_center_latitude (ChamplainView *view)
{
  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), 0.0);

  return view->priv->latitude;
}


/**
 * champlain_view_get_center_longitude:
 * @view: a #ChamplainView
 *
 * Gets the longitude of the view's center.
 *
 * Returns: the longitude.
 *
 * Since: 0.10
 */
gdouble
champlain_view_get_center_longitude (ChamplainView *view)
{
  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), 0.0);

  return view->priv->longitude;
}


/**
 * champlain_view_get_state:
 * @view: a #ChamplainView
 *
 * Gets the view's state.
 *
 * Returns: the state.
 *
 * Since: 0.10
 */
ChamplainState
champlain_view_get_state (ChamplainView *view)
{
  DEBUG_LOG ()

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), CHAMPLAIN_STATE_NONE);

  return view->priv->state;
}

/**
 * champlain_view_get_bounding_box:
 * @view: a #ChamplainView
 *
 * Gets the bounding box for view @view at current zoom-level.
 *
 * Returns: (transfer full): the bounding box
 *
 * Since: 0.12.4
 */
ChamplainBoundingBox *
champlain_view_get_bounding_box (ChamplainView *view)
{
  DEBUG_LOG ()

  ChamplainViewPrivate *priv = view->priv;
  ChamplainBoundingBox *bbox;

  g_return_val_if_fail (CHAMPLAIN_IS_VIEW (view), NULL);

  bbox = champlain_bounding_box_new ();

  bbox->top = champlain_map_source_get_latitude (priv->map_source,
    priv->zoom_level,
    priv->viewport_y);
  bbox->bottom = champlain_map_source_get_latitude (priv->map_source,
    priv->zoom_level,
    priv->viewport_y + priv->viewport_height);

  bbox->left = champlain_map_source_get_longitude (priv->map_source,
    priv->zoom_level,
    priv->viewport_x);
  bbox->right = champlain_map_source_get_longitude (priv->map_source,
    priv->zoom_level,
    priv->viewport_x + priv->viewport_width);

  return bbox;
}
