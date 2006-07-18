/*
 * Copyright (C) 2006 Christian Stigen Larsen, http://csl.sublevel3.org
 * Distributed under the GNU General Public License (GPL) v2 or later.
 *
 * Project homepage on http://jp2a.sf.net
 *
 * $Id$
 */

// curl.c
#ifdef FEAT_CURL
extern int is_url(const char* s);
extern int curl_download(const char* url, const int debug);
#endif

// html.c
extern void print_html_start(const int fontsize);
extern void print_html_end();

// image.c
void decompress(FILE *fin, FILE *fout);

// options.c
void parse_options(int argc, char** argv);
