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

#include "tile.h"
#include "map.h"
#include "champlainprivate.h"

#include <math.h>
#include <errno.h>
#include <gdk/gdk.h>
#include <libsoup/soup.h>
#include <gio/gio.h>

typedef struct {
  Map* map;
  Tile* tile;
} TwoPtr;

#define CACHE_DIR "champlain"
static SoupSession * soup_session;

void
tile_set_position(Map* map, Tile* tile)
{
  clutter_actor_set_position (tile->actor,
    (tile->x * tile->size) - map->current_level->anchor.x,
    (tile->y * tile->size) - map->current_level->anchor.y);
  clutter_actor_set_size (tile->actor, tile->size, tile->size);
  clutter_actor_show (tile->actor);
}

void
tile_setup_animation(Tile* tile)
{
  ClutterEffectTemplate *etemplate = clutter_effect_template_new_for_duration (250, CLUTTER_ALPHA_SINE_INC);
  clutter_actor_set_opacity(tile->actor, 0);
  clutter_effect_fade (etemplate, tile->actor, 255, NULL, NULL);
}

static void
create_error_tile(Map* map, Tile* tile)
{

  ClutterColor color = { 0x99, 0x22, 0x22, 0xFF };
  tile->actor = clutter_rectangle_new_with_color (&color);

  clutter_container_add (CLUTTER_CONTAINER (map->current_level->group), tile->actor, NULL);
  tile_setup_animation(tile);
}

static void
file_loaded_cb (SoupSession *session,
                SoupMessage *msg,
                TwoPtr* ptr)
{
  GdkPixbufLoader* loader;
  GError *error = NULL;
  gchar* path, *filename, *map_filename;

  Tile* tile = ptr->tile;
  Map* map = ptr->map;

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      g_warning ("Unable to download tile %d, %d: %s", tile->x, tile->y, soup_status_get_phrase(msg->status_code));
      create_error_tile(map, tile);
      return;
    }

  loader = gdk_pixbuf_loader_new();
  if (!gdk_pixbuf_loader_write (loader,
                          (const guchar *) msg->response_body->data,
                          msg->response_body->length,
                          &error))
    {
      if (error)
        {
          g_warning ("Unable to load the pixbuf: %s", error->message);
          g_error_free (error);
          create_error_tile(map, tile);
          return;
        }

      g_object_unref (loader);
    }

  gdk_pixbuf_loader_close (loader, &error);
  if (error)
    {
      g_warning ("Unable to close the pixbuf loader: %s", error->message);
      g_error_free (error);
      g_object_unref (loader);
      create_error_tile(map, tile);
      return;
    }
  else
    {
      path = g_build_filename (g_get_user_cache_dir (),
                                    CACHE_DIR,
                                    map->name,
                                    NULL);

      if (g_mkdir_with_parents (path, 0700) == -1)
        {
          if (errno != EEXIST)
            {
              g_warning ("Unable to create the image cache: %s",
                         g_strerror (errno));
              g_object_unref (loader);
            }
        }

      map_filename = map->get_tile_filename(map, tile);
      filename = g_build_filename (g_get_user_cache_dir (),
                                    CACHE_DIR,
                                    map->name,
                                    map_filename,
                                    NULL);

      g_file_set_contents (filename,
                           msg->response_body->data,
                           msg->response_body->length,
                           NULL);
      // If the tile has been marked to be deleted, don't go any further
      if(tile->to_destroy)
        {
          g_object_unref (loader);
          g_free (filename);
          g_free (map_filename);
          g_free (tile);
          return;
        }

      GdkPixbuf* pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);

      tile->actor = clutter_texture_new();
      clutter_texture_set_from_rgb_data(CLUTTER_TEXTURE(tile->actor),
          gdk_pixbuf_get_pixels (pixbuf),
          gdk_pixbuf_get_has_alpha (pixbuf),
          gdk_pixbuf_get_width(pixbuf),
          gdk_pixbuf_get_height(pixbuf),
          gdk_pixbuf_get_rowstride (pixbuf),
          3, 0, NULL);

      tile_set_position(map, tile);

      clutter_container_add (CLUTTER_CONTAINER (map->current_level->group), tile->actor, NULL);
      tile_setup_animation(tile);

      tile->loading = FALSE;

      g_object_unref (loader);
      g_free (filename);
      g_free (map_filename);
    }
  g_free (ptr);
}

Tile*
tile_load (Map* map, guint zoom_level, guint x, guint y, gboolean offline)
{
  gchar* filename, *map_filename;
  Tile* tile = g_new0(Tile, 1);

  tile->x = x;
  tile->y = y;
  tile->level = zoom_level;
  tile->size = map->tile_size;

  TwoPtr* ptr = g_new0(TwoPtr, 1);
  ptr->map = map;
  ptr->tile = tile;

  // Try the cached version first
  map_filename = map->get_tile_filename(map, tile);
  filename = g_build_filename (g_get_user_cache_dir (),
                                CACHE_DIR,
                                map->name,
                                map_filename,
                                NULL);

  if (g_file_test (filename, G_FILE_TEST_EXISTS))
    {
      tile->actor = clutter_texture_new_from_file(filename, NULL);
      tile_set_position(map, tile);

      clutter_container_add (CLUTTER_CONTAINER (map->current_level->group), tile->actor, NULL);
      // Do not animate since it is local and fast
    }
  else if (!offline)
    {
      SoupMessage *msg;
      if (!soup_session)
        soup_session = soup_session_async_new ();

      msg = soup_message_new (SOUP_METHOD_GET, map->get_tile_uri(map, tile));

      soup_session_queue_message (soup_session, msg,
                                  file_loaded_cb,
                                  ptr);
      tile->loading = TRUE;
    }
  // If a tile is neither in cache or can be fetched, do nothing, it'll show up as empty

  g_free (filename);
  g_free (map_filename);
  return tile;
}

void
tile_free(Tile* tile)
{
  if(tile->actor)
    clutter_actor_destroy(tile->actor);
  if(tile->loading)
    tile->to_destroy = TRUE;
  else
    g_free(tile);
}
