# Process this file with autoconf to produce a configure script.
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_LANG(C++)
AC_CONFIG_SRCDIR([Test_optmap.cc])
AC_ARG_WITH(qt, AC_HELP_STRING([--with-qt], [use Qt gui (default no)]),
	[ac_use_qt=yes], [ac_use_qt=no])

# Checks for programs.
AC_PROG_CXX
# AC_PROG_INSTALL

# Checks for libraries.
if test "$ac_use_qt" = yes; then
	AC_PATH_QT
	AC_DEFINE([GUI])
	gui_vars='$(gui_objs)'
	gui_docsrc='$(gui_docsrc)'
	qt_lib='-lqt'
fi
AC_SUBST(gui_vars)
AC_SUBST(gui_docsrc)
AC_SUBST(qt_lib)

# Checks for header files.
AC_CHECK_HEADERS([sstream])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
