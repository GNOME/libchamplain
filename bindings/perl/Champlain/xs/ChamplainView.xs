#include "champlain-perl.h"


MODULE = Champlain::View  PACKAGE = Champlain::View  PREFIX = champlain_view_


ClutterActor*
champlain_view_new (class)
	C_ARGS: /* No args */


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
champlain_view_go_to (ChamplainView *view, gdouble latitude, gdouble longitude)


void
champlain_view_stop_go_to (ChamplainView *view)


void
champlain_view_set_min_zoom_level (ChamplainView *view, gint zoom_level)


void
champlain_view_set_max_zoom_level (ChamplainView *view, gint zoom_level)


void
champlain_view_ensure_visible (ChamplainView *view, gdouble lat1, gdouble lon1, gdouble lat2, gdouble lon2, gboolean animate)


# FIXME the order there is important for perl
#void
#champlain_view_ensure_markers_visible (ChamplainView *view, ChamplainBaseMarker *markers[], gboolean animate)


void
champlain_view_set_decel_rate (ChamplainView *view, gdouble rate)


void
champlain_view_set_scroll_mode (ChamplainView *view, ChamplainScrollMode mode)


void
champlain_view_set_keep_center_on_resize (ChamplainView *view, gboolean value)


void
champlain_view_set_show_license (ChamplainView *view, gboolean value)


void
champlain_view_set_zoom_on_double_click (ChamplainView *view, gboolean value)


void
champlain_view_get_coords_at (ChamplainView *view, guint x, guint y, OUTLIST gdouble latitude, OUTLIST gdouble longitude)
