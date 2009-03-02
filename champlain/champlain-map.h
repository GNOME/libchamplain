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

#ifndef MAP_H
#define MAP_H

#include "champlain.h"
#include "champlain-private.h"
#include "champlain-tile.h"
#include "champlain-map-source.h"
#include "champlain-view.h"
#include "champlain-zoom-level.h"

#include <glib.h>
#include <clutter/clutter.h>

struct _Map
{
  ChamplainZoomLevel *current_level;
  ChamplainZoomLevel *previous_level;
};

Map *map_new ();

void map_load_visible_tiles (Map *map, ChamplainView * view, ChamplainMapSource *source, ChamplainRectangle viewport);

void map_free (Map *map);

gboolean map_zoom_in (Map *map, ChamplainMapSource *map_source);

gboolean map_zoom_out (Map *map, ChamplainMapSource *map_source);

gboolean map_zoom_to (Map *map, ChamplainMapSource *map_source, guint zoomLevel);

void map_load_level(Map *map, ChamplainMapSource *map_source, gint zoom_level);

#endif
