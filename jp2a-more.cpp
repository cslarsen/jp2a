/*
 * Copyright (C) 2006 Christian Stigen Larsen <http://csl.sublevel3.org>
 * Distributed under the BSD License
 *
 * Compilation hint:
 * g++ test.cpp -I/sw/include -L/sw/lib -ljpeg
 *
 * $Id: jp2a.cpp 7 2006-06-21 10:52:57Z csl $
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
char ASCII[256];

const char* DEFAULT_ASCII = "   ...,`'\";:clodxkO0KXNWM";
const char* license = "Copyright (C) 2006 Christian Stigen Larsen.\nDistributed under the BSD license";

struct Image {
	const int width;
	const int height;
	double *arr;
	
	Image(const int width_, const int height_) : width(width_ + 1), height(height_ + 1) {
		arr = new double[width * height];
		clear();
	}

	~Image() {
		delete[] arr;
	}

	inline const double& get(const int x, const int y) const {
		if ( y*width + x > width*height ) {
			fprintf(stderr, "out of bounds x=%d y=%d\n", x, y);
			exit(1);
		}
		return arr[y*width + x];
	}

	inline void set(const int x, const int y, const double& val) {
		if ( y*width + x > width*height ) {
			fprintf(stderr, "out of bounds x=%d y=%d\n", x, y);
			exit(1);
		}
		//printf("dst(%d, %d) = %2.2f\n", x, y, val);
		arr[y*width + x] = val;
	}

	inline void add(const int x, const int y, const double& val) {
		set(x,y, val + get(x,y));
	}

	void clear() {
		for ( int n=0; n < width*height; ++n )
			arr[n] = 0.0;
	}

	void invert() {
		for ( int y=0; y<height; ++y )
		for ( int x=0; x<width; ++x )
			set(x,y, 1.0 - get(x,y));
	}

	void normalize() {
		double min = 0.0f;
		double max = 1.0f;

		for ( int y=0; y<height; ++y )
		for ( int x=0; x<width; ++x ) {
			double val = get(x, y);
			if ( val > max ) max = val;
			if ( val < min ) min = val;
		}

//		printf("min=%2.2f, max=%2.2f\n", min, max);
		double range = max - min;

		// normalize values to [0..1]
		for ( int y=0; y<height; ++y )
		for ( int x=0; x<width; ++x ) {
			set(x, y, (get(x, y) - min) / range);
		}
	}


	void print() {
		int sizeASCII = strlen(ASCII) - 1;

		for ( int y=0; y<height-1; ++y ) {
			for ( int x=0; x<width-1; ++x ) {
				const int p = static_cast<int>(round(sizeASCII * get(x, y)));
				printf("%c", ASCII[p]);
			}
			printf("\n");
		}
	}

};

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
	strcpy(ASCII, DEFAULT_ASCII);

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
		hits += sscanf(argv[n], "--chars=%255s", ASCII);

		if ( hits == 0 ) {
			fprintf(stderr, "Unknown option %s\n\n", argv[n]);
			help();
		}
	}

	// check options
	if ( strlen(ASCII) == 0 ) 
		strcpy(ASCII, DEFAULT_ASCII);

	if ( width <= 1 ) width = 1;
	if ( height <= 1 ) height = 1;
}

int decompress(const char* file) {
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

	Image img(width, height);

	int components = cinfo.out_color_components;

	if ( verbose ) {
		printf("Filename: %s\n", file);
		printf("\n");
		printf("Output width : %d\n", img.width);
		printf("Output height: %d\n", img.height);
		printf("\n");
		printf("Source width : %d\n", cinfo.output_width);
		printf("Source height: %d\n", cinfo.output_height);
		printf("Color components    : %d\n", components);
		printf("\n");
	}

	while ( cinfo.output_scanline < cinfo.output_height ) {
		jpeg_read_scanlines(&cinfo, buffer, 1);

		unsigned int y = cinfo.output_scanline;
		double yf = y;
		yf *= img.height - 1;
		yf /= cinfo.output_height - 1;

		for ( unsigned int x=0; x < (cinfo.output_width - 1); ++x ) {
			double intensity = 0.0f;

			for ( int a=0; a < components; ++a )
				intensity += static_cast<double>(buffer[0][x*components + a] / (255.0f * components) );

			// add intensity to corresponding pixels
			double xf = x;
			xf *= img.width - 1;
			xf /= cinfo.output_width - 1;

			int xi = static_cast<int>(xf);
			int yi = static_cast<int>(yf);

			double xrest = xf - xi;
			double yrest = yf - yi;

			img.add(xi, yi, intensity * (1.0 - xrest*yrest));
			if ( xi<(img.width-2) && yi<=(img.height-2) ) {
				img.add(xi+1, yi, intensity * xrest);
				img.add(xi, yi+1, intensity * yrest);
				img.add(xi+1, yi+1, intensity * xrest * yrest);
			}
	
//			printf("src(%d, %d) = %2.2f -> dst(%d, %d) (xf=%2.2f, yf=%2.2f)\n", x, y, intensity, (int)xf, (int)yf, xf, yf);
//			img.add(static_cast<int>(round(xf)), static_cast<int>(round(yf)), intensity);
		}

		++y;
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);

	img.normalize();
	img.invert();
	img.print();

	return 0;
}

int main(int argc, char** argv) {
	if ( argc<2 )
		help();

	parse_options(argc, argv);

	for ( int n=1; n<argc; ++n ) {
		if ( argv[n][0] != '-')
			decompress(argv[n]);
	}

	return 0;
}


