#ifndef _CHAMPLAIN_PERL_H_

#include <clutterperl.h>


#include <champlain/champlain.h>

#ifdef CHAMPLAINPERL_GTK
#include <gtk2perl.h>
#include <champlain-gtk/champlain-gtk.h>
#endif


/* Custom definitions for the bindings of Champlain::MapSourceDesc */
typedef ChamplainMapSourceDesc ChamplainMapSourceDesc_ornull;

SV* newSVChamplainMapSourceDesc (ChamplainMapSourceDesc *desc);
ChamplainMapSourceDesc* SvChamplainMapSourceDesc (SV *data);

#define SvChamplainMapSourceDesc_ornull(sv)  (gperl_sv_is_defined (sv) ? SvChamplainMapSourceDesc(sv) : NULL)
#define newSVChamplainMapSourceDesc_ornull(val)  (((val) == NULL) ? &PL_sv_undef : newSVChamplainMapSourceDesc(val))


#include "champlain-autogen.h"

#endif /* _CHAMPLAIN_PERL_H_ */
