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

#ifndef CHAMPLAIN_MAP_TILE_H
#define CHAMPLAIN_MAP_TILE_H

#include <champlain/champlain-defines.h>

#include <glib.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_TILE champlain_tile_get_type()

#define CHAMPLAIN_TILE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_TILE, ChamplainTile))

#define CHAMPLAIN_TILE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_TILE, ChamplainTileClass))

#define CHAMPLAIN_IS_TILE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_TILE))

#define CHAMPLAIN_IS_TILE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_TILE))

#define CHAMPLAIN_TILE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_TILE, ChamplainTileClass))

typedef struct {
  GObject parent;
} ChamplainTile;

typedef struct {
  GObjectClass parent_class;
} ChamplainTileClass;

GType champlain_tile_get_type (void);

ChamplainTile* champlain_tile_new (void);
ChamplainTile* champlain_tile_new_full (gint x,
    gint y,
    guint size,
    gint zoom_level);

gint champlain_tile_get_x (ChamplainTile *self);
gint champlain_tile_get_y (ChamplainTile *self);
gint champlain_tile_get_zoom_level (ChamplainTile *self);
guint champlain_tile_get_size (ChamplainTile *self);
ChamplainState champlain_tile_get_state (ChamplainTile *self);
G_CONST_RETURN gchar * champlain_tile_get_uri (ChamplainTile *self);
G_CONST_RETURN gchar * champlain_tile_get_filename (ChamplainTile *self);
ClutterActor * champlain_tile_get_actor (ChamplainTile *self);
ClutterActor * champlain_tile_get_content (ChamplainTile *self);
const GTimeVal * champlain_tile_get_modified_time (ChamplainTile *self);
gchar * champlain_tile_get_modified_time_string (ChamplainTile *self);
const gchar * champlain_tile_get_etag (ChamplainTile *self);

void champlain_tile_set_x (ChamplainTile *self,
    gint x);
void champlain_tile_set_y (ChamplainTile *self, gint y);
void champlain_tile_set_zoom_level (ChamplainTile *self,
    gint zoom_level);
void champlain_tile_set_size (ChamplainTile *self,
    guint size);
void champlain_tile_set_state (ChamplainTile *self,
    ChamplainState state);
void champlain_tile_set_uri (ChamplainTile *self,
    const gchar *uri);
void champlain_tile_set_filename (ChamplainTile *self,
    const gchar *filename);
void champlain_tile_set_content (ChamplainTile *self,
    ClutterActor* actor,
    gboolean fade_in);
void champlain_tile_set_etag (ChamplainTile *self,
    const gchar *etag);
void champlain_tile_set_modified_time (ChamplainTile *self,
    const GTimeVal *time);

G_END_DECLS

#endif /* CHAMPLAIN_MAP_TILE_H */

