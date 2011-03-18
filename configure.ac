#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Initialize autoconf
AC_PREREQ([2.59])
# Library versioning
# Making releases:
#   qahira_micro_version += 1
#   qahira_interface_age += 1
# If functions have been added or backward-compatibility has been broken:
#   qahira_interface_age = 0
m4_define([qahira_major_version], [0])
m4_define([qahira_minor_version], [1])
m4_define([qahira_micro_version], [0])
m4_define([qahira_interface_age], [0])
# Initialize autoconf & automake
AC_INIT([Qahira], [qahira_major_version.qahira_minor_version.qahira_micro_version],
    [http://github.com/msteinert/qahira/issues/], [qahira],
    [http://msteinert.github.com/qahira/])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.9])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_SRCDIR([qahira/qahira.h])
# Initialize Libtool
LT_PREREQ([2.2])
LT_INIT([disable-static])
# Checks for programs
m4_ifdef([AC_PROG_CC_C99], [AC_PROG_CC_C99],
	[AC_MSG_WARN([Attemping c99 workaround due to old version of autoconf])
	AC_PROG_CC
	QAHIRA_TRY_CC_FLAGS([-std=c99], [QAHIRA_CFLAGS([-std=c99])])])
PKG_PROG_PKG_CONFIG([0.17])
# Enable compiler flags
QAHIRA_TRY_CC_FLAGS([-Wall], [QAHIRA_CFLAGS([-Wall])])
QAHIRA_TRY_CC_FLAGS([-Wstrict-prototypes], [QAHIRA_CFLAGS([-Wstrict-prototypes])])
QAHIRA_TRY_CC_FLAGS([-fno-common], [QAHIRA_CFLAGS([-fno-common])])
# Pkg-config versioning
qahira_series_major=qahira_major_version
qahira_series_minor=qahira_minor_version
qahira_series=$qahira_series_major.$qahira_series_minor
AC_SUBST([qahira_series_major])
AC_SUBST([qahira_series_minor])
AC_SUBST([qahira_series])
# Libtool versioning
LT_CURRENT=m4_eval(100 * qahira_minor_version + qahira_micro_version - qahira_interface_age)
LT_REVISION=qahira_interface_age
LT_AGE=m4_eval(m4_eval(100 * qahira_minor_version + qahira_micro_version) - qahira_interface_age)
AC_SUBST([LT_CURRENT])
AC_SUBST([LT_REVISION])
AC_SUBST([LT_AGE])
# Checks for dependencies
qahira_glib_minimum_version=2.12 # should be 2.24
qahira_glib_version="glib-2.0 >= $qahira_glib_minimum_version"
AC_SUBST([qahira_glib_version])
PKG_CHECK_MODULES([GLIB], [$qahira_glib_version])
qahira_gobject_version="gobject-2.0 >= $qahira_glib_minimum_version"
AC_SUBST([qahira_gobject_version])
PKG_CHECK_MODULES([GOBJECT], [$qahira_gobject_version])
qahira_gio_version="gio-2.0 >= $qahira_glib_minimum_version"
AC_SUBST([qahira_gio_version])
PKG_CHECK_MODULES([GIO], [$qahira_gio_version])
qahira_gio_unix_version="gio-unix-2.0 >= $qahira_glib_minimum_version"
AC_SUBST([qahira_gio_unix_version])
PKG_CHECK_MODULES([GIO_UNIX], [$qahira_gio_unix_version])
qahira_cairo_version="cairo >= 1.10"
AC_SUBST([qahira_cairo_version])
PKG_CHECK_MODULES([CAIRO], [$qahira_cairo_version])
# Debug option
AC_ARG_ENABLE([debug],
	[AC_HELP_STRING([--enable-debug],
		[enable runtime debugging features @<:@default=no@:>@])],
	[AC_DEFINE([QAHIRA_DEBUG], [1], [Define to 1 to enable runtime debugging.])
	QAHIRA_TRY_CC_FLAGS([-g], [QAHIRA_CFLAGS([-g])])
	qahira_debug=yes],
	[qahira_debug=no])
# Tracing option
AC_ARG_ENABLE([trace],
	[AC_HELP_STRING([--enable-trace],
		[enable runtime tracing features @<:@default=no@:>@])],
	[AS_IF([test "x$qahira_debug" = "xno"],
		[AC_MSG_ERROR([Debugging must be enabled to enable tracing.])])
	AC_CHECK_LIB([bfd], [bfd_openr],
		[AC_MSG_WARN([Linking with libbfd makes this library subject to the terms of the GPL!])
		AC_DEFINE([HAVE_BFD], [1],
			[Define to 1 if libbfd is available.])])
	AC_CHECK_LIB([dl], [dladdr],
		[AC_DEFINE([_GNU_SOURCE], [1],
			[Define to 1 to enable GNU extensions.])
		AC_DEFINE([HAVE_DLADDR], [1],
			[Define to 1 dladdr is available.])])
	QAHIRA_TRY_CC_FLAGS([-finstrument-functions],
		[QAHIRA_CFLAGS([-finstrument-functions])],
		[AC_MSG_ERROR(['-finstrument-functions' is required for tracing.])])
	AC_DEFINE([QAHIRA_TRACE], [1],
		[Define to 1 to enable runtime tracing.])
	qahira_trace=yes],
	[qahira_trace=no])
# Native language support
AC_ARG_ENABLE([nls],
	[AC_HELP_STRING([--enable-nls],
		[enable native language support @<:@default=no@:>@])],
	[qahira_nls=$enableval],
	[qahira_nls=no])
AS_IF([test "x$qahira_nls" = "xyes"],
	[AC_DEFINE([ENABLE_NLS], [1], [Define to 1 to enable i18n.])
	AC_CHECK_FUNCS([bind_textdomain_codeset])])
# GLib configuration
AC_DEFINE_UNQUOTED([G_LOG_DOMAIN], ["$PACKAGE_NAME"],
	[The GLib log domain for this project.])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$PACKAGE"],
	[The i18n domain for this project.])
AH_BOTTOM(
[#ifdef ENABLE_NLS
/* Include GLib i18n header file */
#include <glib/gi18n-lib.h>
#else // ENABLE_NLS
#define Q_(s) s
#define N_(s) s
#endif // ENABLE_NLS
])dnl
# Output files
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	qahira.pc
	qahira/Makefile
	tests/Makefile
	Makefile
])
AC_OUTPUT
AC_MSG_RESULT([
$PACKAGE_NAME build options:
	Host: $host
	Version: $VERSION
	Native Language Support: $qahira_nls
	Debugging: $qahira_debug
	Tracing: $qahira_trace
])
