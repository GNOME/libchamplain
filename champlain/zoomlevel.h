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

#ifndef CHAMPLAIN_MAP_ZOOM_LEVEL_H
#define CHAMPLAIN_MAP_ZOOM_LEVEL_H

#include <glib.h>
#include <clutter/clutter.h>

typedef struct
{
	int level;
  int row_count;
  int column_count;
  int tile_size;
  
  GPtrArray  *tiles;
  ClutterActor* group;
  
} ZoomLevel;

guint zoom_level_get_width(ZoomLevel* level);

guint zoom_level_get_height(ZoomLevel* level);

ZoomLevel* zoom_level_new(gint zoom_level, gint row, gint column, gint tile_size);

#endif
