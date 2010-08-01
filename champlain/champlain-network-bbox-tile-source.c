/*
 * Copyright (C) 2009 Simon Wenner <simon@wenner.ch>
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
 * SECTION:champlain-network-map-data-source
 * @short_description: Downloads map data for #ChamplainMemphisTileSource
 *
 * This map data source downloads the map data from an OpenStreetMap API
 * server. It supports protocol version 0.5 and 0.6.
 *
 * <ulink role="online-location" url="http://wiki.openstreetmap.org/wiki/API">
 * http://wiki.openstreetmap.org/wiki/API</ulink>
 *
 */

#include "champlain-network-bbox-tile-source.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_LOADING
#include "champlain-debug.h"
#include "champlain-bounding-box.h"
#include "champlain-enum-types.h"
#include "champlain-version.h"
#include "champlain-tile.h"

#ifdef HAVE_LIBSOUP_GNOME
#include <libsoup/soup-gnome.h>
#else
#include <libsoup/soup.h>
#endif

G_DEFINE_TYPE (ChamplainNetworkBboxTileSource, champlain_network_bbox_tile_source, CHAMPLAIN_TYPE_TILE_SOURCE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_NETWORK_BBOX_TILE_SOURCE, ChamplainNetworkBboxTileSourcePrivate))

enum
{
  /* normal signals */
  DATA_LOADED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_API_URI,
  PROP_PROXY_URI
};

struct _ChamplainNetworkBboxTileSourcePrivate {
  gchar *api_uri;
  gchar *proxy_uri;
  SoupSession * soup_session;
};

static guint champlain_network_bbox_tile_source_signals[LAST_SIGNAL] = { 0, };

static void fill_tile (ChamplainMapSource *map_source,
    ChamplainTile *tile);


static void
champlain_network_bbox_tile_source_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainNetworkBboxTileSource *self =
      CHAMPLAIN_NETWORK_BBOX_TILE_SOURCE (object);
  ChamplainNetworkBboxTileSourcePrivate *priv = self->priv;

  switch (property_id)
    {
      case PROP_API_URI:
        g_value_set_string (value,
            champlain_network_bbox_tile_source_get_api_uri (self));
        break;
      case PROP_PROXY_URI:
        g_value_set_string (value, priv->proxy_uri);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_network_bbox_tile_source_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainNetworkBboxTileSource *self =
      CHAMPLAIN_NETWORK_BBOX_TILE_SOURCE (object);
  ChamplainNetworkBboxTileSourcePrivate *priv = self->priv;

  switch (property_id)
    {
      case PROP_API_URI:
        champlain_network_bbox_tile_source_set_api_uri (self,
            g_value_get_string (value));
        break;
      case PROP_PROXY_URI:
        g_free (priv->proxy_uri);

        priv->proxy_uri = g_value_dup_string (value);
        if (priv->soup_session)
          g_object_set (G_OBJECT (priv->soup_session), "proxy-uri",
              soup_uri_new (priv->proxy_uri), NULL);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_network_bbox_tile_source_dispose (GObject *object)
{
  ChamplainNetworkBboxTileSource *self =
      CHAMPLAIN_NETWORK_BBOX_TILE_SOURCE (object);
  ChamplainNetworkBboxTileSourcePrivate *priv = self->priv;

  if (priv->soup_session != NULL)
    {
      soup_session_abort (priv->soup_session);
      priv->soup_session = NULL;
    }

  G_OBJECT_CLASS (champlain_network_bbox_tile_source_parent_class)->dispose (object);
}

static void
champlain_network_bbox_tile_source_finalize (GObject *object)
{
  ChamplainNetworkBboxTileSource *self =
      CHAMPLAIN_NETWORK_BBOX_TILE_SOURCE (object);
  ChamplainNetworkBboxTileSourcePrivate *priv = self->priv;

  g_free (priv->api_uri);
  g_free (priv->proxy_uri);

  G_OBJECT_CLASS (champlain_network_bbox_tile_source_parent_class)->finalize (object);
}

static void
champlain_network_bbox_tile_source_class_init (ChamplainNetworkBboxTileSourceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainNetworkBboxTileSourcePrivate));

  object_class->get_property = champlain_network_bbox_tile_source_get_property;
  object_class->set_property = champlain_network_bbox_tile_source_set_property;
  object_class->dispose = champlain_network_bbox_tile_source_dispose;
  object_class->finalize = champlain_network_bbox_tile_source_finalize;

  ChamplainMapSourceClass *map_source_class = CHAMPLAIN_MAP_SOURCE_CLASS (klass);
  map_source_class->fill_tile = fill_tile;

  /*
  * ChamplainNetworkBboxTileSource:api-uri:
  *
  * The URI of an OpenStreetMap API server
  *
  * Since: 0.6
  */
  g_object_class_install_property (object_class,
      PROP_API_URI,
      g_param_spec_string ("api_uri",
        "API URI",
        "The API URI of an OpenStreetMap server",
        "http://www.informationfreeway.org/api/0.6",
        G_PARAM_READWRITE));

  /*
  * ChamplainNetworkBboxTileSource:proxy-uri:
  *
  * The proxy URI to use to access network
  *
  * Since: 0.6
  */
  g_object_class_install_property (object_class,
      PROP_PROXY_URI,
      g_param_spec_string ("proxy-uri",
        "Proxy URI",
        "The proxy URI to use to access network",
        "",
        G_PARAM_READWRITE));

  champlain_network_bbox_tile_source_signals[DATA_LOADED] =
    g_signal_new ("data-loaded", G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE,
                  0, NULL);
}

