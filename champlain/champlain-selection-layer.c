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

/**
 * SECTION:champlain-selection-layer
 * @short_description: A container for #ChamplainMarker supporting selection
 *
 * A ChamplainSelectionLayer is little more than a #ChamplainLayer. The markers
 * can be selected.
 *
 * Use #clutter_container_add to add markers to the layer and
 * #clutter_container_remove to remove them.
 */

#include "config.h"

#include "champlain-selection-layer.h"

#include "champlain-defines.h"
#include "champlain-base-marker.h"

#include <clutter/clutter.h>
#include <glib.h>

G_DEFINE_TYPE (ChamplainSelectionLayer, champlain_selection_layer, CHAMPLAIN_TYPE_LAYER)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_SELECTION_LAYER, ChamplainSelectionLayerPrivate))

enum
{
  PROP_0
};

typedef struct _ChamplainSelectionLayerPrivate ChamplainSelectionLayerPrivate;

struct _ChamplainSelectionLayerPrivate {
  gpointer spacer;
};

static void
champlain_selection_layer_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  //ChamplainSelectionLayer *self = CHAMPLAIN_SELECTION_LAYER (object);
  switch (property_id)
    {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_selection_layer_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  //ChamplainSelectionLayer *self = CHAMPLAIN_SELECTION_LAYER (object);
  switch (property_id)
    {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_selection_layer_class_init (ChamplainSelectionLayerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainSelectionLayerPrivate));

  object_class->get_property = champlain_selection_layer_get_property;
  object_class->set_property = champlain_selection_layer_set_property;
}

static void
champlain_selection_layer_init (ChamplainSelectionLayer *self)
{

}

/**
 * champlain_selection_layer_new:
 *
 * Returns a new #ChamplainSelectionLayer ready to be used as a #ClutterContainer for the markers.
 *
 * Since: 0.4
 */
ChamplainLayer *
champlain_selection_layer_new ()
{
  return g_object_new (CHAMPLAIN_TYPE_SELECTION_LAYER, NULL);
}
