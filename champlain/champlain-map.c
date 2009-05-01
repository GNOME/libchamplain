/*
 * Copyright (C) 2008-2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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
  g_free (map);
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
