/*
 * Copyright (C) 2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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

#ifndef CHAMPLAIN_POLYGON_H
#define CHAMPLAIN_POLYGON_H

#include <champlain/champlain-point.h>
#include <champlain/champlain-defines.h>

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_POLYGON champlain_polygon_get_type ()

#define CHAMPLAIN_POLYGON(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_POLYGON, ChamplainPolygon))

#define CHAMPLAIN_POLYGON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_POLYGON, ChamplainPolygonClass))

#define CHAMPLAIN_IS_POLYGON(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_POLYGON))

#define CHAMPLAIN_IS_POLYGON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_POLYGON))

#define CHAMPLAIN_POLYGON_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_POLYGON, ChamplainPolygonClass))

typedef struct _ChamplainPolygonPrivate ChamplainPolygonPrivate;

typedef struct _ChamplainPolygon ChamplainPolygon;
typedef struct _ChamplainPolygonClass ChamplainPolygonClass;

struct _ChamplainPolygon
{
  ClutterGroup parent;

  ChamplainPolygonPrivate *priv;
};

struct _ChamplainPolygonClass
{
  ClutterGroupClass parent_class;
};

GType champlain_polygon_get_type (void);

ChamplainPolygon *champlain_polygon_new (void);

ChamplainPoint *champlain_polygon_append_point (ChamplainPolygon *polygon,
    gdouble lat,
    gdouble lon);
ChamplainPoint *champlain_polygon_insert_point (ChamplainPolygon *polygon,
    gdouble lat,
    gdouble lon,
    gint pos);
void champlain_polygon_remove_point (ChamplainPolygon *polygon,
    ChamplainPoint *point);
void champlain_polygon_clear_points (ChamplainPolygon *polygon);
GList *champlain_polygon_get_points (ChamplainPolygon *polygon);

void champlain_polygon_set_fill_color (ChamplainPolygon *polygon,
    const ClutterColor *color);
void champlain_polygon_set_stroke_color (ChamplainPolygon *polygon,
    const ClutterColor *color);
ClutterColor *champlain_polygon_get_fill_color (ChamplainPolygon *polygon);
ClutterColor *champlain_polygon_get_stroke_color (ChamplainPolygon *polygon);

gboolean champlain_polygon_get_fill (ChamplainPolygon *polygon);
void champlain_polygon_set_fill (ChamplainPolygon *polygon,
    gboolean value);
gboolean champlain_polygon_get_stroke (ChamplainPolygon *polygon);
void champlain_polygon_set_stroke (ChamplainPolygon *polygon,
    gboolean value);
void champlain_polygon_set_stroke_width (ChamplainPolygon *polygon,
    gdouble value);
gdouble champlain_polygon_get_stroke_width (ChamplainPolygon *polygon);
void champlain_polygon_set_mark_points (ChamplainPolygon *polygon,
    gboolean value);
gboolean champlain_polygon_get_mark_points (ChamplainPolygon *polygon);

void champlain_polygon_show (ChamplainPolygon *polygon);
void champlain_polygon_hide (ChamplainPolygon *polygon);

void champlain_polygon_draw_polygon (ChamplainPolygon *polygon,
    ChamplainView *view);

G_END_DECLS

#endif
