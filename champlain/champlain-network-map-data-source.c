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

/**
 * SECTION:champlain-network-map-data-source
 * @short_description: ChamplainNetworkMapDataSource downloads data for
 *  ChamplainMemphisMapSource.
 *
 * This map data source downloads the map data from an OpenStreetMap API
 * server. It supports protocol version 0.5 and 0.6.
 *
 * <ulink role="online-location" url="http://wiki.openstreetmap.org/wiki/API">
 * http://wiki.openstreetmap.org/wiki/API</ulink>
 *
 */

#include "champlain-network-map-data-source.h"
#include "champlain-version.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_MEMPHIS
#include "champlain-debug.h"
#include "champlain-bounding-box.h"
#include "champlain-enum-types.h"

#include <memphis/memphis.h>
#ifdef HAVE_LIBSOUP_GNOME
#include <libsoup/soup-gnome.h>
#else
#include <libsoup/soup.h>
#endif

G_DEFINE_TYPE (ChamplainNetworkMapDataSource, champlain_network_map_data_source, CHAMPLAIN_TYPE_MAP_DATA_SOURCE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_NETWORK_MAP_DATA_SOURCE, ChamplainNetworkMapDataSourcePrivate))

static SoupSession * soup_session = NULL;

enum
{
  PROP_0,
  PROP_API_URI,
  PROP_PROXY_URI
};

typedef struct _ChamplainNetworkMapDataSourcePrivate ChamplainNetworkMapDataSourcePrivate;

struct _ChamplainNetworkMapDataSourcePrivate {
  MemphisMap *map;
  gchar *api_uri;
  gchar *proxy_uri;
};

