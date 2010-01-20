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

#include "champlain-map-source-chain.h"
#include "champlain-tile-cache.h"
#include "champlain-tile-source.h"

G_DEFINE_TYPE (ChamplainMapSourceChain, champlain_map_source_chain, CHAMPLAIN_TYPE_MAP_SOURCE);

#define GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_MAP_SOURCE_CHAIN, ChamplainMapSourceChainPrivate))

typedef struct _ChamplainMapSourceChainPrivate ChamplainMapSourceChainPrivate;

struct _ChamplainMapSourceChainPrivate
{
  ChamplainMapSource *stack_top;
  gulong sig_handler_id;
};

static const gchar *get_id (ChamplainMapSource *map_source);
static const gchar *get_name (ChamplainMapSource *map_source);
static const gchar *get_license (ChamplainMapSource *map_source);
static const gchar *get_license_uri (ChamplainMapSource *map_source);
static guint get_min_zoom_level (ChamplainMapSource *map_source);
static guint get_max_zoom_level (ChamplainMapSource *map_source);
static guint get_tile_size (ChamplainMapSource *map_source);

static void fill_tile (ChamplainMapSource *map_source, ChamplainTile *tile);

static void
champlain_map_source_chain_dispose (GObject *object)
{
  ChamplainMapSourceChain *source_chain = CHAMPLAIN_MAP_SOURCE_CHAIN(object);
  ChamplainMapSourceChainPrivate *priv = GET_PRIVATE(object);

  while (priv->stack_top)
    champlain_map_source_chain_pop_map_source(source_chain);

  G_OBJECT_CLASS (champlain_map_source_chain_parent_class)->dispose (object);
}

static void
champlain_map_source_chain_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_map_source_chain_parent_class)->finalize (object);
}

static void
champlain_map_source_chain_class_init (ChamplainMapSourceChainClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainMapSourceChainPrivate));

  object_class->finalize = champlain_map_source_chain_finalize;
  object_class->dispose = champlain_map_source_chain_dispose;

  ChamplainMapSourceClass *map_source_class = CHAMPLAIN_MAP_SOURCE_CLASS (klass);

  map_source_class->get_id = get_id;
  map_source_class->get_name = get_name;
  map_source_class->get_license = get_license;
  map_source_class->get_license_uri = get_license_uri;
  map_source_class->get_min_zoom_level = get_min_zoom_level;
  map_source_class->get_max_zoom_level = get_max_zoom_level;
  map_source_class->get_tile_size = get_tile_size;

  map_source_class->fill_tile = fill_tile;
}

static void
champlain_map_source_chain_init (ChamplainMapSourceChain *source_chain)
{
  ChamplainMapSourceChainPrivate *priv = GET_PRIVATE(source_chain);
  priv->stack_top = NULL;
  priv->sig_handler_id = 0;
}

ChamplainMapSourceChain* champlain_map_source_chain_new (void)
{
  return g_object_new (CHAMPLAIN_TYPE_MAP_SOURCE_CHAIN, NULL);
}

static const gchar *
get_id (ChamplainMapSource *map_source)
{
  ChamplainMapSourceChain *source_chain = CHAMPLAIN_MAP_SOURCE_CHAIN (map_source);
  g_return_val_if_fail (source_chain, NULL);

  ChamplainMapSourceChainPrivate *priv = GET_PRIVATE(source_chain);
  g_return_val_if_fail (priv->stack_top, NULL);

  return champlain_map_source_get_id(priv->stack_top);
}

static const gchar *
get_name (ChamplainMapSource *map_source)
{
  ChamplainMapSourceChain *source_chain = CHAMPLAIN_MAP_SOURCE_CHAIN (map_source);
  g_return_val_if_fail (source_chain, NULL);

  ChamplainMapSourceChainPrivate *priv = GET_PRIVATE(source_chain);
  g_return_val_if_fail (priv->stack_top, NULL);

  return champlain_map_source_get_name(priv->stack_top);
}

static const gchar *
get_license (ChamplainMapSource *map_source)
{
  ChamplainMapSourceChain *source_chain = CHAMPLAIN_MAP_SOURCE_CHAIN (map_source);
  g_return_val_if_fail (source_chain, NULL);

  ChamplainMapSourceChainPrivate *priv = GET_PRIVATE(source_chain);
  g_return_val_if_fail (priv->stack_top, NULL);

  return champlain_map_source_get_license(priv->stack_top);
}

static const gchar *
get_license_uri (ChamplainMapSource *map_source)
{
  ChamplainMapSourceChain *source_chain = CHAMPLAIN_MAP_SOURCE_CHAIN (map_source);
  g_return_val_if_fail (source_chain, NULL);

  ChamplainMapSourceChainPrivate *priv = GET_PRIVATE(source_chain);
  g_return_val_if_fail (priv->stack_top, NULL);

  return champlain_map_source_get_license_uri(priv->stack_top);
}

static guint
get_min_zoom_level (ChamplainMapSource *map_source)
{
  ChamplainMapSourceChain *source_chain = CHAMPLAIN_MAP_SOURCE_CHAIN (map_source);
  g_return_val_if_fail (source_chain, 0);

  ChamplainMapSourceChainPrivate *priv = GET_PRIVATE(source_chain);
  g_return_val_if_fail (priv->stack_top, 0);

  return champlain_map_source_get_min_zoom_level(priv->stack_top);
}

