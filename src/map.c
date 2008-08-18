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
 
#include <map.h>
#include <zoomlevel.h>
 

Map* 
map_new (ChamplainMapSource source)
{
  Map* map = g_new0(Map, 1);

  switch(source) 
    {
      case CHAMPLAIN_MAP_SOURCE_DEBUG:
        debugmap_init(map);
        break;
      case CHAMPLAIN_MAP_SOURCE_OPENSTREETMAP:
        osm_init(map);
        break;
    }
    
  return map;
}

void 
map_load(Map* map, gint zoom_level)
{
    guint row_count = map->get_row_count(map, zoom_level);
    guint column_count = map->get_column_count(map, zoom_level);
    
    map->current_level = zoom_level_new(zoom_level, row_count, column_count, map->tile_size);
    zoom_level_create(map, zoom_level);
}

