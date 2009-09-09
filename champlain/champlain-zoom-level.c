/*
 * Copyright (C) 2008, 2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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

#include "champlain-zoom-level.h"

#include "champlain-map.h"
#include "champlain-tile.h"

#include <clutter/clutter.h>

G_DEFINE_TYPE (ChamplainZoomLevel, champlain_zoom_level, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_ZOOM_LEVEL, ChamplainZoomLevelPrivate))

enum
{
  /* normal signals */
  SIGNAL_TILE_ADDED,
  SIGNAL_TILE_REMOVED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

enum
{
  PROP_0,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_ZOOM_LEVEL,
  PROP_ACTOR
};

struct _ChamplainZoomLevelPrivate {
  guint width; /* The absolute width of the zoom level in tiles */
  guint height; /* The absolute height of the zoom level in tiles */
  gint zoom_level;
  GPtrArray *tiles; /* Contains the tiles of this level */
  ClutterActor *actor;  /* The actor on which tile actors are added */
};

static void
champlain_zoom_level_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainZoomLevel *self = CHAMPLAIN_ZOOM_LEVEL (object);
  switch (property_id)
    {
      case PROP_WIDTH:
        g_value_set_uint (value, champlain_zoom_level_get_width (self));
        break;
      case PROP_HEIGHT:
        g_value_set_uint (value, champlain_zoom_level_get_height (self));
        break;
      case PROP_ZOOM_LEVEL:
        g_value_set_int (value, champlain_zoom_level_get_zoom_level (self));
        break;
      case PROP_ACTOR:
        g_value_set_object (value, champlain_zoom_level_get_actor (self));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_zoom_level_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainZoomLevel *self = CHAMPLAIN_ZOOM_LEVEL (object);
  switch (property_id)
    {
      case PROP_WIDTH:
        champlain_zoom_level_set_width (self, g_value_get_uint (value));
        break;
      case PROP_HEIGHT:
        champlain_zoom_level_set_height (self, g_value_get_uint (value));
        break;
      case PROP_ZOOM_LEVEL:
        champlain_zoom_level_set_zoom_level (self, g_value_get_int (value));
        break;
      case PROP_ACTOR:
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_zoom_level_dispose (GObject *object)
{

  guint k;
  ChamplainZoomLevel *level = CHAMPLAIN_ZOOM_LEVEL (object);
  ChamplainZoomLevelPrivate *priv = level->priv;

  if (priv->actor != NULL)
  {
    g_object_unref (priv->actor);
    priv->actor = NULL;
  }

  if (priv->tiles != NULL)
    {
      int count = champlain_zoom_level_tile_count (level);
      for (k = 0; k < count; k++)
        {
          ChamplainTile *tile = champlain_zoom_level_get_nth_tile (level, 0);
          if (tile != NULL)
              champlain_zoom_level_remove_tile (level, tile);
        }
      g_ptr_array_free (priv->tiles, TRUE);
      priv->tiles = NULL;
    }

  G_OBJECT_CLASS (champlain_zoom_level_parent_class)->dispose (object);
}

static void
champlain_zoom_level_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_zoom_level_parent_class)->finalize (object);
}

