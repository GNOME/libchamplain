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

#ifndef _CHAMPLAIN_MEMPHIS_MAP_SOURCE
#define _CHAMPLAIN_MEMPHIS_MAP_SOURCE

#include <champlain/champlain-map-data-source.h>
#include <champlain/champlain-map-source.h>
#include <champlain/champlain-map-source-desc.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_MEMPHIS_MAP_SOURCE champlain_memphis_map_source_get_type()

#define CHAMPLAIN_MEMPHIS_MAP_SOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_MEMPHIS_MAP_SOURCE, ChamplainMemphisMapSource))

#define CHAMPLAIN_MEMPHIS_MAP_SOURCE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_MEMPHIS_MAP_SOURCE, ChamplainMemphisMapSourceClass))

#define CHAMPLAIN_IS_MEMPHIS_MAP_SOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_MEMPHIS_MAP_SOURCE))

#define CHAMPLAIN_IS_MEMPHIS_MAP_SOURCE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_MEMPHIS_MAP_SOURCE))

#define CHAMPLAIN_MEMPHIS_MAP_SOURCE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_MEMPHIS_MAP_SOURCE, ChamplainMemphisMapSourceClass))

typedef struct {
  ChamplainMapSource parent;
} ChamplainMemphisMapSource;

typedef struct {
  ChamplainMapSourceClass parent_class;
} ChamplainMemphisMapSourceClass;

GType champlain_memphis_map_source_get_type (void);

ChamplainMemphisMapSource * champlain_memphis_map_source_new_full (
    ChamplainMapSourceDesc *desc,
    ChamplainMapDataSource *map_data_source);

void champlain_memphis_map_source_set_tile_size (
    ChamplainMemphisMapSource *map_source,
    guint size);

void champlain_memphis_map_source_load_rules (
    ChamplainMemphisMapSource *map_source,
    const gchar *rules_path);

void champlain_memphis_map_source_set_map_data_source (
    ChamplainMemphisMapSource *map_source,
    ChamplainMapDataSource *map_data_source);

ChamplainMapDataSource * champlain_memphis_map_source_get_map_data_source (
    ChamplainMemphisMapSource *map_source);

void champlain_memphis_map_source_delete_session_cache (
    ChamplainMemphisMapSource *map_source);

G_END_DECLS

#endif /* _CHAMPLAIN_MEMPHIS_MAP_SOURCE */
