/*
 * Copyright (C) 2009 Simon Wenner <simon@wenner.ch>
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

/*
 * SECTION:champlain-memphis-tile-source
 * @short_description: A map source that draws tiles locally
 *
 * The #ChamplainMemphisRenderer uses the rendering library
 * <ulink role="online-location" url="https://trac.openstreetmap.ch/trac/memphis/">
 * LibMemphis</ulink> to draw <ulink role="online-location" url="http://www.openstreetmap.org/">
 * OpenStreetMap</ulink> data. Tiles are rendered in separate threads.
 * It supports zoom levels 12 to 18.
 *
 * The map data is supplied by a #ChamplainMapDataSource.
 * #ChamplainLocalMapDataSource loads data from a local OSM file.
 * #ChamplainNetworkMapDataSource uses the OSM API to download data chunks.
 *
 * The output of the renderer can be configured with a Memphis rules XML file.
 * (TODO: link to the specification) The default rules only show
 * highways as thin black lines.
 * Once loaded, rules can be queried and edited.
 */


#define DEBUG_FLAG CHAMPLAIN_DEBUG_MEMPHIS
#include "champlain-debug.h"
#include "champlain-tile-cache.h"
#include "champlain-renderer.h"
#include "champlain-defines.h"
#include "champlain-enum-types.h"
#include "champlain-private.h"
#include "champlain-memphis-renderer.h"
#include "champlain-bounding-box.h"

#include <gdk/gdk.h>

#include <memphis/memphis.h>
#include <errno.h>
#include <string.h>

/* Tuning parameters */
#define MAX_THREADS 4

const gchar default_rules[] =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
  "<rules version=\"0.1\" background=\"#ffffff\">"
  "<rule e=\"way\" k=\"highway\" v=\"*\">"
  "<line color=\"#000000\" width=\"1.0\"/>"
  "</rule>"
  "</rules>";

enum
{
  PROP_0,
  PROP_TILE_SIZE,
  PROP_BOUNDING_BOX
};

static void render (ChamplainRenderer *renderer,
    ChamplainTile *tile);
static void set_data (ChamplainRenderer *renderer,
    const gchar *data,
    guint size);
static void set_bounding_box (ChamplainMemphisRenderer *renderer,
    ChamplainBoundingBox *bbox);


G_DEFINE_TYPE (ChamplainMemphisRenderer, champlain_memphis_renderer, CHAMPLAIN_TYPE_RENDERER)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_MEMPHIS_RENDERER, ChamplainMemphisRendererPrivate))

struct _ChamplainMemphisRendererPrivate
{
  MemphisRuleSet *rules;
  MemphisRenderer *renderer;
  GThreadPool *thpool;
  guint tile_size;
  ChamplainBoundingBox *bbox;
};

typedef struct _WorkerThreadData WorkerThreadData;

struct _WorkerThreadData
{
  gint x;
  gint y;
  guint z;
  guint size;

  ChamplainRenderer *renderer;
  ChamplainTile *tile;
  cairo_surface_t *cst;
};

/* lock to protect the renderer state while rendering */
GStaticRWLock MemphisLock = G_STATIC_RW_LOCK_INIT;


static void memphis_worker_thread (gpointer data,
    gpointer user_data);


