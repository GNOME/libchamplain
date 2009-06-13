#include "champlain-perl.h"

static GPerlCallback*
champlainperl_constructor_create (SV *func, SV *data) {
	GType param_types[2] = { G_TYPE_POINTER, G_TYPE_POINTER, };
	return gperl_callback_new (func, data, 2, param_types, CHAMPLAIN_TYPE_MAP_SOURCE);
}


static ChamplainMapSource*
champlainperl_constructor (ChamplainMapSourceDesc *desc, gpointer data) {
	GPerlCallback *callback = (GPerlCallback *) data;
	GValue return_value = { 0, };
	ChamplainMapSource *retval;
	
	if (callback == NULL) {
		g_print ("Data is null!\n");
		croak("Chammplain::MapSourceFactory constructor callback is missing the data parameter");
	}
	
	g_value_init(&return_value, callback->return_type);
	/* FIXME desc is not passed as a Champlain::MapSourceDesc to the perl callback */
	gperl_callback_invoke(callback, &return_value, desc);
	
	retval = g_value_get_object (&return_value);
	g_value_unset(&return_value);
	
	return retval;
}


/**
 * Returns the value of the given key or croaks if there's no such key.
 */
static SV*
fetch_or_croak (HV* hash , const char* key , I32 klen) {

	SV **s = hv_fetch(hash, key, klen, 0);
	if (s != NULL && SvOK(*s)) {
		return *s;
	}
	
	croak("Hashref requires the key: '%s'", key);
}


MODULE = Champlain::MapSourceFactory  PACKAGE = Champlain::MapSourceFactory  PREFIX = champlain_map_source_factory_


ChamplainMapSourceFactory*
champlain_map_source_factory_dup_default (class)
	C_ARGS: /* No args */


void
champlain_map_source_factory_get_list (ChamplainMapSourceFactory *factory)
	PREINIT:
		GSList *list = NULL;
		GSList *item = NULL;
	
	PPCODE:
		list = champlain_map_source_factory_get_list(factory);
		
		for (item = list; item != NULL; item = item->next) {
			ChamplainMapSourceDesc *desc = CHAMPLAIN_MAP_SOURCE_DESC(item->data);
			XPUSHs(sv_2mortal(newSVChamplainMapSourceDesc(desc)));
		}
		
		g_slist_free(list);


ChamplainMapSource*
champlain_map_source_factory_create (ChamplainMapSourceFactory *factory, const gchar *id)


gboolean
champlain_map_source_factory_register (ChamplainMapSourceFactory *factory, SV *sv_desc, SV* sv_constructor, SV *sv_data=NULL)
	PREINIT:
		ChamplainMapSourceDesc *desc = NULL;
		SV *sv = NULL;
		GPerlCallback *callback = NULL;
	
	CODE:
		desc = SvChamplainMapSourceDesc(sv_desc);
		callback = champlainperl_constructor_create(sv_constructor, NULL);
		RETVAL = champlain_map_source_factory_register(factory, desc, champlainperl_constructor, callback);

	OUTPUT:
		RETVAL


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
OSM_CYCLE_MAP (class)
	CODE:
		RETVAL = CHAMPLAIN_MAP_SOURCE_OSM_CYCLE_MAP;

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
