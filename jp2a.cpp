/*
 * Copyright (C) 2006 Christian Stigen Larsen <http://csl.sublevel3.org>
 * Distributed under the BSD License
 *
 * Compilation hint:
 * g++ test.cpp -I/sw/include -L/sw/lib -ljpeg
 *
 * $Id$
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <jpeglib.h>

bool verbose = false;
bool color = false;
int width = 80;
int height = 25;

const char* license = "Copyright (C) 2006 Christian Stigen Larsen.\nDistributed under the BSD license";

// Printable ASCII characters, sorted least intensive to most intensive
//const char* origascii = "   ........oooOOOOOOO";
const char* origascii = "   ...',;:clodxkO0KXNWM";
//abcdefghijklmnopqrstuvwxyz*#%$&123456789ACDEFGHIJKLOPRSTUVYZX@0QNBWM";

char ascii[1024];

int test_decompress(const char* file);

void help() {
	fprintf(stderr, "Usage: jp2afilename[s].jpg\n\n");
	fprintf(stderr, "Simple JPEG to ASCII converter\n\n");
	fprintf(stderr, "OPTIONS\n");
	fprintf(stderr, "    -h, --help       Print program help\n");
	fprintf(stderr, "    --size=WxH       Set output ASCII width and height, default is %dx%d\n", width, height);
	fprintf(stderr, "    --chars=...      Select character palette used to paint the image.  Leftmost character\n");
	fprintf(stderr, "                     corresponds to black pixel, rightmost to white pixel.\n");
	fprintf(stderr, "    -v, --verbose    Verbose output\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "EXAMPLE\n");
	fprintf(stderr, "   jpg2ascii --size=80x25 --chars='...oooxx@@' somefile.jpg\n\n");
	fprintf(stderr, "%s\n", license);
	exit(1);
}

void parse_options(int argc, char** argv) {
	strcpy(ascii, origascii);

	for ( int n=1; n<argc; ++n ) {
		if ( argv[n][0] != '-')
			continue;

		int hits = 0;

		if ( !strcmp(argv[n], "-h") || !strcmp(argv[n], "--help") ) help();
		if ( !strcmp(argv[n], "-v") || !strcmp(argv[n], "--verbose") ) {
			++hits;
			verbose = true;
		}

		hits += sscanf(argv[n], "--size=%dx%d", &width, &height);
		hits += sscanf(argv[n], "--chars=%1000s", ascii);

		if ( hits == 0 ) {
			fprintf(stderr, "Unknown option %s\n\n", argv[n]);
			help();
		}
	}

	// check options
	if ( strlen(ascii) == 0 )
		strcpy(ascii, origascii);

	if ( width <= 1 ) width = 1;
	if ( height <= 1 ) height = 1;
}

void print(double* accum, int width, int sizeAscii) {
	for ( int n=0; n < width; ++n ) {
		int pos = static_cast<int>(sizeAscii * accum[n]);

		// the following should never happen
		if ( pos < 0 ) pos = 0;
		if ( pos > sizeAscii ) pos = sizeAscii;

		putc(ascii[pos], stdout);
	}

	putc('\n', stdout);
}

void invert(double* accum, int width) {
	for ( int n=0; n<width; ++n )
		accum[n] = 1.0f - accum[n];
}

void clear(double* accum, int width) {
	for ( int n=0; n<width; ++n )
		accum[n] = 0.0f;
}


int main(int argc, char** argv) {
	if ( argc<2 )
		help();

	parse_options(argc, argv);

	for ( int n=1; n<argc; ++n ) {
		if ( argv[n][0] != '-')
			test_decompress(argv[n]);
	}

	return 0;
}

void normalize(double* accum, int width, double factor) {
	for ( int n=0; n<width; ++n )
		accum[n] /= factor;
}

int test_decompress(const char* file) {
	FILE * infile;

        if ( (infile = fopen(file, "rb")) == NULL ) {
            fprintf(stderr, "can't open %s\n", file);
            return 1;
        }

        struct jpeg_error_mgr jerr;
	struct jpeg_decompress_struct cinfo;

        cinfo.err = jpeg_std_error(&jerr);

        jpeg_create_decompress(&cinfo);
        jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);

	jpeg_start_decompress(&cinfo);

	int row_stride = cinfo.output_width * cinfo.output_components;
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	double accum[width];

	int sizeAscii = strlen(ascii) - 1;
	int comps = cinfo.out_color_components;

	int pixelsPerChar = row_stride / (comps * width);
	if ( pixelsPerChar <= 0 ) pixelsPerChar = 1;

	int linesToAdd = cinfo.output_height / height;
	if ( linesToAdd <= 0 ) linesToAdd = 1;

	if ( verbose ) {
		printf("Filename: %s\n", file);
		printf("\n");
		printf("Output width : %d\n", width);
		printf("Output height: %d\n", height);
		printf("\n");
		printf("Source width : %d\n", cinfo.output_width);
		printf("Source height: %d\n", cinfo.output_height);
		printf("\n");
		printf("ASCII characters used for printing: %d\n", 1 + sizeAscii);
		printf("Pixels per character: %d\n", pixelsPerChar);
		printf("Lines per character : %d\n", linesToAdd);
		printf("Color components    : %d\n", comps);
		printf("\n");
	}

	int linesAdded = 0;
	clear(accum, width);

	while ( cinfo.output_scanline < cinfo.output_height ) {
		jpeg_read_scanlines(&cinfo, buffer, 1);

		int currChar = 0;
		int pixelsAdded = 0;

		for ( int pixel=0; pixel < (row_stride - comps); pixel += comps) {

			for ( int addit=0; addit<comps; ++addit )
				accum[currChar] += static_cast<double>( buffer[0][pixel + addit] / (comps * 255.0f) );

			++pixelsAdded;

			if ( pixelsAdded >= pixelsPerChar ) {
				if ( currChar>=width )
					break;

				pixelsAdded = 0;
				++currChar;
			}
		}

		++linesAdded;

		if ( linesAdded > linesToAdd ) {
			normalize(accum, width, pixelsAdded * linesAdded);
			invert(accum, width);
			print(accum, width, sizeAscii);
			clear(accum, width);
			linesAdded = 0;
		} else {
			// last line? print it
			if ( (1 + cinfo.output_scanline) == cinfo.output_height ) {
				normalize(accum, width, pixelsAdded * linesAdded);
				invert(accum, width);
				print(accum, width, sizeAscii);
				clear(accum, width);
			}
		}

	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);

	return 0;
}
