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

#include "map.h"
#include "zoomlevel.h"
#include "sources/osm_mapnik.h"
#include "sources/mff_relief.h"
#include "sources/google_sat.h"
#include "sources/google_map.h"
#include "sources/google_terrain.h"
#include <math.h>

Map*
map_new (ChamplainMapSource source)
{
  Map *map = g_new0(Map, 1);

  switch(source)
    {
      case CHAMPLAIN_MAP_SOURCE_DEBUG:
        debugmap_init(map);
        break;
      case CHAMPLAIN_MAP_SOURCE_OPENSTREETMAP:
        osm_mapnik_init(map);
        break;
      case CHAMPLAIN_MAP_SOURCE_OPENARIALMAP:
        oam_init(map);
        break;
      case CHAMPLAIN_MAP_SOURCE_MAPSFORFREE_RELIEF:
        mff_relief_init(map);
        break;
    }

  map->previous_level = NULL;
  map->current_level = NULL;

  return map;
}

void
map_load_level(Map *map, gint zoom_level)
{
  if (map->previous_level)
    zoom_level_free(map->previous_level);
  map->previous_level = map->current_level;

  guint row_count = map->get_row_count(map, zoom_level);
  guint column_count = map->get_column_count(map, zoom_level);

  map->current_level = zoom_level_new(zoom_level, row_count, column_count, map->tile_size);
}

void
map_load_visible_tiles (Map *map, ChamplainRectangle viewport, gboolean offline)
{
  if (viewport.x < 0)
    viewport.x = 0;
  if (viewport.y < 0)
    viewport.y = 0;

  gint x_count = ceil((float)viewport.width / map->tile_size) + 1;
  gint y_count = ceil((float)viewport.height / map->tile_size) + 1;

  gint x_first = (map->current_level->anchor.x + viewport.x) / map->tile_size;
  gint y_first = (map->current_level->anchor.y + viewport.y) / map->tile_size;

  x_count += x_first;
  y_count += y_first;

  if(x_count > map->current_level->row_count)
    x_count = map->current_level->row_count;
  if(y_count > map->current_level->column_count)
    y_count = map->current_level->column_count;

  //g_print("Tiles: %d, %d to %d, %d\n", x_first, y_first, x_count, y_count);

  int i, j, k;

  // Get rid of old tiles first
  for (k = 0; k < map->current_level->tiles->len; k++)
    {
      Tile *tile = g_ptr_array_index(map->current_level->tiles, k);
      if (tile->x < x_first || tile->x > x_count || tile->y < y_first || tile->y > y_count)
        {
          g_ptr_array_remove (map->current_level->tiles, tile);
          tile_free(tile);
        }
    }

  //Load new tiles if needed
  for (i = x_first; i < x_count; i++)
    {
      for (j = y_first; j < y_count; j++)
        {
          gboolean exist = FALSE;
          for (k = 0; k < map->current_level->tiles->len && !exist; k++)
            {
              Tile *tile = g_ptr_array_index(map->current_level->tiles, k);
              if ( tile->x == i && tile->y == j)
                exist = TRUE;
            }

          if(!exist)
            {
              Tile *tile = tile_load(map, map->current_level->level, i, j, offline);
              g_ptr_array_add (map->current_level->tiles, tile);
            }
        }
    }
}

gboolean
map_zoom_in (Map *map)
{
  gint new_level = map->current_level->level + 1;
  if(new_level <= map->zoom_levels)
    {
      map_load_level(map, new_level);
      return TRUE;
    }
  return FALSE;
}

gboolean
map_zoom_out (Map *map)
{
  gint new_level = map->current_level->level - 1;
  if(new_level >= 0)
    {
      map_load_level(map, new_level);
      return TRUE;
    }
  return FALSE;
}

void
map_free (Map *map)
{
  zoom_level_free(map->current_level);
}

gboolean
map_zoom_to (Map *map, guint zoomLevel)
{
  if(zoomLevel >= 0 &&
     zoomLevel<= map->zoom_levels)
    {
      map_load_level(map, zoomLevel);
      return TRUE;
    }
  return FALSE;
}
