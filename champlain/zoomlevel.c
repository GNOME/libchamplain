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

#include <zoomlevel.h>
#include <tile.h>
#include "map.h"
#include <clutter/clutter.h>

ZoomLevel*
zoom_level_new(gint zoom_level, gint row, gint column, gint tile_size)
{
  ZoomLevel *level = g_new0(ZoomLevel, 1);

  level->level = zoom_level;
  level->row_count = row;
  level->column_count = column;
  level->tile_size = tile_size;

  //FIXME: this hard coded value means that there can't be more than 16x16 tiles displayed at once
  level->tiles = g_ptr_array_sized_new (256);
  level->group = clutter_group_new ();

  g_object_ref(level->group); // so that the group isn't destroyed when removed from the viewport

  level->anchor.x = 0;
  level->anchor.y = 0;

  return level;
}

void
zoom_level_free(ZoomLevel *level)
{
  guint i;
  for (i = 0; i < level->tiles->len; i++)
    {
      Tile *tile = g_ptr_array_index(level->tiles, i);
      tile_free(tile);
    }
}

guint
zoom_level_get_width(ZoomLevel *level)
{
  return (level->column_count) * level->tile_size;
}

guint
zoom_level_get_height(ZoomLevel *level)
{
  return (level->row_count) * level->tile_size;
}
