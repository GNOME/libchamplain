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

#if !defined (__CHAMPLAIN_CHAMPLAIN_H_INSIDE__) && !defined (CHAMPLAIN_COMPILATION)
#error "Only <champlain/champlain.h> can be included directly."
#endif

#ifndef CHAMPLAIN_VIEW_H
#define CHAMPLAIN_VIEW_H

#include <champlain/champlain-defines.h>
#include <champlain/champlain-layer.h>
#include <champlain/champlain-map-source.h>
#include <champlain/champlain-zoom-level.h>

#include <glib.h>
#include <glib-object.h>
#include <clutter/clutter.h>

#define CHAMPLAIN_TYPE_VIEW     (champlain_view_get_type())
#define CHAMPLAIN_VIEW(obj)     (G_TYPE_CHECK_INSTANCE_CAST((obj), CHAMPLAIN_TYPE_VIEW, ChamplainView))
#define CHAMPLAIN_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST((klass),  CHAMPLAIN_TYPE_VIEW, ChamplainViewClass))
#define CHAMPLAIN_IS_VIEW(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHAMPLAIN_TYPE_VIEW))
#define CHAMPLAIN_IS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  CHAMPLAIN_TYPE_VIEW))
#define CHAMPLAIN_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  CHAMPLAIN_TYPE_VIEW, ChamplainViewClass))

typedef struct _ChamplainViewPrivate ChamplainViewPrivate;

/**
 * ChamplainViewMode:
 * @CHAMPLAIN_VIEW_MODE_PUSH: Non-kinetic scrolling
 * @CHAMPLAIN_VIEW_MODE_KINETIC: Kinetic scrolling
 *
 * Type of scrolling.
 */
typedef enum {
  CHAMPLAIN_SCROLL_MODE_PUSH,
  CHAMPLAIN_SCROLL_MODE_KINETIC
} ChamplainScrollMode;

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

void champlain_view_center_on (ChamplainView *view, gdouble latitude, gdouble longitude);

void champlain_view_zoom_in (ChamplainView *champlainView);

void champlain_view_zoom_out (ChamplainView *champlainView);

void champlain_view_set_zoom_level (ChamplainView *champlainView,
    gint zoom_level);
void champlain_view_set_map_source (ChamplainView *champlainView,
    ChamplainMapSource *map_source);
void champlain_view_set_size (ChamplainView *view, guint width, guint height);
void champlain_view_set_decel_rate (ChamplainView *view, gdouble rate);
void champlain_view_set_scroll_mode (ChamplainView *view,
    ChamplainScrollMode mode);
void champlain_view_set_keep_center_on_resize (ChamplainView *view,
    gboolean value);
void champlain_view_set_show_license (ChamplainView *view, gboolean value);

void champlain_view_add_layer (ChamplainView *champlainView, ChamplainLayer *layer);

gboolean champlain_view_get_coords_from_event (ChamplainView *view, ClutterEvent *event, gdouble *lat, gdouble *lon);

void champlain_view_tile_ready (ChamplainView *view, ChamplainZoomLevel *level, ChamplainTile *tile, gboolean animate);

#endif
