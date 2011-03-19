#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

SUBDIRS = qahira tests
ACLOCAL_AMFLAGS = -I m4
# install pkg-config script
qahira-$(qahira_series).pc: qahira.pc
	$(AM_V_GEN)cp $< $@
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = qahira-$(qahira_series).pc
# cleanup
DISTCLEANFILES = \
	qahira-$(qahira_series).pc