static guint
get_max_zoom_level (ChamplainMapSource *map_source)
{
  ChamplainMapSourceChain *source_chain = CHAMPLAIN_MAP_SOURCE_CHAIN (map_source);
  g_return_val_if_fail (source_chain, 0);

  ChamplainMapSourceChainPrivate *priv = GET_PRIVATE(source_chain);
  g_return_val_if_fail (priv->stack_top, 0);

  return champlain_map_source_get_max_zoom_level(priv->stack_top);
}

static guint
get_tile_size (ChamplainMapSource *map_source)
{
  ChamplainMapSourceChain *source_chain = CHAMPLAIN_MAP_SOURCE_CHAIN (map_source);
  g_return_val_if_fail (source_chain, 0);

  ChamplainMapSourceChainPrivate *priv = GET_PRIVATE(source_chain);
  g_return_val_if_fail (priv->stack_top, 0);

  return champlain_map_source_get_tile_size(priv->stack_top);
}

static void fill_tile (ChamplainMapSource *map_source,
                       ChamplainTile *tile)
{
  ChamplainMapSourceChain *source_chain = CHAMPLAIN_MAP_SOURCE_CHAIN (map_source);
  g_return_if_fail (source_chain);

  ChamplainMapSourceChainPrivate *priv = GET_PRIVATE(source_chain);
  g_return_if_fail (priv->stack_top);

  champlain_map_source_fill_tile(priv->stack_top, tile);
}

static void assign_cache_of_next_source_sequence(ChamplainMapSource *start_map_source, ChamplainTileCache *tile_cache)
{
  ChamplainMapSource *map_source = start_map_source;
  ChamplainTileSource *tile_source;

  do
    {
      map_source = champlain_map_source_get_next_source(map_source);
    }
  while (CHAMPLAIN_IS_TILE_CACHE(map_source));

  tile_source = CHAMPLAIN_TILE_SOURCE(map_source);
  while (tile_source)
    {
      champlain_tile_source_set_cache(tile_source, tile_cache);
      map_source = champlain_map_source_get_next_source(map_source);
      tile_source = CHAMPLAIN_TILE_SOURCE(map_source);
    }
}

static
void reload_tiles_cb(ChamplainMapSource *map_source, ChamplainMapSourceChain *source_chain)
{
  /* propagate the signal from the chain that is inside champlain_map_source_chain */
  g_signal_emit_by_name (source_chain, "reload-tiles", NULL);
}

void champlain_map_source_chain_push(ChamplainMapSourceChain *source_chain, ChamplainMapSource *map_source)
{
  ChamplainMapSourceChainPrivate *priv = GET_PRIVATE(source_chain);
  gboolean is_cache = FALSE;

  if (CHAMPLAIN_IS_TILE_CACHE(map_source))
    is_cache = TRUE;
  else
    g_return_if_fail (CHAMPLAIN_IS_TILE_SOURCE(map_source));

  g_object_ref_sink(map_source);

  if (!priv->stack_top)
    {
      /* tile source has to be last */
      g_return_if_fail(!is_cache);
      priv->stack_top = map_source;
    }
  else
    {
      if (g_signal_handler_is_connected (priv->stack_top, priv->sig_handler_id))
        g_signal_handler_disconnect (priv->stack_top, priv->sig_handler_id);

      champlain_map_source_set_next_source(map_source, priv->stack_top);
      priv->stack_top = map_source;

      if (is_cache)
        {
          ChamplainTileCache *tile_cache = CHAMPLAIN_TILE_CACHE(map_source);
          assign_cache_of_next_source_sequence(priv->stack_top, tile_cache);
        }
    }

  priv->sig_handler_id = g_signal_connect (priv->stack_top, "reload-tiles",
                         G_CALLBACK (reload_tiles_cb), source_chain);
}

void champlain_map_source_chain_pop_map_source(ChamplainMapSourceChain *source_chain)
{
  ChamplainMapSourceChainPrivate *priv = GET_PRIVATE(source_chain);
  ChamplainMapSource *old_stack_top = priv->stack_top;

  g_return_if_fail(priv->stack_top);

  if (g_signal_handler_is_connected (priv->stack_top, priv->sig_handler_id))
    g_signal_handler_disconnect (priv->stack_top, priv->sig_handler_id);

  if (CHAMPLAIN_IS_TILE_CACHE(priv->stack_top))
    {
      ChamplainMapSource *map_source = champlain_map_source_get_next_source(priv->stack_top);
      ChamplainTileCache *tile_cache = NULL;

      if (CHAMPLAIN_IS_TILE_CACHE(map_source))
        tile_cache = CHAMPLAIN_TILE_CACHE(map_source);

      assign_cache_of_next_source_sequence(priv->stack_top, tile_cache);
    }

  priv->stack_top = champlain_map_source_get_next_source(priv->stack_top);
  if (priv->stack_top)
    {
      priv->sig_handler_id = g_signal_connect (priv->stack_top, "reload-tiles",
                             G_CALLBACK (reload_tiles_cb), source_chain);
    }

  g_object_unref(old_stack_top);
}
