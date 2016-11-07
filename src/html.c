/*
 * Copyright 2006-2016 Christian Stigen Larsen
 * Distributed under the GNU General Public License (GPL) v2.
 */

#include <stdio.h>
#include <string.h>
#include "options.h"

void print_html_start(const int fontsize, FILE *f) {
	
	fputs(   "<?xml version='1.0' encoding='ISO-8859-1'?>\n"
		"<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN'"
		"  'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>\n"
		"<html xmlns='http://www.w3.org/1999/xhtml' lang='en' xml:lang='en'>\n"
		"<head>\n", f);
	fprintf(f,
		"<title>%s</title>\n", html_title);
	fputs(
		"<style type='text/css'>\n"
		"body {\n", f);
	fputs(!invert?
		"   background-color: white;\n" : "background-color: black;\n", f);
	fputs(  "}\n"
		".ascii {\n"
		"   font-family: Courier;\n", f); // should be a monospaced font
	if ( !usecolors )
	fputs(!invert?
		"   color: black;\n" : "   color: white;\n", f);
	fprintf(f,
		"   font-size:%dpt;\n", fontsize);
	if ( html_bold )
	fputs( 	"   font-weight: bold;\n", f);
	else
	fputs(  "   font-weight: normal;\n", f);
	fputs(
		"}\n"
		"</style>\n"
		"</head>\n"
		"<body>\n"
		"<div class='ascii'><pre>\n", f);
}

void print_html_end(FILE *f) {
	fputs("</pre>\n</div>\n</body>\n</html>\n", f);
}

const char* html_entity(const char ch) {
	static char s[2];
	switch ( ch ) {
	case ' ': return "&nbsp;"; break;
	case '<': return "&lt;"; break;
	case '>': return "&gt;"; break;
	case '&': return "&amp;"; break;
	default:
		s[0]=ch; s[1]=0; return s; break;
	}
}

void print_html_char(FILE *f, const char ch,
	const int r_fg, const int g_fg, const int b_fg,
	const int r_bg, const int g_bg, const int b_bg)
{
	if ( colorfill ) {
		fprintf(f, "<span style='color:#%02x%02x%02x; background-color:#%02x%02x%02x;'>%s</span>",
			r_fg, g_fg, b_fg,
			r_bg, g_bg, b_bg,
			html_entity(ch));
	} else
		fprintf(f, "<span style='color:#%02x%02x%02x;'>%s</span>",
			r_fg, g_fg, b_fg, html_entity(ch));
}

void print_html_newline(FILE *f) {
	fputs("<br/>", f);
}
