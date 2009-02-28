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

#include "config.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_LOADING
#include "champlain-debug.h"

#include "champlain.h"
#include "champlain-defines.h"
#include "champlain-enum-types.h"
#include "champlain-map-source.h"
#include "champlain-marshal.h"
#include "champlain-private.h"
#include "champlain-zoom-level.h"

#include <errno.h>
#include <gdk/gdk.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib-object.h>
#include <libsoup/soup.h>
#include <math.h>

enum
{
  /* normal signals */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_NAME,
  PROP_LICENSE,
  PROP_LICENSE_URI,
  PROP_MAX_ZOOM_LEVEL,
  PROP_MIN_ZOOM_LEVEL,
  PROP_TILE_SIZE,
  PROP_MAP_PROJECTION
};

/* static guint champlain_map_source_signals[LAST_SIGNAL] = { 0, }; */

G_DEFINE_TYPE (ChamplainMapSource, champlain_map_source, G_TYPE_OBJECT);

#define GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_MAP_SOURCE, ChamplainMapSourcePrivate))

#define CACHE_SUBDIR "champlain"
static SoupSession * soup_session;

struct _ChamplainMapSourcePrivate
{
  gchar *name;
  gchar *license;
  gchar *license_uri;
  guint max_zoom_level;
  guint min_zoom_level;
  guint tile_size;
  ChamplainMapProjection map_projection;
  gchar *tile_uri_format;
  ChamplainMapSourceParameter first_param;
  ChamplainMapSourceParameter second_param;
  ChamplainMapSourceParameter third_param;
};

