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

#ifndef CHAMPLAIN_ZOOM_LEVEL_H
#define CHAMPLAIN_ZOOM_LEVEL_H

#include "champlain-tile.h"

#include <glib.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_ZOOM_LEVEL champlain_zoom_level_get_type()

#define CHAMPLAIN_ZOOM_LEVEL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_ZOOM_LEVEL, ChamplainZoomLevel))

#define CHAMPLAIN_ZOOM_LEVEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_ZOOM_LEVEL, ChamplainZoomLevelClass))

#define CHAMPLAIN_IS_ZOOM_LEVEL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_ZOOM_LEVEL))

#define CHAMPLAIN_IS_ZOOM_LEVEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_ZOOM_LEVEL))

#define CHAMPLAIN_ZOOM_LEVEL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_ZOOM_LEVEL, ChamplainZoomLevelClass))

typedef struct _ChamplainZoomLevelPrivate ChamplainZoomLevelPrivate;

typedef struct _ChamplainZoomLevel        ChamplainZoomLevel;
typedef struct _ChamplainZoomLevelClass   ChamplainZoomLevelClass;

struct _ChamplainZoomLevel{
  ClutterGroup parent;
};

struct _ChamplainZoomLevelClass{
  ClutterGroupClass parent_class;
};

GType champlain_zoom_level_get_type (void);

ChamplainZoomLevel* champlain_zoom_level_new (void);

guint champlain_zoom_level_get_width (ChamplainZoomLevel *self);
guint champlain_zoom_level_get_height (ChamplainZoomLevel *self);
gint champlain_zoom_level_get_zoom_level (ChamplainZoomLevel *self);

void champlain_zoom_level_set_width (ChamplainZoomLevel *self,
    guint width);
void champlain_zoom_level_set_height (ChamplainZoomLevel *self,
    guint height);
void champlain_zoom_level_set_zoom_level (ChamplainZoomLevel *self,
    gint zoom_level);

gboolean champlain_zoom_level_zoom_to (ChamplainZoomLevel *self,
    ChamplainMapSource *source,
    guint zoomLevel);

G_END_DECLS

#endif
