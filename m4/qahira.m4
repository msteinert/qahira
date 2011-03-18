#serial 1

# QAHIRA_TRY_CC_FLAGS(flags, [action-if-true], [action-if-false])
# ---------------------------------------------------------------
# Test if one or more compiler flags are supported.
AC_DEFUN([QAHIRA_TRY_CC_FLAGS],
[dnl
AC_REQUIRE([AC_PROG_CC])
for flag in "$1"
do
	qahira_cflags=$CFLAGS
	CFLAGS="$flag $CFLAGS"
	AC_MSG_CHECKING([if compiler accepts '$flag'])
	AC_TRY_COMPILE([], [],
		[AC_MSG_RESULT([yes])
		CFLAGS=$qahira_cflags
		$2],
		[AC_MSG_RESULT([no])
		CFLAGS=$qahira_cflags
		$3])
done
])dnl

# QAHIRA_TRY_LD_FLAGS(flags, [action-if-true], [action-if-false])
# ---------------------------------------------------------------
# Test if one or more linker flags are supported.
AC_DEFUN([QAHIRA_TRY_LD_FLAGS],
[dnl
for flag in "$1"
do
	qahira_ldflags=$LDFLAGS
	LDFLAGS="$flag $LDFLAGS"
	AC_MSG_CHECKING([if linker accepts '$flag'])
	AC_TRY_LINK([], [],
		[AC_MSG_RESULT([yes])
		LDFLAGS=$qahira_ldflags
		$2],
		[AC_MSG_RESULT([no])
		LDFLAGS=$qahira_ldflags
		$3])
done
])dnl

# QAHIRA_CFLAGS(flags)
# --------------------
# Enable compiler flags.
AC_DEFUN([QAHIRA_CFLAGS],
[dnl
for flag in "$1"
do
	case " $CFLAGS " in #(
	*[[\ \	]]$flag[[\ \	]]*) :
		;; #(
	*) :
		CFLAGS="$CFLAGS $flag" ;; #(
	esac
done
])dnl

# QAHIRA_LDFLAGS(flags)
# ---------------------
# Enable linker flags.
AC_DEFUN([QAHIRA_LDFLAGS],
[dnl
for flag in "$1"
do
	case " $LDFLAGS " in #(
	*[[\ \	]]$flag[[\ \	]]*) :
		;; #(
	*) :
		LDFLAGS="$LDFLAGS $flag" ;; #(
	esac
done
])dnl
