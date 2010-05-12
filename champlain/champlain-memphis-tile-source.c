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

/**
 * SECTION:champlain-memphis-tile-source
 * @short_description: A map source that draws tiles locally
 *
 * The #ChamplainMemphisTileSource uses the rendering library
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

#include "champlain-memphis-tile-source.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_MEMPHIS
#include "champlain-debug.h"
#include "champlain-tile-cache.h"
#include "champlain-defines.h"
#include "champlain-enum-types.h"
#include "champlain-private.h"

#include <gdk/gdk.h>

#include <errno.h>
#include <string.h>

/* Tuning parameters */
#define MAX_THREADS 4

const gchar default_rules[] =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
  <rules version=\"0.1\" background=\"#ffffff\">\
    <rule e=\"way\" k=\"highway\" v=\"*\">\
      <line color=\"#000000\" width=\"1.0\"/>\
    </rule>\
  </rules>";

enum
{
  PROP_0,
  PROP_MAP_DATA_SOURCE
};

G_DEFINE_TYPE (ChamplainMemphisTileSource, champlain_memphis_tile_source, CHAMPLAIN_TYPE_TILE_SOURCE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_MEMPHIS_TILE_SOURCE, ChamplainMemphisTileSourcePrivate))

struct _ChamplainMemphisTileSourcePrivate
{
  ChamplainMapDataSource *map_data_source;
  MemphisRuleSet *rules;
  MemphisRenderer *renderer;
  GThreadPool *thpool;
  gboolean no_map_data;
};

typedef struct _WorkerThreadData WorkerThreadData;

struct _WorkerThreadData
{
  gint x;
  gint y;
  guint z;
  guint size;

  ChamplainMapSource *map_source;
  ChamplainTile *tile;
  cairo_surface_t *cst;
};

/* lock to protect the renderer state while rendering */
GStaticRWLock MemphisLock = G_STATIC_RW_LOCK_INIT;

static void fill_tile (ChamplainMapSource *map_source,
    ChamplainTile *tile);

static void reload_tiles (ChamplainMemphisTileSource *tile_source);
static void memphis_worker_thread (gpointer data, gpointer user_data);
static void map_data_changed_cb (ChamplainMapDataSource *map_data_source,
    GParamSpec *gobject,
    ChamplainMemphisTileSource *tile_source);
void argb_to_rgba (guchar *data,
    guint size);