static void
champlain_memphis_renderer_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainMemphisRenderer *renderer = CHAMPLAIN_MEMPHIS_RENDERER (object);

  switch (property_id)
    {
    case PROP_TILE_SIZE:
      g_value_set_uint (value, champlain_memphis_renderer_get_tile_size (renderer));
      break;

    case PROP_BOUNDING_BOX:
      g_value_set_boxed (value, champlain_memphis_renderer_get_bounding_box (renderer));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
champlain_memphis_renderer_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainMemphisRenderer *renderer = CHAMPLAIN_MEMPHIS_RENDERER (object);

  switch (property_id)
    {
    case PROP_TILE_SIZE:
      champlain_memphis_renderer_set_tile_size (renderer, g_value_get_uint (value));
      break;

    case PROP_BOUNDING_BOX:
      set_bounding_box (renderer, g_value_get_boxed (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
champlain_memphis_renderer_dispose (GObject *object)
{
  ChamplainMemphisRenderer *renderer = CHAMPLAIN_MEMPHIS_RENDERER (object);
  ChamplainMemphisRendererPrivate *priv = renderer->priv;

  if (priv->thpool)
    {
      g_thread_pool_free (priv->thpool, FALSE, TRUE);
      priv->thpool = NULL;
    }
  if (priv->renderer)
    {
      memphis_renderer_free (priv->renderer);
      priv->renderer = NULL;
    }
  if (priv->rules)
    {
      memphis_rule_set_free (priv->rules);
      priv->rules = NULL;
    }

  G_OBJECT_CLASS (champlain_memphis_renderer_parent_class)->dispose (object);
}


static void
champlain_memphis_renderer_finalize (GObject *object)
{
  ChamplainMemphisRenderer *renderer = CHAMPLAIN_MEMPHIS_RENDERER (object);
  ChamplainMemphisRendererPrivate *priv = renderer->priv;

  champlain_bounding_box_free (priv->bbox);

  G_OBJECT_CLASS (champlain_memphis_renderer_parent_class)->finalize (object);
}


static void
champlain_memphis_renderer_class_init (ChamplainMemphisRendererClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ChamplainRendererClass *renderer_class = CHAMPLAIN_RENDERER_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainMemphisRendererPrivate));

  object_class->get_property = champlain_memphis_renderer_get_property;
  object_class->set_property = champlain_memphis_renderer_set_property;
  object_class->dispose = champlain_memphis_renderer_dispose;
  object_class->finalize = champlain_memphis_renderer_finalize;

  renderer_class->set_data = set_data;
  renderer_class->render = render;

  g_object_class_install_property (object_class,
      PROP_TILE_SIZE,
      g_param_spec_uint ("tile-size",
          "Tile Size",
          "The size of the rendered tile",
          0,
          G_MAXINT,
          256,
          G_PARAM_READWRITE));

  /*
   * ChamplainMapDataSource:bounding-box:
   *
   * The bounding box of the area that contains map data.
   *
   * Since: 0.6
   */
  g_object_class_install_property (object_class,
      PROP_BOUNDING_BOX,
      g_param_spec_boxed ("bounding-box",
          "Bounding Box",
          "The bounding box of the renderer",
          CHAMPLAIN_TYPE_BOUNDING_BOX,
          G_PARAM_READWRITE));
}


static void
champlain_memphis_renderer_init (ChamplainMemphisRenderer *renderer)
{
  ChamplainMemphisRendererPrivate *priv = GET_PRIVATE (renderer);

  renderer->priv = priv;

  priv->rules = memphis_rule_set_new ();
  memphis_rule_set_load_from_data (priv->rules, default_rules,
      strlen (default_rules), NULL);

  priv->renderer = memphis_renderer_new_full (priv->rules, memphis_map_new ());

  priv->thpool = g_thread_pool_new (memphis_worker_thread, renderer,
      MAX_THREADS, FALSE, NULL);

  priv->bbox = NULL;
}


ChamplainMemphisRenderer *
champlain_memphis_renderer_new_full (guint tile_size)
{
  return g_object_new (CHAMPLAIN_TYPE_MEMPHIS_RENDERER, "tile-size", tile_size, NULL);
}


/*
 * Transform ARGB (Cairo) to RGBA (GdkPixbuf). RGBA is actualy reversed in
 * memory, so the transformation is ARGB -> ABGR (i.e. swapping B and R)
 */
static void
argb_to_rgba (guchar *data,
    guint size)
{
  guint32 *ptr;
  guint32 *endptr = (guint32 *) data + size / 4;

  for (ptr = (guint32 *) data; ptr < endptr; ptr++)
    *ptr = (*ptr & 0xFF00FF00) ^ ((*ptr & 0xFF0000) >> 16) ^ ((*ptr & 0xFF) << 16);
}


static gboolean
tile_loaded_cb (gpointer worker_data)
{
  WorkerThreadData *data = (WorkerThreadData *) worker_data;
  ChamplainTile *tile = data->tile;
  cairo_surface_t *cst = data->cst;
  ChamplainRenderer *renderer = CHAMPLAIN_RENDERER (data->renderer);
  ChamplainRenderCallbackData callback_data;
  cairo_t *cr_clutter;
  ClutterActor *actor;
  guint size = data->size;
  GError *error = NULL;
  GdkPixbuf *pixbuf = NULL;
  gchar *buffer = NULL;
  gsize buffer_size;

  if (tile)
    g_object_remove_weak_pointer (G_OBJECT (tile), (gpointer *) &data->tile);

  g_free (data);

  if (!tile)
    {
      DEBUG ("Tile destroyed while loading");
      goto error;
    }

  if (!cst)
    goto error;

  /* draw the clutter texture */
  actor = clutter_cairo_texture_new (size, size);

  cr_clutter = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (actor));
  cairo_set_source_surface (cr_clutter, cst, 0, 0);
  cairo_paint (cr_clutter);
  cairo_destroy (cr_clutter);

  /* modify directly the buffer of cairo surface - we don't use it any more
     and we close the surface anyway */
  argb_to_rgba (cairo_image_surface_get_data (cst),
      cairo_image_surface_get_stride (cst) * cairo_image_surface_get_height (cst));

  pixbuf = gdk_pixbuf_new_from_data (cairo_image_surface_get_data (cst),
      GDK_COLORSPACE_RGB, TRUE, 8, size, size,
      cairo_image_surface_get_stride (cst), NULL, NULL);

  if (!gdk_pixbuf_save_to_buffer (pixbuf, &buffer, &buffer_size, "png", &error, NULL))
    goto error;

  champlain_tile_set_content (tile, actor);

  callback_data.error = FALSE;
  callback_data.data = buffer;
  callback_data.size = buffer_size;
  goto finish;

error:
  callback_data.error = TRUE;
  callback_data.data = NULL;
  callback_data.size = 0;

finish:
  g_signal_emit_by_name (tile, "render-complete", &callback_data);

  if (pixbuf)
    g_object_unref (pixbuf);
  if (cst)
    cairo_surface_destroy (cst);
  g_object_unref (renderer);
  g_free (buffer);

  return FALSE;
}


static void
memphis_worker_thread (gpointer worker_data,
    G_GNUC_UNUSED gpointer user_data)
{
  WorkerThreadData *data = (WorkerThreadData *) worker_data;
  ChamplainMemphisRenderer *renderer = CHAMPLAIN_MEMPHIS_RENDERER (data->renderer);
  gboolean has_data = TRUE;

  data->cst = NULL;

/* uncomment when libmemphis works correctly */
/*  g_static_rw_lock_reader_lock (&MemphisLock);
 *  has_data = memphis_renderer_tile_has_data (renderer->priv->renderer, data->x, data->y, data->z);
 *  g_static_rw_lock_reader_unlock (&MemphisLock); */

  if (has_data)
    {
      cairo_t *cr;

      /* create a clutter-independant surface to draw on */
      data->cst = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, data->size, data->size);
      cr = cairo_create (data->cst);

      DEBUG ("Draw Tile (%d, %d, %d)", data->x, data->y, data->z);

      g_static_rw_lock_reader_lock (&MemphisLock);
      memphis_renderer_draw_tile (renderer->priv->renderer, cr, data->x, data->y, data->z);
      g_static_rw_lock_reader_unlock (&MemphisLock);

      cairo_destroy (cr);
    }

  clutter_threads_add_idle_full (G_PRIORITY_DEFAULT, tile_loaded_cb, data, NULL);
}


