/*
 * Copyright (C) 2008-2009 Pierre-Luc Beaudoin <pierre-luc@pierlux.com>
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
 * SECTION:champlain-network-map-source
 * @short_description: A base object for network map sources
 *
 * This object is specialized for map tiles that can be downloaded
 * from a web server.  This include all web based map services such as
 * OpenStreetMap, Google Maps, Yahoo Maps and more.  This object contains
 * all mechanisms necessary to download and cache (with the help of
 * #ChamplainCache) tiles.
 *
 * Some preconfigured network map sources are built-in this library,
 * see #ChamplainMapSourceFactory.
 *
 */
#include "config.h"

#include "champlain-network-map-source.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_NETWORK
#include "champlain-debug.h"

#include "champlain.h"
#include "champlain-cache.h"
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
#include <glib/gstdio.h>
#include <glib-object.h>
#ifdef HAVE_LIBSOUP_GNOME
#include <libsoup/soup-gnome.h>
#else
#include <libsoup/soup.h>
#endif
#include <math.h>
#include <sys/stat.h>
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

static void fill_tile (ChamplainMapSource *map_source,
    ChamplainTile *tile);

static void
champlain_network_map_source_get_property (GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainNetworkMapSource *source = CHAMPLAIN_NETWORK_MAP_SOURCE(object);
  ChamplainNetworkMapSourcePrivate *priv = source->priv;

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
  ChamplainNetworkMapSource *source = CHAMPLAIN_NETWORK_MAP_SOURCE(object);
  ChamplainNetworkMapSourcePrivate *priv = source->priv;

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
champlain_network_map_source_dispose (GObject *object)
{
  //ChamplainNetworkMapSource *source = CHAMPLAIN_NETWORK_MAP_SOURCE (object);
  //ChamplainNetworkMapSourcePrivate *priv = source->priv;

  if (soup_session != NULL)
      soup_session_abort (soup_session);
}

static void
champlain_network_map_source_finalize (GObject *object)
{
  ChamplainNetworkMapSource *source = CHAMPLAIN_NETWORK_MAP_SOURCE (object);
  ChamplainNetworkMapSourcePrivate *priv = source->priv;

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
  object_class->dispose = champlain_network_map_source_dispose;
  object_class->get_property = champlain_network_map_source_get_property;
  object_class->set_property = champlain_network_map_source_set_property;

  ChamplainMapSourceClass *map_source_class = CHAMPLAIN_MAP_SOURCE_CLASS (klass);
  map_source_class->fill_tile = fill_tile;

  /**
  * ChamplainNetworkMapSource:uri-format
  *
  * The uri format for the map source, see #champlain_network_map_source_set_uri_format
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

/**
 * champlain_network_map_source_new_full:
 * @id: the map source's id
 * @name: the map source's name
 * @license: the map source's license
 * @license_uri: the map source's license URI
 * @min_zoom: the map source's minimum zoom level
 * @max_zoom: the map source's maximum zoom level
 * @tile_size: the map source's tile size (in pixels)
 * @projection: the map source's projection
 * @uri_format: the URI to fetch the tiles from, see #champlain_network_map_source_set_uri_format
 *
 * Returns a constructed #ChamplainNetworkMapSource
 *
 * Since: 0.4
 */
ChamplainNetworkMapSource*
champlain_network_map_source_new_full (const gchar *id,
    const gchar *name,
    const gchar *license,
    const gchar *license_uri,
    guint min_zoom,
    guint max_zoom,
    guint tile_size,
    ChamplainMapProjection projection,
    const gchar *uri_format)
{
  ChamplainNetworkMapSource * source;
  source = g_object_new (CHAMPLAIN_TYPE_NETWORK_MAP_SOURCE, "id", id,
      "name", name, "license", license, "license-uri", license_uri,
      "min-zoom-level", min_zoom, "max-zoom-level", max_zoom,
      "tile-size", tile_size, "projection", projection,
      "uri-format", uri_format, NULL);
  return source;
}

/**
 * champlain_network_map_source_get_tile_uri:
 * @source: the #ChamplainNetworkMapSource
 * @x: the x position of the tile
 * @y: the y position of the tile
 * @z: the zool level of the tile
 *
 * Returns a contruscted URI with the given parameters based on the map
 * source's URI format.
 *
 * Since: 0.4
 */
#define SIZE 8
gchar *
champlain_network_map_source_get_tile_uri (ChamplainNetworkMapSource *source,
    gint x,
    gint y,
    gint z)
{
  ChamplainNetworkMapSourcePrivate *priv = source->priv;

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

/**
 * champlain_network_map_source_set_uri_format:
 * @source: the #ChamplainNetworkMapSource
 * @uri_format: the URI format
 *
 * A URI format is a URI where x, y and zoom level information have been
 * marked for parsing and insertion.  There can be an unlimited number of
 * marked items in a URI format.  They are delimited by "#" before and after
 * the variable name. There are 3 defined variable names: X, Y, and Z.
 *
 * For example, this is the OpenStreetMap URI format:
 * "http://tile.openstreetmap.org/#Z#/#X#/#Y#.png"
 *
 *
 * Since: 0.4
 */
void
champlain_network_map_source_set_uri_format (ChamplainNetworkMapSource *source,
    const gchar *uri_format)
{
  ChamplainNetworkMapSourcePrivate *priv = source->priv;

  g_free (priv->uri_format);
  priv->uri_format = g_strdup (uri_format);
}

static gchar *
get_filename (ChamplainNetworkMapSource *source,
    ChamplainTile *tile)
{
  //ChamplainNetworkMapSourcePrivate *priv = source->priv;
  return g_strdup_printf ("%s" G_DIR_SEPARATOR_S "%s" G_DIR_SEPARATOR_S
             "%s" G_DIR_SEPARATOR_S "%d" G_DIR_SEPARATOR_S
             "%d" G_DIR_SEPARATOR_S "%d.png", g_get_user_cache_dir (),
             CACHE_SUBDIR, champlain_map_source_get_id (CHAMPLAIN_MAP_SOURCE (source)),
             champlain_tile_get_zoom_level (tile),
             champlain_tile_get_x (tile), champlain_tile_get_y (tile));
}

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

  champlain_tile_set_content (tile, actor, TRUE);
  clutter_actor_show (actor);

  champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);
}

static void
file_loaded_cb (SoupSession *session,
    SoupMessage *msg,
    gpointer user_data)
{
  ChamplainTile *tile = CHAMPLAIN_TILE (user_data);
  GdkPixbufLoader* loader;
  GError *error = NULL;
  gchar* path = NULL;
  const gchar *filename = NULL;
  ClutterActor *actor;
  GFile *file;
  GFileInfo *info;
  guint filesize = 0;
  ChamplainCache *cache = champlain_cache_dup_default ();

  filename = champlain_tile_get_filename (tile);

  DEBUG ("Got reply %d", msg->status_code);
  if (msg->status_code == SOUP_STATUS_CANCELLED)
    {
        DEBUG ("Download of tile %d, %d got cancelled",
            champlain_tile_get_x (tile), champlain_tile_get_y (tile));
        //champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);
        g_object_unref (tile);
        return;
    }

  if (msg->status_code == SOUP_STATUS_NOT_MODIFIED)
    {
      /* Since we are updating the cache, we can assume that the directories
       * exists */
      GTimeVal now = {0, };

      file = g_file_new_for_path (filename);
      info = g_file_query_info (file, G_FILE_ATTRIBUTE_TIME_MODIFIED,
          G_FILE_QUERY_INFO_NONE, NULL, NULL);

      g_get_current_time (&now);
      g_file_info_set_modification_time (info, &now);
      g_file_set_attributes_from_info (file, info, G_FILE_QUERY_INFO_NONE, NULL,
          NULL);

      g_object_unref (file);
      g_object_unref (info);

      champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);
      g_object_unref (tile);
      return;
    }

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      DEBUG ("Unable to download tile %d, %d: %s",
          champlain_tile_get_x (tile),
          champlain_tile_get_y (tile),
          soup_status_get_phrase (msg->status_code));

      if (champlain_tile_get_state (tile) != CHAMPLAIN_STATE_VALIDATING_CACHE)
        create_error_tile (tile);
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
          create_error_tile (tile);
          goto cleanup;
        }

      g_object_unref (loader);
    }

  gdk_pixbuf_loader_close (loader, &error);
  if (error)
    {
      g_warning ("Unable to close the pixbuf loader: %s", error->message);
      g_error_free (error);
      create_error_tile (tile);
      goto cleanup;
    }

  /* Create, if needed, the cache's dirs */
  path = g_path_get_dirname (filename);
  if (g_mkdir_with_parents (path, 0700) == -1)
    {
      if (errno != EEXIST)
        {
          g_warning ("Unable to create the image cache path '%s': %s",
                     path, g_strerror (errno));
        }
    }

  /* Write the cache */
  if (g_file_set_contents (filename, msg->response_body->data,
      msg->response_body->length, NULL))
    {
      struct stat info;
      g_stat (filename, &info);
      filesize = info.st_size;
    }

  /* Verify if the server sent an etag and save it */
  const gchar *etag = soup_message_headers_get (msg->response_headers, "ETag");
  DEBUG ("Received ETag %s", etag);

  if (etag != NULL)
    {
      champlain_tile_set_etag (tile, etag);
    }
  champlain_cache_update_tile (cache, tile, filesize); //XXX

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
      if (error)
        {
          g_warning ("Unable to transfer to clutter: %s", error->message);
          g_error_free (error);
          create_error_tile (tile);
          g_object_unref (actor);
          goto cleanup;
        }
    }

  champlain_tile_set_content (tile, actor, TRUE);
  DEBUG ("Tile loaded from network");

