/*
 * Copyright (C) 2006 Christian Stigen Larsen, http://csl.sublevel3.org
 * Distributed under the GNU General Public License (GPL) v2 or later.
 *
 * Project homepage on http://jp2a.sf.net
 *
 * $Id$
 */

#include <stdio.h>
#include "options.h"

void print_html_start(const int fontsize, FILE *f) {
	
	fputs(   "<?xml version='1.0' encoding='ISO-8859-1'?>\n"
		"<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN'"
		"  'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>\n"
		"<html xmlns='http://www.w3.org/1999/xhtml' lang='en' xml:lang='en'>\n"
		"<head>\n"
		"<title>jp2a converted image</title>\n"
		"<style type='text/css'>\n"
		".ascii {\n"
		"   font-family: Courier New;\n" // should be a monospaced font
		, f);
	fprintf(f,
		"   font-size:%dpt;\n", fontsize);
	fputs(   "}\n"
		"</style>\n"
		"</head>\n"
		"<body>\n"
		"<div class='ascii'>\n", f);
	if ( !usecolors )
	fputs(  "<pre>\n", f);
}

void print_html_end(FILE *f) {
	if ( !usecolors ) fputs("</pre>", f);
	fputs("\n</div>\n</body>\n</html>\n", f);
}
