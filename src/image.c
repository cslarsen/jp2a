/*
 * Copyright (C) 2006 Christian Stigen Larsen, http://csl.sublevel3.org
 * Distributed under the GNU General Public License (GPL) v2.
 *
 * Project homepage on http://jp2a.sf.net
 *
 * $Id$
 */

#include "image.h"

#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "jpeglib.h"
#include "jp2a.h"
#include "options.h"
#include "round.h"

typedef void (*image_resize_ptrfun)(const image_t * const, const image_t * const);
image_resize_ptrfun global_image_resize_fun = NULL;

//void image_set_resize_function(
image_t* image_new(int width, int height) {
	image_t *p;

	if ( !(p = (image_t*) malloc (sizeof(image_t))) ) {
		perror("jpa: coudln't allocate memory for image");
		exit(1);
	}
	
	if ( !(p->pixels = (rgb_t*) malloc (width*height*sizeof(rgb_t))) ) {
		perror("jp2a: couldn't allocate memory for image");
		exit(1);
	}

	p->w = width;
	p->h = height;
	
	return p;
}

void image_destroy(image_t *p) {
	free(p->pixels);
	free(p);
}

void image_clear(image_t *p) {
	memset(p->pixels, 0, p->w*p->h*sizeof(rgb_t));
}

inline
rgb_t* image_pixel(image_t *p, const int x, const int y) {
	return &p->pixels[x + y*p->w];
}

void image_resize(image_t *s, image_t *d) {
	if ( global_image_resize_fun == NULL ) {
		fputs("jp2a: image_resize function not set\n", stderr);
		exit(1);
	}
	int n;
	for ( n=0; n < 100000; ++n )
	global_image_resize_fun(s, d);
}

void image_resize_nearest_neighbour(const image_t * const source, const image_t * const dest) {
	register int x, y;

	const float dx = source->w / dest->w;
	const float dy = source->h / dest->h;

	const rgb_t * sourcepix = source->pixels;
	rgb_t *destpix = dest->pixels;
	const int offsadd = (int)(dy * (float)source->w);

	int lookupx[dest->w];
	for ( x=0; x < dest->w; ++x )
		lookupx[x] = (int)((float)x * dx);

	for ( y=0; y < dest->h; ++y ) {
		for ( x=0; x < dest->w; ++x )
			destpix[x] = sourcepix[lookupx[x]];
	
		destpix += dest->w;
		sourcepix += offsadd;
	}
}

void print_border(FILE *f, int width) {
	fputc('+', f);
	while ( width-- )
		fputc('-', f);
	fprintf(f, "+\n");
}

void print_progress(const float percent) {
	static char str[64] = "Decompressing image [....................]\r";
	int num = ROUND((float)20 * percent);
	int n;

	for ( n=0; n < num; ++n )
		str[21 + n] = '#';

	fputs(str, stderr);
	fflush(stderr);
}

void print_info(const struct jpeg_decompress_struct* jpg) {
	fprintf(stderr, "Source width: %d\n", jpg->output_width);
	fprintf(stderr, "Source height: %d\n", jpg->output_height);
	fprintf(stderr, "Source color components: %d\n", jpg->output_components);
	fprintf(stderr, "Output width: %d\n", width);
	fprintf(stderr, "Output height: %d\n", height);
	fprintf(stderr, "Output palette (%d chars): '%s'\n", (int)strlen(ascii_palette), ascii_palette);
}

image_t* image_read(FILE *fp) {
	JSAMPARRAY buffer;
	int row_stride;
	struct jpeg_decompress_struct jpg;
	struct jpeg_error_mgr jerr;
	image_t *p;

	global_image_resize_fun = image_resize_nearest_neighbour;

	jpg.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&jpg);
	jpeg_stdio_src(&jpg, fp);
	jpeg_read_header(&jpg, TRUE);
	jpeg_start_decompress(&jpg);

	if ( jpg.data_precision != 8 ) {
		fprintf(stderr, "jp2a: can only handle 8-bit color channels\n");
		exit(1);
	}

	row_stride = jpg.output_width * jpg.output_components;
	buffer = (*jpg.mem->alloc_sarray)((j_common_ptr) &jpg, JPOOL_IMAGE, row_stride, 1);

	aspect_ratio(jpg.output_width, jpg.output_height);
	p = image_new(jpg.output_width, jpg.output_height);

	if ( verbose )
		print_info(&jpg);

	while ( jpg.output_scanline < jpg.output_height ) {
		jpeg_read_scanlines(&jpg, buffer, 1);

		if ( jpg.output_components == 3 ) {
			memcpy(&p->pixels[(jpg.output_scanline-1)*p->w], &buffer[0][0], sizeof(rgb_t)*p->w);
		} else {
			rgb_t c;
			int x;
			int offset = (jpg.output_scanline-1) * p->w;

			// grayscale
			for ( x=0; x < jpg.output_width; ++x ) {
				c.r = c.g = c.b = buffer[0][x];
				p->pixels[x + offset] = c;
			}
		}

		if ( verbose )
			print_progress((float)jpg.output_scanline/(float)jpg.output_height);
	}

	if ( verbose ) {
		fprintf(stderr, "\n");
		fflush(stderr);
	}

	jpeg_finish_decompress(&jpg);
	jpeg_destroy_decompress(&jpg);
	return p;
}

void image_print(const image_t *p, FILE *fout) {

	char line[p->w + 1];
	int x, y, len;

	len = strlen(ascii_palette) - 1;
	line[p->w] = 0;
	
	if ( use_border )
		print_border(fout, width);

	for ( y=0; y < p->h; ++y ) {
		for ( x=0; x < p->w; ++x ) {
			rgb_t *pix = &p->pixels[x + y*p->w];
			float lum = redweight * pix->r + greenweight * pix->g + blueweight * pix->b;
			char ch = ascii_palette[ROUND( (float)len * lum / (float)MAXJSAMPLE )];
			line[x] = ch;
		}

		fprintf(fout, use_border? "|%s|\n" : "%s\n", line);
	}

	//(!usecolors? print_image : print_image_colors) (&image, (int) strlen(ascii_palette) - 1, fout);

	if ( use_border )
		print_border(fout, width);

	if ( html && !html_rawoutput ) print_html_end(fout);
}