static void
champlain_map_source_get_property (GObject *object,
                                   guint prop_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
  ChamplainMapSource *map_source = CHAMPLAIN_MAP_SOURCE(object);
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);

  switch(prop_id)
    {
      case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;
      case PROP_LICENSE:
        g_value_set_string (value, priv->license);
        break;
      case PROP_LICENSE_URI:
        g_value_set_string (value, priv->license_uri);
        break;
      case PROP_MAX_ZOOM_LEVEL:
        g_value_set_uint (value, priv->max_zoom_level);
        break;
      case PROP_MIN_ZOOM_LEVEL:
        g_value_set_uint (value, priv->min_zoom_level);
        break;
      case PROP_TILE_SIZE:
        g_value_set_uint (value, priv->tile_size);
        break;
      case PROP_MAP_PROJECTION:
        g_value_set_enum (value, priv->map_projection);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
champlain_map_source_set_property (GObject *object,
                                   guint prop_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
  ChamplainMapSource *map_source = CHAMPLAIN_MAP_SOURCE(object);
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);

  switch(prop_id)
    {
      case PROP_NAME:
        priv->name = g_value_dup_string (value);
        break;
      case PROP_LICENSE:
        priv->license = g_value_dup_string (value);
        break;
      case PROP_LICENSE_URI:
        priv->license_uri = g_value_dup_string (value);
        break;
      case PROP_MAX_ZOOM_LEVEL:
        priv->max_zoom_level = g_value_get_uint (value);
        break;
      case PROP_MIN_ZOOM_LEVEL:
        priv->min_zoom_level = g_value_get_uint (value);
        break;
      case PROP_TILE_SIZE:
        priv->tile_size = g_value_get_uint (value);
        break;
      case PROP_MAP_PROJECTION:
        priv->map_projection = g_value_get_enum (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
champlain_map_source_finalize (GObject *object)
{
  /* ChamplainMapSource *map_source = CHAMPLAIN_MAP_SOURCE (object);
   * ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);
   */

  G_OBJECT_CLASS (champlain_map_source_parent_class)->finalize (object);
}

static void
champlain_map_source_class_init (ChamplainMapSourceClass *klass)
{
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (ChamplainMapSourcePrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = champlain_map_source_finalize;
  object_class->get_property = champlain_map_source_get_property;
  object_class->set_property = champlain_map_source_set_property;

  /**
  * ChamplainMapSource:name:
  *
  * The name of the map source
  *
  * Since: 0.4
  */
  pspec = g_param_spec_string ("name",
                               "Name",
                               "The name of the map source",
                               "",
                               (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_NAME, pspec);

  /**
  * ChamplainMapSource:license:
  *
  * The usage license of the map source
  *
  * Since: 0.4
  */
  pspec = g_param_spec_string ("license",
                               "License",
                               "The usage license of the map source",
                               "",
                               (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_LICENSE, pspec);

  /**
  * ChamplainMapSource:license-uri:
  *
  * The usage license's uri for more information 
  *
  * Since: 0.4
  */
  pspec = g_param_spec_string ("license-uri",
                               "License-uri",
                               "The usage license's uri for more information",
                               "",
                               (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_LICENSE_URI, pspec);

  /**
  * ChamplainMapSource:max-zoom-level:
  *
  * The maximum zoom level
  *
  * Since: 0.4
  */
  pspec = g_param_spec_uint ("max-zoom-level",
                             "Maximum Zoom Level",
                             "The maximum zoom level",
                             0,
                             50,
                             18,
                             (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_MAX_ZOOM_LEVEL, pspec);

  /**
  * ChamplainMapSource:min-zoom-level:
  *
  * The minimum zoom level
  *
  * Since: 0.4
  */
  pspec = g_param_spec_uint ("min-zoom-level",
                             "Minimum Zoom Level",
                             "The minimum zoom level",
                             0,
                             50,
                             0,
                             (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_MIN_ZOOM_LEVEL, pspec);

  /**
  * ChamplainMapSource:tile-size:
  *
  * The tile size of the map source
  *
  * Since: 0.4
  */
  pspec = g_param_spec_uint ("tile-size",
                             "Tile Size",
                             "The tile size",
                             0,
                             2048,
                             256,
                             (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_TILE_SIZE, pspec);

  /**
  * ChamplainMapSource:map-projection
  *
  * The map projection of the map source
  *
  * Since: 0.4
  */
  pspec = g_param_spec_enum ("map-projection",
                             "Map Projection",
                             "The map projection",
                             CHAMPLAIN_TYPE_MAP_PROJECTION,
                             CHAMPLAIN_MAP_PROJECTION_MERCATOR,
                             (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_MAP_PROJECTION, pspec);

}

static void
champlain_map_source_init (ChamplainMapSource *champlainMapSource)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (champlainMapSource);
}

gint
champlain_map_source_get_max_zoom_level (ChamplainMapSource *map_source)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);
  return priv->max_zoom_level;
}

gint
champlain_map_source_get_min_zoom_level (ChamplainMapSource *map_source)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);
  return priv->min_zoom_level;
}

guint
champlain_map_source_get_tile_size (ChamplainMapSource *map_source)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);
  return priv->tile_size;
}

ChamplainMapSource*
champlain_map_source_new_network (gchar *name,
                                  gchar *license,
                                  gchar *license_uri,
                                  guint min_zoom,
                                  guint max_zoom,
                                  guint tile_size,
                                  ChamplainMapProjection projection,
                                  gchar *uri_format,
                                  ChamplainMapSourceParameter first,
                                  ChamplainMapSourceParameter second,
                                  ChamplainMapSourceParameter third)
{

  ChamplainMapSource * map_source;
  map_source = g_object_new (CHAMPLAIN_TYPE_MAP_SOURCE, "name", name,
      "license", license, "license-uri", license_uri,
      "min-zoom-level", min_zoom, "max-zoom-level", max_zoom,
      "tile-size", tile_size, "map-projection", projection, NULL);
//FIXME no function call in a _new ()
  champlain_map_source_set_tile_uri (map_source, uri_format, first, second,
      third);
  return map_source;
}

static
get_value (guint x, guint y, guint z, ChamplainMapSourceParameter param)
{
  switch (param)
    {
      case CHAMPLAIN_MAP_SOURCE_PARAMETER_X:
        return x;
      case CHAMPLAIN_MAP_SOURCE_PARAMETER_Y:
        return y;
      case CHAMPLAIN_MAP_SOURCE_PARAMETER_Z:
        return z;
    }
}

gchar *
champlain_map_source_get_tile_uri (ChamplainMapSource *map_source,
                                   guint x,
                                   guint y,
                                   gint z)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);

  guint first;
  guint second;
  guint third;

  first = get_value (x, y, z, priv->first_param);
  second = get_value (x, y, z, priv->second_param);
  third = get_value (x, y, z, priv->third_param);

  return g_strdup_printf (priv->tile_uri_format, first, second, third);

}

void
champlain_map_source_set_tile_uri (ChamplainMapSource *map_source,
                                   const gchar *uri_format,
                                   ChamplainMapSourceParameter first,
                                   ChamplainMapSourceParameter second,
                                   ChamplainMapSourceParameter third)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);

  priv->first_param = first;
  priv->second_param = second;
  priv->third_param = third;

  priv->tile_uri_format = g_strdup (uri_format);

}

ChamplainMapSource *
champlain_map_source_new_osm_mapnik ()
{
  champlain_map_source_new_network ("OpenStreetMap Mapnik",
      "(CC) BY 2.0 OpenStreetMap contributors",
      "http://creativecommons.org/licenses/by/2.0/", 0, 18, 256,
      CHAMPLAIN_MAP_PROJECTION_MERCATOR,
      "http://tile.openstreetmap.org/%d/%d/%d.png",
      CHAMPLAIN_MAP_SOURCE_PARAMETER_Z,
      CHAMPLAIN_MAP_SOURCE_PARAMETER_X,
      CHAMPLAIN_MAP_SOURCE_PARAMETER_Y);
}