cleanup:
  g_object_unref (loader);
  g_object_unref (cache);
  g_free (path);
finish:
  champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);
  g_object_unref (tile);
}

static void
fill_tile (ChamplainMapSource *map_source,
    ChamplainTile *tile)
{
  gchar* filename;
  gboolean in_cache = FALSE;
  gboolean validate_cache = FALSE;
  gint zoom_level = champlain_tile_get_zoom_level (tile);

  ChamplainNetworkMapSource *source = CHAMPLAIN_NETWORK_MAP_SOURCE (map_source);
  ChamplainNetworkMapSourcePrivate *priv = source->priv;
  ChamplainCache *cache = champlain_cache_dup_default ();

  /* Try the cached version first */
  filename = get_filename (source, tile);
  champlain_tile_set_filename (tile, filename);
  champlain_tile_set_size (tile, champlain_map_source_get_tile_size (map_source));

  in_cache = champlain_cache_fill_tile (cache, tile);

  if (in_cache == TRUE)
    {
      validate_cache = champlain_cache_tile_is_expired (cache, tile);

      if (validate_cache == TRUE)
        champlain_tile_set_state (tile, CHAMPLAIN_STATE_VALIDATING_CACHE);
      else
        champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);

      DEBUG ("Tile loaded from cache");
    }

  if ((in_cache == FALSE || (in_cache == TRUE && validate_cache == TRUE)) &&
      priv->offline == FALSE)
    {
      SoupMessage *msg;
      gchar *uri;

      /* Ref the tile as it may be freeing during the loading
       * Unref when the loading is done.
       */
      g_object_ref (tile);

      if (!soup_session)
        {
          soup_session = soup_session_async_new_with_options ("proxy-uri",
                soup_uri_new (priv->proxy_uri),
#ifdef HAVE_LIBSOUP_GNOME
              SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_PROXY_RESOLVER_GNOME,
#endif
              NULL);
          g_object_set (G_OBJECT (soup_session),
              "user-agent", "libchamplain/" CHAMPLAIN_VERSION_S,
              "max-conns-per-host", 8, NULL); // This is the same has Firefox
          g_object_add_weak_pointer (G_OBJECT (soup_session),
              (gpointer *) &soup_session);
        }

      uri = champlain_network_map_source_get_tile_uri (source,
               champlain_tile_get_x (tile), champlain_tile_get_y (tile),
               zoom_level);
      champlain_tile_set_uri (tile, uri);

      if (champlain_tile_get_state (tile) != CHAMPLAIN_STATE_VALIDATING_CACHE)
        champlain_tile_set_state (tile, CHAMPLAIN_STATE_LOADING);
      msg = soup_message_new (SOUP_METHOD_GET, uri);

      if (in_cache == TRUE)
        {
          const gchar *etag;

          /* If an etag is available, only use it.
           * OSM servers seems to send now as the modified time for all tiles
           * Omarender servers set the modified time correctly
           */
          etag = champlain_tile_get_etag (tile);
          if (etag != NULL)
            {
              DEBUG("If-None-Match: %s", etag);
              soup_message_headers_append (msg->request_headers,
                  "If-None-Match", etag);
            }
          else
            {
              gchar *date;

              date = champlain_tile_get_modified_time_string (tile);
              DEBUG("If-Modified-Since %s", date);
              soup_message_headers_append (msg->request_headers,
                  "If-Modified-Since", date);

              g_free (date);
            }
        }

      soup_session_queue_message (soup_session, msg,
                                  file_loaded_cb,
                                  tile);
      g_free (uri);
    }
  g_free (filename);
  /* If a tile is neither in cache or can be fetched, do nothing, it'll show up
   * as empty
   */
  g_object_unref (cache);
}

void
champlain_network_map_source_fill_tile (ChamplainMapSource *map_source,
    ChamplainTile *tile)
{
  fill_tile (map_source, tile);
}