static void
render (ChamplainRenderer *renderer,
    ChamplainTile *tile)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_RENDERER (renderer));

  ChamplainMemphisRendererPrivate *priv = CHAMPLAIN_MEMPHIS_RENDERER (renderer)->priv;
  GError *error = NULL;
  WorkerThreadData *data;

  DEBUG ("Render tile (%u, %u, %u)", champlain_tile_get_x (tile),
      champlain_tile_get_y (tile),
      champlain_tile_get_zoom_level (tile));

  data = g_new (WorkerThreadData, 1);
  data->x = champlain_tile_get_x (tile);
  data->y = champlain_tile_get_y (tile);
  data->z = champlain_tile_get_zoom_level (tile);
  data->size = priv->tile_size;
  data->tile = tile;
  data->renderer = renderer;

  g_object_add_weak_pointer (G_OBJECT (tile), (gpointer *) &data->tile);
  g_object_ref (renderer);

  g_thread_pool_push (priv->thpool, data, &error);
  if (error)
    {
      g_error ("Thread pool error: %s", error->message);
      g_error_free (error);
      g_free (data);
      g_object_unref (renderer);
      g_object_remove_weak_pointer (G_OBJECT (tile), (gpointer *) &data->tile);
    }
}


static void
set_data (ChamplainRenderer *renderer,
    const gchar *data,
    guint size)
{
  ChamplainMemphisRendererPrivate *priv = GET_PRIVATE (renderer);
  ChamplainBoundingBox *bbox;
  GError *err = NULL;

  MemphisMap *map = memphis_map_new ();

  memphis_map_load_from_data (map, data, size, &err);

  DEBUG ("BBox data received");

  if (err != NULL)
    {
      g_critical ("Can't load map data: \"%s\"", err->message);
      memphis_map_free (map);
      g_error_free (err);
      return;
    }

  g_static_rw_lock_writer_lock (&MemphisLock);
  memphis_renderer_set_map (priv->renderer, map);
  g_static_rw_lock_writer_unlock (&MemphisLock);

  bbox = champlain_bounding_box_new ();

  memphis_map_get_bounding_box (map, &bbox->left, &bbox->top,
      &bbox->right, &bbox->bottom);
  g_object_set (G_OBJECT (renderer), "bounding-box", bbox, NULL);
  champlain_bounding_box_free (bbox);

  g_signal_emit_by_name (CHAMPLAIN_RENDERER (renderer),
      "reload-tiles", NULL);
}


