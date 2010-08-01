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

#include "champlain-error-tile-renderer.h"
#include <gdk/gdk.h>

G_DEFINE_TYPE (ChamplainErrorTileRenderer, champlain_error_tile_renderer, CHAMPLAIN_TYPE_RENDERER)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_ERROR_TILE_RENDERER, ChamplainErrorTileRendererPrivate))

struct _ChamplainErrorTileRendererPrivate
{
  CoglHandle error_tex;
  guint tile_size;
};

enum
{
  PROP_0,
  PROP_TILE_SIZE
};


static void set_data (ChamplainRenderer *renderer,
    const gchar *data,
    guint size);
static void render (ChamplainRenderer *renderer,
    ChamplainTile *tile);


static void
champlain_error_tile_renderer_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainErrorTileRenderer *renderer = CHAMPLAIN_ERROR_TILE_RENDERER (object);

  switch (property_id)
    {
    case PROP_TILE_SIZE:
      g_value_set_uint (value, champlain_error_tile_renderer_get_tile_size (renderer));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
champlain_error_tile_renderer_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainErrorTileRenderer *renderer = CHAMPLAIN_ERROR_TILE_RENDERER (object);

  switch (property_id)
    {
    case PROP_TILE_SIZE:
      champlain_error_tile_renderer_set_tile_size (renderer, g_value_get_uint (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
champlain_error_tile_renderer_dispose (GObject *object)
{
  ChamplainErrorTileRendererPrivate *priv = CHAMPLAIN_ERROR_TILE_RENDERER (object)->priv;

  if (priv->error_tex)
    {
      cogl_handle_unref (priv->error_tex);
      priv->error_tex = NULL;
    }

  G_OBJECT_CLASS (champlain_error_tile_renderer_parent_class)->dispose (object);
}


static void
champlain_error_tile_renderer_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_error_tile_renderer_parent_class)->finalize (object);
}


static void
champlain_error_tile_renderer_class_init (ChamplainErrorTileRendererClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ChamplainRendererClass *renderer_class = CHAMPLAIN_RENDERER_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainErrorTileRendererPrivate));

  object_class->get_property = champlain_error_tile_renderer_get_property;
  object_class->set_property = champlain_error_tile_renderer_set_property;
  object_class->finalize = champlain_error_tile_renderer_finalize;
  object_class->dispose = champlain_error_tile_renderer_dispose;

  g_object_class_install_property (object_class,
      PROP_TILE_SIZE,
      g_param_spec_uint ("tile-size",
          "Tile Size",
          "The size of the rendered tile",
          0,
          G_MAXINT,
          256,
          G_PARAM_READWRITE));

  renderer_class->set_data = set_data;
  renderer_class->render = render;
}


static void
champlain_error_tile_renderer_init (ChamplainErrorTileRenderer *self)
{
  ChamplainErrorTileRendererPrivate *priv = GET_PRIVATE (self);

  self->priv = priv;

  priv->error_tex = NULL;
}


ChamplainErrorTileRenderer *
champlain_error_tile_renderer_new (guint tile_size)
{
  return g_object_new (CHAMPLAIN_TYPE_ERROR_TILE_RENDERER, "tile-size", tile_size, NULL);
}


static void
set_data (ChamplainRenderer *renderer, const gchar *data, guint size)
{
}


static void
render (ChamplainRenderer *renderer, ChamplainTile *tile)
{
  g_return_if_fail (CHAMPLAIN_IS_ERROR_TILE_RENDERER (renderer));
  g_return_if_fail (CHAMPLAIN_IS_TILE (tile));

  ChamplainErrorTileRenderer *error_renderer = CHAMPLAIN_ERROR_TILE_RENDERER (renderer);
  ChamplainErrorTileRendererPrivate *priv = error_renderer->priv;
  ClutterActor *clone;
  guint size;
  ChamplainRenderCallbackData callback_data;

  callback_data.data = NULL;
  callback_data.size = 0;
  callback_data.error = FALSE;

  if (champlain_tile_get_state (tile) == CHAMPLAIN_STATE_LOADED)
    {
      /* cache is just validating tile - don't generate error tile in this case - instead use what we have */
      g_signal_emit_by_name (tile, "render-complete", &callback_data);
      return;
    }

  size = champlain_error_tile_renderer_get_tile_size (error_renderer);

  if (!priv->error_tex)
    {
      cairo_t *cr;
      cairo_pattern_t *pat;
      ClutterActor *tmp_actor;

      tmp_actor = clutter_cairo_texture_new (size, size);
      cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (tmp_actor));

      /* draw a linear gray to white pattern */
      pat = cairo_pattern_create_linear (size / 2.0, 0.0, size, size / 2.0);
      cairo_pattern_add_color_stop_rgb (pat, 0, 0.686, 0.686, 0.686);
      cairo_pattern_add_color_stop_rgb (pat, 1, 0.925, 0.925, 0.925);
      cairo_set_source (cr, pat);
      cairo_rectangle (cr, 0, 0, size, size);
      cairo_fill (cr);

      cairo_pattern_destroy (pat);

      /* draw the red cross */
      cairo_set_source_rgb (cr, 0.424, 0.078, 0.078);
      cairo_set_line_width (cr, 14.0);
      cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
      cairo_move_to (cr, 24, 24);
      cairo_line_to (cr, 50, 50);
      cairo_move_to (cr, 50, 24);
      cairo_line_to (cr, 24, 50);
      cairo_stroke (cr);

      cairo_destroy (cr);

      priv->error_tex = clutter_texture_get_cogl_texture (CLUTTER_TEXTURE (tmp_actor));
      cogl_handle_ref (priv->error_tex);

      g_object_ref_sink (tmp_actor);
      g_object_unref (tmp_actor);
    }

  clone = clutter_texture_new ();
  clutter_texture_set_cogl_texture (CLUTTER_TEXTURE (clone), priv->error_tex);
  champlain_tile_set_content (tile, clone);
  g_signal_emit_by_name (tile, "render-complete", &callback_data);
}


void
champlain_error_tile_renderer_set_tile_size (ChamplainErrorTileRenderer *renderer,
    guint size)
{
  g_return_if_fail (CHAMPLAIN_IS_ERROR_TILE_RENDERER (renderer));

  renderer->priv->tile_size = size;

  g_object_notify (G_OBJECT (renderer), "tile-size");
}


guint
champlain_error_tile_renderer_get_tile_size (ChamplainErrorTileRenderer *renderer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_ERROR_TILE_RENDERER (renderer), 0);

  return renderer->priv->tile_size;
}
