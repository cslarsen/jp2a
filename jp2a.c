/*
 * Copyright (C) 2006 Christian Stigen Larsen
 * Distributed under the GNU General Public License (GPL) v2 or later.
 *
 * jp2a converts JPEG to ASCII.
 *
 * Project homepage on http://jp2a.sf.net
 * Author's homepage on http://csl.sublevel3.org
 *
 * Compilation hint:
 *
 *   cc -g -O2 jp2a.c -I/sw/include -L/sw/lib -ljpeg
 *
 * $Id$
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

#ifdef HAVE_JPEGLIB_H
  #undef HAVE_STDLIB_H
#endif

#include <jpeglib.h>

#define ROUND(x) (int) ( 0.5 + x )

const char* version   = PACKAGE_STRING;
const char* copyright = "Copyright (C) 2006 Christian Stigen Larsen";
const char* license   = "Distributed under the GNU General Public License (GPL) v2 or later.";
const char* url       = "http://jp2a.sf.net";

typedef struct Image_ {
	int width;
	int height;
	float *p;
	int *yadds;
} Image;

// Options with defaults
int verbose = 0;
int auto_height = 1;
int auto_width = 0;
int width = 70;
int height = 0;
int progress_barlength = 40;
int border = 0;

char ascii_palette[257] = "";
const char* default_palette = "   ...',;:clodxkO0KXNWM";

void help() {
	fprintf(stderr, "%s\n", version);
	fprintf(stderr, "Usage: jp2a [ options ] [ file(s) ]\n\n");
	fprintf(stderr, "jp2a is  a simple JPEG to ASCII viewer.\n\n");

	fprintf(stderr, "OPTIONS\n");
	fprintf(stderr, "    -                Decompress from standard input\n");
	fprintf(stderr, "    --chars=...      Select character palette used to paint the image.  Leftmost character\n");
	fprintf(stderr, "                     corresponds to black pixel, rightmost to white pixel.  Minium two characters\n");
	fprintf(stderr, "                     must be specified.\n");
	fprintf(stderr, "    --border         Print a border around the output image\n");
	fprintf(stderr, "    -h, --help       Print program help\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    --height=H       Set output height calculate width by JPEG aspect ratio\n");
	fprintf(stderr, "    --width=W        Set output width, calculate height by JPEG aspect ratio, default is %d\n", width);
	fprintf(stderr, "    --size=WxH       Set exact output dimension, regardless of JPEG aspect ratio\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "    -v, --verbose    Verbose output\n");
	fprintf(stderr, "    -V, --version    Show program version\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "DEFAULT\n");
	fprintf(stderr,"     The default running mode is ``jp2a --width=70 [ filename ]'', so that aspect\n");
	fprintf(stderr, "    is correct for output mode.\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "EXAMPLES\n");
	fprintf(stderr, "    jp2a --size=80x25 --chars=' ...ooxx@@' picture.jpg\n");
	fprintf(stderr, "    jp2a picture.jpg --width=76\n");
	fprintf(stderr, "    jp2a picture.jpg --size=140x40\n");
	fprintf(stderr, "    cat picture.jpg | jp2a - --size=100x100\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "%s\n", copyright);
	fprintf(stderr, "%s\n", license);
	fprintf(stderr, "\n");
	fprintf(stderr, "Project homepage on %s\n", url);
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
			fprintf(stderr, "%s\n", version);
			fprintf(stderr, "%s\n", copyright);
			fprintf(stderr, "%s\n", license);
			fprintf(stderr, "News and updates on %s\n", url);
			exit(0);
		}

		if ( !strcmp(s, "--border") ) {
			border = 1;
			++hits;
		}

		if ( sscanf(s, "--size=%dx%d", &width, &height) == 2 ) {
			auto_width = auto_height = 0;
			++hits;
		}

		if ( !strncmp(s, "--chars=", 8) ) {
			// don't use sscanf, we need to read spaces as well
			strcpy(ascii_palette, s+8);
			++hits;
		}

		if ( sscanf(s, "--width=%d", &width) == 1 ) {
			auto_height = 1;
			++hits;
		}

		if ( sscanf(s, "--height=%d", &height) == 1 ) {
			auto_width = 1;
			++hits;
		}

		if ( !hits ) {
			fprintf(stderr, "Unknown option %s\n\n", s);
			help();
		}
	}

	// Palette must be at least two characters
	if ( ascii_palette[0] == 0 ) strcpy(ascii_palette, default_palette);
	if ( ascii_palette[1] == 0 ) strcpy(ascii_palette, default_palette);
	
	if ( width < 1 ) width = 1;
	if ( height < 1 ) height = 1;
}

void calc_aspect_ratio(int jpeg_width, int jpeg_height) {
	// Calculate width or height, but not both

	if ( auto_width && !auto_height ) {
		width = 2 * height * jpeg_width / jpeg_height;
	
		// adjust for too small dimensions	
		while ( width==0 ) {
			++height;
			calc_aspect_ratio(jpeg_width, jpeg_height);
		}
	}

	if ( !auto_width && auto_height ) {
		// divide by two, because most unix chars
		// in the terminal twice as tall as they are wide:
		height = width * jpeg_height / jpeg_width / 2;

		// adjust for too small dimensions
		while ( height==0 ) {
			++width;
			calc_aspect_ratio(jpeg_width, jpeg_height);
		}
	}
}

void print(Image* i, int chars) {
	int x, y;

	if ( border ) {
		printf("+");
		for ( x=0; x < i->width; ++x ) printf("-");
		printf("+\n");
	}

	char line[i->width + 1];
	line[i->width] = 0;

	for ( y=0; y < i->height; ++y ) {

		for ( x=0; x < i->width; ++x ) {
			int pos = ROUND( (float) chars * i->p[y*i->width + x] );
//			if ( pos < 0 ) pos = 0;
//			if ( pos > chars ) pos = chars; // we will occasionally get NaN from i->p[]
			line[x] = ascii_palette[chars - pos];
		}

		printf( border==0 ? "%s\n" : "|%s|\n" , line);
	}

	if ( border ) {
		printf("+");
		for ( x=0; x < i->width; ++x ) printf("-");
		printf("+\n");
	}
}

void clear(Image* i) {
	int n = 0;
	while ( n < (i->height * i->width) ) {
		i->p[n] = 0.0f;
		i->yadds[n] = 0;
		++n;
	}
}

// This exaggerates the differences in the picture, good for ASCII output
void normalize(Image* i) {
	int x, y;

	for ( y=0; y < i->height; ++y )
		for ( x=0; x < i->width; ++x ) {
			if ( i->yadds[y] > 0 )
				i->p[y*i->width + x] /= (float) (i->yadds[y] );
		}
}

void print_progress(float progress_0_to_1) {
	char prog[progress_barlength + 2];

	int m;
	for ( m=0; m <= progress_barlength; ++m )
		prog[m] = '.';


	int tenth = ROUND( (float) progress_barlength * progress_0_to_1 );

	while ( tenth > 0 ) prog[--tenth] = '#';

	prog[progress_barlength] = 0;

	fprintf(stderr, "Reading file [%s]\r", prog);
}

void calc_intensity(JSAMPLE* source, float* dest, int components) {
	int c = 0;

	while ( c < components ) {
		*dest += (float) *(source + c++)
			/ (255.0f * (float) components);
	}
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

	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	calc_aspect_ratio(cinfo.output_width, cinfo.output_height);

	Image image;
	image.width = width;
	image.height = height;

	if ( (image.p = malloc(width * height * sizeof(float))) == NULL ) {
		fprintf(stderr, "Not enough memory for given output dimension (image.p)\n");
		exit(1);
	}

	if ( (image.yadds = malloc(width * sizeof(int))) == NULL ) {
		fprintf(stderr, "Not enough memory for given output dimension (iimage.yadds)\n");
		exit(1);
	}

	clear(&image);

	int num_chars = strlen(ascii_palette) - 1;
	int components = cinfo.out_color_components;

	float to_dst_y = (float) (height-1) / (float) (cinfo.output_height-1);
	float to_dst_x = (float) cinfo.output_width / (float) width;
	
	if ( verbose ) {
		fprintf(stderr, "Source width: %d\n", cinfo.output_width);
		fprintf(stderr, "Source height: %d\n", cinfo.output_height);
		fprintf(stderr, "Source color components: %d\n", components);
		fprintf(stderr, "Output width: %d\n", width);
		fprintf(stderr, "Output height: %d\n", height);
		fprintf(stderr, "Output palette (%d chars): '%s'\n", 1 + num_chars, ascii_palette);
	}

	unsigned int last_dsty = 0;

	while ( cinfo.output_scanline < cinfo.output_height ) {
		jpeg_read_scanlines(&cinfo, buffer, 1);

		unsigned int dst_y = to_dst_y * (float) (cinfo.output_scanline-1);
		int dst_x;

		for ( dst_x=0; dst_x < image.width; ++dst_x ) {
			unsigned int src_x = (float) dst_x * to_dst_x;
			calc_intensity(&buffer[0][src_x*components], &image.p[dst_y*width + dst_x], components);
		}

		++image.yadds[dst_y];

		// fill up any scanlines we missed since last iteration
		while ( dst_y - last_dsty > 1 ) {
			++last_dsty;
			for ( dst_x=0; dst_x < image.width; ++dst_x ) {
				unsigned int src_x = (float) dst_x * to_dst_x;
				calc_intensity(&buffer[0][src_x*components], &image.p[(last_dsty)*width + dst_x], components);
			}
			++image.yadds[last_dsty];
		}

		last_dsty = dst_y;

		if ( verbose )
			print_progress( (float) (cinfo.output_scanline + 1.0f) / (float) cinfo.output_height );
	}

	if ( verbose )
		fprintf(stderr, "\n");

	normalize(&image);
	print(&image, num_chars);
	free(image.p);

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	return 0;
}

int main(int argc, char** argv) {
	parse_options(argc, argv);

	int n;
	for ( n=1; n<argc; ++n ) {

		// Skip options
		if ( argv[n][0]=='-' && argv[n][1]!=0 )
			continue;

		// Read from stdin
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
