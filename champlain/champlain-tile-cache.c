/*
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

/**
 * SECTION:champlain-tile-cache
 * @short_description: A base class of tile caches
 *
 * This class defines properties and methods common to all caches (that is, map
 * sources that permit storage and retrieval of tiles). Tiles are typically
 * stored by #ChamplainTileSource objects.
 */

#include "champlain-tile-cache.h"

G_DEFINE_TYPE (ChamplainTileCache, champlain_tile_cache, CHAMPLAIN_TYPE_MAP_SOURCE)

#define GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_TILE_CACHE, ChamplainTileCachePrivate))

enum
{
  PROP_0,
  PROP_PERSISTENT_CACHE
};

typedef struct _ChamplainTileCachePrivate ChamplainTileCachePrivate;

struct _ChamplainTileCachePrivate
{
  gboolean persistent;
};

static const gchar *get_id (ChamplainMapSource *map_source);
static const gchar *get_name (ChamplainMapSource *map_source);
static const gchar *get_license (ChamplainMapSource *map_source);
static const gchar *get_license_uri (ChamplainMapSource *map_source);
static guint get_min_zoom_level (ChamplainMapSource *map_source);
static guint get_max_zoom_level (ChamplainMapSource *map_source);
static guint get_tile_size (ChamplainMapSource *map_source);
static ChamplainMapProjection get_projection (ChamplainMapSource *map_source);

static void
champlain_tile_cache_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainTileCache *tile_cache = CHAMPLAIN_TILE_CACHE (object);

  switch (property_id)
    {
    case PROP_PERSISTENT_CACHE:
      g_value_set_boolean (value, champlain_tile_cache_get_persistent (tile_cache));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_tile_cache_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainTileCachePrivate *priv = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_PERSISTENT_CACHE:
      priv->persistent = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_tile_cache_dispose (GObject *object)
{
  G_OBJECT_CLASS (champlain_tile_cache_parent_class)->dispose (object);
}

static void
champlain_tile_cache_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_tile_cache_parent_class)->finalize (object);
}

static void
champlain_tile_cache_constructed  (GObject *object)
{
  G_OBJECT_CLASS (champlain_tile_cache_parent_class)->constructed (object);
}

static void
champlain_tile_cache_class_init (ChamplainTileCacheClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  ChamplainMapSourceClass *map_source_class = CHAMPLAIN_MAP_SOURCE_CLASS (klass);
  ChamplainTileCacheClass *tile_cache_class = CHAMPLAIN_TILE_CACHE_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (ChamplainTileCachePrivate));

  object_class->finalize = champlain_tile_cache_finalize;
  object_class->dispose = champlain_tile_cache_dispose;
  object_class->get_property = champlain_tile_cache_get_property;
  object_class->set_property = champlain_tile_cache_set_property;
  object_class->constructed = champlain_tile_cache_constructed;

  map_source_class->get_id = get_id;
  map_source_class->get_name = get_name;
  map_source_class->get_license = get_license;
  map_source_class->get_license_uri = get_license_uri;
  map_source_class->get_min_zoom_level = get_min_zoom_level;
  map_source_class->get_max_zoom_level = get_max_zoom_level;
  map_source_class->get_tile_size = get_tile_size;
  map_source_class->get_projection = get_projection;

  map_source_class->fill_tile = NULL;

  tile_cache_class->refresh_tile_time = NULL;
  tile_cache_class->on_tile_filled = NULL;
  tile_cache_class->store_tile = NULL;
  tile_cache_class->clean = NULL;

  /**
  * ChamplainTileCache:persistent-cache
  *
  * Determines whether the cache is persistent or temporary (cleaned upon
  * destruction)
  *
  * Since: 0.6
  */
  pspec = g_param_spec_boolean ("persistent-cache",
                                "Persistent Cache",
                                "Specifies whether the cache is persistent",
                                TRUE,
                                G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_PERSISTENT_CACHE, pspec);
}

static void
champlain_tile_cache_init (ChamplainTileCache *tile_cache)
{
  ChamplainTileCachePrivate *priv = GET_PRIVATE (tile_cache);

  priv->persistent = TRUE;
}

/**
 * champlain_tile_cache_get_persistent:
 * @tile_cache: a #ChamplainTileCache
 *
 * Gets cache persistency information.
 *
 * Returns: TRUE when the cache is persistent; FALSE otherwise.
 *
 * Since: 0.6
 */
gboolean
champlain_tile_cache_get_persistent (ChamplainTileCache *tile_cache)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_CACHE (tile_cache), FALSE);

  ChamplainTileCachePrivate *priv = GET_PRIVATE(tile_cache);
  return priv->persistent;
}

/**
 * champlain_tile_cache_store_tile:
 * @tile_cache: a #ChamplainTileCache
 * @tile: a #ChamplainTile
 * @contents: the tile contents that should be stored
 * @size: size of the contents in bytes
 *
 * Stores the tile including the metadata into the cache.
 *
 * Since: 0.6
 */
