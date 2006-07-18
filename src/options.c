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

// Default options
int verbose = 0;
int auto_height = 1;
int auto_width = 0;
int width = 78;
int height = 0;
int border = 0;
int invert = 0;
int flipx = 0;
int flipy = 0;
int html = 0;
int html_fontsize = 4;
int debug = 0;
int clearscr = 0;

#define ASCII_PALETTE_SIZE 256
char ascii_palette[ASCII_PALETTE_SIZE + 1] = "   ...',;:clodxkO0KXNWM";

// Default weights, must add up to 1.0
float redweight = 0.299f;
float greenweight = 0.587f;
float blueweight = 0.114f;

// calculated in parse_options
float RED[256], GREEN[256], BLUE[256];

const char *fileout = "-"; // stdout

const char* version   = PACKAGE_STRING;
const char* copyright = "Copyright (C) 2006 Christian Stigen Larsen";
const char* license   = "Distributed under the GNU General Public License (GPL) v2 or later.";
const char* url       = "http://jp2a.sf.net";

void print_version() {
	fprintf(stderr, "%s\n%s\n%s\n", version, copyright, license);
}

void help() {
	print_version();

	fputs(
"\n"
#ifdef FEAT_CURL
"Usage: jp2a [ options ] [ file(s) | URL(s) ]\n\n"

"Convert files or URLs from JPEG format to ASCII.\n\n"
#else
"Usage: jp2a [ options ] [ file(s) ]\n\n"

"Convert files in JPEG format to ASCII.\n\n"
#endif
"OPTIONS\n"
"  -                Read JPEG image from standard input.\n"
"      --blue=N.N   Set RGB to grayscale conversion weight, default is 0.114\n"
"  -b, --border     Print a border around the output image.\n"
"      --chars=...  Select character palette used to paint the image.\n"
"                   Leftmost character corresponds to black pixel, right-\n"
"                   most to white.  Minimum two characters must be specified.\n"
"      --clear      Clears screen before drawing each output image.\n"
"  -d, --debug      Print additional debug information.\n"
"  -x, --flipx      Flip image in X direction.\n"
"  -y, --flipy      Flip image in Y direction.\n"
"      --green=N.N  Set RGB to grayscale conversion weight, default is 0.587\n"
"      --height=N   Set output height, calculate width from aspect ratio.\n"
"  -h, --help       Print program help.\n"
"      --html       Produce strict XHTML 1.0 output.\n"
"      --html-fontsize=N  Set fontsize to N pt, default is 4.\n"
"  -i, --invert     Invert output image.  Use if your display has a dark\n"
"                   background.\n"
"      --output=... Write output to file.\n"
"      --red=N.N    Set RGB to grayscale conversion weight, default 0.299f.\n"
"      --size=WxH   Set output width and height.\n"
"  -v, --verbose    Verbose output.\n"
"  -V, --version    Print program version.\n"
"      --width=N    Set output width, calculate height from ratio.\n\n"

"  The default running mode is `jp2a --width=78'.  See the man page for jp2a\n"
"  to see detailed usage examples.\n\n" , stderr);

	fprintf(stderr, "Project homepage on %s\n", url);
	fprintf(stderr, "Report bugs to <%s>\n", PACKAGE_BUGREPORT);
}

void precalc_rgb(const float red, const float green, const float blue) {
	int n;
	for ( n=0; n<256; ++n ) {
		RED[n]   = ((float) n) * red / 255.0f;
		GREEN[n] = ((float) n) * green / 255.0f;
		BLUE[n]  = ((float) n) * blue / 255.0f;
	}
}

void parse_options(int argc, char** argv) {
	// make code more readable
	#define IF_OPTS(sopt, lopt)     if ( !strcmp(s, sopt) || !strcmp(s, lopt) )
	#define IF_OPT(sopt)            if ( !strcmp(s, sopt) )
	#define IF_VARS(format, v1, v2) if ( sscanf(s, format, v1, v2) == 2 )
	#define IF_VAR(format, v1)      if ( sscanf(s, format, v1) == 1 )

	int n, files;
	for ( n=1, files=0; n<argc; ++n ) {
		const char *s = argv[n];

		if ( *s != '-' ) { // count files to read
			++files; continue;
		}
	
                IF_OPT("-")                        { ++files; continue; }
                IF_OPTS("-h", "--help")            { help(); exit(0); }
                IF_OPTS("-v", "--verbose")         { verbose = 1; continue; }
                IF_OPTS("-d", "--debug")           { debug = 1; continue; }
                IF_OPT("--clear")                  { clearscr = 1; continue; }
                IF_OPT("--html")                   { html = 1; continue; }
                IF_OPTS("-b", "--border")          { border = 1; continue; }
                IF_OPTS("-i", "--invert")          { invert = 1; continue; }
                IF_OPTS("-x", "--flipx")           { flipx = 1; continue; }
                IF_OPTS("-y", "--flipy")           { flipy = 1; continue; }
                IF_OPTS("-V", "--version")         { print_version(); exit(0); }
                IF_VAR("--width=%d", &width)       { auto_height += 1; continue; }
                IF_VAR("--height=%d", &height)     { auto_width += 1; continue; }
                IF_VAR("--red=%f", &redweight)     { continue; }
                IF_VAR("--green=%f", &greenweight) { continue; }
                IF_VAR("--blue=%f", &blueweight)   { continue; }
                IF_VAR("--html-fontsize=%d",
			&html_fontsize)            { continue; }
                IF_VARS("--size=%dx%d",&width, &height) {
			auto_width = auto_height = 0; continue;
		}
		
		if ( !strncmp(s, "--output=", 9) ) {
			fileout = s+9;
			continue;
		}

		if ( !strncmp(s, "--chars=", 8) ) {

			if ( strlen(s-8) > ASCII_PALETTE_SIZE ) {
				fprintf(stderr,
					"Too many ascii characters specified (max %d)\n",
					ASCII_PALETTE_SIZE);
				exit(1);
			}
	
			// don't use sscanf, we need to read spaces as well
			strcpy(ascii_palette, s+8);
			continue;
		}

		fprintf(stderr, "Unknown option %s\n\n", s);
		help();
		exit(1);

	} // args ...

	if ( !files ) {
		fputs("No files specified.\n\n", stderr);
		help();
		exit(1);
	}

	// only --width specified, calc width
	if ( auto_width==1 && auto_height == 1 )
		auto_height = 0;

	// --width and --height is the same as using --size
	if ( auto_width==2 && auto_height==1 )
		auto_width = auto_height = 0;

	if ( strlen(ascii_palette) < 2 ) {
		fputs("You must specify at least two characters in --chars.\n",
			stderr);
		exit(1);
	}
	
	if ( (width < 1 && !auto_width) || (height < 1 && !auto_height) ) {
		fputs("Invalid width or height specified.\n", stderr);
		exit(1);
	}

	if ( (redweight + greenweight + blueweight) != 1.0f ) {
		fputs("Red, green and blue must add up to 1.0\n", stderr);
		exit(1);
	}

	if ( *fileout == 0 ) {
		fputs("Empty output filename.\n", stderr);
		exit(1);
	}

	precalc_rgb(redweight, greenweight, blueweight);
}
