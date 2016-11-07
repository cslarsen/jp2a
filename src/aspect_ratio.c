/*
 * Copyright 2006-2016 Christian Stigen Larsen
 * Distributed under the GNU General Public License (GPL) v2.
 */

#include "options.h"
#include "round.h"

// Calculate width or height, but not both
void aspect_ratio(const int jpeg_width, const int jpeg_height) {

	// the 2.0f and 0.5f factors are used for text displays that (usually) have characters
	// that are taller than they are wide.

	#define CALC_WIDTH ROUND(2.0f * (float) height * (float) jpeg_width / (float) jpeg_height)
	#define CALC_HEIGHT ROUND(0.5f * (float) width * (float) jpeg_height / (float) jpeg_width)

	// calc width
	if ( auto_width && !auto_height ) {
		width = CALC_WIDTH;

		// adjust for too small dimensions	
		while ( width==0 ) {
			++height;
			aspect_ratio(jpeg_width, jpeg_height);
		}
		
		while ( termfit==TERM_FIT_AUTO && (width + use_border*2)>term_width ) {
			width = term_width - use_border*2;
			height = 0;
			auto_height = 1;
			auto_width = 0;
			aspect_ratio(jpeg_width, jpeg_height);
		}

	}

	// calc height
	if ( !auto_width && auto_height ) {
		height = CALC_HEIGHT;

		// adjust for too small dimensions
		while ( height==0 ) {
			++width;
			aspect_ratio(jpeg_width, jpeg_height);
		}
	}
}
