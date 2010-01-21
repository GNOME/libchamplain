/*
 * Copyright (C) 2008-2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
 * Copyright (C) 2010 Jiri Techet <techet@gmail.com>
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

#include "champlain-tile-source.h"
#include "champlain-enum-types.h"

G_DEFINE_TYPE (ChamplainTileSource, champlain_tile_source, CHAMPLAIN_TYPE_MAP_SOURCE);

#define GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_TILE_SOURCE, ChamplainTileSourcePrivate))

enum
{
  PROP_0,
  PROP_ID,
  PROP_NAME,
  PROP_LICENSE,
  PROP_LICENSE_URI,
  PROP_MIN_ZOOM_LEVEL,
  PROP_MAX_ZOOM_LEVEL,
  PROP_TILE_SIZE,
  PROP_MAP_PROJECTION,
  PROP_CACHE
};

typedef struct _ChamplainTileSourcePrivate ChamplainTileSourcePrivate;

struct _ChamplainTileSourcePrivate
{
  gchar *id;
  gchar *name;
  gchar *license;
  gchar *license_uri;
  guint min_zoom_level;
  guint max_zoom_level;
  guint tile_size;
  ChamplainMapProjection map_projection;
  ChamplainTileCache *cache;
};

static const gchar *get_id (ChamplainMapSource *map_source);
static const gchar *get_name (ChamplainMapSource *map_source);
static const gchar *get_license (ChamplainMapSource *map_source);
static const gchar *get_license_uri (ChamplainMapSource *map_source);
static guint get_min_zoom_level (ChamplainMapSource *map_source);
static guint get_max_zoom_level (ChamplainMapSource *map_source);
static guint get_tile_size (ChamplainMapSource *map_source);

static void
champlain_tile_source_get_property (GObject *object,
                                    guint prop_id,
                                    GValue *value,
                                    GParamSpec *pspec)
{
  ChamplainTileSourcePrivate *priv = GET_PRIVATE(object);

  switch (prop_id)
    {
    case PROP_ID:
      g_value_set_string (value, priv->id);
      break;
    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;
    case PROP_LICENSE:
      g_value_set_string (value, priv->license);
      break;
    case PROP_LICENSE_URI:
      g_value_set_string (value, priv->license_uri);
      break;
    case PROP_MIN_ZOOM_LEVEL:
      g_value_set_uint (value, priv->min_zoom_level);
      break;
    case PROP_MAX_ZOOM_LEVEL:
      g_value_set_uint (value, priv->max_zoom_level);
      break;
    case PROP_TILE_SIZE:
      g_value_set_uint (value, priv->tile_size);
      break;
    case PROP_MAP_PROJECTION:
      g_value_set_enum (value, priv->map_projection);
      break;
    case PROP_CACHE:
      g_value_set_object (value, priv->cache);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
champlain_tile_source_set_property (GObject *object,
                                    guint prop_id,
                                    const GValue *value,
                                    GParamSpec *pspec)
{
  ChamplainTileSource *tile_source = CHAMPLAIN_TILE_SOURCE(object);

  switch (prop_id)
    {
    case PROP_ID:
      champlain_tile_source_set_id (tile_source,
                                    g_value_get_string (value));
    case PROP_NAME:
      champlain_tile_source_set_name (tile_source,
                                      g_value_get_string (value));
      break;
    case PROP_LICENSE:
      champlain_tile_source_set_license (tile_source,
                                         g_value_get_string (value));
      break;
    case PROP_LICENSE_URI:
      champlain_tile_source_set_license_uri (tile_source,
                                             g_value_get_string (value));
      break;
    case PROP_MIN_ZOOM_LEVEL:
      champlain_tile_source_set_min_zoom_level (tile_source,
          g_value_get_uint (value));
      break;
    case PROP_MAX_ZOOM_LEVEL:
      champlain_tile_source_set_max_zoom_level (tile_source,
          g_value_get_uint (value));
      break;
    case PROP_TILE_SIZE:
      champlain_tile_source_set_tile_size (tile_source,
                                           g_value_get_uint (value));
      break;
    case PROP_MAP_PROJECTION:
      champlain_tile_source_set_projection (tile_source,
                                            g_value_get_enum (value));
      break;
    case PROP_CACHE:
      champlain_tile_source_set_cache (tile_source,
                                       g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
champlain_tile_source_dispose (GObject *object)
{
  ChamplainTileSourcePrivate *priv = GET_PRIVATE(object);

  if (priv->cache)
    {
      g_object_unref (priv->cache);

      priv->cache = NULL;
    }

  G_OBJECT_CLASS (champlain_tile_source_parent_class)->dispose (object);
}

static void
champlain_tile_source_finalize (GObject *object)
{
  ChamplainTileSourcePrivate *priv = GET_PRIVATE(object);

  g_free (priv->id);
  g_free (priv->name);
  g_free (priv->license);
  g_free (priv->license_uri);

  G_OBJECT_CLASS (champlain_tile_source_parent_class)->finalize (object);
}

static void
champlain_tile_source_constructed  (GObject *object)
{
  G_OBJECT_CLASS (champlain_tile_source_parent_class)->constructed (object);
}

static void
champlain_tile_source_class_init (ChamplainTileSourceClass *klass)
{
  ChamplainMapSourceClass *map_source_class = CHAMPLAIN_MAP_SOURCE_CLASS (klass);
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (ChamplainTileSourcePrivate));

  object_class->finalize = champlain_tile_source_finalize;
  object_class->dispose = champlain_tile_source_dispose;
  object_class->get_property = champlain_tile_source_get_property;
  object_class->set_property = champlain_tile_source_set_property;
  object_class->constructed = champlain_tile_source_constructed;

  map_source_class->get_id = get_id;
  map_source_class->get_name = get_name;
  map_source_class->get_license = get_license;
  map_source_class->get_license_uri = get_license_uri;
  map_source_class->get_min_zoom_level = get_min_zoom_level;
  map_source_class->get_max_zoom_level = get_max_zoom_level;
  map_source_class->get_tile_size = get_tile_size;

  map_source_class->fill_tile = NULL;

  pspec = g_param_spec_string ("id",
                               "Id",
                               "The id of the tile source",
                               "",
                               (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_ID, pspec);

  pspec = g_param_spec_string ("name",
                               "Name",
                               "The name of the tile source",
                               "",
                               (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_NAME, pspec);

  pspec = g_param_spec_string ("license",
                               "License",
                               "The usage license of the tile source",
                               "",
                               (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_LICENSE, pspec);

  pspec = g_param_spec_string ("license-uri",
                               "License-uri",
                               "The usage license's uri for more information",
                               "",
                               (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_LICENSE_URI, pspec);

  pspec = g_param_spec_uint ("min-zoom-level",
                             "Minimum Zoom Level",
                             "The minimum zoom level",
                             0,
                             50,
                             0,
                             (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_MIN_ZOOM_LEVEL, pspec);

  pspec = g_param_spec_uint ("max-zoom-level",
                             "Maximum Zoom Level",
                             "The maximum zoom level",
                             0,
                             50,
                             18,
                             (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_MAX_ZOOM_LEVEL, pspec);

  pspec = g_param_spec_uint ("tile-size",
                             "Tile Size",
                             "The tile size",
                             0,
                             2048,
                             256,
                             (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_TILE_SIZE, pspec);

  pspec = g_param_spec_enum ("projection",
                             "Projection",
                             "The map projection",
                             CHAMPLAIN_TYPE_MAP_PROJECTION,
                             CHAMPLAIN_MAP_PROJECTION_MERCATOR,
                             (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_MAP_PROJECTION, pspec);

  pspec = g_param_spec_object ("cache",
                               "Cache",
                               "Cache used for tile sorage",
                               CHAMPLAIN_TYPE_TILE_CACHE,
                               G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_CACHE, pspec);
}

static void
champlain_tile_source_init (ChamplainTileSource *tile_source)
{
  ChamplainTileSourcePrivate *priv = GET_PRIVATE(tile_source);
  priv->cache = NULL;
  priv->id = NULL;
  priv->name = NULL;
  priv->license = NULL;
  priv->license_uri = NULL;
  priv->min_zoom_level = 0;
  priv->max_zoom_level = 0;
  priv->tile_size = 0;
  priv->map_projection = CHAMPLAIN_MAP_PROJECTION_MERCATOR;
}

ChamplainMapProjection
champlain_tile_source_get_projection (ChamplainTileSource *tile_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_SOURCE (tile_source), 0);

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(tile_source);
  return priv->map_projection;
}

ChamplainTileCache *
champlain_tile_source_get_cache (ChamplainTileSource *tile_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_SOURCE (tile_source), NULL);

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(tile_source);
  return priv->cache;
}

void
champlain_tile_source_set_projection (ChamplainTileSource *tile_source,
                                      ChamplainMapProjection projection)
{
  g_return_if_fail (CHAMPLAIN_IS_TILE_SOURCE (tile_source));

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(tile_source);

  priv->map_projection = projection;
  g_object_notify (G_OBJECT (tile_source), "projection");
}

void
champlain_tile_source_set_cache (ChamplainTileSource *tile_source,
                                 ChamplainTileCache *cache)
{
  g_return_if_fail (CHAMPLAIN_IS_TILE_SOURCE (tile_source));

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(tile_source);

  if (priv->cache != NULL)
    g_object_unref (priv->cache);

  if (cache)
    {
      g_return_if_fail (CHAMPLAIN_IS_TILE_CACHE (cache));

      g_object_ref_sink (cache);
    }

  priv->cache = cache;

  g_object_notify (G_OBJECT (tile_source), "cache");
}

static const gchar *
get_id (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_SOURCE (map_source), NULL);

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(map_source);
  return priv->id;
}

static const gchar *
get_name (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_SOURCE (map_source), NULL);

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(map_source);
  return priv->name;
}

static const gchar *
get_license (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_SOURCE (map_source), NULL);

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(map_source);
  return priv->license;
}

static const gchar *
get_license_uri (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_SOURCE (map_source), NULL);

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(map_source);
  return priv->license_uri;
}

static guint
get_min_zoom_level (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_SOURCE (map_source), 0);

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(map_source);
  return priv->min_zoom_level;
}

static guint
get_max_zoom_level (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_SOURCE (map_source), 0);

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(map_source);
  return priv->max_zoom_level;
}

static guint
get_tile_size (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_SOURCE (map_source), 0);

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(map_source);
  return priv->tile_size;
}

void
champlain_tile_source_set_id (ChamplainTileSource *tile_source,
                              const gchar *id)
{
  g_return_if_fail (CHAMPLAIN_IS_TILE_SOURCE (tile_source));

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(tile_source);

  g_free (priv->id);
  priv->id = g_strdup (id);

  g_object_notify (G_OBJECT (tile_source), "id");
}

void
champlain_tile_source_set_name (ChamplainTileSource *tile_source,
                                const gchar *name)
{
  g_return_if_fail (CHAMPLAIN_IS_TILE_SOURCE (tile_source));

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(tile_source);

  g_free (priv->name);
  priv->name = g_strdup (name);

  g_object_notify (G_OBJECT (tile_source), "name");
}

void
champlain_tile_source_set_license (ChamplainTileSource *tile_source,
                                   const gchar *license)
{
  g_return_if_fail (CHAMPLAIN_IS_TILE_SOURCE (tile_source));

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(tile_source);

  g_free (priv->license);
  priv->license = g_strdup (license);

  g_object_notify (G_OBJECT (tile_source), "license");
}

void
champlain_tile_source_set_license_uri (ChamplainTileSource *tile_source,
                                       const gchar *license_uri)
{
  g_return_if_fail (CHAMPLAIN_IS_TILE_SOURCE (tile_source));

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(tile_source);

  g_free (priv->license_uri);
  priv->license_uri = g_strdup (license_uri);

  g_object_notify (G_OBJECT (tile_source), "license-uri");
}

void
champlain_tile_source_set_min_zoom_level (ChamplainTileSource *tile_source,
    guint zoom_level)
{
  g_return_if_fail (CHAMPLAIN_IS_TILE_SOURCE (tile_source));

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(tile_source);

  priv->min_zoom_level = zoom_level;

  g_object_notify (G_OBJECT (tile_source), "min-zoom-level");
}

void
champlain_tile_source_set_max_zoom_level (ChamplainTileSource *tile_source,
    guint zoom_level)
{
  g_return_if_fail (CHAMPLAIN_IS_TILE_SOURCE (tile_source));

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(tile_source);

  priv->max_zoom_level = zoom_level;

  g_object_notify (G_OBJECT (tile_source), "max-zoom-level");
}

void
champlain_tile_source_set_tile_size (ChamplainTileSource *tile_source,
                                     guint tile_size)
{
  g_return_if_fail (CHAMPLAIN_IS_TILE_SOURCE (tile_source));

  ChamplainTileSourcePrivate *priv = GET_PRIVATE(tile_source);

  priv->tile_size = tile_size;

  g_object_notify (G_OBJECT (tile_source), "tile-size");
}
