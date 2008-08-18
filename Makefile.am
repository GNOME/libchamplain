SUBDIRS = tidy src 

pcfiles = champlain-1.0.pc

champlain-1.0.pc: champlain.pc
	@cp -f $< $@

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig
