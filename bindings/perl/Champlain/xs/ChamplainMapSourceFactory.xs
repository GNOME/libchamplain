#include "champlain-perl.h"


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
	SV **s;
	ChamplainMapSourceDesc *desc;
	gboolean is_valid = TRUE;

	if ((!data) || (!SvOK(data)) || (!SvRV(data)) || (SvTYPE(SvRV(data)) != SVt_PVHV)) {
		croak("SvChamplainMapSourceDesc: value must be an hashref");
	}

	hash = (HV *) SvRV(data);
	
	/*
	 * We must set at least one of the following keys:
	 *   - text
	 *   - uris
	 *   - filename
	 */
	desc = g_new0(ChamplainMapSourceDesc, 1);
	if ((s = hv_fetch(hash, "id", 2, 0)) && SvOK(*s)) {
		desc->id = SvGChar(*s);
	}
	else {
		g_free(desc);
		croak("SvChamplainMapSourceDesc: requires the key: 'id'");
	}
	
	if ((s = hv_fetch(hash, "name", 4, 0)) && SvOK(*s)) {
		desc->name = SvGChar(*s);
	}
	else {
		g_free(desc);
		croak("SvChamplainMapSourceDesc: requires the key: 'name'");
	}
	
	if ((s = hv_fetch(hash, "license", 7, 0)) && SvOK(*s)) {
		desc->license = SvGChar(*s);
	}
	else {
		g_free(desc);
		croak("SvChamplainMapSourceDesc: requires the key: 'license'");
	}
	
	if ((s = hv_fetch(hash, "license_uri", 11, 0)) && SvOK(*s)) {
		desc->license_uri = SvGChar(*s);
	}
	else {
		g_free(desc);
		croak("SvChamplainMapSourceDesc: requires the key: 'license_uri'");
	}
	
	if ((s = hv_fetch(hash, "min_zoom_level", 14, 0)) && SvOK(*s)) {
		desc->min_zoom_level = (gint)SvIV(*s);
	}
	else {
		g_free(desc);
		croak("SvChamplainMapSourceDesc: requires the key: 'min_zoom_level'");
	}
	
	if ((s = hv_fetch(hash, "max_zoom_level", 14, 0)) && SvOK(*s)) {
		desc->max_zoom_level = (gint)SvIV(*s);
	}
	else {
		g_free(desc);
		croak("SvChamplainMapSourceDesc: requires the key: 'max_zoom_level'");
	}
	
	if ((s = hv_fetch(hash, "projection", 10, 0)) && SvOK(*s)) {
		desc->projection = SvChamplainMapProjection(*s);
	}
	else {
		g_free(desc);
		croak("SvChamplainMapSourceDesc: requires the key: 'projection'");
	}

	return desc;
}


MODULE = Champlain::MapSourceFactory  PACKAGE = Champlain::MapSourceFactory  PREFIX = champlain_map_source_factory_


ChamplainMapSourceFactory*
champlain_map_source_factory_get_default (class)
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
champlain_map_source_factory_register (ChamplainMapSourceFactory *factory, SV *data)
	PREINIT:
		ChamplainMapSourceDesc *desc = NULL;
		SV *sv = NULL;
	
	CODE:
		
		desc = SvChamplainMapSourceDesc(data);
		RETVAL = champlain_map_source_factory_register(factory, desc);

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
