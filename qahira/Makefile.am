#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

# generate marshalling routines
$(srcdir)/marshal.h: marshal.list
	$(AM_V_GEN)$(GLIB_GENMARSHAL) --prefix=qahira_marshal --header $< > $@
$(srcdir)/marshal.c: marshal.list 
	$(AM_V_GEN)$(GLIB_GENMARSHAL) --prefix=qahira_marshal --body $< > $@
# library definition
lib_LTLIBRARIES = libqahira-@qahira_series_major@.@qahira_series_minor@.la
libqahira_@qahira_series_major@_@qahira_series_minor@_la_SOURCES = \
	accumulator.c \
	accumulator.h \
	error.c \
	image.c \
	image/private.h \
	image-serial.c \
	macros.h \
	marshal.c \
	marshal.h \
	qahira.c
BUILT_SOURCES = \
	marshal.c \
	marshal.h
# include directories & compiler switches
libqahira_@qahira_series_major@_@qahira_series_minor@_la_CFLAGS = \
	-I$(top_srcdir) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GIO_CFLAGS) \
	$(CAIRO_CFLAGS)
libqahira_@qahira_series_major@_@qahira_series_minor@_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	$(GLIB_LIBS) \
	$(GOBJECT_LIBS) \
	$(GIO_LIBS) \
	$(CAIRO_LIBS)
# install development headers
pkgincludedir = $(includedir)/qahira-$(qahira_series)/qahira
pkginclude_HEADERS = \
	error.h \
	image.h \
	types.h \
	qahira.h
# Image Formats
imageincludedir = $(pkgincludedir)/image
imageinclude_HEADERS = \
	image/serial.h
if QAHIRA_HAS_JPEG
libqahira_@qahira_series_major@_@qahira_series_minor@_la_SOURCES += \
	image-jpeg.c
imageinclude_HEADERS += image/jpeg.h
endif
if QAHIRA_HAS_PNG
libqahira_@qahira_series_major@_@qahira_series_minor@_la_CFLAGS += \
	$(PNG_CFLAGS)
libqahira_@qahira_series_major@_@qahira_series_minor@_la_LDFLAGS += \
	$(PNG_LIBS)
libqahira_@qahira_series_major@_@qahira_series_minor@_la_SOURCES += \
	image-png.c
imageinclude_HEADERS += image/png.h
endif
if QAHIRA_HAS_TARGA
libqahira_@qahira_series_major@_@qahira_series_minor@_la_SOURCES += \
	image-targa.c
imageinclude_HEADERS += image/targa.h
endif
EXTRA_DIST = \
	marshal.list
