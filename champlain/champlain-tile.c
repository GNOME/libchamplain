/*
 * Copyright (C) 2008-2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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
/**
 * SECTION:champlain-tile
 * @short_description: An object that represent map tiles
 *
 * This object represents map tiles. Tiles are loaded by #ChamplainMapSource.
 */

#include "champlain-tile.h"

#include "champlain-enum-types.h"
#include "champlain-private.h"

#include <math.h>
#include <errno.h>
#include <gdk/gdk.h>
#include <libsoup/soup.h>
#include <gio/gio.h>
#include <clutter/clutter.h>

G_DEFINE_TYPE (ChamplainTile, champlain_tile, CLUTTER_TYPE_GROUP)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_TILE, ChamplainTilePrivate))

enum
{
  PROP_0,
  PROP_X,
  PROP_Y,
  PROP_ZOOM_LEVEL,
  PROP_SIZE,
  PROP_STATE,
  PROP_CONTENT,
  PROP_ETAG,
  PROP_FADE_IN
};

struct _ChamplainTilePrivate {
  gint x; /* The x position on the map (in pixels) */
  gint y; /* The y position on the map (in pixels) */
  gint size; /* The tile's width and height (only support square tiles */
  gint zoom_level; /* The tile's zoom level */

  ChamplainState state; /* The tile state: loading, validation, done */
  ClutterActor *content_actor; /* The actual tile actor */
  gboolean fade_in;

