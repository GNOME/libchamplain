/*
 * Copyright (C) 2008-2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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

#ifndef _CHAMPLAIN_ERROR_TILE_SOURCE_H_
#define _CHAMPLAIN_ERROR_TILE_SOURCE_H_

#include <champlain/champlain-tile-source.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_ERROR_TILE_SOURCE             (champlain_error_tile_source_get_type ())
#define CHAMPLAIN_ERROR_TILE_SOURCE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_ERROR_TILE_SOURCE, ChamplainErrorTileSource))
#define CHAMPLAIN_ERROR_TILE_SOURCE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_ERROR_TILE_SOURCE, ChamplainErrorTileSourceClass))
#define CHAMPLAIN_IS_ERROR_TILE_SOURCE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_ERROR_TILE_SOURCE))
#define CHAMPLAIN_IS_ERROR_TILE_SOURCE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_ERROR_TILE_SOURCE))
#define CHAMPLAIN_ERROR_TILE_SOURCE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_ERROR_TILE_SOURCE, ChamplainErrorTileSourceClass))

typedef struct _ChamplainErrorTileSource ChamplainErrorTileSource;
typedef struct _ChamplainErrorTileSourceClass ChamplainErrorTileSourceClass;

struct _ChamplainErrorTileSource
{
  ChamplainTileSource parent_instance;
};

struct _ChamplainErrorTileSourceClass
{
  ChamplainTileSourceClass parent_class;
};

GType champlain_error_tile_source_get_type (void);

ChamplainErrorTileSource* champlain_error_tile_source_new_full (guint tile_size);

G_END_DECLS

#endif /* _CHAMPLAIN_ERROR_TILE_SOURCE_H_ */
