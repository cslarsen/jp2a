/*
 * Copyright (C) 2006 Christian Stigen Larsen, http://csl.sublevel3.org
 * Distributed under the GNU General Public License (GPL) v2.
 *
 * Project homepage on http://jp2a.sf.net
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif

#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "jpeglib.h"
#include "jp2a.h"
#include "options.h"

#define ROUND(x) (int) ( 0.5f + x )

typedef struct rgby_t_ {
	float r;
	float g;
	float b;
	float y;
} rgby_t;

typedef struct Image_ {
	int width;
	int height;
	rgby_t *pixels;
	int *yadds;
	float resize_y;
	float resize_x;
	int *lookup_resx;
} Image;

// Calculate width or height, but not both
void aspect_ratio(const int jpeg_width, const int jpeg_height) {

	// the 2.0f and 0.5f factors are used for text displays that (usually) have characters
	// that are taller than they are wide.

	#define CALC_WIDTH ROUND(2.0f * (float) height * (float) jpeg_width / (float) jpeg_height)
	#define CALC_HEIGHT ROUND(0.5f * (float) width * (float) jpeg_height / (float) jpeg_width)

	// calc width
	if ( auto_width && !auto_height ) {
		width = CALC_WIDTH;

		// adjust for too small dimensions	
		while ( width==0 ) {
			++height;
			aspect_ratio(jpeg_width, jpeg_height);
		}
		
		while ( termfit==TERM_FIT_AUTO && (width + use_border*2)>term_width ) {
			width = term_width - use_border*2;
			height = 0;
			auto_height = 1;
			auto_width = 0;
			aspect_ratio(jpeg_width, jpeg_height);
		}

	}

	// calc height
	if ( !auto_width && auto_height ) {
		height = CALC_HEIGHT;

		// adjust for too small dimensions
		while ( height==0 ) {
			++width;
			aspect_ratio(jpeg_width, jpeg_height);
		}
	}
}

void print_border(const int width) {
	#ifndef HAVE_MEMSET
	int n;
	#endif

	#ifdef WIN32
	char *bord = (char*) malloc(width+3);
	#else
	char bord[width + 3];
	#endif

	#ifdef HAVE_MEMSET
	memset(bord, '-', width+2);
	#else
	for ( n=0; n<width+2; ++n ) bord[n] = '-';
	#endif

	bord[0] = bord[width+1] = '+';
	bord[width+2] = 0;
	puts(bord);

	#ifdef WIN32
	free(bord);
	#endif
}

void print_image_colors(const Image* const i, const int chars, FILE* f) {

	int x, y;
	int xstart, xend, xincr;

	for ( y=0;  y < i->height; ++y ) {

		if ( use_border ) fprintf(f, "|");

		xstart = 0;
		xend   = i->width;
		xincr  = 1;

		if ( flipx ) {
			xstart = i->width - 1;
			xend = -1;
			xincr = -1;
		}

		for ( x=xstart; x != xend; x += xincr ) {

			rgby_t pix = i->pixels[x + (flipy? i->height - y - 1 : y ) * i->width];
			float Y_inv = 1.0f - pix.y;
			
			const int pos = ROUND((float)chars * (!invert? Y_inv : pix.y));
			char ch = ascii_palette[pos];

			const float min = 1.0f / 255.0f;

			if ( !html ) {
				const float threshold = 0.1f;
				const float threshold_inv = 1.0f - threshold;

				rgby_t T = pix;
				T.r -= threshold;
				T.g -= threshold;
				T.b -= threshold;

				int colr = 0;
				int highl = 0;

				// ANSI highlite, only use in grayscale
			        if ( pix.y>=0.95f && pix.r < min && pix.g < min && pix.b < min ) highl = 1; // ANSI highlite

				if ( !convert_grayscale ) {
				     if ( T.r > pix.g && T.r > pix.b )                          colr = 31; // red
				else if ( T.g > pix.r && T.g > pix.b )                          colr = 32; // green
				else if ( T.r > pix.b && T.g > pix.b && pix.r + pix.g > threshold_inv )   colr = 33; // yellow
				else if ( T.b > pix.r && T.b > pix.g && pix.y < 0.95f )             colr = 34; // blue
				else if ( T.r > pix.g && T.b > pix.g && pix.r + pix.b > threshold_inv )   colr = 35; // magenta
				else if ( T.g > pix.r && T.b > pix.r && pix.b + pix.g > threshold_inv )   colr = 36; // cyan
				else if ( pix.r + pix.g + pix.b >= 3.0f*pix.y )                    colr = 37; // white
				} else {
					if ( pix.y>=0.7f ) {
						highl = 1;
						colr = 37; // white
					}
				}

				if ( !colr ) {
					if ( !highl ) fprintf(f, "%c", ch);
					else          fprintf(f, "%c[1m%c%c[0m", 27, ch, 27);
				} else {
					if ( colorfill ) colr += 10;          // set to ANSI background color
					fprintf(f, "%c[%dm%c", 27, colr, ch); // ANSI color
					fprintf(f, "%c[0m", 27);              // ANSI reset
				}

			} else {  // HTML output
			
				// either --grayscale is specified (convert_grayscale)
				// or we can see that the image is inherently a grayscale image	
				if ( convert_grayscale || (pix.r<min && pix.g<min && pix.b<min && pix.y>min) ) {
					// Grayscale image
					if ( colorfill )
						print_html_char(f, ch,
							ROUND(255.0f*pix.y*0.5f), ROUND(255.0f*pix.y*0.5f), ROUND(255.0f*pix.y*0.5f),
							ROUND(255.0f*pix.y),      ROUND(255.0f*pix.y),      ROUND(255.0f*pix.y));
					else
						print_html_char(f, ch,
							ROUND(255.0f*pix.y), ROUND(255.0f*pix.y), ROUND(255.0f*pix.y),
							255, 255, 255);
				} else {
					if ( colorfill )
						print_html_char(f, ch,
							ROUND(255.0f*pix.y*pix.r), ROUND(255.0f*pix.y*pix.g), ROUND(255.0f*pix.y*pix.b),
							ROUND(255.0f*pix.r),   ROUND(255.0f*pix.g),   ROUND(255.0f*pix.b));
					else
						print_html_char(f, ch,
							ROUND(255.0f*pix.r), ROUND(255.0f*pix.g), ROUND(255.0f*pix.b),
							255, 255, 255);
				}
			}
		}

		if ( use_border )
			fputc('|', f);

		if ( html )
			print_html_newline(f);
		else
			fputc('\n', f);
	}
}

void print_image(const Image* const i, const int chars, FILE *f) {
	int x, y;

	#ifdef WIN32
	char *line = (char*) malloc(i->width + 1);
	#else
	char line[i->width + 1];
	#endif

	line[i->width] = 0;

	for ( y=0; y < i->height; ++y ) {

		for ( x=0; x < i->width; ++x ) {

			const float lum = i->pixels[x + (flipy? i->height - y - 1 : y) * i->width].y;
			const int pos = ROUND((float)chars * lum);

			line[flipx? i->width - x - 1 : x] = ascii_palette[invert? pos : chars - pos];
		}

		fprintf(f, !use_border? "%s\n" : "|%s|\n", line);
	}

	#ifdef WIN32
	free(line);
	#endif
}

void clear(Image* i) {
	memset(i->yadds, 0, i->height * sizeof(int) );
	memset(i->pixels, 0, i->width * i->height * sizeof(rgby_t));
	memset(i->lookup_resx, 0, (1 + i->width) * sizeof(int) );
}

void normalize(Image* i) {

	rgby_t *pix = i->pixels;

	int x, y;

	for ( y=0; y < i->height; ++y ) {

		if ( i->yadds[y] > 1 ) {

			for ( x=0; x < i->width; ++x ) {
				if ( usecolors ) {
					pix[x].r /= i->yadds[y];
					pix[x].g /= i->yadds[y];
					pix[x].b /= i->yadds[y];
				}
				pix[x].y /= i->yadds[y];
			}
		}

		pix += i->width;
	}
}

void print_progress(const struct jpeg_decompress_struct* jpg) {
	float progress;
	int pos;
	#define BARLEN 56

	static char s[BARLEN];
	s[BARLEN-1] = 0;

 	progress = (float) (jpg->output_scanline + 1.0f) / (float) jpg->output_height;
	pos = ROUND( (float) (BARLEN-2) * progress );

	memset(s, '.', BARLEN-2);
	memset(s, '#', pos);

	fprintf(stderr, "Decompressing image [%s]\r", s);
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

void process_scanline(const struct jpeg_decompress_struct *jpg, const JSAMPLE* scanline, Image* i) {
	static int lasty = 0;
	const int y = ROUND( i->resize_y * (float) (jpg->output_scanline-1) );

	// include all scanlines since last call

	rgby_t *colr = &i->pixels[lasty * i->width];

	while ( lasty <= y ) {

		const int components = jpg->out_color_components;
		const int readcolors = usecolors;

		int x;
		for ( x=0; x < i->width; ++x ) {
			const JSAMPLE *src     = &scanline[i->lookup_resx[x]];
			const JSAMPLE *src_end = &scanline[i->lookup_resx[x+1]];

			int adds = 0;

			float v, r, g, b;
			v = r = g = b = 0.0f;

			while ( src <= src_end ) {

				if ( components != 3 )
					v += GRAY[src[0]];
				else {
					v += RED[src[0]] + GREEN[src[1]] + BLUE[src[2]];

					if ( readcolors ) {
						r += (float) src[0]/255.0f;
						g += (float) src[1]/255.0f;
						b += (float) src[2]/255.0f;
					}
				}

				++adds;
				src += components;
			}

			colr[x].y += adds>1 ? v / (float) adds : v;

			if ( readcolors ) {
				colr[x].r += adds>1 ? r / (float) adds : r;
				colr[x].g += adds>1 ? g / (float) adds : g;
				colr[x].b += adds>1 ? b / (float) adds : b;
			}
		}

		++i->yadds[lasty++];
		colr += i->width;
	}

	lasty = y;
}

void free_image(Image* i) {
	if ( i->pixels ) free(i->pixels);
	if ( i->yadds ) free(i->yadds);
	if ( i->lookup_resx ) free(i->lookup_resx);
}

void malloc_image(Image* i) {
	i->pixels = NULL;
	i->yadds = NULL;
	i->lookup_resx = NULL;

	i->width = width;
	i->height = height;

	i->yadds = (int*) malloc(height * sizeof(int));
	i->pixels = (rgby_t*) malloc(width*height*sizeof(rgby_t));

	// we allocate one extra pixel for resx because of the src .. src_end stuff in process_scanline
	i->lookup_resx = (int*) malloc( (1 + width) * sizeof(int));

	if ( !(i->pixels && i->yadds && i->lookup_resx) ) {
		fprintf(stderr, "Not enough memory for given output dimension\n");
		free_image(i);
		exit(1);
	}
}

void init_image(Image *i, const struct jpeg_decompress_struct *jpg) {
	int dst_x;

	i->resize_y = (float) (i->height - 1) / (float) (jpg->output_height - 1);
	i->resize_x = (float) (jpg->output_width - 1) / (float) (i->width );

	for ( dst_x=0; dst_x <= i->width; ++dst_x ) {
		i->lookup_resx[dst_x] = ROUND( (float) dst_x * i->resize_x );
		i->lookup_resx[dst_x] *= jpg->out_color_components;
	}
}

void decompress(FILE *fp, FILE *fout) {
	int row_stride;
	struct jpeg_error_mgr jerr;
	struct jpeg_decompress_struct jpg;
	JSAMPARRAY buffer;
	Image image;

	jpg.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&jpg);
	jpeg_stdio_src(&jpg, fp);
	jpeg_read_header(&jpg, TRUE);
	jpeg_start_decompress(&jpg);

	if ( jpg.data_precision != 8 ) {
		fprintf(stderr,
			"Image has %d bits color channels, we only support 8-bit.\n",
			jpg.data_precision);
		exit(1);
	}

	row_stride = jpg.output_width * jpg.output_components;

	buffer = (*jpg.mem->alloc_sarray)((j_common_ptr) &jpg, JPOOL_IMAGE, row_stride, 1);

	aspect_ratio(jpg.output_width, jpg.output_height);

	malloc_image(&image);
	clear(&image);

	if ( verbose ) print_info(&jpg);

	init_image(&image, &jpg);

	while ( jpg.output_scanline < jpg.output_height ) {
		jpeg_read_scanlines(&jpg, buffer, 1);
		process_scanline(&jpg, buffer[0], &image);
		if ( verbose ) print_progress(&jpg);
	}

	if ( verbose ) {
		fprintf(stderr, "\n");
		fflush(stderr);
	}

	normalize(&image);

	if ( clearscr ) {
		fprintf(fout, "%c[2J", 27); // ansi code for clear
		fprintf(fout, "%c[0;0H", 27); // move to upper left
	}

	if ( html && !html_rawoutput ) print_html_start(html_fontsize, fout);
	if ( use_border ) print_border(image.width);

	(!usecolors? print_image : print_image_colors) (&image, (int) strlen(ascii_palette) - 1, fout);

	if ( use_border ) print_border(image.width);
	if ( html && !html_rawoutput ) print_html_end(fout);

	free_image(&image);

	jpeg_finish_decompress(&jpg);
	jpeg_destroy_decompress(&jpg);
}
