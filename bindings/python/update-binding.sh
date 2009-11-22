#! /bin/sh
#Manually update headers in pychamplain.override and pychamplaingtk.override.

# Update the list of headers from Makefile.am
cd ../../champlain
python /usr/share/pygobject/2.0/codegen/h2def.py	\
	-m champlain				\
	champlain.h				\
	champlain-cache.h			\
	champlain-view.h			\
	champlain-defines.h			\
	champlain-polygon.h			\
	champlain-point.h			\
	champlain-layer.h			\
	champlain-map-source.h			\
	champlain-map-source-desc.h		\
	champlain-map-source-factory.h		\
	champlain-network-map-source.h		\
	champlain-marker.h			\
	champlain-base-marker.h			\
	champlain-tile.h			\
	champlain-zoom-level.h			\
 > ../bindings/python/champlain/pychamplain-base.defs


# Update the list of headers from Makefile.am
cd ../champlain-gtk
python /usr/share/pygobject/2.0/codegen/h2def.py	\
	-m champlain				\
	champlain-gtk.h				\
	gtk-champlain-embed.h			\
 > ../bindings/python/champlain-gtk/pychamplaingtk.defs

# Keep original version
cd ../bindings/python
cp champlain/pychamplain-base.defs /tmp
cp champlain-gtk/pychamplaingtk.defs /tmp
