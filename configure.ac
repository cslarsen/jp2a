#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([jp2a], [0.9.13], [csl@sublevel3.org])
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
	[  --with-jpeg-prefix=PATH    is the same as --with-jpeg-include=PATH/include and --with-jpeg-lib=PATH/lib],
	[CPPFLAGS="$CPPFLAGS -I$withval/include" LDFLAGS="$LDFLAGS -L$withval/lib"])

AC_ARG_WITH([curl-config],
	[  --with-curl-config=PATH    path to curl-config],
	[curl_config_path="$withval" enable_curl="yes"],
	[curl_config_path="$PATH"])

AC_CHECK_LIB([jpeg],
	[jpeg_start_decompress],
	[LIBS="$LIBS -ljpeg"],
	[
	  AC_MSG_ERROR([I need a working jpeglib; please specify --with-jpeg-lib=PATH or see --help for more explicit options])
	])

AC_ARG_ENABLE([curl],
	[  --enable-curl to be able to use libcurl (http://curl.haxx.se) to download images from the net (default: yes)],
	[enable_curl=$enableval],
	[enable_curl="yes"])

AC_MSG_CHECKING([enable-curl])
AC_MSG_RESULT([$enable_curl])

if test "x$enable_curl" = "xyes" ; then
	AC_PATH_PROGS([curl_config], [curl-config], [no], [$curl_config_path])

	if test "x$curl_config" != "xno" ; then
		curl_cflags="`$curl_config --cflags`"
		curl_libs="`$curl_config --libs`"
		CPPFLAGS="$CPPFLAGS $curl_cflags"

		AC_MSG_CHECKING([curl-config --cflags])
		AC_MSG_RESULT([$curl_cflags])

		AC_MSG_CHECKING([curl-config --libs])
		AC_MSG_RESULT([$curl_libs])
	fi

	if test "x$curl_libs" = "x" ; then
		curl_libs="-lcurl"
	fi

	AC_CHECK_LIB([curl], [curl_easy_init], [LIBS="$LIBS $curl_libs" AC_DEFINE([FEAT_CURL], [1], [Found working libcurl environment])], [
		  AC_MSG_WARN([I need a working libcurl (use --with-curl-config=PATH); download-support will be disabled])
		  enable_curl="no"
		])
fi

AC_ARG_ENABLE([convert],
	[  --enable-convert to use ImageMagick's convert utility for reading non-JPEG images (default: yes)],
	[enable_convert=$enableval],
	[enable_convert="yes"])

AC_MSG_CHECKING([enable-convert])
AC_MSG_RESULT([$enable_convert])

if test "$enable_convert" = "yes" ; then
	AC_PATH_PROG([prog_convert], [convert], [no])
	AC_MSG_WARN([progconvert: $prog_convert])

	if test "x$prog_convert" != "xno" ; then
		AC_DEFINE([FEAT_CONVERT], [1], [Define to 1 if ImageMagick's convert program is to be used])
#		AC_DEFINE([PATH_CONVERT], [$prog_convert], [Absolute path to convert during configuration])
	fi
fi

# Checks for header files.
AC_HEADER_STDC

AC_CHECK_HEADER([jpeglib.h],
	[ AC_DEFINE([HAVE_JPEGLIB_H], [1], [Define to 1 if you have <jpeglib.h> header file.]) ],
	[ AC_MSG_ERROR([required header file jpeglib.h not found, use --with-jpeg-include=PATH or see --help]) ])

if test "x$enable_curl" = "xyes" ; then
	AC_CHECK_HEADER([curl/curl.h],
		[ AC_DEFINE([HAVE_CURL_CURL_H], [1], [Define to 1 if you have the <curl/curl.h> header file]) ],
		[ AC_MSG_WARN([required header file curl/curl.h not found, libcurl will be disabled (see --help)]) ])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([atexit])
AC_FUNC_FORK
AC_TYPE_PID_T

AC_C_INLINE
AC_FUNC_MALLOC
AC_OUTPUT
