/*
 * Copyright (C) 2008 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "champlain-map.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_LOADING
#include "champlain-debug.h"
#include "champlain-zoom-level.h"

#include <math.h>

Map*
map_new (void)
{
  Map *map = g_new0(Map, 1);

  map->previous_level = NULL;
  map->current_level = NULL;

  return map;
}

void
map_load_level(Map *map,
    ChamplainMapSource *map_source,
    gint zoom_level)
{
  if (map->previous_level)
    g_object_unref (map->previous_level);
  map->previous_level = map->current_level;

  guint row_count = champlain_map_source_get_row_count (map_source, zoom_level);
  guint column_count = champlain_map_source_get_column_count (map_source, zoom_level);

  map->current_level = champlain_zoom_level_new ();
  g_object_set (G_OBJECT (map->current_level),
      "zoom-level", zoom_level,
      "width", row_count,
      "height", column_count,
      NULL);
}

void
map_load_visible_tiles (Map *map,
    ChamplainView *view,
    ChamplainMapSource *source,
    ChamplainRectangle viewport)
{
  gint size;

  size = champlain_map_source_get_tile_size (source);

  if (viewport.x < 0)
    viewport.x = 0;
  if (viewport.y < 0)
    viewport.y = 0;

  gint x_count = ceil((float)viewport.width / size) + 1;
  gint y_count = ceil((float)viewport.height / size) + 1;

  gint x_first = viewport.x / size;
  gint y_first = viewport.y / size;

  x_count += x_first;
  y_count += y_first;

  if(x_count > champlain_zoom_level_get_width (map->current_level))
    x_count = champlain_zoom_level_get_width (map->current_level);
  if(y_count > champlain_zoom_level_get_height (map->current_level))
    y_count = champlain_zoom_level_get_height (map->current_level);

  DEBUG ("Range %d, %d to %d, %d", x_first, y_first, x_count, y_count);

  int i, j;
  guint k;

  // Get rid of old tiles first
  for (k = 0; k < champlain_zoom_level_tile_count (map->current_level); k++)
    {
      ChamplainTile *tile = champlain_zoom_level_get_nth_tile (map->current_level, k);
      gint tile_x = champlain_tile_get_x (tile);
      gint tile_y = champlain_tile_get_y (tile);
      if (tile_x < x_first || tile_x > x_count ||
          tile_y < y_first || tile_y > y_count)
      {
        ClutterActor *group, *actor;
        if (champlain_tile_get_state (tile) == CHAMPLAIN_STATE_DONE)
          {
            actor = champlain_tile_get_actor (tile);
            group = champlain_zoom_level_get_actor (map->current_level);
            clutter_container_remove_actor (CLUTTER_CONTAINER (group), actor);
          }
        champlain_zoom_level_remove_tile (map->current_level, tile);
      }
    }

  //Load new tiles if needed
  for (i = x_first; i < x_count; i++)
    {
      for (j = y_first; j < y_count; j++)
        {
          gboolean exist = FALSE;
          for (k = 0; k < champlain_zoom_level_tile_count (map->current_level) && !exist; k++)
            {
              ChamplainTile *tile = champlain_zoom_level_get_nth_tile (map->current_level, k);
              gint tile_x = champlain_tile_get_x (tile);
              gint tile_y = champlain_tile_get_y (tile);

              if ( tile_x == i && tile_y == j)
                exist = TRUE;
            }

          if(!exist)
            {
              DEBUG ("Loading tile %d, %d, %d", champlain_zoom_level_get_zoom_level (map->current_level), i, j);
              ChamplainTile *tile = champlain_tile_new ();
              g_object_set (G_OBJECT (tile), "x", i, "y", j, NULL);
              champlain_map_source_get_tile (source, view, map->current_level, tile);
              champlain_zoom_level_add_tile (map->current_level, tile);
              g_object_unref (tile);
            }
        }
    }
}

gboolean
map_zoom_in (Map *map,
    ChamplainMapSource *source)
{
  guint new_level = champlain_zoom_level_get_zoom_level (map->current_level) + 1;
  if(new_level <= champlain_map_source_get_max_zoom_level (source))
    {
      map_load_level(map, source, new_level);
      return TRUE;
    }
  return FALSE;
}

gboolean
map_zoom_out (Map *map,
    ChamplainMapSource *source)
{
  gint new_level = champlain_zoom_level_get_zoom_level (map->current_level) - 1;
  if(new_level >= champlain_map_source_get_min_zoom_level (source))
    {
      map_load_level(map, source, new_level);
      return TRUE;
    }
  return FALSE;
}

void
map_free (Map *map)
{
  g_object_unref (map->current_level);
}

gboolean
map_zoom_to (Map *map,
    ChamplainMapSource *source,
    guint zoomLevel)
{
  if (zoomLevel<= champlain_map_source_get_max_zoom_level (source))
    {
      map_load_level(map, source, zoomLevel);
      return TRUE;
    }
  return FALSE;
}
