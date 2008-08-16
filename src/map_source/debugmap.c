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
 
#include "map_source/debugmap.h"
#include <math.h>

guint debugmap_row_count(ChamplainMap* map, guint zoom_level);
guint debugmap_column_count(ChamplainMap* map, guint zoom_level);
ChamplainMapTile* debugmap_get_tile (ChamplainMap* map, guint zoom_level, guint x, guint y);

void 
debugmap_init(ChamplainMap* map)
{
	map->name = "Debug";
	map->zoom_levels = 1;
	map->tile_size = 256;
  
  map->get_row_count = debugmap_row_count;
  map->get_column_count = debugmap_column_count;
  map->get_tile = debugmap_get_tile;
}

guint debugmap_row_count(ChamplainMap* map, guint zoom_level)
{
	return pow (2, zoom_level);
}

guint debugmap_column_count(ChamplainMap* map, guint zoom_level)
{
	return pow (2, zoom_level);
}

ChamplainMapTile* debugmap_get_tile (ChamplainMap* map, guint zoom_level, guint x, guint y)
{

	ChamplainMapTile* tile = g_new0(ChamplainMapTile, 1);
	
	ClutterColor white;
	clutter_color_parse ("white", &white);
	ClutterColor blue;
	clutter_color_parse ("blue", &blue);
	
	ClutterColor * color, * textColor;
	if ((y  + x) % 2) 
		{
			color = &blue;
			textColor = &white;
		} 
	else 
		{
			color = &white;
			textColor = &blue;
		}
	
	
	tile->x = x;
	tile->y = y;
	tile->visible = FALSE;
	tile->actor = clutter_group_new();
	
	ClutterActor* actor = clutter_rectangle_new_with_color (color);
	clutter_actor_set_position (actor, x * map->tile_size, y * map->tile_size);
	clutter_actor_set_size (actor, map->tile_size, map->tile_size);
	clutter_actor_show (actor);
  clutter_container_add_actor (CLUTTER_CONTAINER (tile->actor), actor);
  
  actor = clutter_label_new_full ("Arial", g_strdup_printf("%d, %d", x, y), textColor);
	clutter_actor_set_position (actor, x * map->tile_size + map->tile_size/2.25, y * map->tile_size + map->tile_size/2.25);
  clutter_container_add_actor (CLUTTER_CONTAINER (tile->actor), actor);
  
	g_object_ref(tile->actor); // to prevent actors to be destroyed when they are removed from groups
	
	return tile;
}
