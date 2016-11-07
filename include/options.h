/*
 * Copyright 2006-2016 Christian Stigen Larsen
 * Distributed under the GNU General Public License (GPL) v2.
 */

#ifndef INC_JP2A_OPTIONS_H

// see options.c
extern int verbose;
extern int auto_height;
extern int auto_width;
extern int width;
extern int height;
extern int use_border;
extern int invert;
extern int flipx;
extern int flipy;
extern int html;
extern int html_fontsize;
extern int colorfill;
extern int convert_grayscale;
extern const char *html_title;
extern int html_rawoutput;
extern int html_bold;
extern int debug;
extern int clearscr;
extern char ascii_palette[];
extern float redweight, greenweight, blueweight;
extern float RED[256], GREEN[256], BLUE[256], GRAY[256];
extern const char *fileout;
extern int usecolors;
extern int termfit;
extern int term_width;
extern int term_height;
#define TERM_FIT_ZOOM 1
#define TERM_FIT_WIDTH 2
#define TERM_FIT_HEIGHT 3
#define TERM_FIT_AUTO 4

#endif
