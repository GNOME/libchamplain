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

#include "champlain-network-map-data-source.h"

#define DEBUG_FLAG CHAMPLAIN_DEBUG_MEMPHIS
#include "champlain-debug.h"

#include <memphis/memphis.h>
#include <libsoup/soup.h>

G_DEFINE_TYPE (ChamplainNetworkMapDataSource, champlain_network_map_data_source, CHAMPLAIN_TYPE_MAP_DATA_SOURCE)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), CHAMPLAIN_TYPE_NETWORK_MAP_DATA_SOURCE, ChamplainNetworkMapDataSourcePrivate))

enum
{
  PROP_0,
  PROP_API_URI
};

typedef struct _ChamplainNetworkMapDataSourcePrivate ChamplainNetworkMapDataSourcePrivate;

struct _ChamplainNetworkMapDataSourcePrivate {
  MemphisMap *map;
  gchar *api_uri;
};

static void
champlain_network_map_data_source_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  switch (property_id) {
    // TODO
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
  switch (property_id) {
    // TODO
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
champlain_network_map_data_source_dispose (GObject *object)
{
  ChamplainNetworkMapDataSource *self = (ChamplainNetworkMapDataSource *) object;
  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE(self);

  memphis_map_free (priv->map);

  G_OBJECT_CLASS (champlain_network_map_data_source_parent_class)->dispose (object);
}

static void
champlain_network_map_data_source_finalize (GObject *object)
{
  ChamplainNetworkMapDataSource *self = (ChamplainNetworkMapDataSource *) object;
  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE(self);

  g_free (priv->api_uri);

  G_OBJECT_CLASS (champlain_network_map_data_source_parent_class)->finalize (object);
}

static MemphisMap*
get_map_data (ChamplainMapDataSource *self)
{
  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE(self);

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
}

static void
champlain_network_map_data_source_init (ChamplainNetworkMapDataSource *self)
{
  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE(self);

  priv->map = memphis_map_new ();
  priv->api_uri = g_strdup ("http://www.informationfreeway.org/api/0.6");
}

ChamplainNetworkMapDataSource*
champlain_network_map_data_source_new (void)
{
  return g_object_new (CHAMPLAIN_TYPE_NETWORK_MAP_DATA_SOURCE, NULL);
}

static void
load_map_data_cb (SoupSession *session, SoupMessage *msg,
    gpointer user_data)
{
  ChamplainNetworkMapDataSource *self = 
      (ChamplainNetworkMapDataSource *) user_data;
  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE(self);

  // TODO: error handling, error tile?
  MemphisMap *map = memphis_map_new ();
  memphis_map_set_debug_level (map, 0);
  memphis_map_load_from_data (map,
      msg->response_body->data,
      msg->response_body->length);

  DEBUG ("BBox data received");

  priv->map = map;

  g_signal_emit_by_name (CHAMPLAIN_MAP_DATA_SOURCE (self),
       "map-data-changed", NULL);
}


void
champlain_network_map_data_source_load_map_data (
    ChamplainNetworkMapDataSource *self,
    gdouble bound_left,
    gdouble bound_bottom,
    gdouble bound_right,
    gdouble bound_top)
{
  g_return_if_fail (CHAMPLAIN_IS_NETWORK_MAP_DATA_SOURCE (self));

  // TODO: check valid bbox size

  SoupMessage *msg;
  SoupSession *sess = soup_session_sync_new ();

  gchar *url = g_strdup_printf (
      "http://api.openstreetmap.org/api/0.6/map?bbox=%f,%f,%f,%f",
      bound_left, bound_bottom, bound_right, bound_top);
  msg = soup_message_new ("GET", url);
  g_free (url);

  DEBUG ("Request BBox data");
  soup_session_queue_message (sess, msg, load_map_data_cb, self);
}

gchar *
champlain_network_map_data_source_get_api_uri (
    ChamplainNetworkMapDataSource *self)
{
  g_return_val_if_fail (CHAMPLAIN_IS_NETWORK_MAP_DATA_SOURCE (self), NULL);

  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE(self);

  return priv->api_uri;
}

void
champlain_network_map_data_source_set_api_uri (
    ChamplainNetworkMapDataSource *self,
    gchar *api_uri)
{
  g_return_if_fail (CHAMPLAIN_IS_NETWORK_MAP_DATA_SOURCE (self));

  ChamplainNetworkMapDataSourcePrivate *priv = GET_PRIVATE(self);

  if (api_uri != NULL)
    g_free (priv->api_uri);

  priv->api_uri = api_uri;
}
