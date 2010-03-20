/*
 * Copyright (C) 2009 Simon Wenner <simon@wenner.ch>
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

#ifndef _CHAMPLAIN_MEMPHIS_TILE_SOURCE
#define _CHAMPLAIN_MEMPHIS_TILE_SOURCE

#include <champlain/champlain-tile-source.h>
#include <champlain/champlain-map-data-source.h>
#include <memphis/memphis.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_MEMPHIS_TILE_SOURCE champlain_memphis_tile_source_get_type()

#define CHAMPLAIN_MEMPHIS_TILE_SOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_MEMPHIS_TILE_SOURCE, ChamplainMemphisTileSource))

#define CHAMPLAIN_MEMPHIS_TILE_SOURCE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_MEMPHIS_TILE_SOURCE, ChamplainMemphisTileSourceClass))

#define CHAMPLAIN_IS_MEMPHIS_TILE_SOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_MEMPHIS_TILE_SOURCE))

#define CHAMPLAIN_IS_MEMPHIS_TILE_SOURCE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_MEMPHIS_TILE_SOURCE))

#define CHAMPLAIN_MEMPHIS_TILE_SOURCE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_MEMPHIS_TILE_SOURCE, ChamplainMemphisTileSourceClass))

typedef struct _ChamplainMemphisTileSourcePrivate ChamplainMemphisTileSourcePrivate;

typedef struct {
  ChamplainTileSource parent;

  ChamplainMemphisTileSourcePrivate *priv;
} ChamplainMemphisTileSource;

typedef struct {
  ChamplainTileSourceClass parent_class;
} ChamplainMemphisTileSourceClass;

GType champlain_memphis_tile_source_get_type (void);

ChamplainMemphisTileSource* champlain_memphis_tile_source_new_full (const gchar *id,
    const gchar *name,
    const gchar *license,
    const gchar *license_uri,
    guint min_zoom,
    guint max_zoom,
    guint tile_size,
    ChamplainMapProjection projection,
    ChamplainMapDataSource *map_data_source);

void champlain_memphis_tile_source_load_rules (
    ChamplainMemphisTileSource *tile_source,
    const gchar *rules_path);

void champlain_memphis_tile_source_set_map_data_source (
    ChamplainMemphisTileSource *tile_source,
    ChamplainMapDataSource *map_data_source);

ChamplainMapDataSource * champlain_memphis_tile_source_get_map_data_source (
    ChamplainMemphisTileSource *tile_source);

ClutterColor * champlain_memphis_tile_source_get_background_color (
    ChamplainMemphisTileSource *tile_source);

void champlain_memphis_tile_source_set_background_color (
    ChamplainMemphisTileSource *tile_source,
    const ClutterColor *color);

GList * champlain_memphis_tile_source_get_rule_ids (
    ChamplainMemphisTileSource *tile_source);

void champlain_memphis_tile_source_set_rule (
    ChamplainMemphisTileSource *tile_source,
    MemphisRule *rule);

MemphisRule * champlain_memphis_tile_source_get_rule (
    ChamplainMemphisTileSource *tile_source,
    const gchar *id);

void champlain_memphis_tile_source_remove_rule (
    ChamplainMemphisTileSource *tile_source,
    const gchar *id);

G_END_DECLS

#endif /* _CHAMPLAIN_MEMPHIS_TILE_SOURCE */
