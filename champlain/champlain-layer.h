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

typedef struct _ChamplainLayerPrivate ChamplainLayerPrivate;

typedef struct _ChamplainLayer ChamplainLayer;
typedef struct _ChamplainLayerClass ChamplainLayerClass;

/**
 * ChamplainSelectionMode:
 * @CHAMPLAIN_SELECTION_NONE: No marker can be selected.
 * @CHAMPLAIN_SELECTION_SINGLE: Only one marker can be selected.
 * @CHAMPLAIN_SELECTION_MULTIPLE: Multiple marker can be selected.
 *
 * Selection mode
 */
typedef enum
{
  CHAMPLAIN_SELECTION_NONE,
  CHAMPLAIN_SELECTION_SINGLE,
  CHAMPLAIN_SELECTION_MULTIPLE
} ChamplainSelectionMode;

struct _ChamplainLayer
{
  ClutterGroup parent;
  
  ChamplainLayerPrivate *priv;
};

struct _ChamplainLayerClass
{
  ClutterGroupClass parent_class;
};

GType champlain_layer_get_type (void);

ChamplainLayer *champlain_layer_new_full (ChamplainSelectionMode mode);

void champlain_layer_add_marker (ChamplainLayer *layer,
    ChamplainBaseMarker *marker);
void champlain_layer_remove_marker (ChamplainLayer *layer,
    ChamplainBaseMarker *marker);

void champlain_layer_set_view (ChamplainLayer *layer,
    ChamplainView *view);

void champlain_layer_animate_in_all_markers (ChamplainLayer *layer);
void champlain_layer_animate_out_all_markers (ChamplainLayer *layer);

void champlain_layer_show_all_markers (ChamplainLayer *layer);
void champlain_layer_hide_all_markers (ChamplainLayer *layer);

void champlain_layer_select (ChamplainLayer *layer,
    ChamplainBaseMarker *marker);
void champlain_layer_unselect (ChamplainLayer *layer,
    ChamplainBaseMarker *marker);
gboolean champlain_layer_marker_is_selected (ChamplainLayer *layer,
    ChamplainBaseMarker *marker);

void champlain_layer_select_all (ChamplainLayer *layer);
void champlain_layer_unselect_all (ChamplainLayer *layer);
const GList *champlain_layer_get_selected_markers (ChamplainLayer *layer);

void champlain_layer_set_selection_mode (ChamplainLayer *layer,
    ChamplainSelectionMode mode);
ChamplainSelectionMode champlain_layer_get_selection_mode (
    ChamplainLayer *layer);
//void champlain_view_ensure_markers_visible (ChamplainView *view,
//    ChamplainBaseMarker *markers[],
//    gboolean animate);

G_END_DECLS

#endif
