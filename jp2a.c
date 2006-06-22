/*
 * Copyright (C) 2006 Christian Stigen Larsen <http://csl.sublevel3.org>
 * Distributed under the GNU General Public License (GPL) v2 or later.
 *
 * Will print JPEG-files using ASCII characters.
 *
 * Compilation example:
 * cc -g -O2 jp2a.c -I/sw/include -L/sw/lib -ljpeg
 *
 * $Id$
 *
 */

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#ifdef HAVE_STDLIB_H
  #include <stdlib.h>
#endif

#include <stdio.h>

#ifdef HAVE_STRING_H
  #include <string.h>
#endif

#ifdef HAVE_STRINGS_H
  #include <strings.h>
#endif

#ifdef HAVE_JPEGLIB_H
  #undef HAVE_STDLIB_H
  #include <jpeglib.h>
#endif

const char* version   = PACKAGE_STRING;
const char* copyright = "Copyright (C) 2006 Christian Stigen Larsen";
const char* license   = "Distributed under the GNU General Public License (GPL) v2 or later.";

const char* default_palette = "   ...',;:clodxkO0KXNWM";

typedef struct Image_ {
	int width;
	int height;
	float *p;
} Image;

// Options
int verbose = 0;
int color = 0; // unsupported
int width = 80;
int height = 25;

char ascii_palette[257] = "";

