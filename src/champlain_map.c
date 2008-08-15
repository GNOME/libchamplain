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
 
 #include <champlain_map.h>
 
ChamplainMap* 
champlain_map_new (ChamplainMapSource source)
{
	ChamplainMap* map = g_new0(ChamplainMap, 1);
	map->name = "OpenStreetMap";
	map->zoom_levels = 1;
	return map;
}

void 
champlain_map_create_tiles(ChamplainMap* map, gint zoom_level)
{
	if (zoom_level == 1) 
		{
			map->current_level = g_new0(ChamplainMapZoomLevel, 1);
			map->current_level->level = zoom_level;
			map->current_level->row_count = 5;
			map->current_level->column_count = 4;
			map->current_level->tile_size = 200;
			map->current_level->tiles = g_ptr_array_sized_new (20);
			map->current_level->group = clutter_group_new ();
			
			ClutterColor white;
			clutter_color_parse ("white", &white);
			ClutterColor blue;
			clutter_color_parse ("blue", &blue);
			
  		int i;
			for (i = 0; i < 20; i++) 
				{
			 		int x = i % map->current_level->row_count;
			 		int y = i / map->current_level->row_count;
			 		
					ClutterColor * color = ( (y  + x) % 2 ? &blue : &white);
					ClutterActor * tile = clutter_rectangle_new_with_color (color);
					clutter_actor_set_position (tile, x * map->current_level->tile_size, y * map->current_level->tile_size);
					clutter_actor_set_size (tile, map->current_level->tile_size, map->current_level->tile_size);
					clutter_actor_show (tile);
					
					clutter_container_add (CLUTTER_CONTAINER (map->current_level->group), tile, NULL);
					g_ptr_array_add (map->current_level->tiles, tile);
				}
		}
}
