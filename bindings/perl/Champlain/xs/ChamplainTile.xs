#include "champlain-perl.h"


MODULE = Champlain::Tile  PACKAGE = Champlain::Tile  PREFIX = champlain_tile_


ChamplainTile*
champlain_tile_new (class)
	C_ARGS: /* No args */


ChamplainTile*
champlain_tile_new_full (class, gint x, gint y, guint size, gint zoom_level)
	C_ARGS: x, y, size, zoom_level


gint
champlain_tile_get_x (ChamplainTile *self)


gint
champlain_tile_get_y (ChamplainTile *self)


gint
champlain_tile_get_zoom_level (ChamplainTile *self)


guint
champlain_tile_get_size (ChamplainTile *self)


ChamplainState
champlain_tile_get_state (ChamplainTile *self)


const gchar*
champlain_tile_get_uri (ChamplainTile *self)


const gchar*
champlain_tile_get_filename (ChamplainTile *self)


ClutterActor*
champlain_tile_get_actor (ChamplainTile *self)


void
champlain_tile_set_x (ChamplainTile *self, gint x)


void
champlain_tile_set_y (ChamplainTile *self, gint y)


void
champlain_tile_set_zoom_level (ChamplainTile *self, gint zoom_level)


void
champlain_tile_set_size (ChamplainTile *self, guint size)


void
champlain_tile_set_state (ChamplainTile *self, ChamplainState state)


void
champlain_tile_set_uri (ChamplainTile *self, const gchar* uri)


void
champlain_tile_set_filename (ChamplainTile *self, const gchar* filename)


ClutterActor *
champlain_tile_get_content (ChamplainTile *self)


const gchar*
champlain_tile_get_etag (ChamplainTile *self)


void
champlain_tile_get_modified_time (ChamplainTile *self)
	PREINIT:
		const GTimeVal *modified_time = NULL;

	PPCODE:
		modified_time = champlain_tile_get_modified_time(self);

		if (modified_time) {
			EXTEND(SP, 2);
			PUSHs(sv_2mortal(newSViv(modified_time->tv_sec)));
			PUSHs(sv_2mortal(newSViv(modified_time->tv_usec)));
		}
		else {
			EXTEND(SP, 2);
			PUSHs(sv_2mortal(&PL_sv_undef));
			PUSHs(sv_2mortal(&PL_sv_undef));
		}


SV*
champlain_tile_get_modified_time_string (ChamplainTile *self)
	PREINIT:
		gchar *string = NULL;

	CODE:
		string = champlain_tile_get_modified_time_string(self);
		if (string) {
			RETVAL = newSVpvn(string, 0);
			g_print("--time : %s\n", string);
			g_free(string);
		}
		else {
			g_print("--Undef time\n");
			RETVAL = &PL_sv_undef;
		}

	OUTPUT:
		RETVAL



void
champlain_tile_set_content (ChamplainTile *self, ClutterActor* actor, gboolean fade_in)


void
champlain_tile_set_etag (ChamplainTile *self, const gchar *etag)


void
champlain_tile_set_modified_time (ChamplainTile *self, guint seconds = 0, guint microseconds = 0)
	PREINIT:
		GTimeVal modified_time = {0, };

	CODE:
		if (microseconds || seconds) {
			modified_time.tv_sec = seconds;
			modified_time.tv_usec = microseconds;
		}
		else {
			g_get_current_time(&modified_time);
		}

		champlain_tile_set_modified_time(self, &modified_time);
