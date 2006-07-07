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

  #ifdef _WIN32
    // i386-mingw32 doesn't have GNU compatible malloc, as reported
    // by `configure', but it nevertheless works if we just ignore that.
    #undef malloc
  #endif
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

#include "jpeglib.h"

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
int width = 78;
int height = 0;
int progress_barlength = 56;
int border = 0;
int invert = 0;
int flipx = 0;
int flipy = 0;
int html = 0;
int html_fontsize = 4;

char ascii_palette[257] = "";
const char* default_palette = "   ...',;:clodxkO0KXNWM";

void help() {
	fprintf(stderr, "%s\n", version);
	fprintf(stderr, "%s\n", copyright);
	fprintf(stderr, "%s\n", license);

fputs(
"\n"
"Usage: jp2a [ options ] [ file(s) ]\n\n"

"Convert file(s) in JPEG format to ASCII.\n\n"

"OPTIONS\n"
"  -                Read JPEG image from standard input.\n"
"  -b, --border     Print a border around the output image.\n"
"      --chars=...  Select character palette used to paint the image.\n"
"                   Leftmost character corresponds to black pixel, rightmost\n"
"                   to white.  Minimum two characters must be specified.\n"
"      --flipx      Flip image in X direction.\n"
"      --flipy      Flip image in Y direction.\n"
"  -i, --invert     Invert output image.\n"
"  -h, --help       Print program help.\n"
"      --height=H   Set output height, calculate width from aspect ratio.\n"
"      --html       Produce strict XHTML 1.0 output.\n"
"      --html-fontsize=N\n"
"                   With --html, sets fontsize to N pt.  Default is 4.\n"
"      --size=WxH   Set exact output dimension.\n"
"  -v, --verbose    Verbose output.\n"
"      --width=W    Set output width, calculate height from ratio.\n"
"  -V, --version    Print program version.\n\n"

"  The default running mode is `jp2a --width=78'.  See the man page for jp2a\n"
"  to see detailed usage examples.\n\n"
, stderr);

	fprintf(stderr, "Project homepage on %s\n", url);
	fprintf(stderr, "Report bugs to <%s>\n", PACKAGE_BUGREPORT);
}