static void
champlain_memphis_tile_source_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainMemphisTileSource *tile_source = CHAMPLAIN_MEMPHIS_TILE_SOURCE (object);
  ChamplainMemphisTileSourcePrivate *priv = tile_source->priv;

  switch (property_id)
    {
    case PROP_MAP_DATA_SOURCE:
      g_value_set_object (value, priv->map_data_source);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_memphis_tile_source_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainMemphisTileSource *tile_source = CHAMPLAIN_MEMPHIS_TILE_SOURCE (object);

  switch (property_id)
    {
    case PROP_MAP_DATA_SOURCE:
      champlain_memphis_tile_source_set_map_data_source (tile_source,
          g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_memphis_tile_source_dispose (GObject *object)
{
  ChamplainMemphisTileSource *tile_source = CHAMPLAIN_MEMPHIS_TILE_SOURCE(object);
  ChamplainMemphisTileSourcePrivate *priv = tile_source->priv;

  if (priv->thpool)
    {
      g_thread_pool_free (priv->thpool, FALSE, TRUE);
      priv->thpool = NULL;
    }
  if (priv->map_data_source)
    {
      g_object_unref (priv->map_data_source);
      priv->map_data_source = NULL;
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

  G_OBJECT_CLASS (champlain_memphis_tile_source_parent_class)->dispose (object);
}

static void
champlain_memphis_tile_source_finalize (GObject *object)
{
  G_OBJECT_CLASS (champlain_memphis_tile_source_parent_class)->finalize (object);
}

static void
champlain_memphis_tile_source_constructed (GObject *object)
{
  ChamplainMapSource *map_source = CHAMPLAIN_MAP_SOURCE(object);
  ChamplainMemphisTileSourcePrivate *priv = CHAMPLAIN_MEMPHIS_TILE_SOURCE (object)->priv;

  memphis_renderer_set_resolution (priv->renderer, champlain_map_source_get_tile_size (map_source));

  G_OBJECT_CLASS (champlain_memphis_tile_source_parent_class)->constructed (object);
}

static void
champlain_memphis_tile_source_class_init (ChamplainMemphisTileSourceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ChamplainMapSourceClass *map_source_class = CHAMPLAIN_MAP_SOURCE_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainMemphisTileSourcePrivate));

  object_class->get_property = champlain_memphis_tile_source_get_property;
  object_class->set_property = champlain_memphis_tile_source_set_property;
  object_class->dispose = champlain_memphis_tile_source_dispose;
  object_class->finalize = champlain_memphis_tile_source_finalize;
  object_class->constructed = champlain_memphis_tile_source_constructed;

  map_source_class->fill_tile = fill_tile;

  /**
  * ChamplainMemphisTileSource:map-data-source:
  *
  * The data source of the renderer
  *
  * Since: 0.6
  */
  g_object_class_install_property (object_class,
      PROP_MAP_DATA_SOURCE,
      g_param_spec_object ("map-data-source",
          "Map data source",
          "The data source of the renderer",
          CHAMPLAIN_TYPE_MAP_DATA_SOURCE,
          G_PARAM_CONSTRUCT | G_PARAM_READWRITE));
}

static void
champlain_memphis_tile_source_init (ChamplainMemphisTileSource *tile_source)
{
  ChamplainMemphisTileSourcePrivate *priv = GET_PRIVATE(tile_source);

  tile_source->priv = priv;

  priv->map_data_source = NULL;
  priv->rules = NULL;
  priv->renderer = NULL;
  priv->no_map_data = TRUE;

  priv->rules = memphis_rule_set_new ();
  memphis_rule_set_load_from_data (priv->rules, default_rules,
      strlen (default_rules), NULL);

  priv->renderer = memphis_renderer_new_full (priv->rules, memphis_map_new ());

  priv->thpool = g_thread_pool_new (memphis_worker_thread, tile_source,
      MAX_THREADS, FALSE, NULL);
}

/**
 * champlain_memphis_tile_source_new_full:
 * @id: the map source's id
 * @name: the map source's name
 * @license: the map source's license
 * @license_uri: the map source's license URI
 * @min_zoom: the map source's minimum zoom level
 * @max_zoom: the map source's maximum zoom level
 * @tile_size: the map source's tile size (in pixels)
 * @projection: the map source's projection
 * @map_data_source: a #ChamplainMapDataSource
 *
 * Creates a new instance of ChamplainMemphisTileSource.
 *
 * Returns: a new ChamplainMemphisTileSource.
 *
 * Since: 0.6
 */
ChamplainMemphisTileSource* champlain_memphis_tile_source_new_full (const gchar *id,
    const gchar *name,
    const gchar *license,
    const gchar *license_uri,
    guint min_zoom_level,
    guint max_zoom_level,
    guint tile_size,
    ChamplainMapProjection projection,
    ChamplainMapDataSource *map_data_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MAP_DATA_SOURCE (map_data_source), NULL);

  return g_object_new (CHAMPLAIN_TYPE_MEMPHIS_TILE_SOURCE,
             "id", id,
             "name", name,
             "license", license,
             "license-uri", license_uri,
             "min-zoom-level", min_zoom_level,
             "max-zoom-level", max_zoom_level,
             "tile-size", tile_size,
             "projection", projection,
             "map-data-source", map_data_source,
             NULL);
}

static void
map_data_changed_cb (ChamplainMapDataSource *map_data_source,
     G_GNUC_UNUSED GParamSpec *gobject,
     ChamplainMemphisTileSource *tile_source)
{
  g_assert (CHAMPLAIN_IS_MAP_DATA_SOURCE (map_data_source) &&
            CHAMPLAIN_IS_MEMPHIS_TILE_SOURCE (tile_source));

  MemphisMap *map;
  ChamplainMemphisTileSourcePrivate *priv = tile_source->priv;
  ChamplainState state;

  g_object_get (G_OBJECT (map_data_source), "state", &state, NULL);
  if (state != CHAMPLAIN_STATE_DONE)
    return;

  map = champlain_map_data_source_get_map_data (map_data_source);
  if (map == NULL)
    {
      map = memphis_map_new ();
      priv->no_map_data = TRUE;
    }
  else
    priv->no_map_data = FALSE;

  DEBUG ("DataSource has been changed!");

  g_static_rw_lock_writer_lock (&MemphisLock);
  memphis_renderer_set_map (priv->renderer, map);
  g_static_rw_lock_writer_unlock (&MemphisLock);

  reload_tiles (tile_source);
}

/*
Transform ARGB (Cairo) to RGBA (GdkPixbuf). RGBA is actualy reversed in
memory, so the transformation is ARGB -> ABGR (i.e. swapping B and R)
*/
void argb_to_rgba (guchar *data,
    guint size)
{
  guint32 *ptr;
  guint32 *endptr = (guint32 *)data + size / 4;
  for (ptr = (guint32 *)data; ptr < endptr; ptr++)
    *ptr = (*ptr & 0xFF00FF00) ^ ((*ptr & 0xFF0000) >> 16) ^ ((*ptr & 0xFF) << 16);
}

static gboolean
tile_loaded_cb (gpointer worker_data)
{
  WorkerThreadData *data = (WorkerThreadData *) worker_data;
  ChamplainMapSource *map_source = data->map_source;
  ChamplainTile *tile = data->tile;
  cairo_surface_t *cst = data->cst;
  ChamplainTileSource *tile_source = CHAMPLAIN_TILE_SOURCE(map_source);
  ChamplainTileCache *tile_cache = champlain_tile_source_get_cache (tile_source);
  cairo_t *cr_clutter;
  ClutterActor *actor;
  guint size = data->size;
  GError *error = NULL;

  if (tile)
    g_object_remove_weak_pointer (G_OBJECT (tile), (gpointer*)&data->tile);

  g_free (data);

  if (!tile)
    {
      DEBUG ("Tile destroyed while loading");
      goto cleanup;
    }

  if (!cst)
    {
      /* tile not rendered, load next */
      ChamplainMapSource *next_source = champlain_map_source_get_next_source (map_source);
      if (next_source)
        champlain_map_source_fill_tile (next_source, tile);
      goto cleanup;
    }

  /* draw the clutter texture */
  actor = clutter_cairo_texture_new (size, size);

  cr_clutter = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE (actor));
  cairo_set_source_surface (cr_clutter, cst, 0, 0);
  cairo_paint (cr_clutter);
  cairo_destroy (cr_clutter);

  /* update the cache */
  if (tile_cache)
    {
      GdkPixbuf * pixbuf;
      gchar *buffer;
      gsize buffer_size;

      /* modify directly the buffer of cairo surface - we don't use it any more
         and we close the surface anyway */
      argb_to_rgba (cairo_image_surface_get_data (cst),
          cairo_image_surface_get_stride (cst) * cairo_image_surface_get_height (cst));

      pixbuf = gdk_pixbuf_new_from_data (cairo_image_surface_get_data (cst),
                   GDK_COLORSPACE_RGB, TRUE, 8, size, size,
                   cairo_image_surface_get_stride (cst), NULL, NULL);

      if (gdk_pixbuf_save_to_buffer (pixbuf, &buffer, &buffer_size, "png", &error, NULL))
        champlain_tile_cache_store_tile (tile_cache, tile, buffer, buffer_size);

      g_free (buffer);
      g_object_unref (pixbuf);
    }

  champlain_tile_set_content (tile, actor);
  champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);

