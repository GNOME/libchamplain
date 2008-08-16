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

#ifndef CHAMPLAIN_MAP_H
#define CHAMPLAIN_MAP_H

#include "champlain_defines.h"
#include "champlain_map_zoom_level.h"
#include <glib.h>
#include <clutter/clutter.h>

typedef enum
{
  CHAMPLAIN_MAP_SOURCE_DEBUG,
  CHAMPLAIN_MAP_SOURCE_OPENSTREETMAP,
  CHAMPLAIN_MAP_SOURCE_GOOGLE
} ChamplainMapSource;

typedef struct
{
  int zoom_levels;
  const gchar* name;
  ChamplainMapZoomLevel* current_level;
  
} ChamplainMap;


CHAMPLAIN_API ChamplainMap* champlain_map_new (ChamplainMapSource source);


#endif
