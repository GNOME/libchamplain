#include "champlain-perl.h"


MODULE = Champlain::BaseMarker  PACKAGE = Champlain::BaseMarker  PREFIX = champlain_base_marker_


ClutterActor*
champlain_base_marker_new (class)
	C_ARGS: /* No args */


void
champlain_base_marker_set_position (ChamplainBaseMarker *marker, gdouble longitude, gdouble latitude)

