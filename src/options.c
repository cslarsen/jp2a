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

const char* version   = PACKAGE_STRING;
const char* copyright = "Copyright (C) 2006 Christian Stigen Larsen";
const char* license   = "Distributed under the GNU General Public License (GPL) v2 or later.";
const char* url       = "http://jp2a.sf.net";

#include <stdio.h>
#include <string.h>

// Default options
int verbose = 0;
int auto_height = 1;
int auto_width = 0;
int width = 78;
int height = 0;
int progress_barlength = 56;
int border = 0;
int invert = 0;
int flipx = 0;
int flipy = 0;
int html = 0;
int html_fontsize = 4;
int debug = 0;

#define ASCII_PALETTE_SIZE 256
char ascii_palette[ASCII_PALETTE_SIZE + 1] = "   ...',;:clodxkO0KXNWM";

void print_version() {
	fprintf(stderr, "%s\n%s\n%s\n", version, copyright, license);
}

void help() {
	print_version();

	fputs("\n"

#ifdef FEAT_CURL
	"Usage: jp2a [ options ] [ file(s) | URL(s) ]\n\n"

	"Convert files or URLs from JPEG format to ASCII.\n\n"
#else
	"Usage: jp2a [ options ] [ file(s) ]\n\n"

	"Convert files in JPEG format to ASCII.\n\n"
#endif
	"OPTIONS\n"
	"  -                Read JPEG image from standard input.\n"
	"  -b, --border     Print a border around the output image.\n"
	"      --chars=...  Select character palette used to paint the image.\n"
	"                   Leftmost character corresponds to black pixel, right-\n"
	"                   most to white.  Minimum two characters must be specified.\n"
	"  -d, --debug      Print additional debug information.\n"
	"      --flipx      Flip image in X direction.\n"
	"      --flipy      Flip image in Y direction.\n"
	"      --height=N   Set output height, calculate width from aspect ratio.\n"
	"  -h, --help       Print program help.\n"
	"      --html       Produce strict XHTML 1.0 output.\n"
	"      --html-fontsize=N  Set fontsize to N pt when using --html, default is 4.\n"
	"  -i, --invert     Invert output image.  Use if your display has a dark\n"
	"                   background.\n"
	"      --size=WxH   Set output width and height.\n"
	"  -v, --verbose    Verbose output.\n"
	"  -V, --version    Print program version.\n"
	"      --width=N    Set output width, calculate height from ratio.\n\n"

	"  The default running mode is `jp2a --width=78'.  See the man page for jp2a\n"
	"  to see detailed usage examples.\n\n" , stderr);

	fprintf(stderr, "Project homepage on %s\n", url);
	fprintf(stderr, "Report bugs to <%s>\n", PACKAGE_BUGREPORT);
}

// returns positive error code, or -1 for parsing OK
int parse_options(const int argc, char** argv) {

	// define some shorthand defines
	#define IF_OPTS(shortopt, longopt) if ( !strcmp(s, shortopt) || !strcmp(s, longopt) )
	#define IF_OPT(shortopt) if ( !strcmp(s, shortopt) )
	#define IF_VARS(format, var1, var2) if ( sscanf(s, format, var1, var2) == 2 )
	#define IF_VAR(format, var) if ( sscanf(s, format, var) == 1 )

	int n, files;
	for ( n=1, files=0; n<argc; ++n ) {
		const char *s = argv[n];

		if ( *s != '-' ) { // count files to read
			++files; continue;
		}
	
		IF_OPT("-")			{ ++files; continue; }
		IF_OPTS("-h", "--help")		{ help(); return 0; }
		IF_OPTS("-v", "--verbose")	{ verbose = 1; continue; }
		IF_OPTS("-d", "--debug")	{ debug = 1; continue; }
		IF_OPT("--html") 		{ html = 1; continue; }
		IF_OPTS("-b", "--border") 	{ border = 1; continue; }
		IF_OPTS("-i", "--invert") 	{ invert = 1; continue; }
		IF_OPT("--flipx") 		{ flipx = 1; continue; }
		IF_OPT("--flipy") 		{ flipy = 1; continue; }
		IF_OPTS("-V", "--version")	{ print_version(); return 0; }
		IF_VAR("--width=%d", &width)	{ auto_height += 1; continue; }
		IF_VAR("--height=%d", &height)	{ auto_width += 1; continue; }
		IF_VAR("--html-fontsize=%d", &html_fontsize) { continue; }
		IF_VARS("--size=%dx%d", &width, &height) { auto_width = auto_height = 0; continue; }

		if ( !strncmp(s, "--chars=", 8) ) {
			if ( strlen(s-8) > ASCII_PALETTE_SIZE ) {
				fprintf(stderr, "Too many ascii characters specified.\n");
				return 1;
			}
	
			// don't use sscanf, we need to read spaces as well
			strcpy(ascii_palette, s+8);
			continue;
		}

		fprintf(stderr, "Unknown option %s\n\n", s);
		help();
		return 1;

	} // args ...

	if ( !files ) {
		fprintf(stderr, "No files specified.\n\n");
		help();
		return 1;
	}

	// only --width specified, calc width
	if ( auto_width==1 && auto_height == 1 )
		auto_height = 0;

	// --width and --height is the same as using --size
	if ( auto_width==2 && auto_height==1 )
		auto_width = auto_height = 0;

	if ( strlen(ascii_palette) < 2 ) {
		fprintf(stderr, "You must specify at least two characters in --chars.\n");
		return 1;
	}
	
	if ( (width < 1 && !auto_width) || (height < 1 && !auto_height) ) {
		fprintf(stderr, "Invalid width or height specified.\n");
		return 1;
	}

	return -1;
}