/*
 * champlain_memphis_renderer_load_rules:
 * @renderer: a #ChamplainMemphisRenderer
 * @rules_path: a path to a rules file
 *
 * Loads a Memphis rules file.
 *
 * Since: 0.6
 */
void
champlain_memphis_renderer_load_rules (
    ChamplainMemphisRenderer *renderer,
    const gchar *rules_path)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_RENDERER (renderer));

  ChamplainMemphisRendererPrivate *priv = renderer->priv;
  GError *err = NULL;

  /* TODO: Remove test when memphis handles invalid paths properly */
  if (!g_file_test (rules_path, G_FILE_TEST_EXISTS))
    {
      g_critical ("Error: \"%s\" does not exist.", rules_path);
      return;
    }

  g_static_rw_lock_writer_lock (&MemphisLock);
  if (rules_path)
    {
      memphis_rule_set_load_from_file (priv->rules, rules_path, &err);
      if (err != NULL)
        {
          g_critical ("Can't load rules file: \"%s\"", err->message);
          memphis_rule_set_load_from_data (priv->rules, default_rules,
              strlen (default_rules), NULL);
          g_static_rw_lock_writer_unlock (&MemphisLock);
          g_error_free (err);
          return;
        }
    }
  else
    memphis_rule_set_load_from_data (priv->rules, default_rules,
        strlen (default_rules), NULL);

  g_static_rw_lock_writer_unlock (&MemphisLock);

  g_signal_emit_by_name (CHAMPLAIN_RENDERER (renderer),
      "reload-tiles", NULL);
}


/*
 * champlain_memphis_renderer_get_background_color:
 * @renderer: a #ChamplainMemphisRenderer
 *
 * Gets the background color of the map.
 *
 * Returns: the background color of the map as a newly-allocated
 * #ClutterColor.
 *
 * Since: 0.6
 */
ClutterColor *
champlain_memphis_renderer_get_background_color (
    ChamplainMemphisRenderer *renderer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MEMPHIS_RENDERER (renderer), NULL);

  ClutterColor color;
  guint8 r, b, g, a;

  g_static_rw_lock_reader_lock (&MemphisLock);
  memphis_rule_set_get_bg_color (renderer->priv->rules, &r, &g, &b, &a);
  g_static_rw_lock_reader_unlock (&MemphisLock);

  color.red = r;
  color.green = g;
  color.blue = b;
  color.alpha = a;
  return clutter_color_copy (&color);
}


/*
 * champlain_memphis_renderer_set_background_color:
 * @renderer: a #ChamplainMemphisRenderer
 * @color: a #ClutterColor
 *
 * Sets the background color of the map from a #ClutterColor.
 *
 * Since: 0.6
 */
void
champlain_memphis_renderer_set_background_color (
    ChamplainMemphisRenderer *renderer,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_RENDERER (renderer));

  g_static_rw_lock_writer_lock (&MemphisLock);
  memphis_rule_set_set_bg_color (renderer->priv->rules, color->red,
      color->green, color->blue, color->alpha);
  g_static_rw_lock_writer_unlock (&MemphisLock);

  g_signal_emit_by_name (CHAMPLAIN_RENDERER (renderer),
      "reload-tiles", NULL);
}


/*
 * champlain_memphis_renderer_set_rule:
 * @renderer: a #ChamplainMemphisRenderer
 * @rule: a #MemphisRule
 *
 * Edits or adds a #ChamplainMemphisRule to the rules-set. New rules are appended
 *  to the list.
 *
 * Since: 0.6
 */
