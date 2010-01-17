/*
 * Copyright (C) 2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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
 * SECTION:champlain-map-source-factory
 * @short_description: Manages #ChamplainMapSource
 *
 * This factory manages the create of #ChamplainMapSource. It contains names
 * and constructor functions for each available map sources in libchamplain.
 * You can add your own with #champlain_map_source_factory_register.
 *
 * To get the wanted map source, use #champlain_map_source_factory_create. It
 * will return a ready to use #ChamplainMapSource.
 *
 * To get the list of registered map sources, use
 * #champlain_map_source_factory_dup_list.
 *
 */
#include "config.h"

#include "champlain-map-source-factory.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_NETWORK
#include "champlain-debug.h"

#include "champlain.h"
#include "champlain-file-cache.h"
#include "champlain-defines.h"
#include "champlain-enum-types.h"
#include "champlain-map-source.h"
#include "champlain-marshal.h"
#include "champlain-private.h"
#include "champlain-zoom-level.h"
#include "champlain-network-tile-source.h"
#include "champlain-map-source-chain.h"
#include "champlain-error-tile-source.h"

#include <glib.h>
#include <string.h>

enum
{
  /* normal signals */
  LAST_SIGNAL
};

enum
{
  PROP_0,
};

/* static guint champlain_map_source_factory_signals[LAST_SIGNAL] = { 0, }; */
static ChamplainMapSourceFactory *instance = NULL;

G_DEFINE_TYPE (ChamplainMapSourceFactory, champlain_map_source_factory, G_TYPE_OBJECT);

#define GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY, ChamplainMapSourceFactoryPrivate))

struct _ChamplainMapSourceFactoryPrivate
{
  GSList *registered_sources;
};

static ChamplainMapSource * champlain_map_source_new_generic (
     ChamplainMapSourceDesc *desc, gpointer data);

static ChamplainMapSource * champlain_map_source_new_memphis (
    ChamplainMapSourceDesc *desc, gpointer user_data);

