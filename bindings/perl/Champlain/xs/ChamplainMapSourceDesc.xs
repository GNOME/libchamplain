#include "champlain-perl.h"


MODULE = Champlain::MapSourceDesc  PACKAGE = Champlain::MapSourceDesc  PREFIX = champlain_map_source_desc_


ChamplainMapSourceDesc*
champlain_map_source_desc_new (class)
	C_ARGS: /* No args */


ChamplainMapSourceDesc*
champlain_map_source_desc_copy (const ChamplainMapSourceDesc* desc)


void
champlain_map_source_desc_free (ChamplainMapSourceDesc* desc)


#
# Provide nice accessors to the data members of the struct.
#
SV*
get_id (ChamplainMapSourceDesc *desc)
	ALIAS:
		Champlain::MapSourceDesc::get_name = 1
		Champlain::MapSourceDesc::get_license = 2
		Champlain::MapSourceDesc::get_license_uri = 3
		Champlain::MapSourceDesc::get_min_zoom_level = 4
		Champlain::MapSourceDesc::get_max_zoom_level = 5
		Champlain::MapSourceDesc::get_projection = 6
		Champlain::MapSourceDesc::get_constructor = 7
		Champlain::MapSourceDesc::get_uri_format = 8

	CODE:
		switch (ix) {
			case 0:
				RETVAL = newSVGChar(desc->id);
			break;
			
			case 1:
				RETVAL = newSVGChar(desc->name);
			break;
			
			case 2:
				RETVAL = newSVGChar(desc->license);
			break;
			
			case 3:
				RETVAL = newSVGChar(desc->license_uri);
			break;
			
			case 4:
				RETVAL = newSViv(desc->min_zoom_level);
			break;
			
			case 5:
				RETVAL = newSViv(desc->max_zoom_level);
			break;
			
			case 6:
				RETVAL = newSVChamplainMapProjection(desc->projection);
			break;
			
			case 7:
				/* This is tricky as we have to wrap the C callback into a Perl sub. */
				croak("$desc->get_constructor() isn't implemented yet");
			break;
			
			case 8:
				RETVAL = newSVGChar(desc->uri_format);
			break;
			
			default:
				RETVAL = &PL_sv_undef;
				g_assert_not_reached();
			break;
		}

	OUTPUT:
		RETVAL


#
# Provide nice modifiers to the data members of the struct.
#
void
set_id (ChamplainMapSourceDesc *desc, SV *sv)
	ALIAS:
		Champlain::MapSourceDesc::set_name = 1
		Champlain::MapSourceDesc::set_license = 2
		Champlain::MapSourceDesc::set_license_uri = 3
		Champlain::MapSourceDesc::set_min_zoom_level = 4
		Champlain::MapSourceDesc::set_max_zoom_level = 5
		Champlain::MapSourceDesc::set_projection = 6
		Champlain::MapSourceDesc::set_constructor = 7
		Champlain::MapSourceDesc::set_uri_format = 8

	CODE:
		switch (ix) {
			case 0:
				desc->id = g_strdup(SvGChar(sv));
			break;
			
			case 1:
				desc->name = g_strdup(SvGChar(sv));
			break;
			
			case 2:
				desc->license = g_strdup(SvGChar(sv));
			break;
			
			case 3:
				desc->license_uri = g_strdup(SvGChar(sv));
			break;
			
			case 4:
				desc->min_zoom_level = (gint)SvIV(sv);
			break;
			
			case 5:
				desc->max_zoom_level = (gint)SvIV(sv);
			break;
			
			case 6:
				desc->projection = SvChamplainMapProjection(sv);
			break;
			
			case 7:
				/* This is tricky as we have to wrap the Perl sub into a C callback. */
				croak("$desc->set_constructor(\\&code_ref) isn't implemented yet");
			break;
			
			case 8:
				desc->uri_format = g_strdup(SvGChar(sv));
			break;
			
			default:
				croak("Unsupported property %s", GvNAME(CvGV(cv)));
			break;
		}
