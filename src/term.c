/*
 * Copyright (C) 2006 Christian Stigen Larsen, http://csl.sublevel3.org
 * Distributed under the GNU General Public License (GPL) v2 or later.
 *
 * Project homepage on http://jp2a.sf.net
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_CURSES_H
#include <curses.h>
#endif

#ifdef FEAT_TERMLIB
#ifdef HAVE_TERM_H
#include <term.h>
#else
 #ifdef HAVE_NCURSES_TERM_H
 #include <ncurses/term.h>
 #endif
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#endif

/*
 * Returns:  1  success
 *           0  terminal type not defined
 *          -1  termcap database inaccessible
 *          -2  environment variable TERM not set
 */
int get_termsize(int* _width, int* _height, char** err) {
	static char errstr[1024];
	errstr[0] = 0;

	if ( err != NULL )
		*err = errstr;

#ifdef FEAT_TERMLIB

#ifdef WIN32
	CONSOLE_SCREEN_BUFFER_INFO i;
	if ( !GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &i) ) {
		strcpy(errstr, "Could not determine console window size.");
		return -1;
	}
	*_width = i.srWindow.Right - i.srWindow.Left;
	*_height = i.srWindow.Bottom - i.srWindow.Top;
	return 1;
#else

	char *termtype = getenv("TERM");
	char term_buffer[2048];

	if ( !termtype ) {
		strcpy(errstr, "Environment variable TERM not set.");
		return -2;
	}

	int i = tgetent(term_buffer, termtype);

	// There seems to be some confusion regarding the tgetent return
	// values.  The following two values should be swapped, according
	// to the man-pages, but on Mac OS X at least, they are like this.
	// I've also seen some indication of a bug in curses on USENET, so
	// I leave this one like this.

	if ( i == 0 ) {
		snprintf(errstr, sizeof(errstr)/sizeof(char) - 1,
			"Terminal type '%s' not recognized.", termtype);
		return 0;
	}

	if ( i < 0 ) {
		strcpy(errstr, "Could not access the termcap database.");
		return -1;
	}

	*_width = tgetnum("co");
	*_height = tgetnum("li");

	return 1;

#endif // non-WIN32

#else
	strcpy(errstr, "Compiled without termlib support.");
	return 0;

#endif // FEAT_TERMLIB
}