cleanup:
  if (cst)
    cairo_surface_destroy (cst);
  g_object_unref (map_source);

  return FALSE;
}

static void
memphis_worker_thread (gpointer worker_data,
    G_GNUC_UNUSED gpointer user_data)
{
  WorkerThreadData *data = (WorkerThreadData *)worker_data;
  ChamplainMemphisTileSource *tile_source = CHAMPLAIN_MEMPHIS_TILE_SOURCE (data->map_source);
  gboolean has_data = TRUE;

  data->cst = NULL;

// uncomment when libmemphis works correctly
//  g_static_rw_lock_reader_lock (&MemphisLock);
//  has_data = memphis_renderer_tile_has_data (tile_source->priv->renderer, data->x, data->y, data->z);
//  g_static_rw_lock_reader_unlock (&MemphisLock);

  if (has_data)
    {
      cairo_t *cr;

      /* create a clutter-independant surface to draw on */
      data->cst = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, data->size, data->size);
      cr = cairo_create (data->cst);

      DEBUG ("Draw Tile (%d, %d, %d)", data->x, data->y, data->z);

      g_static_rw_lock_reader_lock (&MemphisLock);
      memphis_renderer_draw_tile (tile_source->priv->renderer, cr, data->x, data->y, data->z);
      g_static_rw_lock_reader_unlock (&MemphisLock);

      cairo_destroy (cr);
    }

  clutter_threads_add_idle_full (G_PRIORITY_DEFAULT, tile_loaded_cb, data, NULL);
}