  GTimeVal *modified_time; /* The last modified time of the cache */
  gchar* etag; /* The HTTP ETag sent by the server */
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
      case PROP_CONTENT:
        g_value_set_object (value, champlain_tile_get_content (self));
        break;
      case PROP_ETAG:
        g_value_set_string (value, champlain_tile_get_etag (self));
        break;
      case PROP_FADE_IN:
        g_value_set_boolean (value, champlain_tile_get_fade_in (self));
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
      case PROP_CONTENT:
        champlain_tile_set_content (self, g_value_get_object (value));
        break;
      case PROP_ETAG:
        champlain_tile_set_etag (self, g_value_get_string (value));
        break;
      case PROP_FADE_IN:
        champlain_tile_set_fade_in (self, g_value_get_boolean (value));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_tile_dispose (GObject *object)
{
  G_OBJECT_CLASS (champlain_tile_parent_class)->dispose (object);
}

static void
champlain_tile_finalize (GObject *object)
{
  ChamplainTilePrivate *priv = CHAMPLAIN_TILE (object)->priv;

  g_free (priv->modified_time);
  g_free (priv->etag);

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

  /**
  * ChamplainTile:x:
  *
  * The x position of the tile
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_X,
      g_param_spec_int ("x",
          "x",
          "The X position of the tile",
          G_MININT,
          G_MAXINT,
          0,
          G_PARAM_READWRITE));

  /**
  * ChamplainTile:y:
  *
  * The y position of the tile
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_Y,
      g_param_spec_int ("y",
          "y",
          "The Y position of the tile",
          G_MININT,
          G_MAXINT,
          0,
          G_PARAM_READWRITE));

  /**
  * ChamplainTile:zoom-level:
  *
  * The zoom level of the tile
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_ZOOM_LEVEL,
      g_param_spec_int ("zoom-level",
          "Zoom Level",
          "The zoom level of the tile",
          G_MININT,
          G_MAXINT,
          0,
          G_PARAM_READWRITE));

  /**
  * ChamplainTile:size:
  *
  * The size of the tile in pixels
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_SIZE,
      g_param_spec_uint ("size",
          "Size",
          "The size of the tile",
          0,
          G_MAXINT,
          256,
          G_PARAM_READWRITE));

  /**
  * ChamplainTile:state:
  *
  * The state of the tile
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_STATE,
      g_param_spec_enum ("state",
          "State",
          "The state of the tile",
          CHAMPLAIN_TYPE_STATE,
          CHAMPLAIN_STATE_NONE,
          G_PARAM_READWRITE));

  /**
  * ChamplainTile:content:
  *
  * The #ClutterActor with the specific image content.  When changing this
  * property, the new actor will be faded in.
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_CONTENT,
      g_param_spec_object ("content",
          "Content",
          "The tile's content",
          CLUTTER_TYPE_ACTOR,
          G_PARAM_READWRITE));

  /**
  * ChamplainTile:etag:
  *
  * The tile's ETag. This information is sent by some web servers as a mean
  * to identify if a tile has changed.  This information is saved in the cache
  * and sent in GET queries.
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_ETAG,
      g_param_spec_string ("etag",
          "Entity Tag",
          "The entity tag of the tile",
          NULL,
          G_PARAM_READWRITE));

  /**
  * ChamplainTile:fade-in:
  *
  * Specifies whether the tile should fade in when loading
  *
  * Since: 0.6
  */
  g_object_class_install_property (object_class,
      PROP_FADE_IN,
      g_param_spec_boolean ("fade-in",
          "Fade In",
          "Tile should fade in",
          FALSE,
          G_PARAM_READWRITE));
}

static void
champlain_tile_init (ChamplainTile *self)
{
  ChamplainTilePrivate *priv = GET_PRIVATE (self);

  self->priv = priv;

  priv->state = CHAMPLAIN_STATE_NONE;
  priv->x = 0;
  priv->y = 0;
  priv->zoom_level = 0;
  priv->size = 0;
  priv->modified_time = NULL;
  priv->etag = NULL;
  priv->fade_in = FALSE;

  priv->content_actor = NULL;
}

/**
 * champlain_tile_new:
 *
 * Returns: a new #ChamplainTile
 *
 * Since: 0.4
 */
ChamplainTile*
champlain_tile_new (void)
{
  return g_object_new (CHAMPLAIN_TYPE_TILE, NULL);
}

/**
 * champlain_tile_get_x:
 * @self: the #ChamplainTile
 *
 * Returns: the tile's x position
 *
 * Since: 0.4
 */
gint
champlain_tile_get_x (ChamplainTile *self)
{
  g_return_val_if_fail (CHAMPLAIN_TILE (self), 0);

  return self->priv->x;
}

/**
 * champlain_tile_get_y:
 * @self: the #ChamplainTile
 *
 * Returns: the tile's y position
 *
 * Since: 0.4
 */
gint
champlain_tile_get_y (ChamplainTile *self)
{
  g_return_val_if_fail (CHAMPLAIN_TILE (self), 0);

  return self->priv->y;
}

/**
 * champlain_tile_get_zoom_level:
 * @self: the #ChamplainTile
 *
 * Returns: the tile's zoom level
 *
 * Since: 0.4
 */
gint
champlain_tile_get_zoom_level (ChamplainTile *self)
{
  g_return_val_if_fail (CHAMPLAIN_TILE (self), 0);

  return self->priv->zoom_level;
}

/**
 * champlain_tile_get_size:
 * @self: the #ChamplainTile
 *
 * Returns: the tile's size in pixels
 *
 * Since: 0.4
 */
guint
champlain_tile_get_size (ChamplainTile *self)
{
  g_return_val_if_fail (CHAMPLAIN_TILE (self), 0);

  return self->priv->size;
}

/**
 * champlain_tile_get_state:
 * @self: the #ChamplainTile
 *
 * Returns: the tile's #ChamplainState
 *
 * Since: 0.4
 */
ChamplainState
champlain_tile_get_state (ChamplainTile *self)
{
  g_return_val_if_fail (CHAMPLAIN_TILE (self), CHAMPLAIN_STATE_NONE);

  return self->priv->state;
}

/**
 * champlain_tile_set_x:
 * @self: the #ChamplainTile
 * @x: the position
 *
 * Sets the tile's x position
 *
 * Since: 0.4
 */
void
champlain_tile_set_x (ChamplainTile *self,
    gint x)
{
  g_return_if_fail (CHAMPLAIN_TILE (self));

  self->priv->x = x;

  g_object_notify (G_OBJECT (self), "x");
}

/**
 * champlain_tile_set_y:
 * @self: the #ChamplainTile
 * @y: the position
 *
 * Sets the tile's y position
 *
 * Since: 0.4
 */
void
champlain_tile_set_y (ChamplainTile *self,
    gint y)
{
  g_return_if_fail (CHAMPLAIN_TILE (self));

  self->priv->y = y;

  g_object_notify (G_OBJECT (self), "y");
}

/**
 * champlain_tile_set_zoom_level:
 * @self: the #ChamplainTile
 * @zoom_level: the zoom level
 *
 * Sets the tile's zoom level
 *
 * Since: 0.4
 */
void
champlain_tile_set_zoom_level (ChamplainTile *self,
    gint zoom_level)
{
  g_return_if_fail (CHAMPLAIN_TILE (self));

  self->priv->zoom_level = zoom_level;

  g_object_notify (G_OBJECT (self), "zoom-level");
}

/**
 * champlain_tile_set_size:
 * @self: the #ChamplainTile
 * @size: the size in pixels
 *
 * Sets the tile's size
 *
 * Since: 0.4
 */
void
champlain_tile_set_size (ChamplainTile *self,
    guint size)
{
  g_return_if_fail (CHAMPLAIN_TILE (self));

  self->priv->size = size;

  g_object_notify (G_OBJECT (self), "size");
}

static void
fade_in_completed (ClutterAnimation *animation, ChamplainTile *self)
{
  if (clutter_group_get_n_children (CLUTTER_GROUP (self)) > 1)
    clutter_actor_destroy (clutter_group_get_nth_child (CLUTTER_GROUP (self), 0));
}

/**
 * champlain_tile_set_state:
 * @self: the #ChamplainTile
 * @state: a #ChamplainState
 *
 * Sets the tile's #ChamplainState
 *
 * Since: 0.4
 */
void
champlain_tile_set_state (ChamplainTile *self,
    ChamplainState state)
{
  g_return_if_fail (CHAMPLAIN_TILE (self));

  ChamplainTilePrivate *priv = self->priv;

  if (state == priv->state)
    return;

  if (state == CHAMPLAIN_STATE_DONE && priv->content_actor &&
      clutter_actor_get_parent (priv->content_actor) != CLUTTER_ACTOR (self))
    {
      clutter_container_add_actor (CLUTTER_CONTAINER (self), priv->content_actor);

      ClutterAnimation *animation;

      clutter_actor_set_opacity (priv->content_actor, 0);

      if (priv->fade_in)
        {
          animation = clutter_actor_animate (priv->content_actor,
              CLUTTER_EASE_IN_CUBIC,
              500,
              "opacity", 255,
              NULL);
        }
      else
        {
          animation = clutter_actor_animate (priv->content_actor,
              CLUTTER_LINEAR,
              150,
              "opacity", 255,
              NULL);
        }

      g_signal_connect (animation, "completed", G_CALLBACK (fade_in_completed), self);
    }

  priv->state = state;
  g_object_notify (G_OBJECT (self), "state");
}

/**
 * champlain_tile_new_full:
 * @x: the x position
 * @y: the y position
 * @size: the size in pixels
 * @zoom_level: the zoom level
 *
 * Returns: a #ChamplainTile
 *
 * Since: 0.4
 */
ChamplainTile*
champlain_tile_new_full (gint x,
    gint y,
    guint size,
    gint zoom_level)
{
  return g_object_new (CHAMPLAIN_TYPE_TILE, "x", x, "y", y, "zoom-level",
      zoom_level, "size", size, NULL);
}

/**
 * champlain_tile_get_modified_time:
 * @self: the #ChamplainTile
 *
 * Returns: the tile's last modified time
 *
 * Since: 0.4
 */
G_CONST_RETURN GTimeVal *
champlain_tile_get_modified_time (ChamplainTile *self)
{
  g_return_val_if_fail (CHAMPLAIN_TILE (self), NULL);

  return self->priv->modified_time;
}

/**
 * champlain_tile_set_modified_time:
 * @self: the #ChamplainTile
 * @time: a #GTimeVal, the value will be copied
 *
 * Sets the tile's modified time
 *
 * Since: 0.4
 */
void
champlain_tile_set_modified_time (ChamplainTile *self,
    const GTimeVal *time_)
{
  g_return_if_fail (CHAMPLAIN_TILE (self));
  g_return_if_fail (time != NULL);

  ChamplainTilePrivate *priv = self->priv;

  g_free (priv->modified_time);
  priv->modified_time = g_memdup(time_, sizeof (GTimeVal));
}

/**
 * champlain_tile_get_modified_time_string:
 * @self: the #ChamplainTile
 *
 * Returns: the tile's modified time as a string (formated as per RFC 1123)
 *
 * Since: 0.4
 */
gchar *
champlain_tile_get_modified_time_string (ChamplainTile *self)
{
  g_return_val_if_fail (CHAMPLAIN_TILE (self), NULL);

  ChamplainTilePrivate *priv = self->priv;

  if (priv->modified_time == NULL)
    return NULL;

  struct tm *other_time = gmtime (&priv->modified_time->tv_sec);
  char value [100];

  strftime (value, 100, "%a, %d %b %Y %T %Z", other_time);

  return g_strdup (value);
}

/**
 * champlain_tile_get_etag:
 * @self: the #ChamplainTile
 *
 * Returns: the tile's ETag
 *
 * Since: 0.4
 */
G_CONST_RETURN gchar *
champlain_tile_get_etag (ChamplainTile *self)
{
  g_return_val_if_fail (CHAMPLAIN_TILE (self), "");

  return self->priv->etag;
}

/**
 * champlain_tile_set_etag:
 * @self: the #ChamplainTile
 * @etag: the tile's ETag as sent by the server
 *
 * Sets the tile's ETag
 *
 * Since: 0.4
 */
void
champlain_tile_set_etag (ChamplainTile *self,
    const gchar *etag)
{
  g_return_if_fail (CHAMPLAIN_TILE (self));

  ChamplainTilePrivate *priv = self->priv;

  g_free (priv->etag);
  priv->etag = g_strdup (etag);
  g_object_notify (G_OBJECT (self), "etag");
}

/**
 * champlain_tile_set_content:
 * @self: the #ChamplainTile
 * @actor: the new content
 *
 * Sets the tile's content
 *
 * Since: 0.4
 */
void
champlain_tile_set_content (ChamplainTile *self,
    ClutterActor *actor)
{
  g_return_if_fail (CHAMPLAIN_TILE (self));
  g_return_if_fail (CLUTTER_ACTOR (actor));

  ChamplainTilePrivate *priv = self->priv;

  if (priv->content_actor &&
      clutter_actor_get_parent (priv->content_actor) != CLUTTER_ACTOR (self))
    clutter_actor_destroy (priv->content_actor);

  priv->content_actor = actor;

  g_object_notify (G_OBJECT (self), "content");
}

/**
 * champlain_tile_get_content:
 * @self: the #ChamplainTile
 *
 * Returns: the tile's content, this actor will change each time the tile's content changes.
 * You should not unref this content, it is owned by the tile.
 *
 * Since: 0.4
 */
ClutterActor *
champlain_tile_get_content (ChamplainTile *self)
{
  g_return_val_if_fail (CHAMPLAIN_TILE (self), NULL);

  return self->priv->content_actor;
}

/**
 * champlain_tile_get_fade_in:
 * @self: the #ChamplainTile
 *
 * Returns: the return value determines whether the tile should fade in when loading.
 *
 * Since: 0.6
 */
gboolean
champlain_tile_get_fade_in (ChamplainTile *self)
{
  g_return_val_if_fail (CHAMPLAIN_TILE (self), FALSE);

  return self->priv->fade_in;
}

/**
 * champlain_tile_set_fade_in:
 * @self: the #ChamplainTile
 * @fade_in: determines whether the tile should fade in when loading
 *
 * Sets the flag determining whether the tile should fade in when loading
 *
 * Since: 0.6
 */
void
champlain_tile_set_fade_in (ChamplainTile *self,
    gboolean fade_in)
{
  g_return_if_fail (CHAMPLAIN_TILE (self));

  self->priv->fade_in = fade_in;

  g_object_notify (G_OBJECT (self), "fade-in");
}
