/*
 * Copyright (C) 2010-2013 Jiri Techet <techet@gmail.com>
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
 * SECTION:champlain-image-renderer
 * @short_description: A renderer that renders tiles from binary image data
 *
 * #ChamplainImageRenderer renders tiles from binary image data. The rendering
 * is performed using #GdkPixbufLoader so the set of supported image
 * formats is equal to the set of formats supported by #GdkPixbufLoader.
 */

#include "champlain-image-renderer.h"
#include <gdk/gdk.h>

G_DEFINE_TYPE (ChamplainImageRenderer, champlain_image_renderer, CHAMPLAIN_TYPE_RENDERER)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_IMAGE_RENDERER, ChamplainImageRendererPrivate))

struct _ChamplainImageRendererPrivate
{
  gchar *data;
  guint size;
};

typedef struct _RendererData RendererData;

struct _RendererData
{
  ChamplainRenderer *renderer;
  ChamplainTile *tile;
  gchar *data;
  guint size;
};

static void set_data (ChamplainRenderer *renderer,
    const gchar *data,
    guint size);
static void render (ChamplainRenderer *renderer,
    ChamplainTile *tile);


static void
champlain_image_renderer_dispose (GObject *object)
{
  G_OBJECT_CLASS (champlain_image_renderer_parent_class)->dispose (object);
}


static void
champlain_image_renderer_finalize (GObject *object)
{
  ChamplainImageRendererPrivate *priv = GET_PRIVATE (object);

  g_free (priv->data);

  G_OBJECT_CLASS (champlain_image_renderer_parent_class)->finalize (object);
}


static void
champlain_image_renderer_class_init (ChamplainImageRendererClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ChamplainRendererClass *renderer_class = CHAMPLAIN_RENDERER_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainImageRendererPrivate));

  object_class->finalize = champlain_image_renderer_finalize;
  object_class->dispose = champlain_image_renderer_dispose;

  renderer_class->set_data = set_data;
  renderer_class->render = render;
}


static void
champlain_image_renderer_init (ChamplainImageRenderer *self)
{
  ChamplainImageRendererPrivate *priv = GET_PRIVATE (self);

  self->priv = priv;

  priv->data = NULL;
}


/**
 * champlain_image_renderer_new:
 *
 * Constructor of #ChamplainImageRenderer.
 *
 * Returns: a constructed #ChamplainImageRenderer object
 *
 * Since: 0.8
 */
ChamplainImageRenderer *
champlain_image_renderer_new (void)
{
  return g_object_new (CHAMPLAIN_TYPE_IMAGE_RENDERER, NULL);
}


static void
set_data (ChamplainRenderer *renderer, const gchar *data, guint size)
{
  ChamplainImageRendererPrivate *priv = GET_PRIVATE (renderer);

  if (priv->data)
    g_free (priv->data);

  priv->data = g_memdup (data, size);
  priv->size = size;
}


static void 
image_rendered_cb (GInputStream *stream, GAsyncResult *res, RendererData *data)
{
  ChamplainTile *tile = data->tile;
  gboolean error = TRUE;
  GError *gerror = NULL;
  ClutterActor *actor = NULL;
  GdkPixbuf *pixbuf;
  ClutterContent *content;
  gfloat width, height;
  
  pixbuf = gdk_pixbuf_new_from_stream_finish (res, NULL);
  if (!pixbuf)
    {
      g_warning ("NULL pixbuf");
      goto finish;
    }
  
  /* Load the image into clutter */
  content = clutter_image_new ();
  if (!clutter_image_set_data (CLUTTER_IMAGE (content),
          gdk_pixbuf_get_pixels (pixbuf),
          gdk_pixbuf_get_has_alpha (pixbuf)
            ? COGL_PIXEL_FORMAT_RGBA_8888
            : COGL_PIXEL_FORMAT_RGB_888,
          gdk_pixbuf_get_width (pixbuf),
          gdk_pixbuf_get_height (pixbuf),
          gdk_pixbuf_get_rowstride (pixbuf),
          &gerror))
    {
      if (gerror)
        {
          g_warning ("Unable to transfer to clutter: %s", gerror->message);
          g_error_free (gerror);
        }

      g_object_unref (content);
      goto finish;
    }

  clutter_content_get_preferred_size (content, &width, &height);
  actor = clutter_actor_new ();
  clutter_actor_set_size (actor, width, height);
  clutter_actor_set_content (actor, content);
  g_object_unref (content);
  /* has to be set for proper opacity */
  clutter_actor_set_offscreen_redirect (actor, CLUTTER_OFFSCREEN_REDIRECT_AUTOMATIC_FOR_OPACITY);
  
  error = FALSE;

finish:

  if (actor)
    champlain_tile_set_content (tile, actor);

  g_signal_emit_by_name (tile, "render-complete", data->data, data->size, error);

  if (pixbuf)
    g_object_unref (pixbuf);

  g_object_unref (data->renderer);
  g_object_unref (tile);
  g_object_unref (stream);
  g_free (data->data);
  g_slice_free (RendererData, data);
}



static void
render (ChamplainRenderer *renderer, ChamplainTile *tile)
{
  ChamplainImageRendererPrivate *priv = GET_PRIVATE (renderer);
  GInputStream *stream;

  if (!priv->data || priv->size == 0)
    {
      g_signal_emit_by_name (tile, "render-complete", priv->data, priv->size, TRUE);
      return;
    }
    
  RendererData *data;

  data = g_slice_new (RendererData);
  data->tile = g_object_ref (tile);
  data->renderer = g_object_ref (renderer);
  data->data = priv->data;
  data->size = priv->size;
    
  stream = g_memory_input_stream_new_from_data (priv->data, priv->size, NULL);
  gdk_pixbuf_new_from_stream_async (stream, NULL, (GAsyncReadyCallback)image_rendered_cb, data);
  priv->data = NULL;
}
