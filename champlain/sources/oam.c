/*
 * Copyright (C) 2008 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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

#include "sources/oam.h"
#include "map.h"
#include <math.h>
#include <clutter/clutter.h>

guint oam_row_count(Map *map, guint zoom_level);
guint oam_column_count(Map *map, guint zoom_level);
Tile *oam_get_tile (Map *map, guint zoom_level, guint x, guint y);

gint oam_longitude_to_x (Map *map, gdouble longitude, guint zoom_level);
gint oam_latitude_to_y (Map *map, gdouble latitude, guint zoom_level);
gdouble oam_x_to_longitude (Map *map, gint x, guint zoom_level);
gdouble oam_y_to_latitude (Map *map, gint y, guint zoom_level);

gchar *oam_get_tile_filename(Map *map, Tile *tile);
gchar *oam_get_tile_uri(Map *map, Tile *tile);

void
oam_init(Map *map)
{
  map->name = "OpenArialMap";
  map->zoom_levels = 17;
  map->tile_size = 256;

  map->get_row_count = oam_row_count;
  map->get_column_count = oam_column_count;

  map->longitude_to_x = oam_longitude_to_x;
  map->latitude_to_y = oam_latitude_to_y;
  map->x_to_longitude = oam_x_to_longitude;
  map->y_to_latitude = oam_y_to_latitude;

  map->get_tile_filename = oam_get_tile_filename;
  map->get_tile_uri = oam_get_tile_uri;
}

guint oam_row_count(Map *map, guint zoom_level)
{
  return pow (2, zoom_level);
}

guint
oam_column_count(Map *map, guint zoom_level)
{
  return pow (2, zoom_level);
}

gint
oam_longitude_to_x (Map *map, gdouble longitude, guint zoom_level)
{
  return ((longitude + 180.0) / 360.0 * pow(2.0, zoom_level)) * map->tile_size;
}

gint
oam_latitude_to_y (Map *map, gdouble latitude, guint zoom_level)
{
  return ((1.0 - log( tan(latitude * M_PI/180.0) + 1.0 / cos(latitude * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, zoom_level)) * map->tile_size;
}

gdouble
oam_x_to_longitude (Map *map, gint x, guint zoom_level)
{
  gdouble dx = (float)x / map->tile_size;
  return dx / pow(2.0, zoom_level) * 360.0 - 180;
}

gdouble
oam_y_to_latitude (Map *map, gint y, guint zoom_level)
{
  gdouble dy = (float)y / map->tile_size;
  double n = M_PI - 2.0 * M_PI * dy / pow(2.0, zoom_level);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

gchar *oam_get_tile_filename(Map *map, Tile *tile)
{
  return g_build_filename (g_strdup_printf("%d_%d_%d.png", tile->level, tile->y, tile->x), NULL);
}

gchar *oam_get_tile_uri(Map *map, Tile *tile)
{
  return g_strdup_printf("http://tile.openaerialmap.org/tiles/1.0.0/openaerialmap-900913/%d/%d/%d.jpg", tile->level, tile->x, tile->y);
}
