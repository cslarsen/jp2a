/*
 * Copyright (C) 2006 Christian Stigen Larsen, http://csl.sublevel3.org
 * Distributed under the GNU General Public License (GPL) v2 or later.
 *
 * Project homepage on http://jp2a.sf.net
 *
 * $Id: jp2a.c 168 2006-07-13 12:53:52Z csl $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif

#include <stdio.h>
#include <string.h>
#include "jpeglib.h"
#include "jp2a.h"
#include "options.h"

#define ROUND(x) (int) ( 0.5f + x )

typedef struct Image_ {
	int width;
	int height;
	float *pixel;
	int *yadds;
	float resize_y;
	float resize_x;
	int *lookup_resx;
} Image;

// Calculate width or height, but not both
void aspect_ratio(const int jpeg_width, const int jpeg_height) {

	if ( auto_width && !auto_height ) {
		width = ROUND(2.0f * (float) height * (float) jpeg_width / (float) jpeg_height);

		// adjust for too small dimensions	
		while ( width==0 ) {
			++height;
			aspect_ratio(jpeg_width, jpeg_height);
		}
	}

	if ( !auto_width && auto_height ) {
		height = ROUND(0.5f * (float) width * (float) jpeg_height / (float) jpeg_width);

		// adjust for too small dimensions
		while ( height==0 ) {
			++width;
			aspect_ratio(jpeg_width, jpeg_height);
		}
	}
}

void print_border(const int width) {
	#ifdef WIN32
	char *bord = (char*) malloc(width+3);
	#else
	char bord[width + 3];
	#endif

	memset(bord, '-', width+2);
	bord[0] = bord[width+1] = '+';
	bord[width+2] = 0;
	puts(bord);

	#ifdef WIN32
	free(bord);
	#endif
}

void print_image(const Image* i, const int chars) {
	int x, y;
	const int w = i->width;
	const int h = i->height;

	#ifdef WIN32
	char *line = (char*) malloc(w+1);
	#else
	char line[w + 1];
	#endif

	line[w] = 0;

	for ( y=0; y < h; ++y ) {
		for ( x=0; x < w; ++x ) {
			float intensity = i->pixel[(!flipy? y : h-y-1)*w + x];
			int pos = ROUND( (float) chars * intensity );
			line[!flipx? x : w-x-1] = ascii_palette[ !invert ? chars - pos : pos ];
		}

		printf(!border? "%s\n" : "|%s|\n", line);
	}

	#ifdef WIN32
	free(line);
	#endif
}

void clear(Image* i) {
	memset(i->pixel, 0, i->width * i->height * sizeof(float));
	memset(i->yadds, 0, i->height * sizeof(int) );
}

void normalize(Image* i) {
	const int w = i->width;
	const int h = i->height;

	register int x, y, yoffs;

	for ( y=0, yoffs=0; y < h; ++y, yoffs += w )
	for ( x=0; x < w; ++x ) {
		if ( i->yadds[y] != 0 )
			i->pixel[yoffs + x] /= (float) i->yadds[y];
	}
}

void print_progress(const struct jpeg_decompress_struct* jpg) {
 	float progress = (float) (jpg->output_scanline + 1.0f) / (float) jpg->output_height;
	int pos = ROUND( (float) progress_barlength * progress );

	#ifdef WIN32
	char *s = (char*) malloc(progress_barlength + 1);
	#else
	char s[progress_barlength + 1];
	#endif

	memset(s, '.', progress_barlength);
	memset(s, '#', pos);

	s[progress_barlength] = 0;

	fprintf(stderr, "Decompressing image [%s]\r", s);

	#ifdef WIN32
	free(s);
	#endif
}

inline
float intensity(const JSAMPLE* source, const int components) {
	register float v = source[0];
	register int c=1;

	while ( c < components )
		v += source[c++];

	return v / ( 255.0f * components );
}

void print_info(const struct jpeg_decompress_struct* cinfo) {
	fprintf(stderr, "Source width: %d\n", cinfo->output_width);
	fprintf(stderr, "Source height: %d\n", cinfo->output_height);
	fprintf(stderr, "Source color components: %d\n", cinfo->output_components);
	fprintf(stderr, "Output width: %d\n", width);
	fprintf(stderr, "Output height: %d\n", height);
	fprintf(stderr, "Output palette (%d chars): '%s'\n", (int) strlen(ascii_palette), ascii_palette);
}

inline
void process_scanline(const struct jpeg_decompress_struct *jpg, const JSAMPLE* scanline, Image* i) {
	static int lasty = 0;
	const int y = ROUND( i->resize_y * (float) (jpg->output_scanline-1) );

	// include all scanlines since last call
	while ( lasty <= y ) {
		const int yoff = lasty * i->width;
		int x;

		for ( x=0; x < i->width; ++x ) {
			i->pixel[yoff + x] += intensity( &scanline[ i->lookup_resx[x] ],
				jpg->out_color_components);
		}

		++i->yadds[lasty++];
	}

	lasty = y;
}

void free_image(Image* i) {
	if ( i->pixel ) free(i->pixel);
	if ( i->yadds ) free(i->yadds);
	if ( i->lookup_resx ) free(i->lookup_resx);
}

void malloc_image(Image* i) {
	i->pixel = NULL;
	i->yadds = NULL;
	i->lookup_resx = NULL;

	i->width = width;
	i->height = height;

	if ( (i->pixel = (float*) malloc(width * height * sizeof(float))) == NULL ) {
		fprintf(stderr, "Not enough memory for given output dimension\n");
		exit(1);
	}

	if ( (i->yadds = (int*) malloc(height * sizeof(int))) == NULL ) {
		fprintf(stderr, "Not enough memory for given output dimension (for yadds)\n");
		free_image(i);
		exit(1);
	}

	if ( (i->lookup_resx = (int*) malloc(width * sizeof(int))) == NULL ) {
		fprintf(stderr, "Not enough memory for given output dimension (lookup_resx)\n");
		free_image(i);
		exit(1);
	}
}

void init_image(Image *i, const struct jpeg_decompress_struct *jpg) {
	i->resize_y = (float) (i->height - 1) / (float) (jpg->output_height-1);
	i->resize_x = (float) jpg->output_width / (float) i->width;

	int dst_x;
	for ( dst_x=0; dst_x < i->width; ++dst_x ) {
		i->lookup_resx[dst_x] = (int)( (float) dst_x * i->resize_x );
		i->lookup_resx[dst_x] *= jpg->out_color_components;
	}
}

void decompress(FILE *fp) {
	struct jpeg_error_mgr jerr;
	struct jpeg_decompress_struct jpg;

	jpg.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&jpg);
	jpeg_stdio_src(&jpg, fp);
	jpeg_read_header(&jpg, TRUE);
	jpeg_start_decompress(&jpg);

	int row_stride = jpg.output_width * jpg.output_components;

	JSAMPARRAY buffer = (*jpg.mem->alloc_sarray)
		((j_common_ptr) &jpg, JPOOL_IMAGE, row_stride, 1);

	aspect_ratio(jpg.output_width, jpg.output_height);

	Image image;
	malloc_image(&image);
	clear(&image);

	if ( verbose ) print_info(&jpg);

	init_image(&image, &jpg);

	while ( jpg.output_scanline < jpg.output_height ) {
		jpeg_read_scanlines(&jpg, buffer, 1);
		process_scanline(&jpg, buffer[0], &image);

		if ( verbose ) print_progress(&jpg);
	}

	if ( verbose ) fprintf(stderr, "\n");

	normalize(&image);

	if ( html ) print_html_start();
	if ( border ) print_border(image.width);

	print_image(&image, (int) strlen(ascii_palette) - 1);

	if ( border ) print_border(image.width);
	if ( html ) print_html_end();

	free_image(&image);

	jpeg_finish_decompress(&jpg);
	jpeg_destroy_decompress(&jpg);
}
