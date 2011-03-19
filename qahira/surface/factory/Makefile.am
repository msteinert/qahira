#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

# library definition
noinst_LTLIBRARIES = libqahira-surface-factory.la
libqahira_surface_factory_la_SOURCES = \
	image.c
# include directories & compiler switches
libqahira_surface_factory_la_CFLAGS = \
	-I$(top_srcdir) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(CAIRO_CFLAGS)
# install development headers
imagefactoryincludedir = $(pkgincludedir)/factory
imagefactoryinclude_FILES = \
	image.h
