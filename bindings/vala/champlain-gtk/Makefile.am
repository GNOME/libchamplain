
champlain-gtk-0.4.vapi: $(top_srcdir)/champlain-gtk/GtkChamplain-0.4.gir
	$(VAPIGEN) --vapidir ../champlain --pkg champlain-0.4 --pkg clutter-1.0 --pkg atk --pkg gtk+-2.0 --library champlain-gtk-0.4 $^
	touch $@

vapidir = $(datadir)/vala/vapi

dist_vapi_DATA = \
	champlain-gtk-0.4.vapi \
	$(NULL)

EXTRA_DIST = champlain-gtk-0.4.vapi
