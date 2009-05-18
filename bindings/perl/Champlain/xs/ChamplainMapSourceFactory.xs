#include "champlain-perl.h"

static GPerlCallback*
champlainperl_constructor_create (SV *func, SV *data) {
	GType param_types[2] = { G_TYPE_POINTER, G_TYPE_POINTER, };
	return gperl_callback_new (func, data, 2, param_types, CHAMPLAIN_TYPE_MAP_SOURCE);
}


static ChamplainMapSource*
champlainperl_constructor (ChamplainMapSourceDesc *desc, gpointer *data) {
	GPerlCallback *callback = (GPerlCallback *) data;
	GValue return_value = { 0, };
	ChamplainMapSource *retval;
	
	if (callback == NULL) {
		g_print ("Data is null!\n");
		croak("Chammplain::MapSourceFactory constructor callback is missing the data parameter");
	}
	
	g_value_init(&return_value, callback->return_type);
	
	gperl_callback_invoke(callback, &return_value, desc, data);
	
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


SV*
newSVChamplainMapSourceDesc (ChamplainMapSourceDesc *desc) {
	HV *hash = NULL;
	SV *sv = NULL;
	HV *stash = NULL;
	
	if (desc == NULL) {
		return &PL_sv_undef;
	}
	
	hash = newHV();
	sv = newRV_noinc((SV *) hash);
	
	/* Copy the data members of the struct into the hash */
	hv_store(hash, "id", 2, newSVGChar(desc->id), 0);
	hv_store(hash, "name", 4, newSVGChar(desc->name), 0);
	hv_store(hash, "license", 7, newSVGChar(desc->license), 0);
	hv_store(hash, "license_uri", 11, newSVGChar(desc->license_uri), 0);
	hv_store(hash, "min_zoom_level", 14, newSViv(desc->min_zoom_level), 0);
	hv_store(hash, "max_zoom_level", 14, newSViv(desc->max_zoom_level), 0);
	hv_store(hash, "projection", 10, newSVChamplainMapProjection(desc->projection), 0);

	/*
	   This is tricky as we have to wrap the C callback into a Perl sub.
	   hv_store(hash, "constructor", 11, newSVChamplainMapProjection(desc->projection), 0);
	*/
	
	/* Bless this stuff */
	stash = gv_stashpv("Champlain::MapSourceDesc", TRUE);
	sv_bless(sv, stash);
	
	return sv;
}


ChamplainMapSourceDesc*
SvChamplainMapSourceDesc (SV *data) {
	HV *hash;
	SV *value;
	ChamplainMapSourceDesc desc = {0,};

	if ((!data) || (!SvOK(data)) || (!SvRV(data)) || (SvTYPE(SvRV(data)) != SVt_PVHV)) {
		croak("SvChamplainMapSourceDesc: value must be an hashref");
	}

	hash = (HV *) SvRV(data);
	
	/* All keys are mandatory */
	if (value = fetch_or_croak(hash, "id", 2)) {
		desc.id = g_strdup(SvGChar(value));
	}
	
	if (value = fetch_or_croak(hash, "name", 4)) {
		desc.name = g_strdup(SvGChar(value));
	}
	
	if (value = fetch_or_croak(hash, "license", 7)) {
		desc.license = g_strdup(SvGChar(value));
	}
	
	if (value = fetch_or_croak(hash, "license_uri", 11)) {
		desc.license_uri = g_strdup(SvGChar(value));
	}
	
	if (value = fetch_or_croak(hash, "min_zoom_level", 14)) {
		desc.min_zoom_level = (gint)SvIV(value);
	}
	
	if (value = fetch_or_croak(hash, "max_zoom_level", 14)) {
		desc.max_zoom_level = (gint)SvIV(value);
	}
	
	if (value = fetch_or_croak(hash, "projection", 10)) {
		desc.projection = SvChamplainMapProjection(value);
	}

	return g_memdup(&desc, sizeof(desc));
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
