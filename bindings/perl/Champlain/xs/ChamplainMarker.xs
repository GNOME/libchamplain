#include "champlain-perl.h"


MODULE = Champlain::Marker  PACKAGE = Champlain::Marker  PREFIX = champlain_marker_

PROTOTYPES: DISABLE


ClutterActor*
champlain_marker_new (class)
	C_ARGS:


void
champlain_marker_set_position (ChamplainMarker *marker, gdouble longitude, gdouble latitude)


ClutterActor*
champlain_marker_new_with_label (class, const gchar *label, const gchar *font, ClutterColor_ornull *text_color, ClutterColor_ornull *marker_color)
	C_ARGS: label, font, text_color, marker_color


ClutterActor*
champlain_marker_new_with_image (class, gchar *filename)
	PREINIT:
		GError *error = NULL;
	CODE:
		RETVAL = champlain_marker_new_with_image(filename, &error);
		if (error) {
			gperl_croak_gerror(NULL, error);
		}
	OUTPUT:
		RETVAL


ClutterActor*
champlain_marker_new_with_image_full(class, const gchar *filename, gint width, gint height, gint anchor_x, gint anchor_y)
	PREINIT:
		GError *error = NULL;
	CODE:
		RETVAL = champlain_marker_new_with_image_full(filename, width, height, anchor_x, anchor_y, &error);
		if (error) {
			gperl_croak_gerror(NULL, error);
		}
	OUTPUT:
		RETVAL