void
champlain_memphis_renderer_set_rule (ChamplainMemphisRenderer *renderer,
    ChamplainMemphisRule *rule)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_RENDERER (renderer) &&
      MEMPHIS_RULE (rule));

  g_static_rw_lock_writer_lock (&MemphisLock);
  memphis_rule_set_set_rule (renderer->priv->rules, (MemphisRule *) rule);
  g_static_rw_lock_writer_unlock (&MemphisLock);

  g_signal_emit_by_name (CHAMPLAIN_RENDERER (renderer),
      "reload-tiles", NULL);
}


/*
 * champlain_memphis_renderer_get_rule:
 * @renderer: a #ChamplainMemphisRenderer
 * @id: an id string
 *
 * Gets the requested #ChamplainMemphisRule.
 *
 * Returns: the requested #ChamplainMemphisRule or NULL if none is found.
 *
 * Since: 0.6
 */
ChamplainMemphisRule *
champlain_memphis_renderer_get_rule (ChamplainMemphisRenderer *renderer,
    const gchar *id)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MEMPHIS_RENDERER (renderer) &&
      id != NULL, NULL);

  MemphisRule *rule;

  g_static_rw_lock_reader_lock (&MemphisLock);
  rule = memphis_rule_set_get_rule (renderer->priv->rules, id);
  g_static_rw_lock_reader_unlock (&MemphisLock);

  return (ChamplainMemphisRule *) rule;
}


/*
 * champlain_memphis_renderer_get_rule_ids:
 * @renderer: a #ChamplainMemphisRenderer
 *
 * Get a list of rule id's.
 *
 * Returns: a #GList of id strings of the form:
 * key1|key2|...|keyN:value1|value2|...|valueM
 *
 * Example: "waterway:river|stream|canal"
 *
 * Since: 0.6
 */
GList *
champlain_memphis_renderer_get_rule_ids (ChamplainMemphisRenderer *renderer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MEMPHIS_RENDERER (renderer), NULL);

  GList *list;

  g_static_rw_lock_reader_lock (&MemphisLock);
  list = memphis_rule_set_get_rule_ids (renderer->priv->rules);
  g_static_rw_lock_reader_unlock (&MemphisLock);

  return list;
}


/*
 * champlain_memphis_renderer_remove_rule:
 * @renderer: a #ChamplainMemphisRenderer
 * @id: an id string
 *
 * Removes the rule with the given id.
 *
 * Since: 0.6
 */
void
champlain_memphis_renderer_remove_rule (
    ChamplainMemphisRenderer *renderer,
    const gchar *id)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_RENDERER (renderer));

  g_static_rw_lock_writer_lock (&MemphisLock);
  memphis_rule_set_remove_rule (renderer->priv->rules, id);
  g_static_rw_lock_writer_unlock (&MemphisLock);

  g_signal_emit_by_name (CHAMPLAIN_RENDERER (renderer),
      "reload-tiles", NULL);
}


void
champlain_memphis_renderer_set_tile_size (ChamplainMemphisRenderer *renderer,
    guint size)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_RENDERER (renderer));

  ChamplainMemphisRendererPrivate *priv = GET_PRIVATE (renderer);

  renderer->priv->tile_size = size;

  g_static_rw_lock_writer_lock (&MemphisLock);
  memphis_renderer_set_resolution (priv->renderer, size);
  g_static_rw_lock_writer_unlock (&MemphisLock);

  g_object_notify (G_OBJECT (renderer), "tile-size");
}


guint
champlain_memphis_renderer_get_tile_size (ChamplainMemphisRenderer *renderer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MEMPHIS_RENDERER (renderer), 0);

  return renderer->priv->tile_size;
}


ChamplainBoundingBox *
champlain_memphis_renderer_get_bounding_box (ChamplainMemphisRenderer *renderer)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MEMPHIS_RENDERER (renderer), NULL);

  ChamplainMemphisRendererPrivate *priv = renderer->priv;

  return priv->bbox;
}


static void
set_bounding_box (ChamplainMemphisRenderer *renderer, ChamplainBoundingBox *bbox)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_RENDERER (renderer));

  ChamplainMemphisRendererPrivate *priv = renderer->priv;

  champlain_bounding_box_free (priv->bbox);
  priv->bbox = champlain_bounding_box_copy (bbox);
}
