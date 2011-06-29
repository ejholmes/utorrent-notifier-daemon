#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([utorrent-notifier], [0.1], [eric@ejholmes.net])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])
AC_PREFIX_DEFAULT([])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([curl], [curl_easy_init])
AC_CHECK_LIB([ssl], [MD5])

PKG_CHECK_MODULES([LIBCONFIG], [libconfig])
PKG_CHECK_MODULES([LIBJSON], [json])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h syslog.h unistd.h libconfg.h curl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strchr])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