int parse_options(const int argc, char** argv) {
	if ( argc<2 ) {
		help();
		return 1;
	}

	int n;

	int filehits = 0;

	for ( n=1; n<argc; ++n ) {
		const char *s = argv[n];

		if ( *s != '-' ) { ++filehits; continue; }
		if ( *s=='-' && *(s+1)==0 ) { ++filehits; continue; } // stdin

		int hits = 0;

		if ( !strcmp(s, "-h") || !strcmp(s, "--help") ) {
			help();
			return 0;
		}

		if ( !strcmp(s, "-v") || !strcmp(s, "--verbose") ) {
			verbose = 1;
			++hits;
		}

		if ( !strcmp(s, "-V") || !strcmp(s, "--version") ) {
			fprintf(stderr, "%s\n", version);
			fprintf(stderr, "%s\n", copyright);
			fprintf(stderr, "%s\n", license);
			fprintf(stderr, "News and updates on %s\n", url);
			return 0;
		}

		if ( !strcmp(s, "--html") ) {
			html = 1;
			++hits;
		}

		if ( !strcmp(s, "--border") || !strcmp(s, "-b") ) {
			border = 1;
			++hits;
		}

		if ( !strcmp(s, "--invert") || !strcmp(s, "-i") ) {
			invert = 1;
			++hits;
		}

		if ( !strcmp(s, "--flipx") ) {
			flipx = 1;
			++hits;
		}

		if ( !strcmp(s, "--flipy") ) {
			flipy = 1;
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

		if ( sscanf(s, "--html-fontsize=%d", &html_fontsize) == 1 )
			++hits;

		if ( !hits ) {
			fprintf(stderr, "Unknown option %s\n\n", s);
			help();
			return 1;
		}
	}

	if ( !filehits ) {
		fprintf(stderr, "No files specified\n");
		return 1;
	}

	// Palette must be at least two characters
	if ( ascii_palette[0] == 0 ) strcpy(ascii_palette, default_palette);
	if ( ascii_palette[1] == 0 ) strcpy(ascii_palette, default_palette);
	
	if ( width < 1 ) width = 1;
	if ( height < 1 ) height = 1;

	return -1;
}

void calc_aspect_ratio(const int jpeg_width, const int jpeg_height) {

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

void print(const Image* i, const int chars) {
	int x, y;
	const int w = i->width;
	const int h = i->height;

	// Make "+--------+" string
	char bord[w + 3];
	bord[0] = bord[w+1] = '+';
	bord[w+2] = 0;
	memset(bord+1, '-', w);

	char line[w + 1];
	line[w] = 0;

	if ( html ) {
		printf(
		"<?xml version='1.0' encoding='ISO-8859-1'?>\n"
		"<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN'"
		"  'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>\n"
		"<html xmlns='http://www.w3.org/1999/xhtml' lang='en' xml:lang='en'>\n"
		"<head>\n"
		"<title>jp2a converted image</title>\n"
		"<style type='text/css'>\n"
		".ascii {\n"
		"   font-size:%dpt;\n"
		"}\n"
		"</style>\n"
		"</head>\n"
		"<body>\n"
		"<div class='ascii'>\n"
		"<pre>\n"
		,
		html_fontsize);
	}

	if ( border ) puts(bord);

	for ( y=0; y < h; ++y ) {
		for ( x=0; x < w; ++x ) {
			int pos = ROUND( (float) chars * i->p[(!flipy? y : h-y-1)*w + x] );
			line[!flipx? x : w-x-1] = ascii_palette[ !invert ? chars - pos : pos ];
		}

		printf(!border? "%s\n" : "|%s|\n" , line);
	}

	if ( border ) puts(bord);

	if ( html ) {
		printf("</pre>\n");
		printf("</div>\n");
		printf("</body>\n");
		printf("</html>\n");
	}
}

void clear(Image* i) {
	memset(i->p, 0, i->width * i->height * sizeof(float));
	memset(i->yadds, 0, i->height);
}

// This exaggerates the differences in the picture, good for ASCII output
void normalize(Image* i) {
	const int w = i->width;
	const int h = i->height;

	register int x, y, yoffs;

	for ( y=0, yoffs=0; y < h; ++y, yoffs += w ) {
		for ( x=0; x < w; ++x ) {
			if ( i->yadds[y] != 0 )
				i->p[yoffs + x] /= (float) i->yadds[y];
		}
	}
}

void print_progress(const float progress_0_to_1) {

	int tenth = ROUND( (float) progress_barlength * progress_0_to_1 );

	char s[progress_barlength + 1];
	s[progress_barlength] = 0;

	memset(s, '.', progress_barlength);
	memset(s, '#', tenth);

	fprintf(stderr, "Decompressing image [%s]\r", s);
}

inline void calc_intensity(const JSAMPLE* source, float* dest, const int components) {
	int c;

	for ( c=0; c < components; ++c ) {
		*dest += (float) *(source + c)
			/ (255.0f * (float) components);
	}
}

void print_info(const struct jpeg_decompress_struct* cinfo) {
	fprintf(stderr, "Source width: %d\n", cinfo->output_width);
	fprintf(stderr, "Source height: %d\n", cinfo->output_height);
	fprintf(stderr, "Source color components: %d\n", cinfo->output_components);
	fprintf(stderr, "Output width: %d\n", width);
	fprintf(stderr, "Output height: %d\n", height);
	fprintf(stderr, "Output palette (%d chars): '%s'\n", (int) strlen(ascii_palette), ascii_palette);
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

	image.p = NULL;
	image.yadds = NULL;

	if ( (image.p = malloc(width * height * sizeof(float))) == NULL ) {
		fprintf(stderr, "Not enough memory for given output dimension\n");
		return 1;
	}

	if ( (image.yadds = malloc(height * sizeof(int))) == NULL ) {
		fprintf(stderr, "Not enough memory for given output dimension (for yadds)\n");
		return 1;
	}

	clear(&image);

	size_t num_chars = strlen(ascii_palette) - 1;
	int components = cinfo.out_color_components;

	float to_dst_y = (float) (height-1) / (float) (cinfo.output_height-1);
	float to_dst_x = (float) cinfo.output_width / (float) width;
	
	if ( verbose )
		print_info(&cinfo);

	unsigned int last_dsty = 0;

	while ( cinfo.output_scanline < cinfo.output_height ) {
		jpeg_read_scanlines(&cinfo, buffer, 1);

		unsigned int dst_y = ROUND(to_dst_y * (float) (cinfo.output_scanline-1));
		int dst_x;

		for ( dst_x=0; dst_x < image.width; ++dst_x ) {
			unsigned int src_x = (float) dst_x * to_dst_x;
			calc_intensity(&buffer[0][src_x*components], &image.p[dst_y*image.width + dst_x], components);
		}

		++image.yadds[dst_y];

		// fill up any scanlines we missed since last iteration
		while ( dst_y - last_dsty > 1 ) {
			++last_dsty;

			for ( dst_x=0; dst_x < image.width; ++dst_x ) {
				unsigned int src_x = (float) dst_x * to_dst_x;
				calc_intensity(&buffer[0][src_x*components], &image.p[last_dsty*image.width + dst_x], components);
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
	free(image.yadds);

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return 0;
}

int main(int argc, char** argv) {
	int r = parse_options(argc, argv);
	if ( r >= 0 ) return r;

	int n;
	for ( n=1; n<argc; ++n ) {

		// Skip options
		if ( argv[n][0]=='-' && argv[n][1]!=0 )
			continue;

		// Read from stdin
		if ( argv[n][0]=='-' && argv[n][1]==0 ) {
			int r = decompress(stdin);
			if ( r == 0 )
				continue;
		}

		FILE *fp;
		if ( (fp = fopen(argv[n], "rb")) != NULL ) {

			if ( verbose )
				fprintf(stderr, "File: %s\n", argv[n]);

			int r = decompress(fp);
			fclose(fp);

			if ( r != 0 )
				return r;
		} else {
			fprintf(stderr, "Can't open %s\n", argv[n]);
			return 1;
		}
	}

	return 0;
}
