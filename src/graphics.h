/*
	graphics.h
*/

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "types.h"

//
// DEFINES
//

// Utility macros
#define sign(x)         ((x) < 0 ? -1 : ((x) > 0 ? 1: 0))

#define pixel(array,x,y)	array[y * 320 + x]

//
// FUNCTIONS
//

void load_font (void);
void write_char (l_byte *buf, int x, int y, int ch, int fgcol, int bgcol);
void write_string (l_byte *buf, int x, int y, char *str, int fg, int bg);
int message_box (char *msg, boolean ignore);
int round (float num, float dirn);
void draw_line (int x1, int y1, int x2, int y2);
void flood_fill (int x, int y);
void shake_screen (int times);
int priority_of_pixel (int x, int y);

//
// EXTERNALS
//

extern l_byte *video_buffer, *priority_buffer;
extern l_byte *background_buffer, *tmp_buffer;
extern l_byte video_font[96][8], *real_priority_buffer;

extern int mbox_x, mbox_y, mbox_width;

#endif	// __GRAPHICS_H__