ChamplainMapSource *
champlain_map_source_new_oam ()
{
  champlain_map_source_new_network ("OpenArialMap",
      "(CC) BY 3.0 OpenArialMap contributors",
      "http://creativecommons.org/licenses/by/3.0/", 0, 17, 256,
      CHAMPLAIN_MAP_PROJECTION_MERCATOR,
      "http://tile.openaerialmap.org/tiles/1.0.0/openaerialmap-900913/%d/%d/%d.jpg",
      CHAMPLAIN_MAP_SOURCE_PARAMETER_Z,
      CHAMPLAIN_MAP_SOURCE_PARAMETER_X,
      CHAMPLAIN_MAP_SOURCE_PARAMETER_Y);
}

//FIXME: the API isn't enough flexible for mff's url!
ChamplainMapSource *
champlain_map_source_new_mff_relief ()
{
  champlain_map_source_new_network ("MapsForFree Relief",
      "Map data available under GNU Free Documentation license, Version 1.2 or later",
      "http://www.gnu.org/copyleft/fdl.html", 0, 11, 256,
      CHAMPLAIN_MAP_PROJECTION_MERCATOR,
      "http://maps-for-free.com/layer/relief/z%d/row%d/%d_%d-%d.jpg",
      CHAMPLAIN_MAP_SOURCE_PARAMETER_Z,
      CHAMPLAIN_MAP_SOURCE_PARAMETER_X,
      CHAMPLAIN_MAP_SOURCE_PARAMETER_Y);
}

guint
champlain_map_source_get_x (ChamplainMapSource *map_source,
                            gint zoom_level,
                            gdouble longitude)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);
  // FIXME: support other projections
  return ((longitude + 180.0) / 360.0 * pow(2.0, zoom_level)) * priv->tile_size;
}

guint
champlain_map_source_get_y (ChamplainMapSource *map_source,
                            gint zoom_level,
                            gdouble latitude)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);
  // FIXME: support other projections
  return ((1.0 - log (tan (latitude * M_PI / 180.0) + 1.0 /
          cos (latitude * M_PI / 180.0)) /
        M_PI) / 2.0 * pow (2.0, zoom_level)) * priv->tile_size;
}

guint
champlain_map_source_get_row_count (ChamplainMapSource *map_source,
                                    gint zoom_level)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);
  // FIXME: support other projections
  return pow (2, zoom_level);
}

guint
champlain_map_source_get_column_count (ChamplainMapSource *map_source,
                                       gint zoom_level)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);
  // FIXME: support other projections
  return pow (2, zoom_level);
}

static gchar *
get_filename (ChamplainMapSource *map_source,
              ChamplainZoomLevel *level,
              ChamplainTile *tile)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);
  return g_strdup_printf ("%s" G_DIR_SEPARATOR_S "%s" G_DIR_SEPARATOR_S
             "%s" G_DIR_SEPARATOR_S "%d" G_DIR_SEPARATOR_S
             "%d" G_DIR_SEPARATOR_S "%d.png", g_get_user_cache_dir (),
             CACHE_SUBDIR, priv->name,
             champlain_zoom_level_get_zoom_level (level),
             champlain_tile_get_x (tile), champlain_tile_get_y (tile),
             NULL);
}

typedef struct {
  ChamplainView *view;
  ChamplainZoomLevel *zoom_level;
  ChamplainTile *tile;
} FileLoadedCallbackContext;

static void
create_error_tile (ChamplainTile* tile)
{
  ClutterActor *actor;
  actor = clutter_texture_new_from_file (DATADIR "/champlain/error.svg", NULL);
  if (!actor)
    return;

  champlain_tile_set_actor (tile, actor);
  clutter_actor_show (actor);

  champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);
}

