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

#include "champlain-tile.h"

#include "champlain-enum-types.h"
#include "champlain-map.h"
#include "champlain-private.h"

#include <math.h>
#include <errno.h>
#include <gdk/gdk.h>
#include <libsoup/soup.h>
#include <gio/gio.h>

G_DEFINE_TYPE (ChamplainTile, champlain_tile, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_TILE, ChamplainTilePrivate))

enum
{
  PROP_0,
  PROP_X,
  PROP_Y,
  PROP_ZOOM_LEVEL,
  PROP_SIZE,
  PROP_URI,
  PROP_FILENAME,
  PROP_STATE,
  PROP_ACTOR
};

typedef struct _ChamplainTilePrivate ChamplainTilePrivate;

struct _ChamplainTilePrivate {
  gint x;
  gint y;
  gint size;
  gint zoom_level;

  gchar * uri;
  gpointer data;
  ChamplainStateEnum state;
  gchar *filename;
  ClutterActor *actor;
};

static void
champlain_tile_get_property (GObject *object,
                             guint property_id,
                             GValue *value,
                             GParamSpec *pspec)
{
  ChamplainTile *self = CHAMPLAIN_TILE (object);
  switch (property_id)
    {
      case PROP_X:
        g_value_set_int (value, champlain_tile_get_x (self));
        break;
      case PROP_Y:
        g_value_set_int (value, champlain_tile_get_y (self));
        break;
      case PROP_ZOOM_LEVEL:
        g_value_set_int (value, champlain_tile_get_zoom_level (self));
        break;
      case PROP_SIZE:
        g_value_set_uint (value, champlain_tile_get_size (self));
        break;
      case PROP_STATE:
        g_value_set_enum (value, champlain_tile_get_state (self));
        break;
      case PROP_URI:
        g_value_set_string (value, champlain_tile_get_uri (self));
        break;
      case PROP_FILENAME:
        g_value_set_string (value, champlain_tile_get_filename (self));
        break;
      case PROP_ACTOR:
        g_value_set_object (value, champlain_tile_get_actor (self));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_tile_set_property (GObject *object,
                             guint property_id,
                             const GValue *value,
                             GParamSpec *pspec)
{
  ChamplainTile *self = CHAMPLAIN_TILE (object);
  switch (property_id)
    {
      case PROP_X:
        champlain_tile_set_x (self, g_value_get_int (value));
        break;
      case PROP_Y:
        champlain_tile_set_y (self, g_value_get_int (value));
        break;
      case PROP_ZOOM_LEVEL:
        champlain_tile_set_zoom_level (self, g_value_get_int (value));
        break;
      case PROP_SIZE:
        champlain_tile_set_size (self, g_value_get_uint (value));
        break;
      case PROP_STATE:
        champlain_tile_set_state (self, g_value_get_enum (value));
        break;
      case PROP_URI:
        champlain_tile_set_uri (self, g_value_dup_string (value));
        break;
      case PROP_FILENAME:
        champlain_tile_set_filename (self, g_value_dup_string (value));
        break;
      case PROP_ACTOR:
        champlain_tile_set_actor (self, g_value_get_object (value));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_tile_dispose (GObject *object)
{
  ChamplainTilePrivate *priv = GET_PRIVATE (object);
  /* We call destroy here as an actor should not survive its tile */
  if (priv->actor != NULL)
    clutter_actor_destroy (priv->actor);

  G_OBJECT_CLASS (champlain_tile_parent_class)->dispose (object);
}

static void
champlain_tile_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_tile_parent_class)->finalize (object);
}

static void
champlain_tile_class_init (ChamplainTileClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainTilePrivate));

  object_class->get_property = champlain_tile_get_property;
  object_class->set_property = champlain_tile_set_property;
  object_class->dispose = champlain_tile_dispose;
  object_class->finalize = champlain_tile_finalize;


  g_object_class_install_property (object_class,
      PROP_X,
      g_param_spec_int ("x",
        "x",
        "The X position of the tile",
        G_MININT,
        G_MAXINT,
        0,
        G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
      PROP_Y,
      g_param_spec_int ("y",
        "y",
        "The Y position of the tile",
        G_MININT,
        G_MAXINT,
        0,
        G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
      PROP_ZOOM_LEVEL,
      g_param_spec_int ("zoom-level",
        "Zoom Level",
        "The zoom level of the tile",
        G_MININT,
        G_MAXINT,
        0,
        G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
      PROP_SIZE,
      g_param_spec_uint ("size",
        "Size",
        "The size of the tile",
        0,
        G_MAXINT,
        256,
        G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
      PROP_STATE,
      g_param_spec_enum ("state",
        "State",
        "The state of the tile",
        CHAMPLAIN_TYPE_STATE_ENUM,
        CHAMPLAIN_STATE_NONE,
        G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
      PROP_URI,
      g_param_spec_string ("uri",
        "URI",
        "The URI of the tile",
        "",
        G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
      PROP_FILENAME,
      g_param_spec_string ("filename",
        "Filename",
        "The filename of the tile",
        "",
        G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
      PROP_ACTOR,
      g_param_spec_object ("actor",
        "Actor",
        "The tile's actor",
        CLUTTER_TYPE_ACTOR,
        G_PARAM_READWRITE));
}

static void
champlain_tile_init (ChamplainTile *self)
{
  champlain_tile_set_state (self, CHAMPLAIN_STATE_INIT);
  champlain_tile_set_x (self, 0);
  champlain_tile_set_y (self, 0);
  champlain_tile_set_zoom_level (self, 0);
  champlain_tile_set_size (self, 0);
  champlain_tile_set_uri (self, "");
  champlain_tile_set_filename (self, "");
}

ChamplainTile*
champlain_tile_new (void)
{
  return g_object_new (CHAMPLAIN_TYPE_TILE, NULL);
}

gint
champlain_tile_get_x (ChamplainTile *self)
{
  g_return_val_if_fail(CHAMPLAIN_TILE(self), 0);

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  return priv->x;
}

gint
champlain_tile_get_y (ChamplainTile *self)
{
  g_return_val_if_fail(CHAMPLAIN_TILE(self), 0);

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  return priv->y;
}

gint
champlain_tile_get_zoom_level (ChamplainTile *self)
{
  g_return_val_if_fail(CHAMPLAIN_TILE(self), 0);

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  return priv->zoom_level;
}

guint
champlain_tile_get_size (ChamplainTile *self)
{
  g_return_val_if_fail(CHAMPLAIN_TILE(self), 0);

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  return priv->size;
}

ChamplainStateEnum
champlain_tile_get_state (ChamplainTile *self)
{
  g_return_val_if_fail(CHAMPLAIN_TILE(self), CHAMPLAIN_STATE_NONE);

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  return priv->state;
}

gchar *
champlain_tile_get_uri (ChamplainTile *self)
{
  g_return_val_if_fail(CHAMPLAIN_TILE(self), NULL);

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  return priv->uri;
}

gchar *
champlain_tile_get_filename (ChamplainTile *self)
{
  g_return_val_if_fail(CHAMPLAIN_TILE(self), NULL);

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  return priv->filename;
}

ClutterActor *
champlain_tile_get_actor (ChamplainTile *self)
{
  g_return_val_if_fail(CHAMPLAIN_TILE(self), NULL);

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  return priv->actor;
}

void
champlain_tile_set_x (ChamplainTile *self, gint x)
{
  g_return_if_fail(CHAMPLAIN_TILE(self));

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  priv->x = x;
  g_object_notify (G_OBJECT (self), "x");
}

void
champlain_tile_set_y (ChamplainTile *self, gint y)
{
  g_return_if_fail(CHAMPLAIN_TILE(self));

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  priv->y = y;
  g_object_notify (G_OBJECT (self), "y");
}

void
champlain_tile_set_zoom_level (ChamplainTile *self, gint zoom_level)
{
  g_return_if_fail(CHAMPLAIN_TILE(self));

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  priv->zoom_level = zoom_level;
  g_object_notify (G_OBJECT (self), "zoom-level");
}

void
champlain_tile_set_size (ChamplainTile *self, guint size)
{
  g_return_if_fail(CHAMPLAIN_TILE(self));

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  priv->size = size;
  g_object_notify (G_OBJECT (self), "size");
}

void
champlain_tile_set_state (ChamplainTile *self, ChamplainStateEnum state)
{
  g_return_if_fail(CHAMPLAIN_TILE(self));

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  priv->state = state;
  g_object_notify (G_OBJECT (self), "state");
}

ChamplainTile*
champlain_tile_new_full (gint x,
                         gint y,
                         guint size,
                         gint zoom_level)
{
  return g_object_new (CHAMPLAIN_TYPE_TILE, "x", x, "y", y, "zoom-level",
      zoom_level, "size", size, NULL);
}

void
champlain_tile_set_uri (ChamplainTile *self, gchar *uri)
{
  g_return_if_fail(CHAMPLAIN_TILE(self));
  g_return_if_fail(uri != NULL);

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  priv->uri = uri;
  g_object_notify (G_OBJECT (self), "uri");
}

void
champlain_tile_set_filename (ChamplainTile *self, gchar *filename)
{
  g_return_if_fail(CHAMPLAIN_TILE(self));
  g_return_if_fail(filename != NULL);

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  priv->filename = filename;
  g_object_notify (G_OBJECT (self), "filename");
}

void
champlain_tile_set_actor (ChamplainTile *self, ClutterActor *actor)
{
  g_return_if_fail(CHAMPLAIN_TILE(self));
  g_return_if_fail(actor != NULL);

  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  priv->actor = g_object_ref (actor);
  g_object_notify (G_OBJECT (self), "actor");
}

/* The code below this point is to be refactored */

typedef struct {
  Map* map;
  ChamplainTile* tile;
} TwoPtr;

#define CACHE_DIR "champlain"
static SoupSession * soup_session;

void
tile_set_position(Map* map, ChamplainTile* tile)
{
  ClutterActor *actor;
  gint x;
  gint y;
  guint size;
  g_object_get (G_OBJECT (tile), "actor", &actor,
      "x", &x, "y", &y,
      "size", &size, NULL);
  ChamplainPoint anchor; //XXX
  anchor.x = 0;
  anchor.y = 0;
  clutter_actor_set_position (actor,
    (x * size) - anchor.x,
    (y * size) - anchor.y);
  clutter_actor_set_size (actor, size, size); //XXX Move elsewhere
  clutter_actor_show (actor);
}

void
tile_setup_animation (ChamplainTile* tile)
{
  ClutterActor *actor = champlain_tile_get_actor (tile);
  ClutterEffectTemplate *etemplate = clutter_effect_template_new_for_duration (250, CLUTTER_ALPHA_SINE_INC);
  clutter_actor_set_opacity(actor, 0);
  clutter_effect_fade (etemplate, actor, 255, NULL, NULL);
}

static void
create_error_tile(Map* map, ChamplainTile* tile)
{
  ClutterActor *actor;
  actor = clutter_texture_new_from_file (DATADIR "/champlain/error.svg", NULL);
  if (!actor)
    return;

  champlain_tile_set_actor (tile, actor);
  tile_set_position (map, tile);

  clutter_container_add (CLUTTER_CONTAINER (champlain_zoom_level_get_actor (map->current_level)), actor, NULL);
  tile_setup_animation (tile);

  champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);
}

static void
file_loaded_cb (SoupSession *session,
                SoupMessage *msg,
                TwoPtr* ptr)
{
  GdkPixbufLoader* loader;
  GError *error = NULL;
  gchar* path, *filename, *map_filename;

  ChamplainTile* tile = ptr->tile;
  Map* map = ptr->map;

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      g_warning ("Unable to download tile %d, %d: %s",
          champlain_tile_get_x (tile),
          champlain_tile_get_y (tile),
          soup_status_get_phrase (msg->status_code));
      create_error_tile (map, tile);
      g_object_unref (tile);
      return;
    }

  loader = gdk_pixbuf_loader_new ();
  if (!gdk_pixbuf_loader_write (loader,
                          (const guchar *) msg->response_body->data,
                          msg->response_body->length,
                          &error))
    {
      if (error)
        {
          g_warning ("Unable to load the pixbuf: %s", error->message);
          g_error_free (error);
          create_error_tile (map, tile);
          g_object_unref (tile);
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
      g_object_unref (tile);
      return;
    }

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

  GdkPixbuf* pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);

  ClutterActor *actor = clutter_texture_new();
  clutter_texture_set_from_rgb_data (CLUTTER_TEXTURE (actor),
      gdk_pixbuf_get_pixels (pixbuf),
      gdk_pixbuf_get_has_alpha (pixbuf),
      gdk_pixbuf_get_width (pixbuf),
      gdk_pixbuf_get_height (pixbuf),
      gdk_pixbuf_get_rowstride (pixbuf),
      3, 0, NULL);
  champlain_tile_set_actor (tile, actor);

  tile_set_position (map, tile);

  clutter_container_add (CLUTTER_CONTAINER (champlain_zoom_level_get_actor (map->current_level)), actor, NULL);
  tile_setup_animation (tile);

  champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);

  g_object_unref (tile);
  g_object_unref (loader); /* also frees the pixbuf */
  g_free (filename);
  g_free (map_filename);
  g_free (ptr);
}

ChamplainTile*
tile_load (Map* map, gint zoom_level, gint x, gint y, gboolean offline)
{
  gchar* filename, *map_filename;
  ChamplainTile* tile = champlain_tile_new ();
  g_object_set (G_OBJECT (tile), "x", x, "y" , y, "zoom-level", zoom_level,
      "size", map->tile_size, NULL);

  TwoPtr* ptr = g_new0 (TwoPtr, 1);
  ptr->map = map;
  ptr->tile = tile;

  // Try the cached version first
  map_filename = map->get_tile_filename (map, tile);
  filename = g_build_filename (g_get_user_cache_dir (),
                                CACHE_DIR,
                                map->name,
                                map_filename,
                                NULL);

  if (g_file_test (filename, G_FILE_TEST_EXISTS))
    {
      ClutterActor *actor = clutter_texture_new_from_file (filename, NULL);
      champlain_tile_set_actor (tile, actor);
      tile_set_position (map, tile);

      clutter_container_add (CLUTTER_CONTAINER (champlain_zoom_level_get_actor (map->current_level)), actor, NULL);
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
      champlain_tile_set_state (tile, CHAMPLAIN_STATE_LOADING);
      g_object_ref (tile); //Unref when loading done
    }
  // If a tile is neither in cache or can be fetched, do nothing, it'll show up as empty

  g_free (filename);
  g_free (map_filename);
  return tile;
}
