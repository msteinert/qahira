#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

# library definition
noinst_LTLIBRARIES = libqahira-image.la
libqahira_image_la_SOURCES =
# include directories & compiler switches
libqahira_image_la_CFLAGS = \
	-I$(top_srcdir) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(CAIRO_CFLAGS)
libqahira_image_la_LDFLAGS =
# install development headers
imageincludedir = $(pkgincludedir)/image
imageinclude_FILES =
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =
if QAHIRA_HAS_JPEG
pkgconfig_DATA += qahira-jpeg.pc
libqahira_image_la_SOURCES += jpeg.c
imageinclude_FILES += jpeg.h
endif
if QAHIRA_HAS_PNG
pkgconfig_DATA += qahira-png.pc
libqahira_image_la_CFLAGS += $(PNG_CFLAGS)
libqahira_image_la_LDFLAGS += $(PNG_LIBS)
libqahira_image_la_SOURCES += png.c
imageinclude_FILES += png.h
endif
if QAHIRA_HAS_TARGA
pkgconfig_DATA += qahira-targa.pc
libqahira_image_la_SOURCES += targa.c
imageinclude_FILES += targa.h
endif
