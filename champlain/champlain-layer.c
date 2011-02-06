/*
 * Copyright (C) 2011 Jiri Techet <techet@gmail.com>
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

#include "champlain-layer.h"

G_DEFINE_TYPE (ChamplainLayer, champlain_layer, CLUTTER_TYPE_ACTOR)

static void
champlain_layer_dispose (GObject *object)
{
  G_OBJECT_CLASS (champlain_layer_parent_class)->dispose (object);
}


static void
champlain_layer_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_layer_parent_class)->finalize (object);
}


static void
champlain_layer_class_init (ChamplainLayerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = champlain_layer_finalize;
  object_class->dispose = champlain_layer_dispose;

  klass->set_view = NULL;
}


void champlain_layer_set_view (ChamplainLayer *layer,
    ChamplainView *view)
{
  g_return_if_fail (CHAMPLAIN_IS_LAYER (layer));

  CHAMPLAIN_LAYER_GET_CLASS (layer)->set_view (layer, view);
}


static void
champlain_layer_init (ChamplainLayer *self)
{
}
