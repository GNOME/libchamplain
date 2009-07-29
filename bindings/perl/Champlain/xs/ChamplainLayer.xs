#include "champlain-perl.h"


MODULE = Champlain::Layer  PACKAGE = Champlain::Layer  PREFIX = champlain_layer_


ChamplainLayer*
champlain_layer_new (class)
	C_ARGS: /* No args */


void
champlain_layer_add_marker (ChamplainLayer *layer, ChamplainBaseMarker *marker);
