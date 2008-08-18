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
 

#ifndef CHAMPLAIN_PRIVATE_H
#define CHAMPLAIN_PRIVATE_H

#include <clutter/clutter.h>
#include <clutter/clutter.h>

void champlain_map_create_tiles(gint zoom_level);

//gboolean tile_is_visible(ClutterUnit viewport_w, ClutterUnit viewport_h, ChamplainPoint position, ChamplainMapTile* tile);

#endif
