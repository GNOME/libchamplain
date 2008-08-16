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
 
#include <champlain_map.h>
 
ChamplainMap* 
champlain_map_new (ChamplainMapSource source)
{
	ChamplainMap* map = g_new0(ChamplainMap, 1);
	map->name = "Debug";
	map->zoom_levels = 1;
	return map;
}

void 
champlain_map_load(ChamplainMap* map, gint zoom_level)
{
		map->current_level = champlain_map_zoom_level_new(zoom_level, 15, 15, 100);
		champlain_map_zoom_level_create(map->current_level, zoom_level);
}