static void
file_loaded_cb (SoupSession *session,
                SoupMessage *msg,
                gpointer user_data)
{
  FileLoadedCallbackContext *ctx = (FileLoadedCallbackContext*) user_data;
  GdkPixbufLoader* loader;
  GError *error = NULL;
  gchar* path, *filename;

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      g_warning ("Unable to download tile %d, %d: %s",
          champlain_tile_get_x (ctx->tile),
          champlain_tile_get_y (ctx->tile),
          soup_status_get_phrase(msg->status_code));
      g_object_unref (ctx->tile);
      create_error_tile (ctx->tile);
      return;
    }

  loader = gdk_pixbuf_loader_new();
  if (!gdk_pixbuf_loader_write (loader,
                          (const guchar *) msg->response_body->data,
                          msg->response_body->length,
                          &error))
    {
      if (error)
        {
          g_warning ("Unable to load the pixbuf: %s", error->message);
          g_error_free (error);
          create_error_tile (ctx->tile);
          goto finish;
        }

      g_object_unref (loader);
    }

  gdk_pixbuf_loader_close (loader, &error);
  if (error)
    {
      g_warning ("Unable to close the pixbuf loader: %s", error->message);
      g_error_free (error);
      create_error_tile (ctx->tile);
      goto finish;
    }

  filename = champlain_tile_get_filename (ctx->tile);
  path = g_path_get_dirname (filename);

  if (g_mkdir_with_parents (path, 0700) == -1)
    {
      if (errno != EEXIST)
        {
          g_warning ("Unable to create the image cache: %s",
                     g_strerror (errno));
          g_object_unref (loader);
        }
    }

  g_file_set_contents (filename, msg->response_body->data,
      msg->response_body->length, NULL);

  /* If the tile has been marked to be deleted, don't go any further */
  /*if (tile->to_destroy)
    {
      g_object_unref (loader);
      g_free (filename);
      g_free (map_filename);
      g_free (tile);
      return;
    }
*/
  GdkPixbuf* pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
  ClutterActor *actor = clutter_texture_new();
  clutter_texture_set_from_rgb_data (CLUTTER_TEXTURE (actor),
      gdk_pixbuf_get_pixels (pixbuf),
      gdk_pixbuf_get_has_alpha (pixbuf),
      gdk_pixbuf_get_width (pixbuf),
      gdk_pixbuf_get_height (pixbuf),
      gdk_pixbuf_get_rowstride (pixbuf),
      3, 0, NULL);
  champlain_tile_set_actor (ctx->tile, actor);
  DEBUG ("Tile loaded from network");

finish:
  champlain_tile_set_state (ctx->tile, CHAMPLAIN_STATE_DONE);

  g_object_unref (loader);
  g_free (path);

  champlain_view_tile_ready (ctx->view, ctx->zoom_level, ctx->tile, TRUE);
  g_object_unref (ctx->tile);
  g_object_unref (ctx->zoom_level);
  g_free (ctx);
}

void
champlain_map_source_get_tile (ChamplainMapSource *map_source,
                               ChamplainView *view,
                               ChamplainZoomLevel *zoom_level,
                               ChamplainTile *tile)
{
  gchar* filename;

  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);

  /* Ref the tile as it may be freeing during the loading
   * Unref when the loading is done.
   */
  g_object_ref (tile);
  g_object_ref (zoom_level);

  /* Try the cached version first */
  filename = get_filename (map_source, zoom_level, tile);
  champlain_tile_set_filename (tile, filename);
  champlain_tile_set_size (tile, champlain_map_source_get_tile_size (map_source));

  if (g_file_test (filename, G_FILE_TEST_EXISTS))
    {
      GError *error = NULL;
      ClutterActor *actor;

      actor = clutter_texture_new_from_file (filename, &error);
      champlain_tile_set_actor (tile, actor);
      clutter_actor_show (actor);

      champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);
      DEBUG ("Tile loaded from cache");
      champlain_view_tile_ready (view, zoom_level, tile, FALSE);
      g_object_unref (tile);
      g_object_unref (zoom_level);
    }
  else /* if (!offline) */
    {
      SoupMessage *msg;
      const gchar *uri;
      FileLoadedCallbackContext *ctx = g_new0 (FileLoadedCallbackContext, 1);
      ctx->view = view;
      ctx->zoom_level = zoom_level;
      ctx->tile = tile;

      if (!soup_session)
        soup_session = soup_session_async_new ();

      uri = champlain_map_source_get_tile_uri (map_source,
               champlain_tile_get_x (tile), champlain_tile_get_y (tile),
               champlain_zoom_level_get_zoom_level (zoom_level));
      champlain_tile_set_uri (tile, uri);
      champlain_tile_set_state (tile, CHAMPLAIN_STATE_LOADING);
      msg = soup_message_new (SOUP_METHOD_GET, uri);

      soup_session_queue_message (soup_session, msg,
                                  file_loaded_cb,
                                  ctx);
    }
  /* If a tile is neither in cache or can be fetched, do nothing, it'll show up
   * as empty
   */
}

gdouble
champlain_map_source_get_longitude (ChamplainMapSource *map_source,
                                    gint zoom_level,
                                    guint x)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);
  // FIXME: support other projections
  gdouble dx = (float)x / champlain_map_source_get_tile_size (map_source);
  return dx / pow (2.0, zoom_level) * 360.0 - 180;
}

gdouble
champlain_map_source_get_latitude (ChamplainMapSource *map_source,
                                   gint zoom_level,
                                   guint y)
{
  ChamplainMapSourcePrivate *priv = GET_PRIVATE (map_source);
  // FIXME: support other projections
  gdouble dy = (float)y / champlain_map_source_get_tile_size (map_source);
  double n = M_PI - 2.0 * M_PI * dy / pow (2.0, zoom_level);
  return 180.0 / M_PI * atan (0.5 * (exp (n) - exp (-n)));
}

