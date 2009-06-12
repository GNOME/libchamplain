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

#ifndef CHAMPLAIN_MAP_SOURCE_DESC_H
#define CHAMPLAIN_MAP_SOURCE_DESC_H

#include <glib-object.h>
#include "champlain-map-source.h"

G_BEGIN_DECLS


typedef struct _ChamplainMapSourceDesc ChamplainMapSourceDesc;


/**
 * ChamplainMapSourceConstructor:
 *
 * A #ChamplainMapSource constructor.  It should return a ready to use
 * #ChamplainMapSource.
 *
 * Since: 0.4
 */
typedef ChamplainMapSource * (*ChamplainMapSourceConstructor) (
    ChamplainMapSourceDesc *desc, gpointer data);
#define CHAMPLAIN_MAP_SOURCE_CONSTRUCTOR (f) ((ChamplainMapSourceConstructor) (f))

#define CHAMPLAIN_MAP_SOURCE_DESC(obj)     ((ChamplainMapSourceDesc *) (obj))

/**
 * ChamplainMapSourceDesc:
 *
 * Describes a #ChamplainMapSource.  This is returned by #champlain_map_source_factory_get_list.
 *
 * Since: 0.4
 */
struct _ChamplainMapSourceDesc {
  gchar *id;
  gchar *name;
  gchar *license;
  gchar *license_uri;
  gint min_zoom_level;
  gint max_zoom_level;
  ChamplainMapProjection projection;
  ChamplainMapSourceConstructor constructor;
  gchar *uri_format;
  gpointer data;
};

GType champlain_map_source_desc_get_type (void) G_GNUC_CONST;
#define CHAMPLAIN_TYPE_MAP_SOURCE_DESC (champlain_map_source_desc_get_type ())

ChamplainMapSourceDesc * champlain_map_source_desc_copy (const ChamplainMapSourceDesc *desc);

void champlain_map_source_desc_free (ChamplainMapSourceDesc *desc);

ChamplainMapSourceDesc * champlain_map_source_desc_new (void);

G_END_DECLS

#endif
