#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

# library definition
noinst_LTLIBRARIES = libqahira-loader.la
libqahira_loader_la_SOURCES =
# include directories & compiler switches
libqahira_loader_la_CFLAGS = \
	-I$(top_srcdir) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(CAIRO_CFLAGS)
libqahira_loader_la_LDFLAGS =
# install development headers
loaderincludedir = $(pkgincludedir)/loader
loaderinclude_FILES =
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =
if QAHIRA_HAS_JPEG
pkgconfig_DATA += qahira-jpeg.pc
libqahira_loader_la_SOURCES += jpeg.c
loaderinclude_FILES += jpeg.h
endif
if QAHIRA_HAS_PNG
pkgconfig_DATA += qahira-png.pc
libqahira_loader_la_CFLAGS += $(PNG_CFLAGS)
libqahira_loader_la_LDFLAGS += $(PNG_LIBS)
libqahira_loader_la_SOURCES += png.c
loaderinclude_FILES += png.h
endif
if QAHIRA_HAS_TARGA
pkgconfig_DATA += qahira-targa.pc
libqahira_loader_la_SOURCES += targa.c
loaderinclude_FILES += targa.h
endif
