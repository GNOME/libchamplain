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

#ifndef CHAMPLAIN_LINE_H
#define CHAMPLAIN_LINE_H

#include <champlain/champlain-defines.h>

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_LINE champlain_line_get_type()

#define CHAMPLAIN_LINE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_LINE, ChamplainLine))

#define CHAMPLAIN_LINE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_LINE, ChamplainLineClass))

#define CHAMPLAIN_IS_LINE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_LINE))

#define CHAMPLAIN_IS_LINE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_LINE))

#define CHAMPLAIN_LINE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_LINE, ChamplainLineClass))

typedef struct _ChamplainLinePrivate ChamplainLinePrivate;

typedef struct {
  GObject parent;
  ChamplainLinePrivate *priv;
} ChamplainLine;

typedef struct {
  GObjectClass parent_class;
} ChamplainLineClass;

GType champlain_line_get_type (void);

ChamplainLine* champlain_line_new (void);

void champlain_line_add_point (ChamplainLine *line,
    gdouble lat,
    gdouble lon);

void champlain_line_clear_points (ChamplainLine *line);
void champlain_line_set_fill_color (ChamplainLine *line,
    const ClutterColor *color);
void champlain_line_set_stroke_color (ChamplainLine *line,
    const ClutterColor *color);
ClutterColor * champlain_line_get_fill_color (ChamplainLine *line);
ClutterColor * champlain_line_get_stroke_color (ChamplainLine *line);

G_END_DECLS

#endif
