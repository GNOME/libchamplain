/*
 * Copyright (C) 2019 Daniel Burgess <dnl@tuta.io>
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

#ifndef _CHAMPLAIN_NETWORK_WMS_TILE_SOURCE_H_
#define _CHAMPLAIN_NETWORK_WMS_TILE_SOURCE_H_

#include <champlain/champlain-defines.h>
#include <champlain/champlain-tile-source.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_NETWORK_WMS_TILE_SOURCE champlain_network_wms_tile_source_get_type ()

typedef struct _ChamplainNetworkWmsTileSourcePrivate ChamplainNetworkWmsTileSourcePrivate;
typedef struct _ChamplainNetworkWmsTileSource ChamplainNetworkWmsTileSource;
typedef struct _ChamplainNetworkWmsTileSourceClass ChamplainNetworkWmsTileSourceClass;

/**
 * ChamplainNetworkWmsTileSource:
 * 
 * The #ChamplainNetworkWmsTileSource structure contains only private data and
 * should be accessed using the provided API
 */
struct _ChamplainNetworkWmsTileSource
{
  ChamplainTileSource parent_instance;

  ChamplainNetworkWmsTileSourcePrivate *priv;
};

struct _ChamplainNetworkWmsTileSourceClass
{
  ChamplainTileSourceClass parent_class;
};

GType champlain_network_wms_tile_source_get_type(void);

ChamplainNetworkWmsTileSource *champlain_network_wms_tile_source_new_full(
    const gchar *id,
    const gchar *name;
    const gchar *license_uri,
    guint min_zoom,
    guint max_zoom,
    ChamplainMapProjection projection,
    const gchar *uri_format,
    ChamplainRenderer *renderer);

const gchar *champlain_network_wms_tile_source_get_uri_format(ChamplainNetworkWmsTileSource *tile_source);
void champlain_network_wms_tile_source_set_uri_format(ChamplainNetworkWmsTileSource *tile_source,
    const gchar *uri_format);

gboolean champlain_network_wms_tile_source_get_offline (ChamplainNetworkWmsTileSource *tile_source);
void champlain_network_wms_tile_source_set_offline (ChamplainNetworkWmsTileSource *tile_source,
    gboolean offline);

const gchar *champlain_network_wms_tile_source_get_proxy_uri (ChamplainNetworkWmsTileSource *tile_source);
void champlain_network_wms_tile_source_set_proxy_uri (ChamplainNetworkWmsTileSource *tile_source,
    const gchar *proxy_uri);

gint champlain_network_wms_tile_source_get_max_conns (ChamplainNetworkWmsTileSource *tile_source);
void champlain_network_wms_tile_source_set_max_conns (ChamplainNetworkWmsTileSource *tile_source,
    gint max_conns);

void champlain_network_wms_tile_source_set_user_agent (ChamplainNetworkWmsTileSource *tile_source,
    const gchar *user_agent);

G_END_DECLS

#endif  /* _CHAMPLAIN_NETWORK_WMS_TILE_SOURCE_H_ */

