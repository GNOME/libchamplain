#include "champlain-perl.h"


MODULE = Champlain::ZoomLevel  PACKAGE = Champlain::ZoomLevel  PREFIX = champlain_zoom_level_


ChamplainZoomLevel*
champlain_zoom_level_new (class)
	C_ARGS: /* No args */


gint
champlain_zoom_level_get_width (ChamplainZoomLevel *self)


gint
champlain_zoom_level_get_height (ChamplainZoomLevel *self)


gint
champlain_zoom_level_get_zoom_level (ChamplainZoomLevel *self)


ClutterActor*
champlain_zoom_level_get_actor (ChamplainZoomLevel *self)


void
champlain_zoom_level_set_width (ChamplainZoomLevel *self, guint width)


void
champlain_zoom_level_set_height (ChamplainZoomLevel *self, guint height)


void
champlain_zoom_level_set_zoom_level (ChamplainZoomLevel *self, gint zoom_level)


void
champlain_zoom_level_add_tile (ChamplainZoomLevel *self, ChamplainTile *tile)


void
champlain_zoom_level_remove_tile (ChamplainZoomLevel *self, ChamplainTile *tile)


guint
champlain_zoom_level_tile_count (ChamplainZoomLevel *self)


ChamplainTile*
champlain_zoom_level_get_nth_tile (ChamplainZoomLevel *self, guint index)

