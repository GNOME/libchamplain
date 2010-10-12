
#champlain-gtk-0.6.vapi: $(top_srcdir)/champlain-gtk/GtkChamplain-0.6.gir
#	$(VAPIGEN) --vapidir ../champlain --pkg champlain-0.6 --pkg clutter-1.0 --pkg atk --pkg gtk+-3.0 --library champlain-gtk-0.6 $^
#	touch $@

vapidir = $(datadir)/vala/vapi

dist_vapi_DATA = \
	champlain-gtk-@CHAMPLAIN_API_VERSION@.vapi \
	champlain-gtk-@CHAMPLAIN_API_VERSION@.deps \
	$(NULL)

