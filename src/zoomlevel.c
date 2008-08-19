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

#include <zoomlevel.h>
#include <tile.h>
#include "map.h"
#include <clutter/clutter.h>

ZoomLevel* 
zoom_level_new(gint zoom_level, gint row, gint column, gint tile_size)
{
  ZoomLevel* level = g_new0(ZoomLevel, 1);
  
  level->level = zoom_level;
  level->row_count = row;
  level->column_count = column;
  level->tile_size = tile_size;
  level->tiles = g_ptr_array_sized_new (row * column);
  level->group = clutter_group_new ();
  
  return level;
}

guint
zoom_level_get_width(ZoomLevel* level)
{
  return (level->column_count) * level->tile_size;
}

guint
zoom_level_get_height(ZoomLevel* level)
{
  return (level->row_count) * level->tile_size;
}
