/*
 * Copyright (C) 2006 Christian Stigen Larsen, http://csl.sublevel3.org
 * Distributed under the GNU General Public License (GPL) v2 or later.
 *
 * Project homepage on http://jp2a.sf.net
 *
 * $Id: jp2a.c 164 2006-07-13 12:30:54Z csl $
 */

#include <stdio.h>

void print_html_start(const int fontsize) {
	printf("<?xml version='1.0' encoding='ISO-8859-1'?>\n"
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
	"<pre>\n", fontsize);
}

void print_html_end() {
	printf("</pre>\n");
	printf("</div>\n");
	printf("</body>\n");
	printf("</html>\n");
}
