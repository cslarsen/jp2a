#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([jp2a], [0.9.11], [csl@sublevel3.org])
AM_INIT_AUTOMAKE(dist-bzip2)
AC_CONFIG_SRCDIR([jp2a.c])
AC_CONFIG_HEADER([config.h])
AC_REVISION([$Id$])
AC_CONFIG_FILES([Makefile])

# Checks for programs.
AC_PROG_CC

CPPFLAGS="$CPPFLAGS -Wall -W"
LDFLAGS="$LDFLAGS -Wall -W"

# Checks for libraries.
AC_ARG_WITH([jpeg-include],
	[  --with-jpeg-include=PATH   to directory containing jpeglib.h (http://www.ijg.org/)],
	[CPPFLAGS="$CPPFLAGS -I$withval"])

AC_ARG_WITH([jpeg-lib],
	[  --with-jpeg-lib=PATH       to directory containing jpeglib lib file (http://www.ijg.org/)],
	[LDFLAGS="$LDFLAGS -L$withval"])

AC_ARG_WITH([jpeg-prefix],
	[  --with-jpeg-prefix=PATH    is same as --with-jpeg-include=PATH/include and --with-jpeg-lib=PATH/lib],
	[CPPFLAGS="$CPPFLAGS -I$withval/include" LDFLAGS="$LDFLAGS -L$withval/lib"])

AC_ARG_WITH([curl-prefix],
	[  --with-curl-prefix=PATH    look for curl.h in PATH/include and libcurl in PATH/lib],
	[CPPFLAGS="$CPPFLAGS -I$withval/include" LDFLAGS="$LDFLAGS -L$withval/lib"])

AC_ARG_ENABLE([curl],
	[  --enable-curl to be able to use libcurl (http://curl.haxx.se) to download images from the net, the file curl-config must be in PATH],
	[enable_curl=$enableval])

AC_CHECK_LIB([jpeg],
	[jpeg_start_decompress],
	[LIBS="$LIBS -ljpeg"],
	[
	  AC_MSG_ERROR([I need a working jpeglib; please specify --with-jpeg-lib=PATH or see --help for more explicit options])
	])

if test "$enable_curl" = "yes" ; then
	AC_PATH_PROGS([curl_config], [curl-config], [no])

	if test "$curl_config" != "no" ; then
		AC_CHECK_LIB([curl],
			[curl_easy_init],
			[LIBS="$LIBS `$curl_config --libs`"],
			[
			  AC_MSG_ERROR([I need a working libcurl; see --help])
			])

		CPPFLAGS="$CPPFLAGS `$curl_config --cflags`"
		AC_CHECK_HEADERS([curl/curl.h],,AC_MSG_ERROR([required header file missing (download source from http://curl.haxx.se/)]))
	fi

fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([jpeglib.h],,AC_MSG_ERROR([required header file missing (download source from http://www.ijg.org/)]))

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([memset])
AC_C_INLINE
AC_FUNC_MALLOC
AC_OUTPUT