void
champlain_tile_cache_store_tile (ChamplainTileCache *tile_cache,
    ChamplainTile *tile,
    const gchar *contents,
    gsize size)
{
  g_return_if_fail (CHAMPLAIN_IS_TILE_CACHE (tile_cache));

  CHAMPLAIN_TILE_CACHE_GET_CLASS (tile_cache)->store_tile (tile_cache, tile, contents, size);
}

/**
 * champlain_tile_cache_refresh_tile_time:
 * @tile_cache: a #ChamplainTileCache
 * @tile: a #ChamplainTile
 *
 * Refreshes the tile access time in the cache.
 *
 * Since: 0.6
 */
void
champlain_tile_cache_refresh_tile_time (ChamplainTileCache *tile_cache,
    ChamplainTile *tile)
{
  g_return_if_fail (CHAMPLAIN_IS_TILE_CACHE (tile_cache));

  CHAMPLAIN_TILE_CACHE_GET_CLASS (tile_cache)->refresh_tile_time (tile_cache, tile);
}

/**
 * champlain_tile_cache_on_tile_filled:
 * @tile_cache: a #ChamplainTileCache
 * @tile: a #ChamplainTile
 *
 * When a cache fills a tile and the next source in the chain is a tile cache,
 * it should call this function on the next source. This way all the caches
 * preceding a tile source in the chain get informed that the tile was used and
 * can modify their metadata accordingly in the implementation of this function.
 * In addition, the call of this function should be chained so within the
 * implementation of this function it should be called on the next source
 * in the chain when next source is a tile cache.
 *
 * Since: 0.6
 */
void
champlain_tile_cache_on_tile_filled (ChamplainTileCache *tile_cache,
    ChamplainTile *tile)
{
  g_return_if_fail (CHAMPLAIN_IS_TILE_CACHE (tile_cache));

  CHAMPLAIN_TILE_CACHE_GET_CLASS (tile_cache)->on_tile_filled (tile_cache, tile);
}

/**
 * champlain_tile_cache_clean:
 * @tile_cache: a #ChamplainTileCache
 *
 * Erases the contents of the cache. This function will fail when the cache is
 * not temporary.
 *
 * Since: 0.6
 */
void
champlain_tile_cache_clean (ChamplainTileCache *tile_cache)
{
  g_return_if_fail (CHAMPLAIN_IS_TILE_CACHE (tile_cache));

  CHAMPLAIN_TILE_CACHE_GET_CLASS (tile_cache)->clean (tile_cache);
}

static const gchar *
get_id (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_CACHE (map_source), NULL);

  ChamplainMapSource *next_source = champlain_map_source_get_next_source (map_source);

  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (next_source), NULL);

  return champlain_map_source_get_id (next_source);
}

static const gchar *
get_name (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_CACHE (map_source), NULL);

  ChamplainMapSource *next_source = champlain_map_source_get_next_source (map_source);

  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (next_source), NULL);

  return champlain_map_source_get_name (next_source);
}

static const gchar *
get_license (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_CACHE (map_source), NULL);

  ChamplainMapSource *next_source = champlain_map_source_get_next_source (map_source);

  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (next_source), NULL);

  return champlain_map_source_get_license (next_source);
}

static const gchar *
get_license_uri (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_CACHE (map_source), NULL);

  ChamplainMapSource *next_source = champlain_map_source_get_next_source (map_source);

  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (next_source), NULL);

  return champlain_map_source_get_license_uri (next_source);
}

static guint
get_min_zoom_level (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_CACHE (map_source), 0);

  ChamplainMapSource *next_source = champlain_map_source_get_next_source (map_source);

  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (next_source), 0);

  return champlain_map_source_get_min_zoom_level (next_source);
}

static guint
get_max_zoom_level (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_CACHE (map_source), 0);

  ChamplainMapSource *next_source = champlain_map_source_get_next_source (map_source);

  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (next_source), 0);

  return champlain_map_source_get_max_zoom_level (next_source);
}

static guint
get_tile_size (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_CACHE (map_source), 0);

  ChamplainMapSource *next_source = champlain_map_source_get_next_source (map_source);

  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (next_source), 0);

  return champlain_map_source_get_tile_size (next_source);
}

static ChamplainMapProjection
get_projection (ChamplainMapSource *map_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_TILE_CACHE (map_source), CHAMPLAIN_MAP_PROJECTION_MERCATOR);

  ChamplainMapSource *next_source = champlain_map_source_get_next_source (map_source);

  g_return_val_if_fail (CHAMPLAIN_IS_MAP_SOURCE (next_source), CHAMPLAIN_MAP_PROJECTION_MERCATOR);

  return champlain_map_source_get_projection (next_source);
}
