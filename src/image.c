/*
 * Copyright 2006-2016 Christian Stigen Larsen
 * Distributed under the GNU General Public License (GPL) v2.
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

#include "aspect_ratio.h"
#include "image.h"
#include "jp2a.h"
#include "options.h"

#define ROUND(x) (int) ( 0.5f + x )

typedef struct Image_ {
	int width;
	int height;
	float *pixel; // luminosity
	float *red, *green, *blue;
	int *yadds;
	float resize_y;
	float resize_x;
	int *lookup_resx;
} Image;

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

			float Y = i->pixel[x + (flipy? i->height - y - 1 : y ) * i->width];
			float Y_inv = 1.0f - Y;
			float R = i->red  [x + (flipy? i->height - y - 1 : y ) * i->width];
			float G = i->green[x + (flipy? i->height - y - 1 : y ) * i->width];
			float B = i->blue [x + (flipy? i->height - y - 1 : y ) * i->width];

			const int pos = ROUND((float)chars * (!invert? Y_inv : Y));
			char ch = ascii_palette[pos];

			const float min = 1.0f / 255.0f;

			if ( !html ) {
				const float t = 0.1f; // threshold
				const float i = 1.0f - t;

				int colr = 0;
				int highl = 0;

				// ANSI highlite, only use in grayscale
			        if ( Y>=0.95f && R<min && G<min && B<min ) highl = 1; // ANSI highlite

				if ( !convert_grayscale ) {
				     if ( R-t>G && R-t>B )            colr = 31; // red
				else if ( G-t>R && G-t>B )            colr = 32; // green
				else if ( R-t>B && G-t>B && R+G>i )   colr = 33; // yellow
				else if ( B-t>R && B-t>G && Y<0.95f ) colr = 34; // blue
				else if ( R-t>G && B-t>G && R+B>i )   colr = 35; // magenta
				else if ( G-t>R && B-t>R && B+G>i )   colr = 36; // cyan
				else if ( R+G+B>=3.0f*Y )             colr = 37; // white
				} else {
					if ( Y>=0.7f ) { highl=1; colr = 37; }
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
				if ( convert_grayscale || (R<min && G<min && B<min && Y>min) ) {
					// Grayscale image
					if ( colorfill )
						print_html_char(f, ch,
							ROUND(255.0f*Y*0.5f), ROUND(255.0f*Y*0.5f), ROUND(255.0f*Y*0.5f),
							ROUND(255.0f*Y),      ROUND(255.0f*Y),      ROUND(255.0f*Y));
					else
						print_html_char(f, ch,
							ROUND(255.0f*Y), ROUND(255.0f*Y), ROUND(255.0f*Y),
							255, 255, 255);
				} else {
					if ( colorfill )
						print_html_char(f, ch,
							ROUND(255.0f*Y*R), ROUND(255.0f*Y*G), ROUND(255.0f*Y*B),
							ROUND(255.0f*R),   ROUND(255.0f*G),   ROUND(255.0f*B));
					else
						print_html_char(f, ch,
							ROUND(255.0f*R), ROUND(255.0f*G), ROUND(255.0f*B),
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

			const float lum = i->pixel[x + (flipy? i->height - y - 1 : y) * i->width];
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
	memset(i->pixel, 0, i->width * i->height * sizeof(float));
	memset(i->lookup_resx, 0, (1 + i->width) * sizeof(int) );

	if ( usecolors ) {
		memset(i->red,   0, i->width * i->height * sizeof(float));
		memset(i->green, 0, i->width * i->height * sizeof(float));
		memset(i->blue,  0, i->width * i->height * sizeof(float));
	}
}

void normalize(Image* i) {

	float *pixel = i->pixel;
	float *red   = i->red;
	float *green = i->green;
	float *blue  = i->blue;

	int x, y;

	for ( y=0; y < i->height; ++y ) {

		if ( i->yadds[y] > 1 ) {

			for ( x=0; x < i->width; ++x ) {
				pixel[x] /= i->yadds[y];

				if ( usecolors ) {
					red  [x] /= i->yadds[y];
					green[x] /= i->yadds[y];
					blue [x] /= i->yadds[y];
				}
			}
		}

		pixel += i->width;

		if ( usecolors ) {
			red   += i->width;
			green += i->width;
			blue  += i->width;
		}
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

	float *pixel, *red, *green, *blue;

	pixel  = &i->pixel[lasty * i->width];
	red = green = blue = NULL;

	if ( usecolors ) {
		int offset = lasty * i->width;
		red   = &i->red  [offset];
		green = &i->green[offset];
		blue  = &i->blue [offset];
	}

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

			pixel[x] += adds>1 ? v / (float) adds : v;

			if ( readcolors ) {
				red  [x] += adds>1 ? r / (float) adds : r;
				green[x] += adds>1 ? g / (float) adds : g;
				blue [x] += adds>1 ? b / (float) adds : b;
			}
		}

		++i->yadds[lasty++];

		pixel += i->width;

		if ( readcolors ) {
			red   += i->width;
			green += i->width;
			blue  += i->width;
		}
	}

	lasty = y;
}

void free_image(Image* i) {
	if ( i->pixel ) free(i->pixel);
	if ( i->red ) free(i->red);
	if ( i->green ) free(i->green);
	if ( i->blue ) free(i->blue);
	if ( i->yadds ) free(i->yadds);
	if ( i->lookup_resx ) free(i->lookup_resx);
}

void malloc_image(Image* i) {
	i->pixel = i->red = i->green = i->blue = NULL;
	i->yadds = NULL;
	i->lookup_resx = NULL;

	i->width = width;
	i->height = height;

	i->yadds = (int*) malloc(height * sizeof(int));
	i->pixel = (float*) malloc(width*height*sizeof(float));

	if ( usecolors ) {
		i->red   = (float*) malloc(width*height*sizeof(float));
		i->green = (float*) malloc(width*height*sizeof(float));
		i->blue  = (float*) malloc(width*height*sizeof(float));
	}

	// we allocate one extra pixel for resx because of the src .. src_end stuff in process_scanline
	i->lookup_resx = (int*) malloc( (1 + width) * sizeof(int));

	if ( !(i->pixel && i->yadds && i->lookup_resx) ||
	     (usecolors && !(i->red && i->green && i->blue)) )
	{
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