static void
champlain_zoom_level_class_init (ChamplainZoomLevelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainZoomLevelPrivate));

  object_class->get_property = champlain_zoom_level_get_property;
  object_class->set_property = champlain_zoom_level_set_property;
  object_class->dispose = champlain_zoom_level_dispose;
  object_class->finalize = champlain_zoom_level_finalize;

  signals[SIGNAL_TILE_ADDED] =
      g_signal_new ("tile-added", G_OBJECT_CLASS_TYPE (klass),
      G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT,
      G_TYPE_NONE, 1, CHAMPLAIN_TYPE_TILE);

  signals[SIGNAL_TILE_REMOVED] =
      g_signal_new ("tile-removed", G_OBJECT_CLASS_TYPE (klass),
      G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__OBJECT,
      G_TYPE_NONE, 1, CHAMPLAIN_TYPE_TILE);

  /**
  * ChamplainZoomLevel:width:
  *
  * The absolute width of the zoom level in tiles
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_WIDTH,
      g_param_spec_uint ("width",
          "Width",
          "The width of this zoom level",
          0,
          G_MAXINT,
          0,
          G_PARAM_READWRITE));

  /**
  * ChamplainZoomLevel:height:
  *
  * The absolute height of the zoom level in tiles
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_HEIGHT,
      g_param_spec_uint ("height",
          "height",
          "The height of this zoom level",
          0,
          G_MAXINT,
          0,
          G_PARAM_READWRITE));

  /**
  * ChamplainZoomLevel:zoom-level:
  *
  * The zoom level
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_ZOOM_LEVEL,
      g_param_spec_int ("zoom-level",
          "zoom-level",
          "The level of this zoom level",
          G_MININT,
          G_MAXINT,
          0,
          G_PARAM_READWRITE));

  /**
  * ChamplainZoomLevel:actor:
  *
  * The #ClutterActor containing all the tiles
  *
  * Since: 0.4
  */
  g_object_class_install_property (object_class,
      PROP_ACTOR,
      g_param_spec_object ("actor",
          "Actor",
          "The actor containing all the tiles",
          CLUTTER_TYPE_ACTOR,
          G_PARAM_READABLE));
}

static void
champlain_zoom_level_init (ChamplainZoomLevel *self)
{
  ChamplainZoomLevelPrivate *priv = GET_PRIVATE (self);
  self->priv = priv;

  priv->tiles = g_ptr_array_sized_new (64);
  priv->actor = g_object_ref (clutter_group_new ());
}

/**
 * champlain_zoom_level_new:
 *
 * Returns a new ChamplainZoomLevel
 *
 * Since: 0.4
 */
ChamplainZoomLevel*
champlain_zoom_level_new (void)
{
  return g_object_new (CHAMPLAIN_TYPE_ZOOM_LEVEL, NULL);
}

/**
 * champlain_zoom_level_add_tile:
 * @self: a #ChamplainZoomLevel
 * @tile: a #ChamplainTile
 *
 * Adds a #ChamplainTile to a #ChamplainZoomLevel.
 *
 * Since: 0.4
 */
void
champlain_zoom_level_add_tile (ChamplainZoomLevel *self,
    ChamplainTile *tile)
{
  g_return_if_fail (CHAMPLAIN_ZOOM_LEVEL (self));

  ChamplainZoomLevelPrivate *priv = self->priv;

  g_object_ref (tile);
  g_ptr_array_add (priv->tiles, tile);

  g_signal_emit (self, signals[SIGNAL_TILE_ADDED], 0, tile);
}

/**
 * champlain_zoom_level_remove_tile:
 * @self: a #ChamplainZoomLevel
 * @tile: a #ChamplainTile
 *
 * Removes a #ChamplainTile to a #ChamplainZoomLevel.
 *
 * Since: 0.4
 */
void
champlain_zoom_level_remove_tile (ChamplainZoomLevel *self,
    ChamplainTile *tile)
{
  g_return_if_fail (CHAMPLAIN_ZOOM_LEVEL (self));

  ChamplainZoomLevelPrivate *priv = self->priv;

  g_signal_emit (self, signals[SIGNAL_TILE_REMOVED], 0, tile);

  g_ptr_array_remove_fast (priv->tiles, tile);
  g_object_unref (tile);
}

/**
 * champlain_zoom_level_tile_count:
 * @self: a #ChamplainZoomLevel
 *
 * Returns the number of tiles in a #ChamplainZoomLevel.
 *
 * Since: 0.4
 */
guint
champlain_zoom_level_tile_count (ChamplainZoomLevel *self)
{
  g_return_val_if_fail (CHAMPLAIN_ZOOM_LEVEL (self), 0);

  return self->priv->tiles->len;
}

