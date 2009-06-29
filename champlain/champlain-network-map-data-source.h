/*
 * Copyright (C) 2009 Simon Wenner <simon@wenner.ch>
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

#ifndef _CHAMPLAIN_NETWORK_MAP_DATA_SOURCE
#define _CHAMPLAIN_NETWORK_MAP_DATA_SOURCE

#include <glib-object.h>

#include <champlain/champlain-map-data-source.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_NETWORK_MAP_DATA_SOURCE champlain_network_map_data_source_get_type()

#define CHAMPLAIN_NETWORK_MAP_DATA_SOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_NETWORK_MAP_DATA_SOURCE, ChamplainNetworkMapDataSource))

#define CHAMPLAIN_NETWORK_MAP_DATA_SOURCE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_NETWORK_MAP_DATA_SOURCE, ChamplainNetworkMapDataSourceClass))

#define CHAMPLAIN_IS_NETWORK_MAP_DATA_SOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_NETWORK_MAP_DATA_SOURCE))

#define CHAMPLAIN_IS_NETWORK_MAP_DATA_SOURCE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_NETWORK_MAP_DATA_SOURCE))

#define CHAMPLAIN_NETWORK_MAP_DATA_SOURCE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_NETWORK_MAP_DATA_SOURCE, ChamplainNetworkMapDataSourceClass))

typedef struct {
  ChamplainMapDataSource parent;
} ChamplainNetworkMapDataSource;

typedef struct {
  ChamplainMapDataSourceClass parent_class;
} ChamplainNetworkMapDataSourceClass;

GType champlain_network_map_data_source_get_type (void);

ChamplainNetworkMapDataSource* champlain_network_map_data_source_new (void);

void champlain_network_map_data_source_load_map_data (
    ChamplainNetworkMapDataSource *map_data_source,
    gdouble bound_left,
    gdouble bound_bottom,
    gdouble bound_right,
    gdouble bound_top);

gchar * champlain_network_map_data_source_get_api_uri (
    ChamplainNetworkMapDataSource *map_data_source);

void champlain_network_map_data_source_set_api_uri (
    ChamplainNetworkMapDataSource *map_data_source,
    gchar *api_uri);

G_END_DECLS

#endif /* _CHAMPLAIN_NETWORK_MAP_DATA_SOURCE */
