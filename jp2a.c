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
 // below is needed for jpeglib.h
 #undef HAVE_STDLIB_H
#endif

#include <unistd.h>
#include <stdio.h>

#ifdef HAVE_STRING_H
 #include <string.h>
#endif

#include "jpeglib.h"

#ifdef FEAT_CURL
 #ifdef HAVE_CURL_CURL_H
  #include "curl/curl.h"
 #endif
#endif

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
int debug = 0;

char ascii_palette[257] = "";
const char* default_palette = "   ...',;:clodxkO0KXNWM";

void help() {
	fprintf(stderr, "%s\n", version);
	fprintf(stderr, "%s\n", copyright);
	fprintf(stderr, "%s\n", license);

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
"  -                Read JPEG image from standard input.\n"
"  -b, --border     Print a border around the output image.\n"
"      --chars=...  Select character palette used to paint the image.\n"
"                   Leftmost character corresponds to black pixel, right-\n"
"                   most to white.  Minimum two characters must be specified.\n"
"  -d, --debug      Print additional debug information.\n"
"      --flipx      Flip image in X direction.\n"
"      --flipy      Flip image in Y direction.\n"
"  -i, --invert     Invert output image.  Use if your display has a dark\n"
"                   background.\n"
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

	// define some shorthand defines
	#define IF_OPTS(shortopt, longopt) if ( !strcmp(s, shortopt) || !strcmp(s, longopt) )
	#define IF_OPT(shortopt) if ( !strcmp(s, shortopt) )

	int n, files;
	for ( n=1, files=0; n<argc; ++n ) {
		const char *s = argv[n];

		if ( *s != '-' ) { // count files to read
			++files; continue;
		}
	
		IF_OPT("-")			{ /* stdin to be read */ ++files; continue; }
		IF_OPTS("-h", "--help")		{ help(); return 0; }
		IF_OPTS("-v", "--verbose")	{ verbose = 1; continue; }
		IF_OPTS("-d", "--debug")	{ debug = 1; continue; }
		IF_OPT("--html") 		{ html = 1; continue; }
		IF_OPTS("-b", "--border") 	{ border = 1; continue; }
		IF_OPTS("-i", "--invert") 	{ invert = 1; continue; }
		IF_OPT("--flipx") 		{ flipx = 1; continue; }
		IF_OPT("--flipy") 		{ flipy = 1; continue; }
		IF_OPTS("-V", "--version") {
			fprintf(stderr, "%s\n%s\n%s\n\nProject homepage %s\n",
				version, copyright, license, url);
			return 0;
		}

		if ( sscanf(s, "--size=%dx%d", &width, &height) == 2 ) {
			auto_width = auto_height = 0;
			continue;
		}

		if ( !strncmp(s, "--chars=", 8) ) {
			// don't use sscanf, we need to read spaces as well
			strcpy(ascii_palette, s+8);
			continue;
		}

		if ( sscanf(s, "--width=%d", &width) == 1 ) {
			auto_height += 1;
			continue;
		}

		if ( sscanf(s, "--height=%d", &height) == 1 ) {
			auto_width += 1;
			continue;
		}

		if ( sscanf(s, "--html-fontsize=%d", &html_fontsize) == 1 )
			continue;

		fprintf(stderr, "Unknown option %s\n\n", s);
		help();
		return 1;

	} // args ...

	if ( !files ) {
		fprintf(stderr, "No files specified\n\n");
		help();
		return 1;
	}

	// only --width specified, calc width
	if ( auto_width==1 && auto_height == 1 )
		auto_height = 0;

	// if both --width and --height are set, we will use that
	// as an explicit setting, and not calculate
	if ( auto_width==2 && auto_height==1 ) {
		auto_width = auto_height = 0;
	}

	// Palette must be at least two characters
	if ( ascii_palette[0] == 0 ) strcpy(ascii_palette, default_palette);
	if ( ascii_palette[1] == 0 ) strcpy(ascii_palette, default_palette);
	
	if ( width < 1 && !auto_width ) {
		fprintf(stderr, "Negative or zero width specified.\n");
		return 1;
	}

	if ( height < 1 && !auto_height ) {
		fprintf(stderr, "Negative or zero height specified.\n");
		return 1;
	}

	return -1;
}

void calc_aspect_ratio(const int jpeg_width, const int jpeg_height) {

	// Calculate width or height, but not both

	if ( auto_width && !auto_height ) {
		width = ROUND(2.0f * (float) height * (float) jpeg_width / (float) jpeg_height);

		// adjust for too small dimensions	
		while ( width==0 ) {
			++height;
			calc_aspect_ratio(jpeg_width, jpeg_height);
		}
	}

	if ( !auto_width && auto_height ) {
		height = ROUND(0.5f * (float) width * (float) jpeg_height / (float) jpeg_width);

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

#ifdef FEAT_CURL
//! return 1 if `s' is an URL, 0 if not
int is_url(const char* s) {
	int r = 0;
	r |= !strncmp(s, "ftp://", 6);
	r |= !strncmp(s, "ftps://", 7);
	r |= !strncmp(s, "file://", 7);
	r |= !strncmp(s, "http://", 7);
	r |= !strncmp(s, "https://", 8);
	r |= !strncmp(s, "tftp://", 7);
//	r |= !strncmp(s, "dict://", 7);  // don't think we need to support this
//	r |= !strncmp(s, "ldap://", 7);  // same here
	return r;
}

// Fork and return filedescriptor of read-pipe for downloaded data
// Returns -1 in case of errors
// You must close() the filedescriptor after using it.
int curl_download(const char* url, const int debug) {
	int p, fd[2];
	pid_t pid;

	if ( (p = pipe(fd)) != 0 ) {
		fprintf(stderr, "Could not create pipe (returned %d)\n", p);
		return -1;
	}

	if ( (pid = fork()) == 0 ) {
		// CHILD process
		close(fd[0]); // close read end

		FILE *fw = fdopen(fd[1], "wb");

		if ( fw == NULL ) {
			fprintf(stderr, "Could not write to pipe\n");
			exit(1);
		}

		curl_global_init(CURL_GLOBAL_ALL);
		atexit(curl_global_cleanup);

		CURL *curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, url);

		if ( debug )
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1); // fail silently on errors
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL); // use default handler
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fw);

		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		fflush(fw);
		fclose(fw);
		close(fd[1]);
		exit(0);
	} else if ( pid < 0 ) {
		fprintf(stderr, "Failed to fork\n");
		return -1;
	}

	// PARENT process

	close(fd[1]); // close write end of pipe
	return fd[0];
}
#endif

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

#ifdef FEAT_CURL
		if ( is_url(argv[n]) ) {
			if ( verbose )
				fprintf(stderr, "URL: %s\n", argv[n]);

			int fd = curl_download(argv[n], debug);
				
			if ( fd < 0 )
				return 1; // error message printed in curl_download

			FILE *fr = fdopen(fd, "rb");

			if ( fr == NULL ) {
				fprintf(stderr, "Could not fdopen read pipe\n");
				return 1;
			}

			int r = decompress(fr);

			fclose(fr);
			close(fd);

			if ( r != 0 ) return r;
			continue;
		}
#endif

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
