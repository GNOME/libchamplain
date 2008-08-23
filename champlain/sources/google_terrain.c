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
 
/*
 * WARNING: Using the Google Map Tiles is in viloation of the Terms of Service.
 * The current code isn't working because the web server is returning Forbiden error message.
 */
 
#include "sources/google_terrain.h"
#include "map.h"
#include <math.h>
#include <clutter/clutter.h>

guint google_terrain_row_count(Map* map, guint zoom_level);
guint google_terrain_column_count(Map* map, guint zoom_level);
Tile* google_terrain_get_tile (Map* map, guint zoom_level, guint x, guint y);

gint google_terrain_longitude_to_x (Map* map, gdouble longitude, guint zoom_level);
gint google_terrain_latitude_to_y (Map* map, gdouble latitude, guint zoom_level);
gdouble google_terrain_x_to_longitude (Map* map, gint x, guint zoom_level);
gdouble google_terrain_y_to_latitude (Map* map, gint y, guint zoom_level);

gchar* google_terrain_get_tile_filename(Map* map, Tile* tile);
gchar* google_terrain_get_tile_uri(Map* map, Tile* tile);

void
google_terrain_init(Map* map)
{
  map->name = "OpenStreetMap";
  map->zoom_levels = 17;
  map->tile_size = 256;
  
  map->get_row_count = google_terrain_row_count;
  map->get_column_count = google_terrain_column_count;
  
  map->longitude_to_x = google_terrain_longitude_to_x;
  map->latitude_to_y = google_terrain_latitude_to_y;
  map->x_to_longitude = google_terrain_x_to_longitude;
  map->y_to_latitude = google_terrain_y_to_latitude;
  
  map->get_tile_filename = google_terrain_get_tile_filename;
  map->get_tile_uri = google_terrain_get_tile_uri;
}

guint google_terrain_row_count(Map* map, guint zoom_level)
{
  return pow (2, zoom_level);
}

guint 
google_terrain_column_count(Map* map, guint zoom_level)
{
  return pow (2, zoom_level);
}

gint 
google_terrain_longitude_to_x (Map* map, gdouble longitude, guint zoom_level)
{
  return ((longitude + 180.0) / 360.0 * pow(2.0, zoom_level)) * map->tile_size;
}

gint 
google_terrain_latitude_to_y (Map* map, gdouble latitude, guint zoom_level)
{
  return ((1.0 - log( tan(latitude * M_PI/180.0) + 1.0 / cos(latitude * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, zoom_level)) * map->tile_size;
}

gdouble 
google_terrain_x_to_longitude (Map* map, gint x, guint zoom_level)
{
  gdouble dx = (float)x / map->tile_size;
  return dx / pow(2.0, zoom_level) * 360.0 - 180;
}

gdouble 
google_terrain_y_to_latitude (Map* map, gint y, guint zoom_level)
{
  gdouble dy = (float)y / map->tile_size;
  double n = M_PI - 2.0 * M_PI * dy / pow(2.0, zoom_level);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

gchar* google_terrain_get_tile_filename(Map* map, Tile* tile)
{
  return g_build_filename (g_strdup_printf("%d_%d_%d.png", tile->level, tile->y, tile->x), NULL);
}

gchar* google_terrain_get_tile_uri(Map* map, Tile* tile)
{
  return g_strdup_printf("http://tile.openstreetmap.org/%d/%d/%d.png", tile->level, tile->x, tile->y, NULL);
}
