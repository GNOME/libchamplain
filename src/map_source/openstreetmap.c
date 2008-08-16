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
 
#include "map_source/openstreetmap.h"
#include <champlain_map.h>
#include <math.h>

//http://wiki.openstreetmap.org/index.php/Slippy_map_tilenames#C.2FC.2B.2B

guint osm_row_count(ChamplainMap* map, guint zoom_level);
guint osm_column_count(ChamplainMap* map, guint zoom_level);
ChamplainMapTile* osm_get_tile (ChamplainMap* map, guint zoom_level, guint x, guint y);

void
osm_init(ChamplainMap* map)
{
	map->name = "OpenStreetMap";
	map->zoom_levels = 17;
  map->tile_size = 256;
  
  map->get_row_count = osm_row_count;
  map->get_column_count = osm_column_count;
  map->get_tile = osm_get_tile;
}

guint osm_row_count(ChamplainMap* map, guint zoom_level)
{
	return pow (2, zoom_level);
}

guint osm_column_count(ChamplainMap* map, guint zoom_level)
{
	return pow (2, zoom_level);
}

ChamplainMapTile* osm_get_tile (ChamplainMap* map, guint zoom_level, guint x, guint y)
{
	ChamplainMapTile* tile = g_new0(ChamplainMapTile, 1);
	
	tile->x = x;
	tile->y = y;
	tile->visible = FALSE;
  // For no apparent reason, the group is necessary even if 
  // it contains only one actor... if missing, the viewport will break
	tile->actor = clutter_group_new();
																			
	ClutterActor* actor = clutter_texture_new_from_file(g_strdup_printf("/home/plbeaudoin/champlain/tiles/%d/%d/%d.png", zoom_level, x, y), NULL);
	clutter_actor_set_position (actor, x * map->tile_size, y * map->tile_size);
	clutter_actor_set_size (actor, map->tile_size, map->tile_size);
	clutter_actor_show (actor);
  clutter_container_add_actor (CLUTTER_CONTAINER (tile->actor), actor);
  
	g_object_ref(tile->actor); // to prevent actors to be destroyed when they are removed from groups
	
	return tile;
	
}
