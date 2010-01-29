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

#if !defined (__CHAMPLAIN_CHAMPLAIN_H_INSIDE__) && !defined (CHAMPLAIN_COMPILATION)
#error "Only <champlain/champlain.h> can be included directly."
#endif

#ifndef CHAMPLAIN_MAP_SOURCE_FACTORY_H
#define CHAMPLAIN_MAP_SOURCE_FACTORY_H

#include <champlain/champlain-defines.h>
#include <champlain/champlain-map-source.h>
#include <champlain/champlain-map-source-desc.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY     (champlain_map_source_factory_get_type())
#define CHAMPLAIN_MAP_SOURCE_FACTORY(obj)     (G_TYPE_CHECK_INSTANCE_CAST((obj), CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY, ChamplainMapSourceFactory))
#define CHAMPLAIN_MAP_SOURCE_FACTORY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY, ChamplainMapSourceFactoryClass))
#define CHAMPLAIN_IS_MAP_SOURCE_FACTORY(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY))
#define CHAMPLAIN_IS_MAP_SOURCE_FACTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY))
#define CHAMPLAIN_MAP_SOURCE_FACTORY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  CHAMPLAIN_TYPE_MAP_SOURCE_FACTORY, ChamplainMapSourceFactoryClass))

typedef struct _ChamplainMapSourceFactory ChamplainMapSourceFactory;
typedef struct _ChamplainMapSourceFactoryClass ChamplainMapSourceFactoryClass;
typedef struct _ChamplainMapSourceFactoryPrivate ChamplainMapSourceFactoryPrivate;

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

ChamplainMapSourceFactory * champlain_map_source_factory_dup_default (void);

GSList * champlain_map_source_factory_dup_list (ChamplainMapSourceFactory *factory);

ChamplainMapSource * champlain_map_source_factory_create (ChamplainMapSourceFactory *factory,
    const gchar *id);

gboolean
champlain_map_source_factory_register (ChamplainMapSourceFactory *factory,
    ChamplainMapSourceDesc *desc, ChamplainMapSourceConstructor constructor,
    gpointer data);

#ifdef USE_MAEMO
#define CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK "OpenStreetMap I"
#define CHAMPLAIN_MAP_SOURCE_OSM_OSMARENDER "OpenStreetMap II"
#define CHAMPLAIN_MAP_SOURCE_OSM_CYCLE_MAP "OpenCycleMap"
#define CHAMPLAIN_MAP_SOURCE_OSM_TRANSPORT_MAP "Public Transport"
#define CHAMPLAIN_MAP_SOURCE_OAM "OpenAerialMap"
#define CHAMPLAIN_MAP_SOURCE_MFF_RELIEF "MapsForFree Relief"
#else
#define CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK "osm-mapnik"
#define CHAMPLAIN_MAP_SOURCE_OSM_OSMARENDER "osm-osmarender"
#define CHAMPLAIN_MAP_SOURCE_OSM_CYCLE_MAP "osm-cyclemap"
#define CHAMPLAIN_MAP_SOURCE_OSM_TRANSPORT_MAP "osm-transportmap"
#define CHAMPLAIN_MAP_SOURCE_OAM "oam"
#define CHAMPLAIN_MAP_SOURCE_MFF_RELIEF "mff-relief"
#endif

G_END_DECLS

#endif
