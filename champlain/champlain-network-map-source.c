/*
 * Copyright (C) 2008, 2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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

#include "champlain-network-map-source.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_NETWORK
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
#include <glib/gprintf.h>
#include <glib-object.h>
#ifdef HAVE_LIBSOUP_GNOME
#include <libsoup/soup-gnome.h>
#else
#include <libsoup/soup.h>
#endif
#include <math.h>
#include <string.h>
#include <clutter-cairo.h>

enum
{
  /* normal signals */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_URI_FORMAT,
  PROP_OFFLINE,
  PROP_PROXY_URI
};

/* static guint champlain_network_map_source_signals[LAST_SIGNAL] = { 0, }; */

G_DEFINE_TYPE (ChamplainNetworkMapSource, champlain_network_map_source, CHAMPLAIN_TYPE_MAP_SOURCE);

#define GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), CHAMPLAIN_TYPE_NETWORK_MAP_SOURCE, ChamplainNetworkMapSourcePrivate))

#define CACHE_SUBDIR "champlain"
static SoupSession * soup_session = NULL;

struct _ChamplainNetworkMapSourcePrivate
{
  gboolean offline;
  gchar *uri_format;
  gchar *proxy_uri;
};

static void
champlain_network_map_source_get_property (GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainNetworkMapSource *network_map_source = CHAMPLAIN_NETWORK_MAP_SOURCE(object);
  ChamplainNetworkMapSourcePrivate *priv = network_map_source->priv;

  switch(prop_id)
    {
      case PROP_URI_FORMAT:
        g_value_set_string (value, priv->uri_format);
        break;
      case PROP_OFFLINE:
        g_value_set_boolean (value, priv->offline);
        break;
      case PROP_PROXY_URI:
        g_value_set_string (value, priv->proxy_uri);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
champlain_network_map_source_set_property (GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainNetworkMapSource *network_map_source = CHAMPLAIN_NETWORK_MAP_SOURCE(object);
  ChamplainNetworkMapSourcePrivate *priv = network_map_source->priv;

  switch(prop_id)
    {
      case PROP_URI_FORMAT:
        g_free (priv->uri_format);
        priv->uri_format = g_value_dup_string (value);
        break;
      case PROP_OFFLINE:
        priv->offline = g_value_get_boolean (value);
        break;
      case PROP_PROXY_URI:
        g_free (priv->proxy_uri);

        priv->proxy_uri = g_value_dup_string (value);
        if (soup_session)
          g_object_set (G_OBJECT (soup_session), "proxy-uri",
              soup_uri_new (priv->proxy_uri), NULL);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
champlain_network_map_source_finalize (GObject *object)
{
  ChamplainNetworkMapSource *network_map_source = CHAMPLAIN_NETWORK_MAP_SOURCE (object);
  ChamplainNetworkMapSourcePrivate *priv = network_map_source->priv;

  g_free (priv->proxy_uri);
  g_free (priv->uri_format);

  G_OBJECT_CLASS (champlain_network_map_source_parent_class)->finalize (object);
}

static void
champlain_network_map_source_class_init (ChamplainNetworkMapSourceClass *klass)
{
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (ChamplainNetworkMapSourcePrivate));

  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = champlain_network_map_source_finalize;
  object_class->get_property = champlain_network_map_source_get_property;
  object_class->set_property = champlain_network_map_source_set_property;

  ChamplainMapSourceClass *map_source_class = CHAMPLAIN_MAP_SOURCE_CLASS (klass);
  map_source_class->get_tile = champlain_network_map_source_get_tile;

  /**
  * ChamplainNetworkMapSource:uri-format
  *
  * The uri format for the map source
  *
  * Since: 0.4
  */
  pspec = g_param_spec_string ("uri-format",
                               "URI Format",
                               "The URI format",
                               "",
                               (G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_URI_FORMAT, pspec);

  /**
  * ChamplainNetworkMapSource:offline
  *
  * If the network map source can access network
  *
  * Since: 0.4
  */
  pspec = g_param_spec_boolean ("offline",
                                "Offline",
                                "Offline",
                                FALSE,
                                G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_OFFLINE, pspec);

  /**
  * ChamplainNetworkMapSource:proxy-uri
  *
  * The proxy uri to use to access network
  *
  * Since: 0.4
  */
  pspec = g_param_spec_string ("proxy-uri",
                               "Proxy URI",
                               "The proxy URI to use to access network",
                               "",
                               G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_PROXY_URI, pspec);
}

static void
champlain_network_map_source_init (ChamplainNetworkMapSource *champlainMapSource)
{
  ChamplainNetworkMapSourcePrivate *priv = GET_PRIVATE (champlainMapSource);

  champlainMapSource->priv = priv;

  priv->proxy_uri = g_strdup ("");
  priv->uri_format = NULL;
  priv->offline = FALSE;
}

ChamplainNetworkMapSource*
champlain_network_map_source_new_full (const gchar *name,
    const gchar *license,
    const gchar *license_uri,
    guint min_zoom,
    guint max_zoom,
    guint tile_size,
    ChamplainMapProjection projection,
    const gchar *uri_format)
{
  ChamplainNetworkMapSource * network_map_source;
  network_map_source = g_object_new (CHAMPLAIN_TYPE_NETWORK_MAP_SOURCE, "name", name,
      "license", license, "license-uri", license_uri,
      "min-zoom-level", min_zoom, "max-zoom-level", max_zoom,
      "tile-size", tile_size, "map-projection", projection,
      "uri-format", uri_format, NULL);
  return network_map_source;
}

#define SIZE 8
gchar *
champlain_network_map_source_get_tile_uri (ChamplainNetworkMapSource *network_map_source,
    gint x,
    gint y,
    gint z)
{
  ChamplainNetworkMapSourcePrivate *priv = network_map_source->priv;

  gchar **tokens;
  gchar *token;
  GString *ret;
  gint i = 0;

  tokens = g_strsplit (priv->uri_format, "#", 20);
  token = tokens[i];
  ret = g_string_sized_new (strlen (priv->uri_format));

  while (token != NULL)
    {
      gint number = G_MAXINT;
      gchar value[SIZE];

      if (strcmp (token, "X") == 0)
        number = x;
      if (strcmp (token, "Y") == 0)
        number = y;
      if (strcmp (token, "Z") == 0)
        number = z;

      if (number != G_MAXINT)
        {
          g_snprintf (value, SIZE, "%d", number);
          g_string_append (ret, value);
        }
      else
        g_string_append (ret, token);

      token = tokens[++i];
    }
  token = ret->str;
  g_string_free (ret, FALSE);
  g_strfreev (tokens);

  return token;
}

void
champlain_network_map_source_set_tile_uri (ChamplainNetworkMapSource *network_map_source,
    const gchar *uri_format)
{
  ChamplainNetworkMapSourcePrivate *priv = network_map_source->priv;

  g_free (priv->uri_format);
  priv->uri_format = g_strdup (uri_format);
}

ChamplainMapSource *
champlain_map_source_new_osm_cyclemap (void)
{
  return CHAMPLAIN_MAP_SOURCE (champlain_network_map_source_new_full ("OpenStreetMap Cycle Map",
      "(CC) BY 2.0 OpenStreetMap contributors",
      "http://creativecommons.org/licenses/by/2.0/", 0, 18, 256,
      CHAMPLAIN_MAP_PROJECTION_MERCATOR,
      "http://andy.sandbox.cloudmade.com/tiles/cycle/#Z#/#X#/#Y#.png"));
}

ChamplainMapSource *
champlain_map_source_new_osm_osmarender (void)
{
  return CHAMPLAIN_MAP_SOURCE (champlain_network_map_source_new_full ("OpenStreetMap Osmarender",
      "(CC) BY 2.0 OpenStreetMap contributors",
      "http://creativecommons.org/licenses/by/2.0/", 0, 18, 256,
      CHAMPLAIN_MAP_PROJECTION_MERCATOR,
      "http://tah.openstreetmap.org/Tiles/tile/#Z#/#X#/#Y#.png"));
}

ChamplainMapSource *
champlain_map_source_new_osm_mapnik (void)
{
  return CHAMPLAIN_MAP_SOURCE (champlain_network_map_source_new_full ("OpenStreetMap Mapnik",
      "(CC) BY 2.0 OpenStreetMap contributors",
      "http://creativecommons.org/licenses/by/2.0/", 0, 18, 256,
      CHAMPLAIN_MAP_PROJECTION_MERCATOR,
      "http://tile.openstreetmap.org/#Z#/#X#/#Y#.png"));
}

ChamplainMapSource *
champlain_map_source_new_oam (void)
{
  return CHAMPLAIN_MAP_SOURCE (champlain_network_map_source_new_full ("OpenArialMap",
      "(CC) BY 3.0 OpenArialMap contributors",
      "http://creativecommons.org/licenses/by/3.0/", 0, 17, 256,
      CHAMPLAIN_MAP_PROJECTION_MERCATOR,
      "http://tile.openaerialmap.org/tiles/1.0.0/openaerialmap-900913/#Z#/#X#/#Y#.jpg"));
}

ChamplainMapSource *
champlain_map_source_new_mff_relief (void)
{
  return CHAMPLAIN_MAP_SOURCE (champlain_network_map_source_new_full ("MapsForFree Relief",
      "Map data available under GNU Free Documentation license, Version 1.2 or later",
      "http://www.gnu.org/copyleft/fdl.html", 0, 11, 256,
      CHAMPLAIN_MAP_PROJECTION_MERCATOR,
      "http://maps-for-free.com/layer/relief/z#Z#/row#Y#/#Z#_#X#-#Y#.jpg"));
}

static gchar *
get_filename (ChamplainNetworkMapSource *network_map_source,
    ChamplainZoomLevel *level,
    ChamplainTile *tile)
{
  //ChamplainNetworkMapSourcePrivate *priv = network_map_source->priv;
  return g_strdup_printf ("%s" G_DIR_SEPARATOR_S "%s" G_DIR_SEPARATOR_S
             "%s" G_DIR_SEPARATOR_S "%d" G_DIR_SEPARATOR_S
             "%d" G_DIR_SEPARATOR_S "%d.png", g_get_user_cache_dir (),
             CACHE_SUBDIR, champlain_map_source_get_name (CHAMPLAIN_MAP_SOURCE (network_map_source)),
             champlain_zoom_level_get_zoom_level (level),
             champlain_tile_get_x (tile), champlain_tile_get_y (tile));
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
  cairo_t *cr;
  cairo_pattern_t *pat;
  guint size;

  size = champlain_tile_get_size (tile);
  actor = clutter_cairo_new (size, size);
  cr = clutter_cairo_create (CLUTTER_CAIRO(actor));

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
  gchar* path = NULL;
  const gchar *filename = NULL;
  ClutterActor *actor, *previous_actor = NULL;
  GFile *file;
  GFileInfo *info;

  filename = champlain_tile_get_filename (ctx->tile);

  DEBUG ("Got reply %d", msg->status_code);
  if (msg->status_code == 304)
  {
    /* Since we are updating the cache, we can assume that the directories
     * exists */
    GTimeVal *now = g_new0 (GTimeVal, 1);

    file = g_file_new_for_path (filename);
    info = g_file_query_info (file, G_FILE_ATTRIBUTE_TIME_MODIFIED,
        G_FILE_QUERY_INFO_NONE, NULL, NULL);

    g_get_current_time (now);
    g_file_info_set_modification_time (info, now);
    g_file_set_attributes_from_info (file, info, G_FILE_QUERY_INFO_NONE, NULL,
        NULL);

    g_object_unref (file);
    g_object_unref (info);
    g_free (now);
    goto finish;
  }
  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      DEBUG ("Unable to download tile %d, %d: %s",
          champlain_tile_get_x (ctx->tile),
          champlain_tile_get_y (ctx->tile),
          soup_status_get_phrase(msg->status_code));
      create_error_tile (ctx->tile);
      goto finish;
    }

  /* Load the data from the http response */
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
          goto cleanup;
        }

      g_object_unref (loader);
    }

  gdk_pixbuf_loader_close (loader, &error);
  if (error)
    {
      g_warning ("Unable to close the pixbuf loader: %s", error->message);
      g_error_free (error);
      create_error_tile (ctx->tile);
      goto cleanup;
    }

  /* Create, if needed, the cache's dirs */
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

  /* Write the cache */
  g_file_set_contents (filename, msg->response_body->data,
      msg->response_body->length, NULL);

  /* Verify if the server sent an etag and save it */
  const gchar *etag = soup_message_headers_get (msg->response_headers, "ETag");
  DEBUG ("Received ETag %s", etag);

  if (etag != NULL)
    {
      file = g_file_new_for_path (filename);
      info = g_file_query_info (file, G_FILE_ATTRIBUTE_ETAG_VALUE,
          G_FILE_QUERY_INFO_NONE, NULL, NULL);
      g_file_info_set_attribute_string (info, G_FILE_ATTRIBUTE_ETAG_VALUE, etag);
      g_file_set_attributes_from_info (file, info, G_FILE_QUERY_INFO_NONE, NULL,
          NULL);

      g_object_unref (file);
      g_object_unref (info);
    }

  /* Load the image into clutter */
  GdkPixbuf* pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
  actor = clutter_texture_new();
  if (!clutter_texture_set_from_rgb_data (CLUTTER_TEXTURE (actor),
      gdk_pixbuf_get_pixels (pixbuf),
      gdk_pixbuf_get_has_alpha (pixbuf),
      gdk_pixbuf_get_width (pixbuf),
      gdk_pixbuf_get_height (pixbuf),
      gdk_pixbuf_get_rowstride (pixbuf),
      gdk_pixbuf_get_bits_per_sample (pixbuf) *
      gdk_pixbuf_get_n_channels (pixbuf) / 8,
      0, &error))
    {
      g_print ("BPP: %d", gdk_pixbuf_get_bits_per_sample (pixbuf));
      if (error)
        {
          g_warning ("Unable to transfer to clutter: %s", error->message);
          g_error_free (error);
          create_error_tile (ctx->tile);
          g_object_unref (actor);
          goto cleanup;
        }
    }

  previous_actor = champlain_tile_get_actor (ctx->tile);
  if (previous_actor)
    g_object_ref (previous_actor); /* to be unrefed by the view */

  champlain_tile_set_actor (ctx->tile, actor);
  DEBUG ("Tile loaded from network");

cleanup:
  g_object_unref (loader);
  g_free (path);
finish:
  champlain_tile_set_state (ctx->tile, CHAMPLAIN_STATE_DONE);
  champlain_view_tile_updated (ctx->view, ctx->zoom_level, ctx->tile, previous_actor);
  g_object_unref (ctx->tile);
  g_object_unref (ctx->zoom_level);
  g_free (ctx);
}

void
champlain_network_map_source_get_tile (ChamplainMapSource *map_source,
    ChamplainView *view,
    ChamplainZoomLevel *zoom_level,
    ChamplainTile *tile)
{
  gchar* filename;
  gboolean in_cache = FALSE;
  gboolean validate_cache = FALSE;
  GTimeVal *modified_time = g_new0 (GTimeVal, 1);
  gchar * etag = NULL;

  ChamplainNetworkMapSource *network_map_source = CHAMPLAIN_NETWORK_MAP_SOURCE (map_source);
  ChamplainNetworkMapSourcePrivate *priv = network_map_source->priv;

  /* Try the cached version first */
  filename = get_filename (network_map_source, zoom_level, tile);
  champlain_tile_set_filename (tile, filename);
  champlain_tile_set_size (tile, champlain_map_source_get_tile_size (map_source));

  if (g_file_test (filename, G_FILE_TEST_EXISTS))
    {
      GTimeVal *now = g_new0 (GTimeVal, 1);
      GFileInfo *info;
      GFile *file;
      GError *error = NULL;
      ClutterActor *actor;
      const gchar *etag_tmp;

      in_cache = TRUE;

      /* Verify since when is the file in cache */
      file = g_file_new_for_path (filename);
      info = g_file_query_info (file,
          G_FILE_ATTRIBUTE_TIME_MODIFIED "," G_FILE_ATTRIBUTE_ETAG_VALUE,
          G_FILE_QUERY_INFO_NONE, NULL, NULL);
      g_file_info_get_modification_time (info, modified_time);
      etag_tmp = g_file_info_get_attribute_string (info, G_FILE_ATTRIBUTE_ETAG_VALUE);
      if (etag_tmp != NULL)
        etag = g_strdup (etag_tmp);

      g_get_current_time (now);
      g_time_val_add (now, (-60ul * 60ul * 1000ul * 1000ul)); // Cache expires 1 hour
      validate_cache = modified_time->tv_sec < now->tv_sec;
      validate_cache = TRUE;

      g_object_unref (file);
      g_object_unref (info);
      g_free (now);

      /* Load the cached version */
      actor = clutter_texture_new_from_file (filename, &error);
      champlain_tile_set_actor (tile, actor);

      if (validate_cache == TRUE)
        champlain_tile_set_state (tile, CHAMPLAIN_STATE_VALIDATING_CACHE);
      else
        champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);

      DEBUG ("Tile loaded from cache");
      champlain_view_tile_ready (view, zoom_level, tile);
    }


  if ((in_cache == FALSE || (in_cache == TRUE && validate_cache == TRUE)) &&
      priv->offline == FALSE)
    {
      SoupMessage *msg;
      gchar *uri;
      FileLoadedCallbackContext *ctx = g_new0 (FileLoadedCallbackContext, 1);
      ctx->view = view;
      ctx->zoom_level = zoom_level;
      ctx->tile = tile;

      /* Ref the tile as it may be freeing during the loading
       * Unref when the loading is done.
       */
      g_object_ref (tile);
      g_object_ref (zoom_level);

      if (!soup_session)
        soup_session = soup_session_async_new_with_options ("proxy-uri",
            soup_uri_new (priv->proxy_uri),
#ifdef HAVE_LIBSOUP_GNOME
            SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_PROXY_RESOLVER_GNOME,
#endif
            NULL);

      uri = champlain_network_map_source_get_tile_uri (network_map_source,
               champlain_tile_get_x (tile), champlain_tile_get_y (tile),
               champlain_zoom_level_get_zoom_level (zoom_level));
      champlain_tile_set_uri (tile, uri);
      champlain_tile_set_state (tile, CHAMPLAIN_STATE_LOADING);
      msg = soup_message_new (SOUP_METHOD_GET, uri);

      if (in_cache == TRUE)
        {
          char value [100];
          struct tm *other_time = gmtime (&modified_time->tv_sec);

          strftime (value, 100, "%a, %d %b %Y %T %Z", other_time);

          DEBUG("If-Modified-Since %s", value);
          soup_message_headers_append (msg->request_headers,
              "If-Modified-Since", value);

          if (etag != NULL)
            {
              DEBUG("If-None-Match: %s", etag);
              soup_message_headers_append (msg->request_headers,
                  "If-None-Match", etag);
            }

        }

      soup_session_queue_message (soup_session, msg,
                                  file_loaded_cb,
                                  ctx);
      g_free (uri);
    }
  g_free (filename);
  /* If a tile is neither in cache or can be fetched, do nothing, it'll show up
   * as empty
   */
  g_free (modified_time);
  g_free (etag);
}