static void
fill_tile (ChamplainMapSource *map_source,
    ChamplainTile *tile)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_TILE_SOURCE (map_source));

  ChamplainMemphisTileSourcePrivate *priv = CHAMPLAIN_MEMPHIS_TILE_SOURCE (map_source)->priv;

  DEBUG ("Render tile (%u, %u, %u)", champlain_tile_get_x (tile),
         champlain_tile_get_y (tile),
         champlain_tile_get_zoom_level (tile));

  if (priv->no_map_data)
    {
      ChamplainMapSource *next_source = champlain_map_source_get_next_source (map_source);

      if (CHAMPLAIN_IS_MAP_SOURCE(next_source))
        champlain_map_source_fill_tile (next_source, tile);
    }
  else
    {
      GError *error = NULL;
      WorkerThreadData *data;

      data = g_new (WorkerThreadData, 1);
      data->x = champlain_tile_get_x (tile);
      data->y = champlain_tile_get_y (tile);
      data->z = champlain_tile_get_zoom_level (tile);
      data->size = champlain_map_source_get_tile_size (map_source);
      data->tile = tile;
      data->map_source = map_source;

      g_object_add_weak_pointer (G_OBJECT (tile), (gpointer*)&data->tile);

      g_object_ref (map_source);

      g_thread_pool_push (priv->thpool, data, &error);
      if (error)
        {
          g_error ("Thread pool error: %s", error->message);
          g_error_free (error);
          g_object_unref (map_source);
          g_free (data);
        }
    }
}

static void
reload_tiles (ChamplainMemphisTileSource *tile_source)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_TILE_SOURCE (tile_source));

  ChamplainTileSource *tile_src = CHAMPLAIN_TILE_SOURCE(tile_source);
  ChamplainTileCache *tile_cache = champlain_tile_source_get_cache (tile_src);

  if (tile_cache && !champlain_tile_cache_get_persistent (tile_cache))
    {
      DEBUG ("Clean temporary cache");

      champlain_tile_cache_clean (tile_cache);
    }

  g_signal_emit_by_name (CHAMPLAIN_MAP_SOURCE (tile_src),
                         "reload-tiles", NULL);
}

/**
 * champlain_memphis_tile_source_load_rules:
 * @tile_source: a #ChamplainMemphisTileSource
 * @rules_path: a path to a rules file
 *
 * Loads a Memphis rules file.
 *
 * Since: 0.6
 */
void
champlain_memphis_tile_source_load_rules (
  ChamplainMemphisTileSource *tile_source,
  const gchar *rules_path)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_TILE_SOURCE (tile_source));

  // TODO: Remove test when memphis handles invalid paths properly
  if (!g_file_test (rules_path, G_FILE_TEST_EXISTS))
    {
      g_critical ("Error: \"%s\" does not exist.", rules_path);
      return;
    }

  ChamplainMemphisTileSourcePrivate *priv = tile_source->priv;
  GError *err = NULL;

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

  reload_tiles (tile_source);
}

/**
 * champlain_memphis_tile_source_set_map_data_source:
 * @tile_source: a #ChamplainMemphisTileSource
 * @map_data_source: a #ChamplainMapDataSource
 *
 * Sets the map data source.
 *
 * Since: 0.6
 */
void
champlain_memphis_tile_source_set_map_data_source (
    ChamplainMemphisTileSource *tile_source,
    ChamplainMapDataSource *map_data_source)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_TILE_SOURCE (tile_source) &&
                    CHAMPLAIN_IS_MAP_DATA_SOURCE (map_data_source));

  ChamplainMemphisTileSourcePrivate *priv = tile_source->priv;
  MemphisMap *map;

  if (priv->map_data_source)
    g_object_unref (priv->map_data_source);

  priv->map_data_source = g_object_ref_sink (map_data_source);

  g_signal_connect (priv->map_data_source, "notify::state",
      G_CALLBACK (map_data_changed_cb), tile_source);

  map = champlain_map_data_source_get_map_data (priv->map_data_source);
  if (map == NULL)
    {
      map = memphis_map_new ();
      priv->no_map_data = TRUE;
    }
  else
    priv->no_map_data = FALSE;

  g_static_rw_lock_writer_lock (&MemphisLock);
  memphis_renderer_set_map (priv->renderer, map);
  g_static_rw_lock_writer_unlock (&MemphisLock);
}

/**
 * champlain_memphis_tile_source_get_map_data_source:
 * @tile_source: a #ChamplainMemphisTileSource
 *
 * Gets the assigned #ChamplainMapDataSource.
 *
 * Returns: the #ChamplainMapDataSource.
 *
 * Since: 0.6
 */
ChamplainMapDataSource *
champlain_memphis_tile_source_get_map_data_source (
    ChamplainMemphisTileSource *tile_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MEMPHIS_TILE_SOURCE (tile_source), NULL);

  return tile_source->priv->map_data_source;
}