static void
champlain_network_bbox_tile_source_init (ChamplainNetworkBboxTileSource *self)
{
  ChamplainNetworkBboxTileSourcePrivate *priv = GET_PRIVATE (self);

  self->priv = priv;

  priv->api_uri = g_strdup ("http://www.informationfreeway.org/api/0.6");
  /* informationfreeway.org is a load-balancer for different api servers */
  priv->proxy_uri = g_strdup ("");
  priv->soup_session = soup_session_async_new_with_options (
      "proxy-uri", soup_uri_new (priv->proxy_uri),
#ifdef HAVE_LIBSOUP_GNOME
      SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_PROXY_RESOLVER_GNOME,
#endif
      NULL);
      g_object_set (G_OBJECT (priv->soup_session),
          "user-agent", "libchamplain/" CHAMPLAIN_VERSION_S,
          "max-conns-per-host", 2, NULL);
}

/*
 * champlain_network_bbox_tile_source_new:
 *
 * Creates an instance of #ChamplainNetworkBboxTileSource.
 *
 * Returns: a new #ChamplainNetworkBboxTileSource
 *
 * Since: 0.6
 */

ChamplainNetworkBboxTileSource*
champlain_network_bbox_tile_source_new_full (const gchar *id,
    const gchar *name,
    const gchar *license,
    const gchar *license_uri,
    guint min_zoom,
    guint max_zoom,
    guint tile_size,
    ChamplainMapProjection projection)
{
  ChamplainNetworkBboxTileSource * source;
  source = g_object_new (CHAMPLAIN_TYPE_NETWORK_BBOX_TILE_SOURCE, "id", id,
      "name", name,
      "license", license,
      "license-uri", license_uri,
      "min-zoom-level", min_zoom,
      "max-zoom-level", max_zoom,
      "tile-size", tile_size,
      "projection", projection,
      NULL);
  return source;
}

static void
load_map_data_cb (G_GNUC_UNUSED SoupSession *session, SoupMessage *msg,
    gpointer user_data)
{
  ChamplainNetworkBboxTileSource *self =
      CHAMPLAIN_NETWORK_BBOX_TILE_SOURCE (user_data);
  ChamplainRenderer *renderer;

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      DEBUG ("Unable to download file: %s",
          soup_status_get_phrase (msg->status_code));

      return;
    }

  g_signal_emit_by_name (self, "data-loaded", NULL);

  renderer =  champlain_map_source_get_renderer (CHAMPLAIN_MAP_SOURCE (self));
  champlain_renderer_set_data (renderer, msg->response_body->data, msg->response_body->length);
}

/*
 * champlain_network_bbox_tile_source_load_map_data:
 * @map_data_source: a #ChamplainNetworkBboxTileSource
 * @bound_left: the left bound in degree
 * @bound_bottom: the lower bound in degree
 * @bound_right: the right bound in degree
 * @bound_top: the upper bound in degree
 *
 * Asynchronously loads map data within a bounding box from the server.
 * The box must not exceed an edge size of 0.25 degree. There are also
 * limitations on the maximum number of nodes that can be requested.
 *
 * For details, see: <ulink role="online-location"
 * url="http://api.openstreetmap.org/api/capabilities">
 * http://api.openstreetmap.org/api/capabilities</ulink>
 *
 * Since: 0.6
 */
