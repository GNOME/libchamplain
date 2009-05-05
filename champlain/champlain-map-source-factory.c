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
 * #champlain_map_source_factory_get_list.
 * 
 */
#include "config.h"

#include "champlain-map-source-factory.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_NETWORK
#include "champlain-debug.h"

#include "champlain.h"
#include "champlain-cache.h"
#include "champlain-defines.h"
#include "champlain-enum-types.h"
#include "champlain-map-source.h"
#include "champlain-marshal.h"
#include "champlain-private.h"
#include "champlain-zoom-level.h"

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

G_DEFINE_TYPE (ChamplainMapSourceFactory, champlain_map_source_factory, G_TYPE_OBJECT);

#define GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY, ChamplainMapSourceFactoryPrivate))

struct _ChamplainMapSourceFactoryPrivate
{
  GSList *registered_sources;
};

static ChamplainMapSource * champlain_map_source_new_osm_mapnik (void);
static ChamplainMapSource * champlain_map_source_new_osm_cyclemap (void);
static ChamplainMapSource * champlain_map_source_new_osm_osmarender (void);
static ChamplainMapSource * champlain_map_source_new_oam (void);
static ChamplainMapSource * champlain_map_source_new_mff_relief (void);


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

static void
champlain_map_source_factory_class_init (ChamplainMapSourceFactoryClass *klass)
{
  g_type_class_add_private (klass, sizeof (ChamplainMapSourceFactoryPrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = champlain_map_source_factory_finalize;
  object_class->get_property = champlain_map_source_factory_get_property;
  object_class->set_property = champlain_map_source_factory_set_property;
}

static
ChamplainMapSourceDesc OSM_MAPNIK_DESC =
  {
    CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK,
    "OpenStreetMap Mapnik",
    "(CC) BY 2.0 OpenStreetMap contributor",
    "http://creativecommons.org/licenses/by/2.0/",
    0,
    18,
    CHAMPLAIN_MAP_PROJECTION_MERCATOR,
    champlain_map_source_new_osm_mapnik
  };

static
ChamplainMapSourceDesc OSM_OSMARENDER_DESC =
  {
    CHAMPLAIN_MAP_SOURCE_OSM_OSMARENDER,
    "OpenStreetMap Osmarender",
    "(CC) BY 2.0 OpenStreetMap contributor",
    "http://creativecommons.org/licenses/by/2.0/",
    0,
    18,
    CHAMPLAIN_MAP_PROJECTION_MERCATOR,
    champlain_map_source_new_osm_osmarender
  };

static
ChamplainMapSourceDesc OSM_CYCLEMAP_DESC =
  {
    CHAMPLAIN_MAP_SOURCE_OSM_CYCLE_MAP,
    "OpenStreetMap Cycle Map",
    "(CC) BY 2.0 OpenStreetMap contributor",
    "http://creativecommons.org/licenses/by/2.0/",
    0,
    18,
    CHAMPLAIN_MAP_PROJECTION_MERCATOR,
    champlain_map_source_new_osm_cyclemap
  };

static
ChamplainMapSourceDesc OAM_DESC =
  {
    CHAMPLAIN_MAP_SOURCE_OAM,
    "OpenAerialMap",
    "(CC) BY 3.0 OpenAerialMap contributor",
    "http://creativecommons.org/licenses/by/3.0/",
    0,
    17,
    CHAMPLAIN_MAP_PROJECTION_MERCATOR,
    champlain_map_source_new_oam
  };

static
ChamplainMapSourceDesc MFF_RELIEF_DESC =
  {
    CHAMPLAIN_MAP_SOURCE_MFF_RELIEF,
    "Maps For Free Relief",
    "GNU Free Documentation Licence, version 1.2 or later",
    "http://www.gnu.org/copyleft/fdl.html",
    0,
    11,
    CHAMPLAIN_MAP_PROJECTION_MERCATOR,
    champlain_map_source_new_mff_relief
  };

static void
champlain_map_source_factory_init (ChamplainMapSourceFactory *factory)
{
  ChamplainMapSourceFactoryPrivate *priv = GET_PRIVATE (factory);

  factory->priv = priv;

  priv->registered_sources = NULL;

  champlain_map_source_factory_register (factory, &OSM_MAPNIK_DESC);
  champlain_map_source_factory_register (factory, &OSM_CYCLEMAP_DESC);
  champlain_map_source_factory_register (factory, &OSM_OSMARENDER_DESC);
  champlain_map_source_factory_register (factory, &OAM_DESC);
  champlain_map_source_factory_register (factory, &MFF_RELIEF_DESC);
}

/**
 * champlain_map_source_factory_get_default:
 *
 * Returns the singleton #ChamplainMapSourceFactory
 *
 * Since: 0.4
 */
ChamplainMapSourceFactory *
champlain_map_source_factory_get_default (void)
{
  static ChamplainMapSourceFactory *instance = NULL;

  if (G_UNLIKELY (instance == NULL))
    {
      instance = g_object_new (CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY, NULL);
      return instance;
    }

  return g_object_ref (instance);
}

/**
 * champlain_map_source_factory_get_list:
 *
 * Returns the list of registered map sources, the items should not be freed,
 * the list should be freed with #g_slist_free.
 *
 * Since: 0.4
 */
GSList *
champlain_map_source_factory_get_list (ChamplainMapSourceFactory *factory)
{
  return g_slist_copy (factory->priv->registered_sources);
}

/**
 * champlain_map_source_factory_create:
 * @factory: the Factory
 * @id: the wanted map source id
 *
 * Returns a ready to use #ChamplainMapSource matching the given name, returns
 * NULL is none match.
 *
 * Since: 0.4
 */
ChamplainMapSource *
champlain_map_source_factory_create (ChamplainMapSourceFactory *factory,
    const gchar *id)
{
  GSList *item;

  item = factory->priv->registered_sources;

  while (item != NULL)
    {
      ChamplainMapSourceDesc *desc = (ChamplainMapSourceDesc*) item->data;
      if (strcmp (desc->id, id) == 0)
        return desc->constructor ();
      item = g_slist_next (item);
    }
  return NULL;
}

/**
 * champlain_map_source_factory_register:
 * @factory: the Factory
 * @name: the new map source name
 * @constructor: the new map source constructor function
 *
 * Registers the new map source with the given constructor.  When this map
 * source is requested, the given constructor will be used to build the
 * map source.  #ChamplainMapSourceFactory will take ownership of the passed
 * #ChamplainMapSourceDesc, so don't free it. They will not be freed either so
 * you can use static structs here.
 *
 * Returns TRUE if the registration suceeded.
 *
 * Since: 0.4
 */
gboolean
champlain_map_source_factory_register (ChamplainMapSourceFactory *factory,
    ChamplainMapSourceDesc *desc)
{

  /* FIXME: check for existing factory with that name? */
  factory->priv->registered_sources = g_slist_append (factory->priv->registered_sources, desc);
  return TRUE;
}

static ChamplainMapSource *
champlain_map_source_new_osm_cyclemap (void)
{
  return CHAMPLAIN_MAP_SOURCE (champlain_network_map_source_new_full (CHAMPLAIN_MAP_SOURCE_OSM_CYCLE_MAP,
      "OpenStreetMap Cycle Map",
      "(CC) BY 2.0 OpenStreetMap contributors",
      "http://creativecommons.org/licenses/by/2.0/", 0, 18, 256,
      CHAMPLAIN_MAP_PROJECTION_MERCATOR,
      "http://andy.sandbox.cloudmade.com/tiles/cycle/#Z#/#X#/#Y#.png"));
}

static ChamplainMapSource *
champlain_map_source_new_osm_osmarender (void)
{
  return CHAMPLAIN_MAP_SOURCE (champlain_network_map_source_new_full (CHAMPLAIN_MAP_SOURCE_OSM_OSMARENDER,
      "OpenStreetMap Osmarender",
      "(CC) BY 2.0 OpenStreetMap contributors",
      "http://creativecommons.org/licenses/by/2.0/", 0, 18, 256,
      CHAMPLAIN_MAP_PROJECTION_MERCATOR,
      "http://tah.openstreetmap.org/Tiles/tile/#Z#/#X#/#Y#.png"));
}

static ChamplainMapSource *
champlain_map_source_new_osm_mapnik (void)
{
  return CHAMPLAIN_MAP_SOURCE (champlain_network_map_source_new_full (CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK,
      "OpenStreetMap Mapnik",
      "(CC) BY 2.0 OpenStreetMap contributors",
      "http://creativecommons.org/licenses/by/2.0/", 0, 18, 256,
      CHAMPLAIN_MAP_PROJECTION_MERCATOR,
      "http://tile.openstreetmap.org/#Z#/#X#/#Y#.png"));
}

static ChamplainMapSource *
champlain_map_source_new_oam (void)
{
  return CHAMPLAIN_MAP_SOURCE (champlain_network_map_source_new_full (CHAMPLAIN_MAP_SOURCE_OAM,
      "OpenAerialMap",
      "(CC) BY 3.0 OpenAerialMap contributors",
      "http://creativecommons.org/licenses/by/3.0/", 0, 17, 256,
      CHAMPLAIN_MAP_PROJECTION_MERCATOR,
      "http://tile.openaerialmap.org/tiles/1.0.0/openaerialmap-900913/#Z#/#X#/#Y#.jpg"));
}

static ChamplainMapSource *
champlain_map_source_new_mff_relief (void)
{
  return CHAMPLAIN_MAP_SOURCE (champlain_network_map_source_new_full (CHAMPLAIN_MAP_SOURCE_MFF_RELIEF,
      "Maps for Free Relief",
      "Map data available under GNU Free Documentation license, Version 1.2 or later",
      "http://www.gnu.org/copyleft/fdl.html", 0, 11, 256,
      CHAMPLAIN_MAP_PROJECTION_MERCATOR,
      "http://maps-for-free.com/layer/relief/z#Z#/row#Y#/#Z#_#X#-#Y#.jpg"));
} 