static void
champlain_network_map_data_source_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  ChamplainNetworkMapDataSource *self =
      CHAMPLAIN_NETWORK_MAP_DATA_SOURCE (object);
  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE (self);

  switch (property_id)
    {
      case PROP_API_URI:
        g_value_set_string (value,
            champlain_network_map_data_source_get_api_uri (self));
        break;
      case PROP_PROXY_URI:
        g_value_set_string (value, priv->proxy_uri);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_network_map_data_source_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  ChamplainNetworkMapDataSource *self =
      CHAMPLAIN_NETWORK_MAP_DATA_SOURCE (object);
  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE (self);

  switch (property_id)
    {
      case PROP_API_URI:
        champlain_network_map_data_source_set_api_uri (self,
            g_value_get_string (value));
        break;
      case PROP_PROXY_URI:
        g_free (priv->proxy_uri);

        priv->proxy_uri = g_value_dup_string (value);
        if (soup_session)
          g_object_set (G_OBJECT (soup_session), "proxy-uri",
              soup_uri_new (priv->proxy_uri), NULL);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
champlain_network_map_data_source_dispose (GObject *object)
{
  ChamplainNetworkMapDataSource *self =
      CHAMPLAIN_NETWORK_MAP_DATA_SOURCE (object);
  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE (self);

  if (priv->map)
    memphis_map_free (priv->map);

  if (soup_session != NULL)
    soup_session_abort (soup_session);

  G_OBJECT_CLASS (champlain_network_map_data_source_parent_class)->dispose (object);
}

static void
champlain_network_map_data_source_finalize (GObject *object)
{
  ChamplainNetworkMapDataSource *self =
      CHAMPLAIN_NETWORK_MAP_DATA_SOURCE (object);
  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE (self);

  g_free (priv->api_uri);
  g_free (priv->proxy_uri);

  G_OBJECT_CLASS (champlain_network_map_data_source_parent_class)->finalize (object);
}

static MemphisMap *
get_map_data (ChamplainMapDataSource *self)
{
  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE (self);

  return priv->map;
}

static void
champlain_network_map_data_source_class_init (ChamplainNetworkMapDataSourceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (ChamplainNetworkMapDataSourcePrivate));

  object_class->get_property = champlain_network_map_data_source_get_property;
  object_class->set_property = champlain_network_map_data_source_set_property;
  object_class->dispose = champlain_network_map_data_source_dispose;
  object_class->finalize = champlain_network_map_data_source_finalize;

  ChamplainMapDataSourceClass *map_data_source_class = CHAMPLAIN_MAP_DATA_SOURCE_CLASS (klass);
  map_data_source_class->get_map_data = get_map_data;

  /**
  * ChamplainNetworkMapDataSource:api-uri:
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

  /**
  * ChamplainNetworkMapDataSource:proxy-uri:
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
}

static void
champlain_network_map_data_source_init (ChamplainNetworkMapDataSource *self)
{
  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE (self);

  priv->map = NULL;
  priv->api_uri = g_strdup ("http://www.informationfreeway.org/api/0.6");
  /* informationfreeway.org is a load-balancer for different api server */
  priv->proxy_uri = g_strdup ("");
}

/**
 * champlain_network_map_data_source_new:
 *
 * Returns a new #ChamplainNetworkMapDataSource
 *
 * Since: 0.6
 */
ChamplainNetworkMapDataSource *
champlain_network_map_data_source_new (void)
{
  return g_object_new (CHAMPLAIN_TYPE_NETWORK_MAP_DATA_SOURCE, NULL);
}

static void
load_map_data_cb (SoupSession *session, SoupMessage *msg,
    gpointer user_data)
{
  ChamplainNetworkMapDataSource *self = 
      CHAMPLAIN_NETWORK_MAP_DATA_SOURCE (user_data);
  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE (self);
  ChamplainBoundingBox *bbox;

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
    {
      DEBUG ("Unable to download file: %s",
          soup_status_get_phrase (msg->status_code));

      if (priv->map)
        memphis_map_free (priv->map);
      priv->map = NULL;
      return;
    }

  MemphisMap *map = memphis_map_new ();
  memphis_map_set_debug_level (map, 0);
  memphis_map_load_from_data (map,
      msg->response_body->data,
      msg->response_body->length);

  DEBUG ("BBox data received");

  if (priv->map)
    memphis_map_free (priv->map);

  priv->map = map;

  bbox = champlain_bounding_box_new ();
  memhis_map_get_bounding_box (priv->map, &bbox->left, &bbox->top,
      &bbox->right, &bbox->bottom);
  g_object_set (G_OBJECT (self), "bounding-box", bbox, NULL);
  champlain_bounding_box_free (bbox);

  g_object_set (G_OBJECT (self), "state", CHAMPLAIN_STATE_DONE, NULL);
}

/**
 * champlain_network_map_data_source_load_map_data:
 * @source: a #ChamplainNetworkMapDataSource
 * @bound_left: the left bound in degree
 * @bound_buttom: the lower bound in degree
 * @bound_right: the right bound in degree
 * @bound_top: the upper bound in degree
 *
 * Asynchronously loads map data within a bounding box from the server.
 * The box must not exceed an edge size of 0.25 degree. There are also
 * limitations on the maximum number of nodes that can be requested.
 *
 * For details, see: <ulink role="online-location"
 *  url="http://api.openstreetmap.org/api/capabilities">
 * http://api.openstreetmap.org/api/capabilities</ulink>
 *
 * Since: 0.6
 */
void
champlain_network_map_data_source_load_map_data (
    ChamplainNetworkMapDataSource *self,
    gdouble bound_left,
    gdouble bound_bottom,
    gdouble bound_right,
    gdouble bound_top)
{
  g_return_if_fail (CHAMPLAIN_IS_NETWORK_MAP_DATA_SOURCE (self));

  g_return_if_fail (bound_right - bound_left < 0.25 &&
      bound_top - bound_bottom < 0.25);

  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE (self);
  SoupMessage *msg;
  gchar *url;

  g_object_set (G_OBJECT (self), "state", CHAMPLAIN_STATE_LOADING, NULL);

  if (!soup_session)
    {
      soup_session = soup_session_async_new_with_options (
          "proxy-uri", soup_uri_new (priv->proxy_uri),
#ifdef HAVE_LIBSOUP_GNOME
          SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_PROXY_RESOLVER_GNOME,
#endif
          NULL);
      g_object_set (G_OBJECT (soup_session),
          "user-agent", "libchamplain/" CHAMPLAIN_VERSION_S,
          "max-conns-per-host", 8, NULL);
      g_object_add_weak_pointer (G_OBJECT (soup_session),
          (gpointer *) &soup_session);
    }

  url = g_strdup_printf (
      "http://api.openstreetmap.org/api/0.6/map?bbox=%f,%f,%f,%f",
      bound_left, bound_bottom, bound_right, bound_top);
  msg = soup_message_new ("GET", url);

  DEBUG ("Request BBox data: '%s'", url);

  g_free (url);

  soup_session_queue_message (soup_session, msg, load_map_data_cb, self);
}

/**
 * champlain_network_map_data_source_get_api_uri:
 * @source: a #ChamplainNetworkMapDataSource
 *
 * Returns the URI of the API server.
 *
 * Since: 0.6
 */
const gchar *
champlain_network_map_data_source_get_api_uri (
    ChamplainNetworkMapDataSource *self)
{
  g_return_val_if_fail (CHAMPLAIN_IS_NETWORK_MAP_DATA_SOURCE (self), NULL);

  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE (self);

  return priv->api_uri;
}

/**
 * champlain_network_map_data_source_set_api_uri:
 * @source: a #ChamplainNetworkMapDataSource
 * @api_uri: an URI of an API server
 *
 * Sets the URI of the API server.
 *
 * Since: 0.6
 */
void
champlain_network_map_data_source_set_api_uri (
    ChamplainNetworkMapDataSource *self,
    const gchar *api_uri)
{
  g_return_if_fail (CHAMPLAIN_IS_NETWORK_MAP_DATA_SOURCE (self)
      && api_uri != NULL);

  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE (self);

  g_free (priv->api_uri);
  priv->api_uri = g_strdup (api_uri);
}
