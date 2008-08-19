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
}

void 
map_load_visible_tiles (Map* map, ChamplainRect viewport)
{
  gint x_count = ceil((float)viewport.width / map->tile_size) + 1;
  gint y_count = ceil((float)viewport.height / map->tile_size) + 1;
  
  gint x_first = viewport.x / map->tile_size;
  gint y_first = viewport.y / map->tile_size;
  
  x_count += x_first;
  y_count += y_first;
  
  int i, j, k;
  for (i = x_first; i < x_count; i++)
    {
      for (j = y_first; j < y_count; j++)
        {
          if(i >= map->current_level->row_count || j >= map->current_level->column_count)
            continue;
            
          gboolean exist = FALSE;
          for (k = 0; k < map->current_level->tiles->len && !exist; k++)
            {
              Tile* tile = g_ptr_array_index(map->current_level->tiles, k);
              if ( tile->x == i && tile->y == j)
                exist = TRUE;
            }

          if(!exist)
            {
              Tile* tile = map->get_tile(map, map->current_level->level, i, j);

              g_ptr_array_add (map->current_level->tiles, tile);
            }
        }
    }
}
