#include "champlain-perl.h"


MODULE = Gtk2::ChamplainEmbed  PACKAGE = Gtk2::ChamplainEmbed  PREFIX = gtk_champlain_embed_


GtkWidget*
gtk_champlain_embed_new (class)
	C_ARGS: /* No args */


ChamplainView *
gtk_champlain_embed_get_view (GtkChamplainEmbed* embed)
