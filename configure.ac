# Process this file with autoconf to produce a configure script.
# $Id: configure.ac,v 1.2 2003/05/13 12:42:51 ahsu Rel $

AC_INIT(mutt_vc_query, 002, ahsu@users.sf.net)
AM_INIT_AUTOMAKE
AC_PREREQ(2.57)
AC_COPYRIGHT([Copyright (C) 2003  Andrew Hsu])
AC_REVISION($Revision: 1.2 $)
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB(vc, vc_new)

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strdup])

AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile])
AC_OUTPUT
