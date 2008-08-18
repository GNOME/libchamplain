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
 
#include "sources/openstreetmap.h"
#include <map.h>
#include <math.h>

//http://wiki.openstreetmap.org/index.php/Slippy_map_tilenames#C.2FC.2B.2B

guint osm_row_count(Map* map, guint zoom_level);
guint osm_column_count(Map* map, guint zoom_level);
Tile* osm_get_tile (Map* map, guint zoom_level, guint x, guint y);

gdouble osm_longitude_to_x (Map* map, gdouble longitude, guint zoom_level);
gdouble osm_latitude_to_y (Map* map, gdouble latitude, guint zoom_level);
gdouble osm_x_to_longitude (Map* map, gdouble x, guint zoom_level);
gdouble osm_y_to_latitude (Map* map, gdouble y, guint zoom_level);

void
osm_init(Map* map)
{
  map->name = "OpenStreetMap";
  map->zoom_levels = 17;
  map->tile_size = 256;
  
  map->get_row_count = osm_row_count;
  map->get_column_count = osm_column_count;
  map->get_tile = osm_get_tile;
  
  map->longitude_to_x = osm_longitude_to_x;
  map->latitude_to_y = osm_latitude_to_y;
  map->x_to_longitude = osm_x_to_longitude;
  map->y_to_latitude = osm_y_to_latitude;
}

guint osm_row_count(Map* map, guint zoom_level)
{
  return pow (2, zoom_level);
}

guint osm_column_count(Map* map, guint zoom_level)
{
  return pow (2, zoom_level);
}

Tile* osm_get_tile (Map* map, guint zoom_level, guint x, guint y)
{
  Tile* tile = g_new0(Tile, 1);
  
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

gdouble osm_longitude_to_x (Map* map, gdouble longitude, guint zoom_level)
{
  return ((longitude + 180.0) / 360.0 * pow(2.0, zoom_level)) * map->tile_size; 
}

gdouble osm_latitude_to_y (Map* map, gdouble latitude, guint zoom_level)
{
  return ((1.0 - log( tan(latitude * M_PI/180.0) + 1.0 / cos(latitude * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, zoom_level)) * map->tile_size;
}

gdouble osm_x_to_longitude (Map* map, gdouble x, guint zoom_level)
{
  x /= map->tile_size;
  return x / map->tile_size * pow(2.0, zoom_level) * 360.0 - 180;
}

gdouble osm_y_to_latitude (Map* map, gdouble y, guint zoom_level)
{
  y /= map->tile_size;
  double n = M_PI - 2.0 * M_PI * y / pow(2.0, zoom_level);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