/**
 * champlain_memphis_tile_source_get_background_color:
 * @tile_source: a #ChamplainMemphisTileSource
 *
 * Gets the background color of the map.
 *
 * Returns: the background color of the map as a newly-allocated
 * #ClutterColor.
 *
 * Since: 0.6
 */
ClutterColor *
champlain_memphis_tile_source_get_background_color (
    ChamplainMemphisTileSource *tile_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MEMPHIS_TILE_SOURCE (tile_source), NULL);

  ClutterColor color;
  guint8 r, b, g, a;

  g_static_rw_lock_reader_lock (&MemphisLock);
  memphis_rule_set_get_bg_color (tile_source->priv->rules, &r, &g, &b, &a);
  g_static_rw_lock_reader_unlock (&MemphisLock);

  color.red = r;
  color.green = g;
  color.blue = b;
  color.alpha = a;
  return clutter_color_copy (&color);
}

/**
 * champlain_memphis_tile_source_set_background_color:
 * @tile_source: a #ChamplainMemphisTileSource
 * @color: a #ClutterColor
 *
 * Sets the background color of the map from a #ClutterColor.
 *
 * Since: 0.6
 */
void
champlain_memphis_tile_source_set_background_color (
    ChamplainMemphisTileSource *tile_source,
    const ClutterColor *color)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_TILE_SOURCE (tile_source));

  g_static_rw_lock_writer_lock (&MemphisLock);
  memphis_rule_set_set_bg_color (tile_source->priv->rules, color->red,
                                 color->green, color->blue, color->alpha);
  g_static_rw_lock_writer_unlock (&MemphisLock);

  reload_tiles (tile_source);
}

/**
 * champlain_memphis_tile_source_set_rule:
 * @tile_source: a #ChamplainMemphisTileSource
 * @rule: a #MemphisRule
 *
 * Edits or adds a #MemphisRule to the rules-set. New rules are appended
 *  to the list.
 *
 * Since: 0.6
 */
void
champlain_memphis_tile_source_set_rule (ChamplainMemphisTileSource *tile_source,
    MemphisRule *rule)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_TILE_SOURCE (tile_source) &&
                    MEMPHIS_RULE (rule));

  g_static_rw_lock_writer_lock (&MemphisLock);
  memphis_rule_set_set_rule (tile_source->priv->rules, rule);
  g_static_rw_lock_writer_unlock (&MemphisLock);

  reload_tiles (tile_source);
}

/**
 * champlain_memphis_tile_source_get_rule:
 * @tile_source: a #ChamplainMemphisTileSource
 * @id: an id string
 *
 * Gets the requested #MemphisRule.
 *
 * Returns: the requested #MemphisRule or NULL if none is found.
 *
 * Since: 0.6
 */
MemphisRule *
champlain_memphis_tile_source_get_rule (ChamplainMemphisTileSource *tile_source,
    const gchar *id)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MEMPHIS_TILE_SOURCE (tile_source) &&
                        id != NULL, NULL);

  MemphisRule *rule;

  g_static_rw_lock_reader_lock (&MemphisLock);
  rule = memphis_rule_set_get_rule (tile_source->priv->rules, id);
  g_static_rw_lock_reader_unlock (&MemphisLock);

  return rule;
}

/**
 * champlain_memphis_tile_source_get_rule_ids:
 * @tile_source: a #ChamplainMemphisTileSource
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
champlain_memphis_tile_source_get_rule_ids (ChamplainMemphisTileSource *tile_source)
{
  g_return_val_if_fail (CHAMPLAIN_IS_MEMPHIS_TILE_SOURCE (tile_source), NULL);

  GList *list;

  g_static_rw_lock_reader_lock (&MemphisLock);
  list = memphis_rule_set_get_rule_ids (tile_source->priv->rules);
  g_static_rw_lock_reader_unlock (&MemphisLock);

  return list;
}

/**
 * champlain_memphis_tile_source_remove_rule:
 * @tile_source: a #ChamplainMemphisTileSource
 * @id: an id string
 *
 * Removes the rule with the given id.
 *
 * Since: 0.6
 */
void champlain_memphis_tile_source_remove_rule (
    ChamplainMemphisTileSource *tile_source,
    const gchar *id)
{
  g_return_if_fail (CHAMPLAIN_IS_MEMPHIS_TILE_SOURCE (tile_source));

  g_static_rw_lock_writer_lock (&MemphisLock);
  memphis_rule_set_remove_rule (tile_source->priv->rules, id);
  g_static_rw_lock_writer_unlock (&MemphisLock);

  reload_tiles (tile_source);
}
