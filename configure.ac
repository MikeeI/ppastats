#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])
AC_INIT([ppastats], [1.3.3],[jeanfi@gmail.com],[ppastats],[http://wpitchoune.net/ppastats])

AM_INIT_AUTOMAKE([-Wall gnu])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks lib build
AC_PROG_RANLIB

# Intl support
AM_GNU_GETTEXT_VERSION([0.18.1])
AM_GNU_GETTEXT([external])

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([getopt.h stdio.h stdlib.h string.h sys/stat.h sys/time.h sys/types.h time.h])

# Checks CURL 
PKG_CHECK_MODULES(CURL, libcurl )
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)

# Checks JSON
JSON_LIBS=
PKG_CHECK_MODULES(JSON, 
		  json-c >= 0.11.99,
		  [AC_DEFINE([HAVE_JSON],[1],[Use JSON])], 
		  [AC_MSG_WARN("json-c missing")])

if test "$JSON_LIBS" == ""; then
PKG_CHECK_MODULES(JSON, 
		  json >= 0.11.99, 
		  [AC_DEFINE([HAVE_JSON],[1],[Use JSON])
		   AC_DEFINE([HAVE_JSON_0],[1],[Use JSON0])], 
		  [AC_MSG_WARN("json0 missing")])
fi

AM_CONDITIONAL(JSON, test -n "$JSON_LIBS")
AC_SUBST(JSON_CFLAGS)
AC_SUBST(JSON_LIBS)

AC_CHECK_TYPE(json_bool, [AC_DEFINE([HAVE_JSON_BOOL], 1, [Use json_bool])], [], [[#include <json/json.h>]])

AC_CONFIG_FILES([ po/Makefile.in
 Makefile
 src/Makefile
 tests/Makefile
 themes/Makefile
 themes/default/css/Makefile
 themes/default/js/Makefile
 themes/default/templates/Makefile
 themes/default/templates/footer.tpl
])

AC_CHECK_PROGS([HELP2MAN], [help2man])

AC_CHECK_PROG(HAVE_CPPCHECK, cppcheck, yes)
AM_CONDITIONAL(CPPCHECK, test -n "$HAVE_CPPCHECK")

AC_OUTPUT
