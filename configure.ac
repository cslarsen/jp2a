#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)
AC_INIT([jp2a], [1.0.8], [csl@csl.name])
AM_INIT_AUTOMAKE(dist-bzip2)
AC_CONFIG_SRCDIR(src/jp2a.c)
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_ARG_WITH([jpeg-include],
	AC_HELP_STRING([--with-jpeg-include=PATH], [specify path to directory containing jpeglib.h -- http://www.ijg.org/]),
	[CPPFLAGS="$CPPFLAGS -I$withval"])

AC_ARG_WITH([jpeg-lib],
	AC_HELP_STRING([--with-jpeg-lib=PATH], [specify path to directory containing jpeglib lib file -- http://www.ijg.org/]),
	[LDFLAGS="$LDFLAGS -L$withval"])

AC_ARG_WITH([jpeg-prefix],
	AC_HELP_STRING([--with-jpeg-prefix=PATH], [this is the same as --with-jpeg-include=PATH/include and --with-jpeg-lib=PATH/lib]),
	[CPPFLAGS="$CPPFLAGS -I$withval/include" LDFLAGS="$LDFLAGS -L$withval/lib"])

AC_ARG_WITH([curl-config],
	AC_HELP_STRING([--with-curl-config=PATH], [specify path to curl-config]),
	[curl_config_path="$withval" enable_curl="yes"],
	[curl_config_path="$PATH"])

AC_CHECK_LIB([jpeg], [jpeg_start_decompress], [LIBS="$LIBS -ljpeg"])

if test "x$ac_cv_lib_jpeg_jpeg_start_decompress" != "xyes" ; then
	AC_MSG_ERROR([I need a working jpeglib; please specify --with-jpeg-lib=PATH or see --help for more explicit options.

On many systems, this is installed in /usr/local/lib/ so you can try --with-jpeg-prefix=/usr/local/
])
fi

AC_ARG_ENABLE([curl],
	AC_HELP_STRING([--enable-curl], [enable to be to use libcurl (http://curl.haxx.se) to download images from the net (default: yes)]),
	[enable_curl=$enableval],
	[enable_curl="yes"])

AC_ARG_ENABLE([termlib],
	AC_HELP_STRING([--enable-termlib], [enable to use term/curses for detecting terminal dimensions (default: yes)]),
	[enable_termlib=$enableval],
	[enable_termlib="yes"])

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

AC_CHECK_HEADERS([fcntl.h curses.h term.h ncurses/term.h])

if test "$enable_termlib" = "yes" ; then

	# Some GNU/Linux systems (e.g., SuSE 4.3, 1996) don't have curses, but
	# rather ncurses.  So we check for it.
	TERMLIBS=
	# Check for termlib before termcap because Solaris termcap needs libucb.
	TERMLIB_VARIANTS="ncurses curses termlib termcap terminfo"
	for termlib in ${TERMLIB_VARIANTS}; do
	  AC_CHECK_LIB(${termlib}, tgetent,
	     [TERMLIBS="${TERMLIBS} -l${termlib}"; break])
	done
	# don't bother warning on djgpp, it doesn't have a term library, it
	# ports each termcap-needing program separately according to its needs.
	if test -z "$TERMLIBS" && echo "$build" | grep -v djgpp >/dev/null; then
	  AC_MSG_WARN([probably need a terminal library, one of: ${TERMLIB_VARIANTS}])
	else
	  LIBS="$LIBS $TERMLIBS"
	  AC_DEFINE([FEAT_TERMLIB], [1], [Found working term/curses environment])
	fi

fi


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([atexit])
AC_FUNC_FORK
AC_TYPE_PID_T

AC_HEADER_ASSERT

AC_FUNC_MALLOC
AC_OUTPUT
