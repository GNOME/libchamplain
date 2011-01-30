/*
 * Copyright (C) 2008 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
 * Copyright (C) 2010 Jiri Techet <techet@gmail.com>
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

#if !defined (__CHAMPLAIN_CHAMPLAIN_H_INSIDE__) && !defined (CHAMPLAIN_COMPILATION)
#error "Only <champlain/champlain.h> can be included directly."
#endif

#ifndef CHAMPLAIN_VIEW_H
#define CHAMPLAIN_VIEW_H

#include <champlain/champlain-defines.h>
#include <champlain/champlain-layer.h>
#include <champlain/champlain-map-source.h>

#include <glib.h>
#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_VIEW champlain_view_get_type ()

#define CHAMPLAIN_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_VIEW, ChamplainView))

#define CHAMPLAIN_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_VIEW, ChamplainViewClass))

#define CHAMPLAIN_IS_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_VIEW))

#define CHAMPLAIN_IS_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_VIEW))

#define CHAMPLAIN_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_VIEW, ChamplainViewClass))

typedef struct _ChamplainViewPrivate ChamplainViewPrivate;

/**
 * ChamplainScrollMode:
 * @CHAMPLAIN_SCROLL_MODE_PUSH: Non-kinetic scrolling
 * @CHAMPLAIN_SCROLL_MODE_KINETIC: Kinetic scrolling
 *
 * Type of scrolling.
 */
typedef enum
{
  CHAMPLAIN_SCROLL_MODE_PUSH,
  CHAMPLAIN_SCROLL_MODE_KINETIC
} ChamplainScrollMode;


/**
 * ChamplainUnit:
 * @CHAMPLAIN_UNIT_KM: kilometers
 * @CHAMPLAIN_UNIT_MILES: miles
 *
 * Units used by the scale.
 */
typedef enum
{
  CHAMPLAIN_UNIT_KM,
  CHAMPLAIN_UNIT_MILES,
} ChamplainUnit;

struct _ChamplainView
{
  ClutterGroup group;

  ChamplainViewPrivate *priv;
};

struct _ChamplainViewClass
{
  ClutterGroupClass parent_class;
};

GType champlain_view_get_type (void);

ClutterActor *champlain_view_new (void);

void champlain_view_center_on (ChamplainView *view,
    gdouble latitude,
    gdouble longitude);
void champlain_view_go_to (ChamplainView *view,
    gdouble latitude,
    gdouble longitude);
void champlain_view_stop_go_to (ChamplainView *view);

void champlain_view_zoom_in (ChamplainView *view);
void champlain_view_zoom_out (ChamplainView *view);
void champlain_view_set_zoom_level (ChamplainView *view,
    gint zoom_level);
void champlain_view_set_min_zoom_level (ChamplainView *view,
    gint zoom_level);
void champlain_view_set_max_zoom_level (ChamplainView *view,
    gint zoom_level);

void champlain_view_ensure_visible (ChamplainView *view,
    gdouble lat1,
    gdouble lon1,
    gdouble lat2,
    gdouble lon2,
    gboolean animate);

void champlain_view_set_map_source (ChamplainView *view,
    ChamplainMapSource *map_source);
void champlain_view_set_decel_rate (ChamplainView *view,
    gdouble rate);
void champlain_view_set_scroll_mode (ChamplainView *view,
    ChamplainScrollMode mode);
void champlain_view_set_keep_center_on_resize (ChamplainView *view,
    gboolean value);
void champlain_view_set_show_license (ChamplainView *view,
    gboolean value);
void champlain_view_set_license_text (ChamplainView *view,
    const gchar *text);
void champlain_view_set_show_scale (ChamplainView *view,
    gboolean value);
void champlain_view_set_scale_unit (ChamplainView *view,
    ChamplainUnit unit);
void champlain_view_set_max_scale_width (ChamplainView *view,
    guint value);
void champlain_view_set_zoom_on_double_click (ChamplainView *view,
    gboolean value);

void champlain_view_add_layer (ChamplainView *view,
    ChamplainLayer *layer);
void champlain_view_remove_layer (ChamplainView *view,
    ChamplainLayer *layer);


gint champlain_view_get_zoom_level (ChamplainView *view);
gint champlain_view_get_min_zoom_level (ChamplainView *view);
gint champlain_view_get_max_zoom_level (ChamplainView *view);
ChamplainMapSource *champlain_view_get_map_source (ChamplainView *view);
gdouble champlain_view_get_decel_rate (ChamplainView *view);
ChamplainScrollMode champlain_view_get_scroll_mode (ChamplainView *view);
gboolean champlain_view_get_keep_center_on_resize (ChamplainView *view);
gboolean champlain_view_get_show_license (ChamplainView *view);
const gchar *champlain_view_get_license_text (ChamplainView *view);
gboolean champlain_view_get_show_scale (ChamplainView *view);
guint champlain_view_get_max_scale_width (ChamplainView *view);
ChamplainUnit champlain_view_get_scale_unit (ChamplainView *view);
gboolean champlain_view_get_zoom_on_double_click (ChamplainView *view);

void champlain_view_reload_tiles (ChamplainView *view);


gdouble champlain_view_x_to_longitude (ChamplainView *view,
    gdouble x);
gdouble champlain_view_y_to_latitude (ChamplainView *view,
    gdouble y);
gdouble champlain_view_longitude_to_x (ChamplainView *view, 
    gdouble longitude);
gdouble champlain_view_latitude_to_y (ChamplainView *view, 
    gdouble latitude);

double champlain_view_get_viewport_x (ChamplainView *view);
double champlain_view_get_viewport_y (ChamplainView *view);


G_END_DECLS

#endif