void
champlain_network_bbox_tile_source_load_map_data (
    ChamplainNetworkBboxTileSource *self,
    gdouble bound_left,
    gdouble bound_bottom,
    gdouble bound_right,
    gdouble bound_top)
{
  g_return_if_fail (CHAMPLAIN_IS_NETWORK_BBOX_TILE_SOURCE (self));

  g_return_if_fail (bound_right - bound_left < 0.25 &&
      bound_top - bound_bottom < 0.25);

  ChamplainNetworkBboxTileSourcePrivate *priv = self->priv;
  SoupMessage *msg;
  gchar *url;

  url = g_strdup_printf (
      "http://api.openstreetmap.org/api/0.6/map?bbox=%f,%f,%f,%f",
      bound_left, bound_bottom, bound_right, bound_top);
  msg = soup_message_new ("GET", url);

  DEBUG ("Request BBox data: '%s'", url);

  g_free (url);

  soup_session_queue_message (priv->soup_session, msg, load_map_data_cb, self);
}

static void
tile_rendered_cb (ChamplainTile *tile,
    ChamplainRenderCallbackData *data,
    ChamplainMapSource *map_source)
{
  ChamplainTileSource *tile_source = CHAMPLAIN_TILE_SOURCE(map_source);
  ChamplainTileCache *tile_cache = champlain_tile_source_get_cache (tile_source);
  ChamplainMapSource *next_source = champlain_map_source_get_next_source (map_source);

  if (!data->error)
    {
      if (tile_cache && data->data)
        champlain_tile_cache_store_tile (tile_cache, tile, data->data, data->size);

      champlain_tile_set_fade_in (tile, TRUE);
      champlain_tile_set_state (tile, CHAMPLAIN_STATE_DONE);
      champlain_tile_display_content (tile);
    }
  else if (next_source)
    champlain_map_source_fill_tile (next_source, tile);

  g_object_unref (map_source);
  g_signal_handlers_disconnect_by_func (tile, tile_rendered_cb, map_source);
}

static void
fill_tile (ChamplainMapSource *map_source,
    ChamplainTile *tile)
{
  g_return_if_fail (CHAMPLAIN_IS_NETWORK_BBOX_TILE_SOURCE (map_source));
  g_return_if_fail (CHAMPLAIN_IS_TILE (tile));

  ChamplainRenderer *renderer;

  renderer = champlain_map_source_get_renderer (map_source);

  g_return_if_fail (CHAMPLAIN_IS_RENDERER (renderer));

  g_object_ref (map_source);

  g_signal_connect (tile, "render-complete", G_CALLBACK (tile_rendered_cb), map_source);

  champlain_renderer_render (renderer, tile);
}

/*
 * champlain_network_bbox_tile_source_get_api_uri:
 * @map_data_source: a #ChamplainNetworkBboxTileSource
 *
 * Gets the URI of the API server.
 *
 * Returns: the URI of the API server.
 *
 * Since: 0.6
 */
const gchar *
champlain_network_bbox_tile_source_get_api_uri (
    ChamplainNetworkBboxTileSource *self)
{
  g_return_val_if_fail (CHAMPLAIN_IS_NETWORK_BBOX_TILE_SOURCE (self), NULL);

  return self->priv->api_uri;
}

/*
 * champlain_network_bbox_tile_source_set_api_uri:
 * @map_data_source: a #ChamplainNetworkBboxTileSource
 * @api_uri: an URI of an API server
 *
 * Sets the URI of the API server.
 *
 * Since: 0.6
 */
void
champlain_network_bbox_tile_source_set_api_uri (
    ChamplainNetworkBboxTileSource *self,
    const gchar *api_uri)
{
  g_return_if_fail (CHAMPLAIN_IS_NETWORK_BBOX_TILE_SOURCE (self)
      && api_uri != NULL);

  ChamplainNetworkBboxTileSourcePrivate *priv = self->priv;

  g_free (priv->api_uri);
  priv->api_uri = g_strdup (api_uri);
}
