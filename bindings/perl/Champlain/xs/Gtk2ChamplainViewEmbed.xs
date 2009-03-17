#include "champlain-perl.h"


MODULE = Gtk2::Champlain::ViewEmbed  PACKAGE = Gtk2::Champlain::ViewEmbed  PREFIX = champlain_view_embed_

PROTOTYPES: DISABLE


GtkWidget*
champlain_view_embed_new (class, ChamplainView *view)
	C_ARGS: view


# FIXME the parameter embed should be of type ChamplainViewEmbed. The bindings
#       fail to generate the proper C code and leave unresolved symbols when
#       using ChamplainViewEmbed. In order to provide 100% coverage of the API
#       a GtkWidget has to be passed instead.
ChamplainView*
champlain_view_embed_get_view (GtkWidget *embed)
