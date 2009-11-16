#include "champlain-gtk-perl.h"


MODULE = Gtk2::Champlain::Embed  PACKAGE = Gtk2::Champlain::Embed  PREFIX = gtk_champlain_embed_


GtkWidget*
gtk_champlain_embed_new (class)
	C_ARGS: /* No args */


ChamplainView*
gtk_champlain_embed_get_view (GtkChamplainEmbed *embed)
