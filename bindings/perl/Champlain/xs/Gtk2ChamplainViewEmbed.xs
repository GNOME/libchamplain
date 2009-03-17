#include "champlain-perl.h"


MODULE = Gtk2::Champlain::ViewEmbed  PACKAGE = Gtk2::Champlain::ViewEmbed  PREFIX = champlain_view_embed_

PROTOTYPES: DISABLE


GtkWidget*
champlain_view_embed_new (class, ChamplainView *view)
	C_ARGS: view


ChamplainView*
champlain_view_embed_get_view (ChamplainViewEmbed *embed)
