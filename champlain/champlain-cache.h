/*
 * Copyright (C) 2008 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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

#ifndef CHAMPLAIN_CACHE_H
#define CHAMPLAIN_CACHE_H

#include <champlain/champlain-defines.h>

#include "champlain-tile.h"

#include <glib.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_CACHE champlain_cache_get_type()

#define CHAMPLAIN_CACHE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_CACHE, ChamplainCache))

#define CHAMPLAIN_CACHE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_CACHE, ChamplainCacheClass))

#define CHAMPLAIN_IS_CACHE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_CACHE))

#define CHAMPLAIN_IS_CACHE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_CACHE))

#define CHAMPLAIN_CACHE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_CACHE, ChamplainCacheClass))

typedef struct _ChamplainCache      ChamplainCache;
typedef struct _ChamplainCacheClass ChamplainCacheClass;

struct _ChamplainCache {
  GObject parent;
};

struct _ChamplainCacheClass {
  GObjectClass parent_class;
};

GType champlain_cache_get_type (void);

ChamplainCache* champlain_cache_dup_default (void);

void champlain_cache_update_tile (ChamplainCache *self,
    ChamplainTile *tile,
    guint filesize);
gboolean champlain_cache_fill_tile (ChamplainCache *self,
    ChamplainTile *tile);
gboolean champlain_cache_tile_is_expired (ChamplainCache *self,
    ChamplainTile *tile);

void champlain_cache_set_size_limit (ChamplainCache *self,
    guint size_limit);
guint champlain_cache_get_size_limit (ChamplainCache *self);

void champlain_cache_purge (ChamplainCache *self);
void champlain_cache_purge_on_idle (ChamplainCache *self);

G_END_DECLS

#endif /* CHAMPLAIN_CACHE_H */

