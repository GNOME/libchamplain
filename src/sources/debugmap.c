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
 
#include "sources/debugmap.h"
#include <math.h>

guint debugmap_row_count(Map* map, guint zoom_level);
guint debugmap_column_count(Map* map, guint zoom_level);
Tile* debugmap_get_tile (Map* map, guint zoom_level, guint x, guint y);

gint debugmap_longitude_to_x (Map* map, gdouble longitude, guint zoom_level);
gint debugmap_latitude_to_y (Map* map, gdouble latitude, guint zoom_level);
gdouble debugmap_x_to_longitude (Map* map, gint x, guint zoom_level);
gdouble debugmap_y_to_latitude (Map* map, gint y, guint zoom_level);

void 
debugmap_init(Map* map)
{
  map->name = "Debug";
  map->zoom_levels = 1;
  map->tile_size = 256;
  
  map->get_row_count = debugmap_row_count;
  map->get_column_count = debugmap_column_count;
  
  map->longitude_to_x = debugmap_longitude_to_x;
  map->latitude_to_y = debugmap_latitude_to_y;
  map->x_to_longitude = debugmap_x_to_longitude;
  map->y_to_latitude = debugmap_y_to_latitude;
}

guint debugmap_row_count(Map* map, guint zoom_level)
{
  return pow (2, zoom_level);
}

guint debugmap_column_count(Map* map, guint zoom_level)
{
  return pow (2, zoom_level);
}

Tile* debugmap_get_tile (Map* map, guint zoom_level, guint x, guint y)
{

  Tile* tile = g_new0(Tile, 1);
  
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
  tile->actor = clutter_group_new();
  
  ClutterActor* actor = clutter_rectangle_new_with_color (color);
  clutter_actor_set_position (actor, x * map->tile_size, y * map->tile_size);
  clutter_actor_set_size (actor, map->tile_size, map->tile_size);
  clutter_actor_show (actor);
  clutter_container_add_actor (CLUTTER_CONTAINER (tile->actor), actor);
  
  x *= map->tile_size;
  y *= map->tile_size;
  
  gdouble lon, lat;
  lon = debugmap_x_to_longitude(map, x, zoom_level);
  lat = debugmap_y_to_latitude(map, x, zoom_level);
  
  actor = clutter_label_new_full ("Arial", g_strdup_printf("%.2f, %.2f", lon, lat), textColor);
  clutter_actor_set_position (actor, x, y);
  clutter_container_add_actor (CLUTTER_CONTAINER (tile->actor), actor);
  
  g_object_ref(tile->actor); // to prevent actors to be destroyed when they are removed from groups
  
  return tile;
}

//FIXME: These functions need to be fixed
gint debugmap_longitude_to_x (Map* map, gdouble longitude, guint zoom_level)
{
  return ((longitude + 180.0) / 360.0 * pow(2.0, zoom_level)) * map->tile_size; 
}

gint debugmap_latitude_to_y (Map* map, gdouble latitude, guint zoom_level)
{
  return ((latitude + 90.0) / 180.0 * pow(2.0, zoom_level)) * map->tile_size; 
}

gdouble debugmap_x_to_longitude (Map* map, gint x, guint zoom_level)
{
  x /= map->tile_size;
  return x / map->tile_size * pow(2.0, zoom_level) * 360.0 - 180;
}

gdouble debugmap_y_to_latitude (Map* map, gint y, guint zoom_level)
{
  y /= map->tile_size;
  return y / map->tile_size * pow(2.0, zoom_level) * 180.0 - 90;
}

