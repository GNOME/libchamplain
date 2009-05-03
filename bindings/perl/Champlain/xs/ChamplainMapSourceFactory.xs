#include "champlain-perl.h"


MODULE = Champlain::MapSourceFactory  PACKAGE = Champlain::MapSourceFactory  PREFIX = champlain_map_source_factory_


ChamplainMapSourceFactory*
champlain_map_source_factory_get_default ()


#gchar**
#champlain_map_source_factory_get_list (ChamplainMapSourceFactory *factory)


ChamplainMapSource*
champlain_map_source_factory_create (ChamplainMapSourceFactory *factory, const gchar *id)


#gboolean
#champlain_map_source_factory_register (ChamplainMapSourceFactory *factory, const gchar *id, ChamplainMapSourceConstructor callback)


const gchar*
OSM_MAPNIK (class)
	CODE:
		RETVAL = CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK;

	OUTPUT:
		RETVAL


const gchar*
OSM_OSMARENDER (class)
	CODE:
		RETVAL = CHAMPLAIN_MAP_SOURCE_OSM_OSMARENDER;

	OUTPUT:
		RETVAL


const gchar*
OSM_CYCLEMA (class)
	CODE:
		RETVAL = CHAMPLAIN_MAP_SOURCE_OSM_CYCLEMAP;

	OUTPUT:
		RETVAL


const gchar*
OAM (class)
	CODE:
		RETVAL = CHAMPLAIN_MAP_SOURCE_OAM;

	OUTPUT:
		RETVAL


const gchar*
MFF_RELIEF (class)
	CODE:
		RETVAL = CHAMPLAIN_MAP_SOURCE_MFF_RELIEF;

	OUTPUT:
		RETVAL
