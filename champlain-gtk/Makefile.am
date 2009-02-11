SOURCES_XML = champlain-gtk-sources.xml
CHAMPLAIN_METADATA = champlain-gtk.metadata
CHAMPLAIN_RAW_API = champlain-gtk-api.raw
CHAMPLAIN_FIXED_API = champlain-gtk-api.xml

ASSEMBLY_NAME = champlain-gtk-sharp
ASSEMBLY = champlain-gtk-sharp.dll

TARGET = $(ASSEMBLY) $(ASSEMBLY).config

custom_extradist= champlain-gtk-sharp.pc.in champlain-gtk-sharp.dll.config.in
custom_api_includes=../champlain/champlain-api.xml
custom_references=-r:../champlain/champlain-sharp.dll

sources =
#	$(srcdir)/Timeout.cs

customs =

include ../Makefile.include

