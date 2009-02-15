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

#include "sources/osmmapnik.h"

#include "champlain-map.h"

#include <math.h>
#include <clutter/clutter.h>


//http://wiki.openstreetmap.org/index.php/Slippy_map_tilenames#C.2FC.2B.2B

guint osm_mapnik_row_count(Map *map, guint zoom_level);
guint osm_mapnik_column_count(Map *map, guint zoom_level);
ChamplainTile *osm_mapnik_get_tile (Map *map, guint zoom_level, guint x, guint y);

gint osm_mapnik_longitude_to_x (Map *map, gdouble longitude, guint zoom_level);
gint osm_mapnik_latitude_to_y (Map *map, gdouble latitude, guint zoom_level);
gdouble osm_mapnik_x_to_longitude (Map *map, gint x, guint zoom_level);
gdouble osm_mapnik_y_to_latitude (Map *map, gint y, guint zoom_level);

gchar *osm_mapnik_get_tile_filename(Map *map, ChamplainTile *tile);
gchar *osm_mapnik_get_tile_uri(Map *map, ChamplainTile *tile);

void
osm_mapnik_init(Map *map)
{
  map->name = "OpenStreetMap";
  map->license = "Map data is CC BY-SA 2.0 by OpenStreetMap Contributors";
  map->license_uri = "http://creativecommons.org/licenses/by-sa/2.0/";
  map->zoom_levels = 18;
  map->tile_size = 256;

  map->get_row_count = osm_mapnik_row_count;
  map->get_column_count = osm_mapnik_column_count;

  map->longitude_to_x = osm_mapnik_longitude_to_x;
  map->latitude_to_y = osm_mapnik_latitude_to_y;
  map->x_to_longitude = osm_mapnik_x_to_longitude;
  map->y_to_latitude = osm_mapnik_y_to_latitude;

  map->get_tile_filename = osm_mapnik_get_tile_filename;
  map->get_tile_uri = osm_mapnik_get_tile_uri;
}

guint osm_mapnik_row_count(Map *map, guint zoom_level)
{
  return pow (2, zoom_level);
}

guint
osm_mapnik_column_count(Map *map, guint zoom_level)
{
  return pow (2, zoom_level);
}

gint
osm_mapnik_longitude_to_x (Map *map, gdouble longitude, guint zoom_level)
{
  return ((longitude + 180.0) / 360.0 * pow(2.0, zoom_level)) * map->tile_size;
}

gint
osm_mapnik_latitude_to_y (Map *map, gdouble latitude, guint zoom_level)
{
  return ((1.0 - log( tan(latitude * M_PI/180.0) + 1.0 / cos(latitude * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, zoom_level)) * map->tile_size;
}

gdouble
osm_mapnik_x_to_longitude (Map *map, gint x, guint zoom_level)
{
  gdouble dx = (float)x / map->tile_size;
  return dx / pow(2.0, zoom_level) * 360.0 - 180;
}

gdouble
osm_mapnik_y_to_latitude (Map *map, gint y, guint zoom_level)
{
  gdouble dy = (float)y / map->tile_size;
  double n = M_PI - 2.0 * M_PI * dy / pow(2.0, zoom_level);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

gchar *osm_mapnik_get_tile_filename(Map *map, ChamplainTile *tile)
{
  gint x, y, level;
  g_object_get (G_OBJECT (tile), "x", &x, "y", &y, "zoom-level", &level, NULL);
  return g_build_filename (g_strdup_printf("%d_%d_%d.png", level, y, x), NULL);
}

gchar *osm_mapnik_get_tile_uri(Map *map, ChamplainTile *tile)
{
  gint x, y, level;
  g_object_get (G_OBJECT (tile), "x", &x, "y", &y, "zoom-level", &level, NULL);
  return g_strdup_printf("http://tile.openstreetmap.org/%d/%d/%d.png", level, x, y);
}
