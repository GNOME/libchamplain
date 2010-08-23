/*
 * Copyright (C) 2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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

#if !defined (__CHAMPLAIN_CHAMPLAIN_H_INSIDE__) && !defined (CHAMPLAIN_COMPILATION)
#error "Only <champlain/champlain.h> can be included directly."
#endif

#ifndef CHAMPLAIN_MAP_SOURCE_FACTORY_H
#define CHAMPLAIN_MAP_SOURCE_FACTORY_H

#include <champlain/champlain-features.h>
#include <champlain/champlain-defines.h>
#include <champlain/champlain-map-source.h>
#include <champlain/champlain-map-source-desc.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY champlain_map_source_factory_get_type ()

#define CHAMPLAIN_MAP_SOURCE_FACTORY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY, ChamplainMapSourceFactory))

#define CHAMPLAIN_MAP_SOURCE_FACTORY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY, ChamplainMapSourceFactoryClass))

#define CHAMPLAIN_IS_MAP_SOURCE_FACTORY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY))

#define CHAMPLAIN_IS_MAP_SOURCE_FACTORY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY))

#define CHAMPLAIN_MAP_SOURCE_FACTORY_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY, ChamplainMapSourceFactoryClass))

typedef struct _ChamplainMapSourceFactoryPrivate ChamplainMapSourceFactoryPrivate;

typedef struct _ChamplainMapSourceFactory ChamplainMapSourceFactory;
typedef struct _ChamplainMapSourceFactoryClass ChamplainMapSourceFactoryClass;

struct _ChamplainMapSourceFactory
{
  GObject parent;
  ChamplainMapSourceFactoryPrivate *priv;
};

struct _ChamplainMapSourceFactoryClass
{
  GObjectClass parent_class;
};

GType champlain_map_source_factory_get_type (void);

ChamplainMapSourceFactory *champlain_map_source_factory_dup_default (void);

GSList *champlain_map_source_factory_dup_list (ChamplainMapSourceFactory *factory);

ChamplainMapSource *champlain_map_source_factory_create (ChamplainMapSourceFactory *factory,
    const gchar *id);
ChamplainMapSource *champlain_map_source_factory_create_cached_source (ChamplainMapSourceFactory *factory,
    const gchar *id);
ChamplainMapSource *champlain_map_source_factory_create_error_source (ChamplainMapSourceFactory *factory,
    guint tile_size);

gboolean champlain_map_source_factory_register (ChamplainMapSourceFactory *factory,
    ChamplainMapSourceDesc *desc,
    ChamplainMapSourceConstructor constructor,
    gpointer data);

#ifndef CHAMPLAIN_HAS_MAEMO
/**
 * CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK:
 *
 * OpenStreetMap Mapnik
 */
#define CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK "osm-mapnik"
/**
 * CHAMPLAIN_MAP_SOURCE_OSM_OSMARENDER:
 *
 * OpenStreetMap Osmarender
 */
#define CHAMPLAIN_MAP_SOURCE_OSM_OSMARENDER "osm-osmarender"
/**
 * CHAMPLAIN_MAP_SOURCE_OSM_CYCLE_MAP:
 *
 * OpenStreetMap Cycle Map
 */
#define CHAMPLAIN_MAP_SOURCE_OSM_CYCLE_MAP "osm-cyclemap"
/**
 * CHAMPLAIN_MAP_SOURCE_OSM_TRANSPORT_MAP:
 *
 * OpenStreetMap Transport Map
 */
#define CHAMPLAIN_MAP_SOURCE_OSM_TRANSPORT_MAP "osm-transportmap"
/**
 * CHAMPLAIN_MAP_SOURCE_OAM:
 *
 * OpenAerialMap
 */
#define CHAMPLAIN_MAP_SOURCE_OAM "OpenAerialMap"
/**
 * CHAMPLAIN_MAP_SOURCE_MFF_RELIEF:
 *
 * Maps for Free Relief
 */
#define CHAMPLAIN_MAP_SOURCE_MFF_RELIEF "mff-relief"
#else
#define CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK "OpenStreetMap I"
#define CHAMPLAIN_MAP_SOURCE_OSM_OSMARENDER "OpenStreetMap II"
#define CHAMPLAIN_MAP_SOURCE_OSM_CYCLE_MAP "OpenCycleMap"
#define CHAMPLAIN_MAP_SOURCE_OSM_TRANSPORT_MAP "Public Transport"
#define CHAMPLAIN_MAP_SOURCE_OAM "OpenAerialMap"
#define CHAMPLAIN_MAP_SOURCE_MFF_RELIEF "MapsForFree Relief"
#endif

#ifdef CHAMPLAIN_HAS_MEMPHIS
/**
 * CHAMPLAIN_MAP_SOURCE_MEMPHIS_LOCAL:
 *
 * OpenStreetMap Memphis Local Map
 */
#define CHAMPLAIN_MAP_SOURCE_MEMPHIS_LOCAL "memphis-local"
/**
 * CHAMPLAIN_MAP_SOURCE_MEMPHIS_NETWORK:
 *
 * OpenStreetMap Memphis Network Map
 */
#define CHAMPLAIN_MAP_SOURCE_MEMPHIS_NETWORK "memphis-network"
#endif

G_END_DECLS

#endif
