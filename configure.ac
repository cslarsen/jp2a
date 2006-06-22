#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([jp2a], [0.9.0], [csl@sublevel3.org])
AM_INIT_AUTOMAKE(dist-bzip2)
AC_CONFIG_SRCDIR([jp2a.c])
AC_CONFIG_HEADER([config.h])
AC_REVISION([$Id$])
AC_CONFIG_FILES([Makefile])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_ARG_WITH([jpeg-include],
	[  --with-jpeg-include=PATH to directory containing jpeglib.h],
	[CPPFLAGS="$CPPFLAGS -I$withval"])

AC_ARG_WITH([jpeg-lib],
	[  --with-jpeg-lib=PATH to directory containing jpeglib lib file],
	[LDFLAGS="$LDFLAGS -L$withval"])

AC_CHECK_LIB([jpeg],
	[jpeg_start_decompress],
	[LIBS="$LIBS -ljpeg"],
	[
	  AC_MSG_ERROR([You must have a working jpeglib.h environment, see configure --help])
	])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h jpeglib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])
AC_OUTPUT
