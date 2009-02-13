if ENABLE_GTK
GTK_ASSEMBLY=champlain-gtk-sharp.dll
GTK_REF=-r:champlain-gtk-sharp.dll
GTK_EXAMPLES=gtk-test.cs
else
GTK_ASSEMBLY=
GTK_REF=
GTK_EXAMPLES=
endif

SAMPLES= $(GTK_EXAMPLES)
MCSFLAGS= -debug -nologo -pkg:clutter-sharp -pkg:clutter-gtk-sharp -r:champlain-sharp.dll $(GTK_REF)

assemblies=champlain-sharp.dll $(GTK_ASSEMBLY)

noinst_SCRIPTS=$(SAMPLES:.cs=.exe)
%.exe: %.cs $(assemblies)
	$(MCS) $(MCSFLAGS) $(GTKSHARP_LIBS) -out:$@ $<

champlain-sharp.dll: $(top_builddir)/bindings/managed/champlain/champlain-sharp.dll
	cp $(top_builddir)/bindings/managed/champlain/champlain-sharp.dll* .

champlain-gtk-sharp.dll: $(top_builddir)/bindings/managed/champlain-gtk/champlain-gtk-sharp.dll
	cp $(top_builddir)/bindings/managed/champlain-gtk/champlain-gtk-sharp.dll* .

EXTRA_DIST=$(SAMPLES)
CLEANFILES= *.exe *.mdb *.pdb *.dll *.config
