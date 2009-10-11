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

#ifndef CHAMPLAIN_SELECTION_LAYER_H
#define CHAMPLAIN_SELECTION_LAYER_H

#include <champlain/champlain-defines.h>
#include <champlain/champlain-base-marker.h>
#include <champlain/champlain-layer.h>

#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CHAMPLAIN_TYPE_SELECTION_LAYER champlain_selection_layer_get_type()

#define CHAMPLAIN_SELECTION_LAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAMPLAIN_TYPE_SELECTION_LAYER, ChamplainSelectionLayer))

#define CHAMPLAIN_SELECTION_LAYER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CHAMPLAIN_TYPE_SELECTION_LAYER, ChamplainSelectionLayerClass))

#define CHAMPLAIN_IS_SELECTION_LAYER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAMPLAIN_TYPE_SELECTION_LAYER))

#define CHAMPLAIN_IS_SELECTION_LAYER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAMPLAIN_TYPE_SELECTION_LAYER))

#define CHAMPLAIN_SELECTION_LAYER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAMPLAIN_TYPE_SELECTION_LAYER, ChamplainSelectionLayerClass))

typedef struct _ChamplainSelectionLayerPrivate ChamplainSelectionLayerPrivate;

/**
 * ChamplainSelectionMode:
 * @CHAMPLAIN_SELECTION_NONE: No marker can be selected.
 * @CHAMPLAIN_SELECTION_SINGLE: Only one marker can be selected.
 * @CHAMPLAIN_SELECTION_MULTIPLE: Multiple marker can be selected.
 *
 * Selection mode
 */
typedef enum {
  CHAMPLAIN_SELECTION_NONE,
  CHAMPLAIN_SELECTION_SINGLE,
  CHAMPLAIN_SELECTION_MULTIPLE
} ChamplainSelectionMode;

typedef struct _ChamplainSelectionLayer      ChamplainSelectionLayer;
typedef struct _ChamplainSelectionLayerClass ChamplainSelectionLayerClass;

struct _ChamplainSelectionLayer {
  ChamplainLayer parent;

  ChamplainSelectionLayerPrivate *priv;
};

struct _ChamplainSelectionLayerClass {
  ChamplainLayerClass parent_class;
};

GType champlain_selection_layer_get_type (void);

ChamplainLayer * champlain_selection_layer_new (void);

ChamplainBaseMarker * champlain_selection_layer_get_selected (ChamplainSelectionLayer *layer);
const GList *champlain_selection_layer_get_selected_markers (ChamplainSelectionLayer *layer);
guint champlain_selection_layer_count_selected_markers (ChamplainSelectionLayer *layer);

void champlain_selection_layer_select (ChamplainSelectionLayer *layer,
    ChamplainBaseMarker *marker);
void champlain_selection_layer_unselect (ChamplainSelectionLayer *layer,
    ChamplainBaseMarker *marker);
gboolean champlain_selection_layer_marker_is_selected (ChamplainSelectionLayer *layer,
    ChamplainBaseMarker *marker);
void champlain_selection_layer_select_all (ChamplainSelectionLayer *layer);
void champlain_selection_layer_unselect_all (ChamplainSelectionLayer *layer);

void champlain_selection_layer_set_selection_mode (ChamplainSelectionLayer *layer,
    ChamplainSelectionMode mode);
ChamplainSelectionMode champlain_selection_layer_get_selection_mode (
    ChamplainSelectionLayer *layer);

G_END_DECLS

#endif
