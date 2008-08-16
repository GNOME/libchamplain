/*
 * Copyright (C) 2008 Pierre-Luc Beaudoin <pierre-luc@squidy.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <champlain_map_zoom_level.h>
#include <champlain_map_tile.h>
#include <champlain_map.h>
#include <champlain_private.h>
#include <clutter/clutter.h>

ChamplainMapZoomLevel* 
champlain_map_zoom_level_new(gint zoom_level, gint row, gint column, gint tile_size)
{
	ChamplainMapZoomLevel* level = g_new0(ChamplainMapZoomLevel, 1);
	
	level->level = zoom_level;
	level->row_count = row;
	level->column_count = column;
	level->tile_size = tile_size;
	level->tiles = g_ptr_array_sized_new (row * column);
	level->group = clutter_group_new ();
	
	return level;
}

void 
champlain_map_zoom_level_create(ChamplainMap* map, gint zoom_level)
{
	int i;
	for (i = 0; i < map->current_level->row_count * map->current_level->column_count; i++) 
		{
	 		int x = i % map->current_level->column_count;
	 		int y = i / map->current_level->column_count;
	 		
	 		ChamplainMapTile* tile = map->get_tile(map, zoom_level, x, y);
			
			clutter_container_add (CLUTTER_CONTAINER (map->current_level->group), tile->actor, NULL);
			g_ptr_array_add (map->current_level->tiles, tile);
		}
}

guint
champlain_map_zoom_level_get_width(ChamplainMapZoomLevel* level)
{
		return (level->column_count + 1) * level->tile_size;
}

guint
champlain_map_zoom_level_get_height(ChamplainMapZoomLevel* level)
{
		return (level->row_count + 1) * level->tile_size;
}
