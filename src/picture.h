/*
	picture.h
*/

#ifndef __PICTURE_H__
#define __PICTURE_H__

#include "graphics.h"
#include "resource.h"
#include "types.h"

//
// DEFINES
//

#define draw_point(x,y)							\
	if (1) {							\
		if (picture_draw)					\
			pixel (background_buffer, x, y) = picture_color;\
		if (priority_draw)					\
			pixel (priority_buffer, x, y) = priority_color;	\
	}

//
// FUNCTIONS
//

void draw_picture (agi_resource_t *res, boolean clear_first);
void plot_pattern (int x, int y, int pattern);

//
// EXTERNALS
//

extern boolean picture_draw, priority_draw;
extern int picture_color, priority_color, brush;

#endif	// __PICTURE_H__
