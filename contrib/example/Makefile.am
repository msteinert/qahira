#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

INCLUDES = \
	-I$(top_srcdir) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS)
bin_PROGRAMS =
if QAHIRA_HAS_X11
bin_PROGRAMS += qahira-x11
qahira_x11_SOURCES = \
	qahira-x11.c
INCLUDES += \
	$(CAIRO_XLIB_CFLAGS) \
	$(X11_CFLAGS)
qahira_x11_LDFLAGS = \
	$(CAIRO_XLIB_CFLAGS) \
	$(X11_LIBS)
qahira_x11_LDADD = \
	$(top_builddir)/qahira/libqahira-$(qahira_series).la
endif
