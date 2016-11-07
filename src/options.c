/*
 * Copyright 2006-2016 Christian Stigen Larsen
 * Distributed under the GNU General Public License (GPL) v2.
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

#ifdef HAVE_TERM_H
#include <term.h>
#endif

#include "jp2a.h"
#include "options.h"

// Default options
int verbose = 0;
int auto_height = 1;
int auto_width = 0;

int width =
#ifdef FEAT_TERMLIB
 0;
#else
 78;
#endif

int height = 0;
int use_border = 0;
int invert = 1;
int flipx = 0;
int flipy = 0;
int html = 0;
int colorfill = 0;
int convert_grayscale = 0;
int html_fontsize = 8;
int html_bold = 1;
const char* html_title = "jp2a converted image";
int html_rawoutput = 0;
int debug = 0;
int clearscr = 0;
int term_width = 0;
int term_height = 0;
int usecolors = 0;

int termfit =
#ifdef FEAT_TERMLIB
 TERM_FIT_AUTO;
#else
 0;
#endif

#define ASCII_PALETTE_SIZE 256
char ascii_palette[ASCII_PALETTE_SIZE + 1] = "   ...',;:clodxkO0KXNWM";

// Default weights, must add up to 1.0
float redweight = 0.2989f;
float greenweight = 0.5866f;
float blueweight = 0.1145f;

// calculated in parse_options
float RED[256], GREEN[256], BLUE[256], GRAY[256];

const char *fileout = "-"; // stdout

const char* version   = PACKAGE_STRING;
const char* copyright = "Copyright 2006-2016 Christian Stigen Larsen";
const char* license   = "Distributed under the GNU General Public License (GPL) v2.";
const char* url       = "https://github.com/cslarsen/jp2a";

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
"  -                 Read images from standard input.\n"
"      --blue=N.N    Set RGB to grayscale conversion weight, default is 0.1145\n"
"  -b, --border      Print a border around the output image.\n"
"      --chars=...   Select character palette used to paint the image.\n"
"                    Leftmost character corresponds to black pixel, right-\n"
"                    most to white.  Minimum two characters must be specified.\n"
"      --clear       Clears screen before drawing each output image.\n"
"      --colors      Use ANSI colors in output.\n"
"  -d, --debug       Print additional debug information.\n"
"      --fill        When used with --color and/or --html, color each character's\n"
"                    background color.\n"
"  -x, --flipx       Flip image in X direction.\n"
"  -y, --flipy       Flip image in Y direction.\n"
#ifdef FEAT_TERMLIB
"  -f, --term-fit    Use the largest image dimension that fits in your terminal\n"
"                    display with correct aspect ratio.\n"
"      --term-height Use terminal display height.\n"
"      --term-width  Use terminal display width.\n"
"  -z, --term-zoom   Use terminal display dimension for output.\n"
#endif
"      --grayscale   Convert image to grayscale when using --html or --colors\n"
"      --green=N.N   Set RGB to grayscale conversion weight, default is 0.5866\n"
"      --height=N    Set output height, calculate width from aspect ratio.\n"
"  -h, --help        Print program help.\n"
"      --html        Produce strict XHTML 1.0 output.\n"
"      --html-fill   Same as --fill (will be phased out)\n"
"      --html-fontsize=N   Set fontsize to N pt, default is 4.\n"
"      --html-no-bold      Do not use bold characters with HTML output\n"
"      --html-raw    Output raw HTML codes, i.e. without the <head> section etc.\n"
"      --html-title=...  Set HTML output title\n"
"  -i, --invert      Invert output image.  Use if your display has a dark\n"
"                    background.\n"
"      --background=dark   These are just mnemonics whether to use --invert\n"
"      --background=light  or not.  If your console has light characters on\n"
"                    a dark background, use --background=dark.\n"
"      --output=...  Write output to file.\n"
"      --red=N.N     Set RGB to grayscale conversion weight, default 0.2989f.\n"
"      --size=WxH    Set output width and height.\n"
"  -v, --verbose     Verbose output.\n"
"  -V, --version     Print program version.\n"
"      --width=N     Set output width, calculate height from ratio.\n"
"\n"
#ifdef FEAT_TERMLIB
"  The default mode is `jp2a --term-fit --background=dark'.\n"
#else
"  The default mode is `jp2a --width=78 --background=dark'.\n"
#endif
"  See the man-page for jp2a for more detailed help text.\n"
"\n", stderr);

	fprintf(stderr, "Project homepage on %s\n", url);
	fprintf(stderr, "Report bugs to <%s>\n", PACKAGE_BUGREPORT);
}

void precalc_rgb(const float red, const float green, const float blue) {
	int n;
	for ( n=0; n<256; ++n ) {
		RED[n]   = ((float) n) * red / 255.0f;
		GREEN[n] = ((float) n) * green / 255.0f;
		BLUE[n]  = ((float) n) * blue / 255.0f;
		GRAY[n]  = ((float) n) / 255.0f;
	}
}

void parse_options(int argc, char** argv) {
	// make code more readable
	#define IF_OPTS(sopt, lopt)     if ( !strcmp(s, sopt) || !strcmp(s, lopt) )
	#define IF_OPT(sopt)            if ( !strcmp(s, sopt) )
	#define IF_VARS(format, v1, v2) if ( sscanf(s, format, v1, v2) == 2 )
	#define IF_VAR(format, v1)      if ( sscanf(s, format, v1) == 1 )

	int n, files, fit_to_use;

	for ( n=1, files=0; n<argc; ++n ) {
		const char *s = argv[n];

		if ( *s != '-' ) { // count files to read
			++files; continue;
		}
	
		IF_OPT ("-")                        { ++files; continue; }
		IF_OPTS("-h", "--help")             { help(); exit(0); }
		IF_OPTS("-v", "--verbose")          { verbose = 1; continue; }
		IF_OPTS("-d", "--debug")            { debug = 1; continue; }
		IF_OPT ("--clear")                  { clearscr = 1; continue; }
		IF_OPTS("--color", "--colors")      { usecolors = 1; continue; }
		IF_OPT ("--fill")                   { colorfill = 1; continue; }
		IF_OPT ("--grayscale")              { usecolors = 1; convert_grayscale = 1; continue; }
		IF_OPT ("--html")                   { html = 1; continue; }
		IF_OPT ("--html-fill")              { colorfill = 1; fputs("warning: --html-fill has changed to --fill\n", stderr); continue; } // TODO: phase out
		IF_OPT ("--html-no-bold")           { html_bold = 0; continue; }	
		IF_OPT ("--html-raw")               { html = 1; html_rawoutput = 1; continue; }
		IF_OPTS("-b", "--border")           { use_border = 1; continue; }
		IF_OPTS("-i", "--invert")           { invert = !invert; continue; }
		IF_OPT("--background=dark")         { invert = 1; continue; }
		IF_OPT("--background=light")        { invert = 0; continue; }
		IF_OPTS("-x", "--flipx")            { flipx = 1; continue; }
		IF_OPTS("-y", "--flipy")            { flipy = 1; continue; }
		IF_OPTS("-V", "--version")          { print_version(); exit(0); }
		IF_VAR ("--width=%d", &width)       { auto_height += 1; continue; }
		IF_VAR ("--height=%d", &height)     { auto_width += 1; continue; }
		IF_VAR ("--red=%f", &redweight)     { continue; }
		IF_VAR ("--green=%f", &greenweight) { continue; }
		IF_VAR ("--blue=%f", &blueweight)   { continue; }
		IF_VAR ("--html-fontsize=%d",
			&html_fontsize)             { continue; }

		IF_VARS("--size=%dx%d",&width, &height) {
			auto_width = auto_height = 0; continue;
		}

#ifdef FEAT_TERMLIB
		IF_OPTS("-z", "--term-zoom")        { termfit = TERM_FIT_ZOOM; continue; }
		IF_OPT ("--term-height")            { termfit = TERM_FIT_HEIGHT; continue; }
		IF_OPT ("--term-width")             { termfit = TERM_FIT_WIDTH; continue; }
		IF_OPTS("-f", "--term-fit")         { termfit = TERM_FIT_AUTO; continue; }
#endif

		if ( !strncmp(s, "--output=", 9) ) {
			fileout = s+9;
			continue;
		}

		if ( !strncmp(s, "--html-title=", 13) ) {
			html_title = s + 13;
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

#ifdef FEAT_TERMLIB
	if ( (width || height) && termfit==TERM_FIT_AUTO ) {
		// disable default --term-fit if dimensions are given
		termfit = 0;
	}
#endif

	if ( termfit ) {
		char* err = "";

		if ( get_termsize(&term_width, &term_height, &err) <= 0 ) {
			fputs(err, stderr);
			fputc('\n', stderr);
			exit(1);
		}

#ifdef __CYGWIN__
	// On Cygwin, if I don't decrement term_width, then you'll get extra
	// blank lines for some window sizes, hence we decrease by one.
	--term_width;
#endif

		fit_to_use = termfit;

		if ( termfit == TERM_FIT_AUTO ) {
			// use the smallest of terminal width or height 
			// to guarantee that image fits in display.

			if ( term_width <= term_height )
				fit_to_use = TERM_FIT_WIDTH;
			else
				fit_to_use = TERM_FIT_HEIGHT;
		}

		switch ( fit_to_use ) {
		case TERM_FIT_ZOOM:
			auto_width = auto_height = 0;
			width = term_width - use_border*2;
			height = term_height - 1 - use_border*2;
			break;

		case TERM_FIT_WIDTH:
			width = term_width - use_border*2;
			height = 0;
			auto_height += 1;
			break;

		case TERM_FIT_HEIGHT:
			width = 0;
			height = term_height - 1 - use_border*2;
			auto_width += 1;
			break;
		}
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
		fputs("Invalid width or height specified\n", stderr);
		exit(1);
	}

	if ( (int)((redweight + greenweight + blueweight)*10000000.0f) != 10000000 ) {
		fputs("Weights RED + GREEN + BLUE must equal 1.0\n", stderr);
		exit(1);
	}

	if ( *fileout == 0 ) {
		fputs("Empty output filename.\n", stderr);
		exit(1);
	}

	precalc_rgb(redweight, greenweight, blueweight);
}
