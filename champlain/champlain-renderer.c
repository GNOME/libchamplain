/*
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

#include "champlain-renderer.h"

G_DEFINE_TYPE (ChamplainRenderer, champlain_renderer, G_TYPE_OBJECT)

enum
{
  /* normal signals */
  RELOAD_TILES,
  LAST_SIGNAL
};

static guint champlain_renderer_signals[LAST_SIGNAL] = { 0, };

static void
champlain_renderer_dispose (GObject *object)
{
  G_OBJECT_CLASS (champlain_renderer_parent_class)->dispose (object);
}


static void
champlain_renderer_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_renderer_parent_class)->finalize (object);
}


static void
champlain_renderer_class_init (ChamplainRendererClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = champlain_renderer_finalize;
  object_class->dispose = champlain_renderer_dispose;

  klass->set_data = NULL;
  klass->render = NULL;

  champlain_renderer_signals[RELOAD_TILES] =
    g_signal_new ("reload-tiles", G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL,
        g_cclosure_marshal_VOID__VOID, G_TYPE_NONE,
        0, NULL);
}


void
champlain_renderer_set_data (ChamplainRenderer *renderer,
    const gchar *data, 
    guint size)
{
  g_return_if_fail (CHAMPLAIN_IS_RENDERER (renderer));

  CHAMPLAIN_RENDERER_GET_CLASS (renderer)->set_data (renderer, data, size);
}


void
champlain_renderer_render (ChamplainRenderer *renderer,
    ChamplainTile *tile)
{
  g_return_if_fail (CHAMPLAIN_IS_RENDERER (renderer));

  CHAMPLAIN_RENDERER_GET_CLASS (renderer)->render (renderer, tile);
}


static void
champlain_renderer_init (ChamplainRenderer *self)
{
}