static void
champlain_map_source_factory_get_property (GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
  //ChamplainMapSourceFactory *map_source_factory = CHAMPLAIN_MAP_SOURCE_FACTORY(object);
  //ChamplainMapSourceFactoryPrivate *priv = map_source_factory->priv;

  switch(prop_id)
    {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
champlain_map_source_factory_set_property (GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec)
{
  //ChamplainMapSourceFactory *map_source_factory = CHAMPLAIN_MAP_SOURCE_FACTORY(object);
  //ChamplainMapSourceFactoryPrivate *priv = map_source_factory->priv;

  switch(prop_id)
    {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
champlain_map_source_factory_finalize (GObject *object)
{
  ChamplainMapSourceFactory *factory = CHAMPLAIN_MAP_SOURCE_FACTORY (object);

  g_slist_free (factory->priv->registered_sources);

  G_OBJECT_CLASS (champlain_map_source_factory_parent_class)->finalize (object);
}

static GObject *
champlain_map_source_factory_constructor (GType type,
    guint n_construct_params,
    GObjectConstructParam *construct_params)
{
  GObject *retval;

  if (instance == NULL)
    {
      retval = G_OBJECT_CLASS (champlain_map_source_factory_parent_class)->constructor
          (type, n_construct_params, construct_params);

      instance = CHAMPLAIN_MAP_SOURCE_FACTORY (retval);
      g_object_add_weak_pointer (retval, (gpointer) &instance);
    }
  else
    {
      retval = g_object_ref (instance);
    }

  return retval;
}

static void
champlain_map_source_factory_class_init (ChamplainMapSourceFactoryClass *klass)
{
  g_type_class_add_private (klass, sizeof (ChamplainMapSourceFactoryPrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = champlain_map_source_factory_constructor;
  object_class->finalize = champlain_map_source_factory_finalize;
  object_class->get_property = champlain_map_source_factory_get_property;
  object_class->set_property = champlain_map_source_factory_set_property;
}

static
ChamplainMapSourceDesc OSM_MAPNIK_DESC =
  {
    CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK,
    "OpenStreetMap Mapnik",
    "Map data is CC-BY-SA 2.0 OpenStreetMap contributors",
    "http://creativecommons.org/licenses/by-sa/2.0/",
    0,
    18,
    CHAMPLAIN_MAP_PROJECTION_MERCATOR,
    champlain_map_source_new_generic,
    "http://tile.openstreetmap.org/#Z#/#X#/#Y#.png",
    NULL
  };

static
ChamplainMapSourceDesc OSM_OSMARENDER_DESC =
  {
    CHAMPLAIN_MAP_SOURCE_OSM_OSMARENDER,
    "OpenStreetMap Osmarender",
    "Map data is CC-BY-SA 2.0 OpenStreetMap contributors",
    "http://creativecommons.org/licenses/by-sa/2.0/",
    0,
    17,
    CHAMPLAIN_MAP_PROJECTION_MERCATOR,
    champlain_map_source_new_generic,
    "http://tah.openstreetmap.org/Tiles/tile/#Z#/#X#/#Y#.png",
    NULL
  };

static
ChamplainMapSourceDesc OSM_CYCLEMAP_DESC =
  {
    CHAMPLAIN_MAP_SOURCE_OSM_CYCLE_MAP,
    "OpenStreetMap Cycle Map",
    "Map data is CC-BY-SA 2.0 OpenStreetMap contributors",
    "http://creativecommons.org/licenses/by-sa/2.0/",
    0,
    18,
    CHAMPLAIN_MAP_PROJECTION_MERCATOR,
    champlain_map_source_new_generic,
    "http://andy.sandbox.cloudmade.com/tiles/cycle/#Z#/#X#/#Y#.png",
    NULL
  };

static
ChamplainMapSourceDesc OSM_TRANSPORTMAP_DESC =
  {
    CHAMPLAIN_MAP_SOURCE_OSM_TRANSPORT_MAP,
    "OpenStreetMap Transport Map",
    "Map data is CC-BY-SA 2.0 OpenStreetMap contributors",
    "http://creativecommons.org/licenses/by-sa/2.0/",
    0,
    18,
    CHAMPLAIN_MAP_PROJECTION_MERCATOR,
    champlain_map_source_new_generic,
    "http://tile.xn--pnvkarte-m4a.de/tilegen/#Z#/#X#/#Y#.png",
    NULL
  };

#if 0
/* Disabling until OpenArealMap works again */
static
ChamplainMapSourceDesc OAM_DESC =
  {
    CHAMPLAIN_MAP_SOURCE_OAM,
    "OpenAerialMap",
    "(CC) BY 3.0 OpenAerialMap contributors",
    "http://creativecommons.org/licenses/by/3.0/",
    0,
    17,
    CHAMPLAIN_MAP_PROJECTION_MERCATOR,
    champlain_map_source_new_generic,
    "http://tile.openaerialmap.org/tiles/1.0.0/openaerialmap-900913/#Z#/#X#/#Y#.jpg",
    NULL
  };
#endif

static
ChamplainMapSourceDesc MFF_RELIEF_DESC =
  {
    CHAMPLAIN_MAP_SOURCE_MFF_RELIEF,
    "Maps for Free Relief",
    "Map data available under GNU Free Documentation license, Version 1.2 or later",
    "http://www.gnu.org/copyleft/fdl.html",
    0,
    11,
    CHAMPLAIN_MAP_PROJECTION_MERCATOR,
    champlain_map_source_new_generic,
    "http://maps-for-free.com/layer/relief/z#Z#/row#Y#/#Z#_#X#-#Y#.jpg",
    NULL
  };

static
ChamplainMapSourceDesc MEMPHIS_LOCAL_DESC =
  {
    CHAMPLAIN_MAP_SOURCE_MEMPHIS_LOCAL,
    "OpenStreetMap Memphis Local Map",
    "(CC) BY 2.0 OpenStreetMap contributors",
    "http://creativecommons.org/licenses/by/2.0/",
    12,
    18,
    CHAMPLAIN_MAP_PROJECTION_MERCATOR,
    champlain_map_source_new_memphis,
    "",
    NULL
  };

static
ChamplainMapSourceDesc MEMPHIS_NETWORK_DESC =
  {
    CHAMPLAIN_MAP_SOURCE_MEMPHIS_NETWORK,
    "OpenStreetMap Memphis Network Map",
    "(CC) BY 2.0 OpenStreetMap contributors",
    "http://creativecommons.org/licenses/by/2.0/",
    12,
    18,
    CHAMPLAIN_MAP_PROJECTION_MERCATOR,
    champlain_map_source_new_memphis,
    "",
    NULL
  };

static void
champlain_map_source_factory_init (ChamplainMapSourceFactory *factory)
{
  ChamplainMapSourceFactoryPrivate *priv = GET_PRIVATE (factory);

  factory->priv = priv;

  priv->registered_sources = NULL;

  champlain_map_source_factory_register (factory, &OSM_MAPNIK_DESC,
      OSM_MAPNIK_DESC.constructor, OSM_MAPNIK_DESC.data);
  champlain_map_source_factory_register (factory, &OSM_CYCLEMAP_DESC,
      OSM_CYCLEMAP_DESC.constructor, OSM_CYCLEMAP_DESC.data);
  champlain_map_source_factory_register (factory, &OSM_TRANSPORTMAP_DESC,
      OSM_TRANSPORTMAP_DESC.constructor, OSM_TRANSPORTMAP_DESC.data);
  champlain_map_source_factory_register (factory, &OSM_OSMARENDER_DESC,
      OSM_OSMARENDER_DESC.constructor, OSM_OSMARENDER_DESC.data);
#if 0
  champlain_map_source_factory_register (factory, &OAM_DESC,
      OAM_DESC.constructor, OAM_DESC.data);
#endif
  champlain_map_source_factory_register (factory, &MFF_RELIEF_DESC,
      MFF_RELIEF_DESC.constructor, MFF_RELIEF_DESC.data);
  champlain_map_source_factory_register (factory, &MEMPHIS_LOCAL_DESC,
      MEMPHIS_LOCAL_DESC.constructor, MEMPHIS_LOCAL_DESC.data);
  champlain_map_source_factory_register (factory, &MEMPHIS_NETWORK_DESC,
      MEMPHIS_NETWORK_DESC.constructor, MEMPHIS_NETWORK_DESC.data);
}

/**
 * champlain_map_source_factory_dup_default:
 *
 * Returns: the singleton #ChamplainMapSourceFactory, it should be freed
 * using #g_object_unref when not needed.
 *
 * Since: 0.4
 */
ChamplainMapSourceFactory *
champlain_map_source_factory_dup_default (void)
{
  return g_object_new (CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY, NULL);
}

/**
 * champlain_map_source_factory_dup_list:
 *
 * Returns: the list of registered map sources, the items should not be freed,
 * the list should be freed with #g_slist_free.
 *
 * Since: 0.4
 */
GSList *
champlain_map_source_factory_dup_list (ChamplainMapSourceFactory *factory)
{
  return g_slist_copy (factory->priv->registered_sources);
}

/**
 * champlain_map_source_factory_create:
 * @factory: the Factory
 * @id: the wanted map source id
 *
 * Returns: a ready to use #ChamplainMapSource matching the given name, returns
 * NULL is none match.
 *
 * The id should not contain any character that can't be in a filename as it
 * will be used as the cache directory name for that map source.
 *
 * Since: 0.4
 */
ChamplainMapSource *
champlain_map_source_factory_create (ChamplainMapSourceFactory *factory,
    const gchar *id)
{
//  ChamplainMapSource *map_source = NULL;
//  ChamplainMapSourceChain *source_chain;
//  ChamplainMapSource *source;
  GSList *item;
//  guint tile_size;
//  gchar *cache_path;

  item = factory->priv->registered_sources;

  while (item != NULL)
    {
      ChamplainMapSourceDesc *desc = CHAMPLAIN_MAP_SOURCE_DESC (item->data);
      if (strcmp (desc->id, id) == 0)
        return desc->constructor (desc, desc->data); //map_source = desc->constructor (desc, desc->data);
      item = g_slist_next (item);
    }

//  if (!map_source)
     return NULL;

/*  source_chain = champlain_map_source_chain_new ();

  tile_size = champlain_map_source_get_tile_size(map_source);
  source = CHAMPLAIN_MAP_SOURCE(champlain_error_tile_source_new_full (tile_size));

  champlain_map_source_chain_push_map_source(source_chain, source);
  champlain_map_source_chain_push_map_source(source_chain, map_source);

  cache_path = g_build_path (G_DIR_SEPARATOR_S, g_get_user_cache_dir (), "champlain", NULL);
  source = CHAMPLAIN_MAP_SOURCE(champlain_file_cache_new_full (100000000, cache_path, TRUE));
  g_free(cache_path);

  champlain_map_source_chain_push_map_source(source_chain, source);

  return CHAMPLAIN_MAP_SOURCE(source_chain);*/
}

/**
 * champlain_map_source_factory_register:
 * @factory: A #ChamplainMapSourceFactory
 * @desc: the description of the map source
 * @constructor: the new map source constructor function
 * @data: data to be passed to the constructor function, or NULL
 *
 * Registers the new map source with the given constructor.  When this map
 * source is requested, the given constructor will be used to build the
 * map source.  #ChamplainMapSourceFactory will take ownership of the passed
 * #ChamplainMapSourceDesc, so don't free it. They will not be freed either so
 * you can use static structs here.
 *
 * Returns: TRUE if the registration suceeded.
 *
 * Since: 0.4
 */
gboolean
champlain_map_source_factory_register (ChamplainMapSourceFactory *factory,
    ChamplainMapSourceDesc *desc, ChamplainMapSourceConstructor constructor,
    gpointer data)
{

  /* FIXME: check for existing factory with that name? */
  desc->constructor = constructor;
  desc->data = data;
  factory->priv->registered_sources = g_slist_append (factory->priv->registered_sources, desc);
  return TRUE;
}

static ChamplainMapSource *
champlain_map_source_new_generic (
     ChamplainMapSourceDesc *desc, gpointer user_data)
{
  return CHAMPLAIN_MAP_SOURCE (champlain_network_tile_source_new_full (
      desc->id,
      desc->name,
      desc->license,
      desc->license_uri,
      desc->min_zoom_level,
      desc->max_zoom_level,
      256,
      desc->projection,
      desc->uri_format));
}

static ChamplainMapSource *
champlain_map_source_new_memphis (ChamplainMapSourceDesc *desc,
    gpointer user_data)
{
  ChamplainMapDataSource *map_data_source;

  if (g_strcmp0 (desc->id, CHAMPLAIN_MAP_SOURCE_MEMPHIS_LOCAL) == 0)
    {
      map_data_source = CHAMPLAIN_MAP_DATA_SOURCE (champlain_local_map_data_source_new ());

      /* Abuse the uri_format field to store an initial data path (optional) */
      if (desc->uri_format && g_strcmp0 (desc->uri_format, "") != 0)
        champlain_local_map_data_source_load_map_data (
            CHAMPLAIN_LOCAL_MAP_DATA_SOURCE (map_data_source),
            desc->uri_format);
    }
  else if (g_strcmp0 (desc->id, CHAMPLAIN_MAP_SOURCE_MEMPHIS_NETWORK) == 0)
      map_data_source = CHAMPLAIN_MAP_DATA_SOURCE (champlain_network_map_data_source_new ());
  else
    return NULL;

  return CHAMPLAIN_MAP_SOURCE (champlain_memphis_map_source_new_full (
      desc->id,
      desc->name,
      desc->license,
      desc->license_uri,
      desc->min_zoom_level,
      desc->max_zoom_level,
      256,
      desc->projection,
      map_data_source));
}
