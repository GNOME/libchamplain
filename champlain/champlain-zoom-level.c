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

#include "champlain-tile.h"
#include "champlain-map-source.h"

#include <clutter/clutter.h>

G_DEFINE_TYPE (ChamplainZoomLevel, champlain_zoom_level, CLUTTER_TYPE_GROUP)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_ZOOM_LEVEL, ChamplainZoomLevelPrivate))

enum
{
  PROP_0,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_ZOOM_LEVEL
};

struct _ChamplainZoomLevelPrivate {
  guint width; /* The absolute width of the zoom level in tiles */
  guint height; /* The absolute height of the zoom level in tiles */
  gint zoom_level;
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
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_zoom_level_dispose (GObject *object)
{
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
}

static void
champlain_zoom_level_init (ChamplainZoomLevel *self)
{
  ChamplainZoomLevelPrivate *priv = GET_PRIVATE (self);

  priv->width = 0;
  priv->height = 0;
  priv->zoom_level = 0;
}

/**
 * champlain_zoom_level_new:
 *
 * Returns: a new #ChamplainZoomLevel
 *
 * Since: 0.4
 */
ChamplainZoomLevel*
champlain_zoom_level_new (void)
{
  return g_object_new (CHAMPLAIN_TYPE_ZOOM_LEVEL, NULL);
}

/**
 * champlain_zoom_level_get_width:
 * @self: a #ChamplainZoomLevel
 *
 * Returns: the width of a #ChamplainZoomLevel in tiles
 *
 * Since: 0.4
 */
guint
champlain_zoom_level_get_width (ChamplainZoomLevel *self)
{
  g_return_val_if_fail (CHAMPLAIN_ZOOM_LEVEL (self), 0);
  ChamplainZoomLevelPrivate *priv = GET_PRIVATE (self);

  return priv->width;
}

/**
 * champlain_zoom_level_get_height:
 * @self: a #ChamplainZoomLevel
 *
 * Returns: the height of a #ChamplainZoomLevel in tiles
 *
 * Since: 0.4
 */
guint
champlain_zoom_level_get_height (ChamplainZoomLevel *self)
{
  g_return_val_if_fail (CHAMPLAIN_ZOOM_LEVEL (self), 0);
  ChamplainZoomLevelPrivate *priv = GET_PRIVATE (self);

  return priv->height;
}

/**
 * champlain_zoom_level_get_zoom_level:
 * @self: a #ChamplainZoomLevel
 *
 * Returns: the zoom level of a #ChamplainZoomLevel
 *
 * Since: 0.4
 */
gint
champlain_zoom_level_get_zoom_level (ChamplainZoomLevel *self)
{
  g_return_val_if_fail (CHAMPLAIN_ZOOM_LEVEL (self), 0);
  ChamplainZoomLevelPrivate *priv = GET_PRIVATE (self);

  return priv->zoom_level;
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
  ChamplainZoomLevelPrivate *priv = GET_PRIVATE (self);

  priv->width = width;

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
  ChamplainZoomLevelPrivate *priv = GET_PRIVATE (self);

  priv->height = height;

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
  ChamplainZoomLevelPrivate *priv = GET_PRIVATE (self);

  priv->zoom_level = zoom_level;

  g_object_notify (G_OBJECT (self), "zoom-level");
}

gboolean
champlain_zoom_level_zoom_to (ChamplainZoomLevel *self,
    ChamplainMapSource *source,
    guint zoom_level)
{
  g_return_val_if_fail (CHAMPLAIN_ZOOM_LEVEL (self), FALSE);

  ChamplainZoomLevelPrivate *priv = GET_PRIVATE (self);

  if (zoom_level <= champlain_map_source_get_max_zoom_level (source) &&
      zoom_level >= champlain_map_source_get_min_zoom_level (source))
    {
      clutter_group_remove_all (CLUTTER_GROUP (self));

      priv->width = champlain_map_source_get_row_count (source, zoom_level);
      priv->height = champlain_map_source_get_column_count (source, zoom_level);
      priv->zoom_level = zoom_level;
      return TRUE;
    }

  return FALSE;
}
