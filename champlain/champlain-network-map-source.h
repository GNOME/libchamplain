/*
 * Copyright (C) 2008-2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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

#ifndef CHAMPLAIN_NETWORK_MAP_SOURCE_H
#define CHAMPLAIN_NETWORK_MAP_SOURCE_H

#include <champlain/champlain-defines.h>
#include <champlain/champlain-map-source.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_NETWORK_MAP_SOURCE     (champlain_network_map_source_get_type())
#define CHAMPLAIN_NETWORK_MAP_SOURCE(obj)     (G_TYPE_CHECK_INSTANCE_CAST((obj), CHAMPLAIN_TYPE_NETWORK_MAP_SOURCE, ChamplainNetworkMapSource))
#define CHAMPLAIN_NETWORK_MAP_SOURCE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  CHAMPLAIN_TYPE_NETWORK_MAP_SOURCE, ChamplainNetworkMapSourceClass))
#define CHAMPLAIN_IS_NETWORK_MAP_SOURCE(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), CHAMPLAIN_TYPE_NETWORK_MAP_SOURCE))
#define CHAMPLAIN_IS_NETWORK_MAP_SOURCE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  CHAMPLAIN_TYPE_NETWORK_MAP_SOURCE))
#define CHAMPLAIN_NETWORK_MAP_SOURCE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  CHAMPLAIN_TYPE_NETWORK_MAP_SOURCE, ChamplainNetworkMapSourceClass))

typedef struct _ChamplainNetworkMapSource ChamplainNetworkMapSource;
typedef struct _ChamplainNetworkMapSourceClass ChamplainNetworkMapSourceClass;
typedef struct _ChamplainNetworkMapSourcePrivate ChamplainNetworkMapSourcePrivate;

struct _ChamplainNetworkMapSource
{
  ChamplainMapSource parent;
  ChamplainNetworkMapSourcePrivate *priv;
};

struct _ChamplainNetworkMapSourceClass
{
  ChamplainMapSourceClass parent_class;
};

GType champlain_network_map_source_get_type (void);

ChamplainNetworkMapSource* champlain_network_map_source_new_full (const gchar *id,
    const gchar *name,
    const gchar *license,
    const gchar *license_uri,
    guint min_zoom,
    guint max_zoom,
    guint tile_size,
    ChamplainMapProjection projection,
    const gchar *uri_format);

gchar * champlain_network_map_source_get_tile_uri (ChamplainNetworkMapSource *source,
    gint x,
    gint y,
    gint z);

void champlain_network_map_source_set_uri_format (ChamplainNetworkMapSource *source,
    const gchar *uri_format);

void champlain_network_map_source_fill_tile (ChamplainMapSource *map_source,
    ChamplainTile *tile) G_GNUC_DEPRECATED;

G_END_DECLS

#endif
