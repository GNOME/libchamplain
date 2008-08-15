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
champlain_map_zoom_level_create(ChamplainMapZoomLevel* level, gint zoom_level)
{
	if (zoom_level == 1) 
		{
			ClutterColor white;
			clutter_color_parse ("white", &white);
			ClutterColor blue;
			clutter_color_parse ("blue", &blue);
			
  		int i;
			for (i = 0; i < level->row_count * level->column_count; i++) 
				{
			 		int x = i % level->row_count;
			 		int y = i / level->row_count;
			 		
			 		ChamplainMapTile* tile = champlain_map_tile_new(x, y, level->tile_size);
					
					//clutter_container_add (CLUTTER_CONTAINER (level->group), tile->actor, NULL);
					g_ptr_array_add (level->tiles, tile);
				}
		}
}
