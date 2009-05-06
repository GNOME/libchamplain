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
