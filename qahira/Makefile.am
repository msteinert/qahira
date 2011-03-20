#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

SUBDIRS = surface/factory loader
# library definition
lib_LTLIBRARIES = libqahira-@qahira_series_major@.@qahira_series_minor@.la
libqahira_@qahira_series_major@_@qahira_series_minor@_la_SOURCES = \
	error.c \
	loader-factory.c \
	loader/private.h \
	loader.c \
	macros.h \
	qahira.c \
	surface-factory.c
# include directories & compiler switches
libqahira_@qahira_series_major@_@qahira_series_minor@_la_CFLAGS = \
	-I$(top_srcdir) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
	$(CAIRO_CFLAGS)
libqahira_@qahira_series_major@_@qahira_series_minor@_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	$(GLIB_LIBS) \
	$(GOBJECT_LIBS) \
	$(GIO_LIBS) \
	$(GIO_UNIX_LIBS) \
	$(CAIRO_LIBS)
libqahira_@qahira_series_major@_@qahira_series_minor@_la_LIBADD = \
	surface/factory/libqahira-surface-factory.la \
	loader/libqahira-loader.la
# install development headers
pkgincludedir = $(includedir)/qahira-$(qahira_series)/qahira
pkginclude_HEADERS = \
	error.h \
	loader.h \
	qahira.h \
	types.h
loaderincludedir = $(pkgincludedir)/loader
loaderinclude_HEADERS = \
	loader/factory.h
surfaceincludedir = $(pkgincludedir)/surface
surfaceinclude_HEADERS = \
	surface/factory.h
