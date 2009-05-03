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

ChamplainMapSourceFactory * champlain_map_source_factory_get_default (void);

gchar ** champlain_map_source_factory_get_list (ChamplainMapSourceFactory *factory);

ChamplainMapSource * champlain_map_source_factory_create (ChamplainMapSourceFactory *factory,
    const gchar *id);

/**
 * ChamplainMapSourceConstructor:
 *
 * A #ChamplainMapSource constructor.  It should return a ready to use
 * #ChamplainMapSource.
 *
 * Since: 0.4
 */
typedef ChamplainMapSource * (*ChamplainMapSourceConstructor) ();
#define CHAMPLAIN_MAP_SOURCE_CONSTRUCTOR (f) ((ChamplainMapSourceConstructor) (f))

gboolean champlain_map_source_factory_register (ChamplainMapSourceFactory *factory,
    const gchar *id,
    ChamplainMapSourceConstructor callback);

#define CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK "OpenStreetMap Mapnik"
#define CHAMPLAIN_MAP_SOURCE_OSM_OSMARENDER "OpenStreetMap Osmarender"
#define CHAMPLAIN_MAP_SOURCE_OSM_CYCLEMAP "OpenStreetMap CycleMap"
#define CHAMPLAIN_MAP_SOURCE_OAM "OpenAerialMap"
#define CHAMPLAIN_MAP_SOURCE_MFF_RELIEF "MapsForFree Relief"

G_END_DECLS

#endif
