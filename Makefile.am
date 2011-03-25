#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

SUBDIRS = qahira tests contrib/example
ACLOCAL_AMFLAGS = -I m4
# install pkg-config script
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = pkgconfig/qahira.pc
if QAHIRA_HAS_JPEG
pkgconfig_DATA += pkgconfig/qahira-jpeg.pc
endif
if QAHIRA_HAS_PNG
pkgconfig_DATA += pkgconfig/qahira-png.pc
endif
if QAHIRA_HAS_TARGA
pkgconfig_DATA += pkgconfig/qahira-targa.pc
endif
