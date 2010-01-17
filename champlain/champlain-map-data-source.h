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

#ifndef _CHAMPLAIN_MAP_DATA_SOURCE
#define _CHAMPLAIN_MAP_DATA_SOURCE

#include <glib-object.h>
#include <memphis/memphis.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_MAP_DATA_SOURCE champlain_map_data_source_get_type()

#define CHAMPLAIN_MAP_DATA_SOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_MAP_DATA_SOURCE, ChamplainMapDataSource))

#define CHAMPLAIN_MAP_DATA_SOURCE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_MAP_DATA_SOURCE, ChamplainMapDataSourceClass))

#define CHAMPLAIN_IS_MAP_DATA_SOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_MAP_DATA_SOURCE))

#define CHAMPLAIN_IS_MAP_DATA_SOURCE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_MAP_DATA_SOURCE))

#define CHAMPLAIN_MAP_DATA_SOURCE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_MAP_DATA_SOURCE, ChamplainMapDataSourceClass))

typedef struct {
  GInitiallyUnowned parent;
} ChamplainMapDataSource;

typedef struct {
  GInitiallyUnownedClass parent_class;
  MemphisMap* (*get_map_data) (ChamplainMapDataSource *data_source);
} ChamplainMapDataSourceClass;

GType champlain_map_data_source_get_type (void);

MemphisMap * champlain_map_data_source_get_map_data (ChamplainMapDataSource *data_source);

G_END_DECLS

#endif /* _CHAMPLAIN_MAP_DATA_SOURCE */
