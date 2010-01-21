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

#include "champlain-error-tile-source.h"

G_DEFINE_TYPE (ChamplainErrorTileSource, champlain_error_tile_source, CHAMPLAIN_TYPE_TILE_SOURCE);

#define GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_ERROR_TILE_SOURCE, ChamplainErrorTileSourcePrivate))

typedef struct _ChamplainErrorTileSourcePrivate ChamplainErrorTileSourcePrivate;

struct _ChamplainErrorTileSourcePrivate
{
  ClutterActor *error_actor;
};

static void fill_tile (ChamplainMapSource *map_source, ChamplainTile *tile);

static void
champlain_error_tile_source_dispose (GObject *object)
{
  G_OBJECT_CLASS (champlain_error_tile_source_parent_class)->dispose (object);
}

static void
champlain_error_tile_source_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_error_tile_source_parent_class)->finalize (object);
}

static void
champlain_error_tile_source_class_init (ChamplainErrorTileSourceClass *klass)
{
  ChamplainMapSourceClass *map_source_class = CHAMPLAIN_MAP_SOURCE_CLASS (klass);
  GObjectClass* object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainErrorTileSourcePrivate));

  object_class->finalize = champlain_error_tile_source_finalize;
  object_class->dispose = champlain_error_tile_source_dispose;

  map_source_class->fill_tile = fill_tile;
}

static void
champlain_error_tile_source_init (ChamplainErrorTileSource *error_source)
{
  ChamplainErrorTileSourcePrivate *priv = GET_PRIVATE(error_source);
  priv->error_actor = NULL;
}

ChamplainErrorTileSource* champlain_error_tile_source_new_full (guint tile_size)
{
  return g_object_new (CHAMPLAIN_TYPE_ERROR_TILE_SOURCE, "tile-size", tile_size, NULL);
}

static void
fill_tile (ChamplainMapSource *map_source, ChamplainTile *tile)
{
  g_return_if_fail (CHAMPLAIN_IS_ERROR_TILE_SOURCE (map_source));
  g_return_if_fail (CHAMPLAIN_IS_TILE (tile));

  ChamplainErrorTileSourcePrivate *priv = GET_PRIVATE(map_source);
  ClutterActor *clone;
  guint size;

  if (champlain_tile_get_content (tile))
    /* cache is just validating tile - don't generate error tile in this case */
    return;

  size = champlain_map_source_get_tile_size (map_source);

  if (!priv->error_actor)
    {
      cairo_t *cr;
      cairo_pattern_t *pat;
      ClutterActor *stage;

      priv->error_actor = clutter_cairo_texture_new (size, size);
      cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (priv->error_actor));
      stage = clutter_stage_get_default ();

      /* draw a linear gray to white pattern */
      pat = cairo_pattern_create_linear (size / 2.0, 0.0,  size, size / 2.0);
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

      clutter_container_add_actor (CLUTTER_CONTAINER (stage), priv->error_actor);
      clutter_actor_hide (priv->error_actor);
    }

  clone = clutter_clone_new (priv->error_actor);

  champlain_tile_set_content (tile, clone, TRUE);
  champlain_tile_set_size (tile, size);
  champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);
}
