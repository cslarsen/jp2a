/*
 * Copyright 2006-2016 Christian Stigen Larsen
 * Distributed under the GNU General Public License (GPL) v2.
 */

#ifndef INC_JP2A_H

#include <stdio.h>

// curl.c
#ifdef FEAT_CURL
int is_url(const char* s);
int curl_download(const char* url, const int debug);
#endif

// html.c
void print_html_start(const int fontsize, FILE *fout);
void print_html_end(FILE *fout);
void print_html_char(FILE *fout, const char ch,
	const int red_fg, const int green_fg, const int blue_fg,
	const int red_bg, const int green_bg, const int blue_bg);
void print_html_newline(FILE *fout);

// image.c
void decompress(FILE *fin, FILE *fout);

// options.c
void parse_options(int argc, char** argv);

// term.c
int get_termsize(int* width_, int* height_, char** error);

#endif
