#include "champlain-perl.h"


MODULE = Champlain::View  PACKAGE = Champlain::View  PREFIX = champlain_view_

PROTOTYPES: DISABLE


ClutterActor*
champlain_view_new (class)
	C_ARGS:


void
champlain_view_center_on (ChamplainView *view, gdouble latitude, gdouble longitude)


void
champlain_view_set_size (ChamplainView *view, guint width, guint height)


void
champlain_view_zoom_in (ChamplainView *view)


void
champlain_view_zoom_out (ChamplainView *view)
	

void
champlain_view_add_layer (ChamplainView *view, ChamplainLayer *layer)


void
champlain_view_get_coords_from_event (ChamplainView *view, ClutterEvent *event, OUTLIST gdouble latitude, OUTLIST gdouble longitude)


void
champlain_view_set_zoom_level (ChamplainView *view, gint zoom_level)


void
champlain_view_set_map_source (ChamplainView *view, ChamplainMapSource *map_source);


void
champlain_view_tile_ready (ChamplainView *view, ChamplainZoomLevel *level, ChamplainTile *tile, gboolean animate);