/**
 * champlain_zoom_level_get_nth_tile:
 * @self: a #ChamplainZoomLevel
 * @index: an index
 *
 * Returns the #ChamplainTile at given index
 *
 * Since: 0.4
 */
ChamplainTile *
champlain_zoom_level_get_nth_tile (ChamplainZoomLevel *self,
    guint index)
{
  g_return_val_if_fail (CHAMPLAIN_ZOOM_LEVEL (self), NULL);
  g_return_val_if_fail (index < self->priv->tiles->len, NULL);
  g_return_val_if_fail (index >= 0, NULL);

  return g_ptr_array_index (self->priv->tiles, index);
}

/**
 * champlain_zoom_level_get_width:
 * @self: a #ChamplainZoomLevel
 *
 * Returns the absolute width of a #ChamplainZoomLevel in tiles
 *
 * Since: 0.4
 */
guint
champlain_zoom_level_get_width (ChamplainZoomLevel *self)
{
  g_return_val_if_fail (CHAMPLAIN_ZOOM_LEVEL (self), 0);

  return self->priv->width;
}

/**
 * champlain_zoom_level_get_height:
 * @self: a #ChamplainZoomLevel
 *
 * Returns the absolute height of a #ChamplainZoomLevel in tiles
 *
 * Since: 0.4
 */
guint
champlain_zoom_level_get_height (ChamplainZoomLevel *self)
{
  g_return_val_if_fail (CHAMPLAIN_ZOOM_LEVEL (self), 0);

  return self->priv->height;
}

/**
 * champlain_zoom_level_get_zoom_level:
 * @self: a #ChamplainZoomLevel
 *
 * Returns the zoom level of a #ChamplainZoomLevel
 *
 * Since: 0.4
 */
gint
champlain_zoom_level_get_zoom_level (ChamplainZoomLevel *self)
{
  g_return_val_if_fail (CHAMPLAIN_ZOOM_LEVEL (self), 0);

  return self->priv->zoom_level;
}

/**
 * champlain_zoom_level_set_width:
 * @self: a #ChamplainZoomLevel
 * @width: a width
 *
 * Sets the absolute width of a #ChamplainZoomLevel in tiles
 *
 * Since: 0.4
 */
void
champlain_zoom_level_set_width (ChamplainZoomLevel *self,
    guint width)
{
  g_return_if_fail (CHAMPLAIN_ZOOM_LEVEL (self));

  self->priv->width = width;

  g_object_notify (G_OBJECT (self), "width");
}

/**
 * champlain_zoom_level_set_height:
 * @self: a #ChamplainZoomLevel
 * @height: a height
 *
 * Sets the absolute height of a #ChamplainZoomLevel in tiles
 *
 * Since: 0.4
 */
void
champlain_zoom_level_set_height (ChamplainZoomLevel *self,
    guint height)
{
  g_return_if_fail (CHAMPLAIN_ZOOM_LEVEL (self));

  self->priv->height = height;

  g_object_notify (G_OBJECT (self), "height");
}

/**
 * champlain_zoom_level_set_zoom_level:
 * @self: a #ChamplainZoomLevel
 * @zoom_level: a zoom_level
 *
 * Sets the zoom_level of a #ChamplainZoomLevel
 *
 * Since: 0.4
 */
void
champlain_zoom_level_set_zoom_level (ChamplainZoomLevel *self,
    gint zoom_level)
{
  g_return_if_fail (CHAMPLAIN_ZOOM_LEVEL (self));

  self->priv->zoom_level = zoom_level;

  g_object_notify (G_OBJECT (self), "zoom-level");
}

/**
 * champlain_zoom_level_get_actor:
 * @self: a #ChamplainZoomLevel
 *
 * Returns the #ClutterActor containing all the tiles of a #ChamplainZoomLevel,
 * it should not be unref.
 *
 * Since: 0.4
 */
ClutterActor *
champlain_zoom_level_get_actor (ChamplainZoomLevel *self)
{
  g_return_val_if_fail (CHAMPLAIN_ZOOM_LEVEL (self), NULL);

  return self->priv->actor;
}
