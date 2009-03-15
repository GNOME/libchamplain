#include "champlain-perl.h"


MODULE = Champlain::Layer  PACKAGE = Champlain::Layer  PREFIX = champlain_layer_

PROTOTYPES: DISABLE


ChamplainLayer*
champlain_layer_new (class)
	C_ARGS:
