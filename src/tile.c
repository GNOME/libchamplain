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


#include "tile.h"


/*
gboolean
tile_is_visible(ClutterUnit viewport_w, ClutterUnit viewport_h, ChamplainPoint position, MapTile* tile)
{
  ClutterUnit size = CLUTTER_UNITS_FROM_INT(tile->size);

  if( ((tile->x + 1)* size + position.x < 0 || tile->x* size + position.x > viewport_w) ||
      ((tile->y + 1)* size + position.y < 0 || tile->y* size + position.y > viewport_h))
    {
      g_print ("Tile I: %d, %d\t p: %d, %d \n",
         tile->x, tile->y,
         CLUTTER_UNITS_TO_INT (position.x),
         CLUTTER_UNITS_TO_INT (position.y));
      return FALSE;
    }
  //g_print ("Tile V: %d, %d\t p: %d, %d \n",
         //tile->x, tile->y,
         //CLUTTER_UNITS_TO_INT (position.x),
         //CLUTTER_UNITS_TO_INT (position.y));
  return TRUE;

}*/
