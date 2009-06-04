#include "champlain-perl.h"


MODULE = Champlain::NetworkMapSource  PACKAGE = Champlain::NetworkMapSource  PREFIX = champlain_network_map_source_


ChamplainNetworkMapSource*
champlain_network_map_source_new_full (class, gchar *id, gchar *name, gchar *license, gchar *license_uri, guint min_zoom, guint map_zoom, guint tile_size, ChamplainMapProjection projection, gchar *uri_format)
	C_ARGS: id, name, license, license_uri, min_zoom, map_zoom, tile_size, projection, uri_format


const gchar*
champlain_network_map_source_get_tile_uri (ChamplainNetworkMapSource *source, gint x, gint y, gint z)


void
champlain_network_map_source_set_uri_format (ChamplainNetworkMapSource *source, const gchar *uri_format)
