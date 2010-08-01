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

#if !defined (__CHAMPLAIN_CHAMPLAIN_H_INSIDE__) && !defined (CHAMPLAIN_COMPILATION)
#error "Only <champlain/champlain.h> can be included directly."
#endif

#ifndef CHAMPLAIN_LAYER_H
#define CHAMPLAIN_LAYER_H

#include <champlain/champlain-defines.h>
#include <champlain/champlain-base-marker.h>

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_LAYER champlain_layer_get_type ()

#define CHAMPLAIN_LAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_LAYER, ChamplainLayer))

#define CHAMPLAIN_LAYER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_LAYER, ChamplainLayerClass))

#define CHAMPLAIN_IS_LAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_LAYER))

#define CHAMPLAIN_IS_LAYER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_LAYER))

#define CHAMPLAIN_LAYER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_LAYER, ChamplainLayerClass))

typedef struct _ChamplainLayer ChamplainLayer;
typedef struct _ChamplainLayerClass ChamplainLayerClass;

struct _ChamplainLayer
{
  ClutterGroup parent;
};

struct _ChamplainLayerClass
{
  ClutterGroupClass parent_class;
};

GType champlain_layer_get_type (void);

ChamplainLayer *champlain_layer_new (void);

void champlain_layer_show (ChamplainLayer *layer);
void champlain_layer_hide (ChamplainLayer *layer);

void champlain_layer_add_marker (ChamplainLayer *layer,
    ChamplainBaseMarker *marker);
void champlain_layer_remove_marker (ChamplainLayer *layer,
    ChamplainBaseMarker *marker);

void champlain_layer_animate_in_all_markers (ChamplainLayer *layer);
void champlain_layer_animate_out_all_markers (ChamplainLayer *layer);

void champlain_layer_show_all_markers (ChamplainLayer *layer);
void champlain_layer_hide_all_markers (ChamplainLayer *layer);

G_END_DECLS

#endif