void help() {
	fprintf(stderr, "%s\n", version);
	fprintf(stderr, "Usage: jp2a file.jpg [file2.jpg [...]]\n\n");
	fprintf(stderr, "jp2a is  a simple JPEG-to-ASCII converter.\n\n");

	fprintf(stderr, "OPTIONS\n");
	fprintf(stderr, "    -                Decompress from standard input\n");
	fprintf(stderr, "    -h, --help       Print program help\n");
	fprintf(stderr, "    --size=WxH       Set output ASCII width and height, default is %dx%d\n", width, height);
	fprintf(stderr, "    --chars=...      Select character palette used to paint the image.  Leftmost character\n");
	fprintf(stderr, "                     corresponds to black pixel, rightmost to white pixel.\n");
	fprintf(stderr, "    -v, --verbose    Verbose output\n");
	fprintf(stderr, "    -V, --version    Show program version\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "EXAMPLES\n");
	fprintf(stderr, "    jp2a --size=80x25 --chars='...oooxx@@' somefile.jpg\n");
	fprintf(stderr, "    cat picture.jpg | jp2a - --size=100x100\n\n");

	fprintf(stderr, "%s\n", copyright);
	fprintf(stderr, "%s\n", license);
	fprintf(stderr, "Report bugs to <%s>\n", PACKAGE_BUGREPORT);
	exit(1);
}

void parse_options(int argc, char** argv) {
	if ( argc<2 )
		help();

	int n;

	for ( n=1; n<argc; ++n ) {
		char *s = argv[n];

		if ( *s != '-' ) continue;

		int hits = 0;

		if ( !strcmp(s, "-h") || !strcmp(s, "--help") )
			help();

		if ( !strcmp(s, "-v") || !strcmp(s, "--verbose") ) {
			verbose = 1;
			++hits;
		}

		if ( !strcmp(s, "-V") || !strcmp(s, "--version") ) {
			fprintf(stderr, "%s\n\n", version);
			fprintf(stderr, "%s\n", copyright);
			fprintf(stderr, "%s\n", license);
			exit(0);
		}

		hits += sscanf(s, "--size=%dx%d", &width, &height);
		hits += sscanf(s, "--chars=%256s", ascii_palette);

		if ( !hits ) {
			fprintf(stderr, "Unknown option %s\n\n", s);
			help();
		}
	}

	// Palette must be at least two characters
	if ( ascii_palette[0] == 0 ) strcpy(ascii_palette, default_palette);
	if ( ascii_palette[1] == 0 ) strcpy(ascii_palette, default_palette);

	if ( width <= 1 ) width = 1;
	if ( height <= 1 ) height = 1;
}

void print(Image* i, int chars) {
	#define ROUND(x) (int) ( 0.5 + x )
	char buf[width+1];
	int n;

	for ( n=0; n < i->width; ++n ) {
		int pos = ROUND( chars * i->p[n] );

		// The following should never happen
		if ( pos < 0 ) pos = 0;
		if ( pos > chars ) pos = chars;

		buf[n] = ascii_palette[pos];
	}

	buf[width] = 0;
	puts(buf);
}

void invert(Image* i) {
	int n;
	for ( n=0; n < i->width; ++n )
		i->p[n] = 1.0 - i->p[n];
}

void clear(Image* i) {
#ifdef HAVE_MEMSET
	memset(i->p, 0, i->width * sizeof(float) );
#else
	int n;
	for ( n=0; n < i->width; ++n )
		i->p[n] = 0.0;
#endif
}

void normalize(Image* i) {
	int n;
	float min = 10000000.0;
	float max = -1000000.0;

	// find min and max values
	for ( n=0; n < i->width; ++n ) {
		float v = i->p[n];
		if ( v < min ) min = v;
		if ( v > max ) max = v;
	}

	// normalize to [0..1] range
	float range = max - min;
	for ( n=0; n < i->width; ++n )
		i->p[n] = (i->p[n] - min) / range;
}

int decompress(FILE *fp) {
        struct jpeg_error_mgr jerr;
	struct jpeg_decompress_struct cinfo;

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);
        jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	int row_stride = cinfo.output_width * cinfo.output_components;
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	Image image;
	image.width = width;
	image.height = height;

	if ( (image.p = malloc( width * sizeof(float) )) == NULL ) {
		fprintf(stderr, "Could not allocate %ld bytes for output image", width * sizeof(float) );
		exit(1);
	}

	clear(&image);

	int chars = strlen(ascii_palette) - 1;
	int components = cinfo.out_color_components;

	int pixelsPerChar = row_stride / (components * width);
	if ( pixelsPerChar <= 0 ) pixelsPerChar = 1;

	int linesToAdd = cinfo.output_height / height;
	if ( linesToAdd <= 0 ) linesToAdd = 1;
	
	int linesAdded = 0;

	if ( verbose ) {
		fprintf(stderr, "Output width : %d\n", width);
		fprintf(stderr, "Output height: %d\n", height);
		fprintf(stderr, "Source width : %d\n", cinfo.output_width);
		fprintf(stderr, "Source height: %d\n", cinfo.output_height);
		fprintf(stderr, "ASCII characters used for printing: %d\n", 1 + chars);
		fprintf(stderr, "Pixels per character: %d\n", pixelsPerChar);
		fprintf(stderr, "Lines per character : %d\n", linesToAdd);
		fprintf(stderr, "Color components    : %d\n", components);
	}

	while ( cinfo.output_scanline < cinfo.output_height ) {

		jpeg_read_scanlines(&cinfo, buffer, 1);

		int x = 0;
		int pixelsAdded = 0;
		int pixel;

		for ( pixel=0; pixel < (row_stride - components); ++pixel ) {

			// calculate intensity
			image.p[x] += buffer[0][pixel] / 255.0;

			if ( pixel % components == 0 )
			if ( ++pixelsAdded >= pixelsPerChar ) {
				if ( x >= width ) break;
				pixelsAdded = 0;
				++x;
			}
		}

		if ( ++linesAdded > linesToAdd                          /* time to print */
		|| (cinfo.output_scanline + 1 == cinfo.output_height) ) /* last scanline */
		{
			normalize(&image);
			invert(&image);
			print(&image, chars);
			clear(&image);
			linesAdded = 0;
		}
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	free(image.p);
	return 0;
}

int main(int argc, char** argv) {
	parse_options(argc, argv);

	int n;
	for ( n=1; n<argc; ++n ) {

		// Skip options
		if ( argv[n][0]=='-' && argv[n][1]!=0 )
			continue;

		if ( argv[n][0]=='-' && argv[n][1]==0 ) {
			decompress(stdin);
			continue;
		}

		FILE *fp;
		if ( (fp = fopen(argv[n], "rb")) != NULL ) {
			if ( verbose ) fprintf(stderr, "File: %s\n", argv[n]);
			decompress(fp);
			fclose(fp);
		} else {
			fprintf(stderr, "Can't open %s\n", argv[n]);
			return 1;
		}
	}

	return 0;
}
