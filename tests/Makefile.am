#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

# include directories
INCLUDES = \
	-I$(top_srcdir) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(CAIRO_CFLAGS)
# add libraries
LIBS = \
	$(top_builddir)/qahira/libqahira-$(qahira_series).la
# add unit tests
check_PROGRAMS = \
	qahira-test \
	loader-factory-test \
	surface-factory-test
TESTS = $(check_PROGRAMS)
# add source files
qahira_test_SOURCES = qahira-test.c
loader_factory_test_SOURCES = loader-factory-test.c
surface_factory_test_SOURCES = surface-factory-test.c
# generate XML report
qahira.xml: $(check_PROGRAMS)
	gtester -k -o $@ $(check_PROGRAMS)
# generate HTML report
qahira.html: qahira.xml
	gtester-report $< > $@
CLEANFILES = \
	qahira.xml
DISTCLEANFILES = \
	qahira.html core core.*
