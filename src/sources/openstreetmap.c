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
 
#include "sources/openstreetmap.h"
#include <map.h>
#include <math.h>
#include <clutter/clutter.h>
#include <libsoup/soup.h>


typedef struct {
  Map* map;
  Tile* tile;
} TwoPtr ;


//http://wiki.openstreetmap.org/index.php/Slippy_map_tilenames#C.2FC.2B.2B

guint osm_row_count(Map* map, guint zoom_level);
guint osm_column_count(Map* map, guint zoom_level);
Tile* osm_get_tile (Map* map, guint zoom_level, guint x, guint y);

gdouble osm_longitude_to_x (Map* map, gdouble longitude, guint zoom_level);
gdouble osm_latitude_to_y (Map* map, gdouble latitude, guint zoom_level);
gdouble osm_x_to_longitude (Map* map, gdouble x, guint zoom_level);
gdouble osm_y_to_latitude (Map* map, gdouble y, guint zoom_level);

void
osm_init(Map* map)
{
  map->name = "OpenStreetMap";
  map->zoom_levels = 17;
  map->tile_size = 256;
  
  map->get_row_count = osm_row_count;
  map->get_column_count = osm_column_count;
  map->get_tile = osm_get_tile;
  
  map->longitude_to_x = osm_longitude_to_x;
  map->latitude_to_y = osm_latitude_to_y;
  map->x_to_longitude = osm_x_to_longitude;
  map->y_to_latitude = osm_y_to_latitude;
}

guint osm_row_count(Map* map, guint zoom_level)
{
  return pow (2, zoom_level);
}

guint 
osm_column_count(Map* map, guint zoom_level)
{
  return pow (2, zoom_level);
}
#define MAX_READ  100000

static void
file_loaded_cb (SoupSession *session,
                 SoupMessage *msg,
                 TwoPtr* ptr)
{
  GError *error = NULL;
  GdkPixbufLoader* pixloader;
  Tile* tile = ptr->tile;
  Map* map = ptr->map;
  
  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code)) 
    {
      g_warning ("Unable to download tile %d, %d", tile->x, tile->y);
      return;
    }

  pixloader = gdk_pixbuf_loader_new();
  if (!gdk_pixbuf_loader_write (pixloader,
                          (const guchar *) msg->response_body->data,
                          msg->response_body->length,
                          NULL))
    {
      if (error)
        {
          g_warning ("Unable to load the pixbuf: %s", error->message);
          g_error_free (error);
        }
 
      g_object_unref (pixloader);
    }
    
  gdk_pixbuf_loader_close (pixloader, NULL);
  if (error)
    {
      g_warning ("Unable to close the pixbuf loader: %s", error->message);
      g_error_free (error);
      g_object_unref (pixloader);
    }
  else
    {
      GdkPixbuf* pixbuf = gdk_pixbuf_loader_get_pixbuf(pixloader);
      
      tile->actor = clutter_texture_new();
      clutter_texture_set_from_rgb_data(tile->actor, 
          gdk_pixbuf_get_pixels (pixbuf),
          gdk_pixbuf_get_has_alpha (pixbuf),
          gdk_pixbuf_get_width(pixbuf),
          gdk_pixbuf_get_height(pixbuf),
          gdk_pixbuf_get_rowstride (pixbuf),
          3, 0, NULL);
           
      
      clutter_actor_set_position (tile->actor, tile->x * map->tile_size, tile->y * map->tile_size);
      clutter_actor_set_size (tile->actor, map->tile_size, map->tile_size);
      clutter_actor_show (tile->actor);
      //g_object_ref(tile->actor); // to prevent actors to be destroyed when they are removed from groups
      
      clutter_container_add (CLUTTER_CONTAINER (map->current_level->group), tile->actor, NULL);
    }
}

Tile* 
osm_get_tile (Map* map, guint zoom_level, guint x, guint y)
{
  static SoupSession * session;
  
  Tile* tile = g_new0(Tile, 1);
  
  tile->x = x;
  tile->y = y;
  
  TwoPtr* ptr = g_new0(TwoPtr, 1);
  ptr->map = map;
  ptr->tile = tile;
  
  SoupMessage *msg;
  if (!session)
    session = soup_session_async_new ();

  msg = soup_message_new (SOUP_METHOD_GET, g_strdup_printf("http://tile.openstreetmap.org/%d/%d/%d.png", zoom_level, x, y));

  soup_session_queue_message (session, msg,
                              file_loaded_cb,
                              ptr);
  
  return tile;
  
}

gdouble 
osm_longitude_to_x (Map* map, gdouble longitude, guint zoom_level)
{
  return ((longitude + 180.0) / 360.0 * pow(2.0, zoom_level)) * map->tile_size; 
}

gdouble 
osm_latitude_to_y (Map* map, gdouble latitude, guint zoom_level)
{
  return ((1.0 - log( tan(latitude * M_PI/180.0) + 1.0 / cos(latitude * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, zoom_level)) * map->tile_size;
}

gdouble 
osm_x_to_longitude (Map* map, gdouble x, guint zoom_level)
{
  x /= map->tile_size;
  return x / map->tile_size * pow(2.0, zoom_level) * 360.0 - 180;
}

gdouble 
osm_y_to_latitude (Map* map, gdouble y, guint zoom_level)
{
  y /= map->tile_size;
  double n = M_PI - 2.0 * M_PI * y / pow(2.0, zoom_level);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

