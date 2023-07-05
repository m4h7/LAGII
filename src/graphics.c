/*
	graphics.c
*/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "driver_main.h"
#include "events.h"
#include "font.h"
#include "graphics.h"
#include "lagii.h"
#include "logic.h"
#include "mem.h"
#include "picture.h"
#include "types.h"
#include "utility.h"


// video_buffer is actual screen, priority_buffer is depth map
// (and background_buffer stores the current background)
// (and tmp_buffer is a temporary storage buffer)
l_byte *video_buffer, *priority_buffer, *background_buffer, *tmp_buffer;
l_byte video_font[96][8];		// 96 chars, 8 bytes each
l_byte *real_priority_buffer;

void load_font (void)
{
	int i;

	for (i=0; i < 96; i++)
		memcpy (video_font[i], &agi_font[i * 8], 8);
}

// (x, y) = top-left corner, ch = ASCII code
// fgcol = foreground, bgcol = background
// buf = buffer to write on to.....{background,video}_buffer
void write_char (l_byte *buf, int x, int y, int ch, int fgcol, int bgcol)
{
	int i, j, c, q;
	l_byte *t;

	// Check validity of char (ch)
	if ((ch < 0x20) || (ch > 0x7F))
		return;

	t = video_font[(ch - 0x20)];
	for (i=0; i < 8; i++) {		// Loop through lines
		if ((y + i) >= 200)
			return;
		if ((y + i) < 0)
			continue;
		c = t[i];
		for (j=0; j < 8; j++) {
			if (((y + i) * 320 + (x + j)) >= (320 * 200))
				return;
			if (((y + i) * 320 + (x + j)) < 0)
				continue;
			q = c & (1 << j);
			if (q)
				pixel (buf, (x + j), (y + i)) = fgcol;
			else
				pixel (buf, (x + j), (y + i)) = bgcol;
		}
	}
}

void write_string (l_byte *buf, int x, int y, char *str, int fg, int bg)
{
	int i;

	for (i=0; i < strlen (str); i++) {
		write_char (buf, x, y, str[i], fg, bg);
		x += 8;
	}
}

// Draw a traditional Sierra message box, wait for key, then remove it
// (Returns ASCII code of key pressed)
// 'msg' must be null (0x00) terminated, and can include '\n'
// (If 'ignore' is true, ignore vm_variables[21] et al)

int mbox_x = -1, mbox_y = -1, mbox_width = -1;

int message_box (char *msg, boolean ignore)
{
	char *local_copy;
	int key;
	int i, j, len, c_height, c_width, llen, maxlen;
	int height, width, x1, y1, x2, y2, x, y;

	if (!msg)
		msg = "(null)";

	// Length of message
	len = strlen (msg);

	// Create a local copy
	local_copy = l_strdup (msg);
	msg = local_copy;

	// Sanity check
	if ((len > 10) && (mbox_width < 10))
		mbox_width = -1;

	// Default width is 29
	maxlen = 29;
	if (mbox_width > -1)
		maxlen = mbox_width;

        // Insert line-wraps
	llen = 0;
	for (i=0; i < len; i++) {
		llen++;
		if (msg[i] == '\n')
			llen = 0;
		if (llen > maxlen) {
			// Search backwards for a space
			for (j=i; j > 0; j--) {
				if (msg[j] == ' ') {
					msg[j] = '\n';
					i = j + 1;
					llen = 1;
					j = 0;
				}
			}
		}
	}

	// Count lines
	c_height = 1;
	for (i=0; i < len; i++) {
		if (msg[i] == '\n')
			c_height++;
	}

	// Find maximum line length
	c_width = 0;
	j = 0;
	for (i=0; i < len; i++) {
		if (msg[i] == '\n') {
			if (j > c_width)
				c_width = j;
			j = 0;
		} else
			j++;
	}
	// Catch last line
	if (j > c_width)
		c_width = j;

	// Requested differently?
	if ((mbox_width > -1) && (mbox_width > c_width))
		c_width = mbox_width;

	// Calculate pixel dimensions
	height = 9 * c_height;	// (normally 8, but +1 for line gap)
	width = 8 * c_width;
	if ((mbox_x == -1) || (mbox_y == -1)) {
		x1 = 160 - (width >> 1);
		x2 = 160 + (width >> 1);
		y1 = 100 - (height >> 1);
		// -1 to compensate fp trunc.
		y2 = 100 + ((height-1) >> 1);
	} else {
		x1 = mbox_x;
		x2 = x1 + width;
		y1 = mbox_y;
		y2 = y1 + height;
	}

	// Store current screen in temporary buffer
	memcpy (tmp_buffer, video_buffer, 320 * 200);

	// Draw message box onto screen
	for (y=(y1 - 4); y < (y2 + 5); y++) {
		for (x=(x1 - 5); x < (x2 + 6); x++)
			pixel (video_buffer, x, y) = 15;
	}

	// Red border
	for (x=(x1 - 3); x <= (x2 + 3); x++) {
		pixel (video_buffer, x, (y1 - 3)) = 4;
		pixel (video_buffer, x, (y2 + 3)) = 4;
	}

	// Sides (double width)
	for (y=(y1 - 3); y <= (y2 + 3); y++) {
		// left
		pixel (video_buffer, (x1 - 3), y) = 4;
		pixel (video_buffer, (x1 - 4), y) = 4;
		// right
		pixel (video_buffer, (x2 + 3), y) = 4;
		pixel (video_buffer, (x2 + 4), y) = 4;
	}

	// Write text
	x = 0;
	y = 0;
	for (i=0; i < len; i++) {
		if (msg[i] == '\n') {
			x = 0;
			y++;
		} else {
			write_char (video_buffer, (x * 8 + x1 + 1),
					(y * 9 + y1), msg[i], 0, 15);
			x++;
		}
	}

	// Copy to physical screen
	driver->blit (video_buffer);


if (decompile_toggle) {
	decomp ("MESSAGE BOX => \"");
	decomp (msg);
	decomp ("\"\n");
}


	key = 0;
	if ((vm_variables[21] == 0) || (ignore)) {
		// Wait for keystroke
		key = event_wait_for_key ();
	} else {
		if (!turbo_toggle)
			usleep (vm_variables[21] * 500 * 1000);
		else
			usleep (vm_variables[21] * 100 * 1000);
		vm_variables[21] = 0;
	}

	// Restore screen
	if ((!vm_flags[15]) || (ignore)) {
		memcpy (video_buffer, tmp_buffer, 320 * 200);
		driver->blit (video_buffer);
	}

	// Free local copy
	MemFree (local_copy);

	// Reset parameters
	mbox_x = -1;
	mbox_y = -1;
	mbox_width = -1;

	// Return
	return key;
}

// "Sierra correct" round function
int round (float num, float dirn)
{
	float fl_num, ce_num;

	fl_num = floor (num);
	ce_num = ceil (num);
	if (dirn < 0) {
		if ((num - fl_num) <= 0.501)
			return fl_num;
		else
			return ce_num;
	}
	if ((num - fl_num) < 0.499)
		return fl_num;
	else
		return ce_num;
}

#define swap(x,y)	\
	{		\
		int i;	\
		i = x;	\
		x = y;	\
		y = i;	\
	}

void draw_line (int x1, int y1, int x2, int y2)
{
	int rx, ry;
	float x, y, height, width;

	if (y1 == y2) {
		// Horizontal line
		if (x2 < x1)
			swap (x1, x2);
		for (rx=x1; rx <= x2; rx++) {
			draw_point (rx, y1);
		}
		return;
	} else if (x1 == x2) {
		// Vertical line
		if (y2 < y1)
			swap (y1, y2);
		for (ry=y1; ry <= y2; ry++) {
			draw_point (x1, ry);
		}
		return;
	}

	width = (x2 - x1);
	height = (y2 - y1);

	if (fabs (width) > fabs (height)) {
		if (x2 < x1) {
			swap (x1, x2);
			swap (y1, y2);
			width = -width;
			height = -height;
		}
		for (x=x1; x <= x2; x++) {
			y = ((x - x1) * height) / width + y1;
			rx = round (x, 1);
			ry = round (y, height);
			draw_point (rx, ry);
		}
	} else {
		if (y2 < y1) {
			swap (y1, y2);
			swap (x1, x2);
			height = -height;
			width = -width;
		}
		for (y=y1; y <= y2; y++) {
			x = ((y - y1) * width) / height + x1;
			ry = round (y, 1);
			rx = round (x, width);
			draw_point (rx, ry);
		}
	}
}

// Flood fill stuff
#define QMAX 8000
#define EMPTY -1

int queue[QMAX+1];
int rpos = 0, spos = 0;

void qstore (int q);
void qstore (int q)
{
	if (((spos + 1)==rpos) || (((spos + 1) == QMAX) && !rpos)) {
		DEBUG_MSG1 ("Queue overflow.\n");
		exit (1);
	}
	queue[spos] = q;
	spos++;
	if (spos == QMAX)
		spos = 0;
}

int qretrieve (void);
int qretrieve (void)
{
	if (rpos == QMAX)
		rpos=0;
	if (rpos == spos) {
		return EMPTY;
	}
	rpos++;
	return queue[rpos-1];
}

boolean okToFill (int x, int y);
boolean okToFill (int x, int y)
{
	if (!priority_draw) {
		if (pixel (background_buffer, x, y) == 15)
			return true;
		else
			return false;
	}
	if (!picture_draw) {
		if (pixel (priority_buffer, x, y) == 4)
			return true;
		else
			return false;
	}
	// At this point, (pic AND pri) == 1
	if (pixel (background_buffer, x, y) == 15)
		return true;

	return false;
}

void flood_fill (int x, int y)
{
	int x1, y1;
int cycle=0;

	rpos = spos = 0;

	if ((!picture_draw) && (!priority_draw))
		return;
	if ((!priority_draw) && (picture_color == 15))
		return;
	if ((!picture_draw) && (priority_color == 4))
		return;

	qstore (x);
	qstore (y);

	while (1) {
		x1 = qretrieve ();
		y1 = qretrieve ();

cycle++;
cycle %= 10;
//if (cycle == 0)
//driver->blit (background_buffer);
		if ((x1 == EMPTY) || (y1 == EMPTY))
			break;
		if (okToFill (x1, y1) == false)
			continue;

		draw_point (x1, y1);

		if (okToFill (x1, y1-1) && (y1 > 0)) {
			qstore (x1);
			qstore (y1-1);
		}
		if (okToFill (x1-1, y1) && (x1 > 0)) {
			qstore (x1-1);
			qstore (y1);
		}
		if (okToFill (x1+1, y1) && (x1 < 159)) {
			qstore (x1+1);
			qstore (y1);
		}
		if (okToFill (x1, y1+1) && (y1 < 167)) {
			qstore (x1);
			qstore (y1+1);
		}
	}

}

#define vpix(x,y)	pixel (video_buffer, x, y)
#define drop(col,y1,y2,num)					\
	if (num > 0) {						\
		for (i=y2; i >= y1; i--)			\
			vpix (col, i) = vpix (col, (i-num));	\
	} else {						\
		for (i=y1; i <= y2; i++)			\
			vpix (col, i) = vpix (col, (i-num));	\
	}
#define push(row,x1,x2,num)					\
	if (num > 0) {						\
		for (i=x2; i >= x1; i--)			\
			vpix (i, row) = vpix ((i-num), row);	\
	} else {						\
		for (i=x1; i <= x2; i++)			\
			vpix (i, row) = vpix ((i-num), row);	\
	}

void shake_screen (int times)
{
#define MAG 2
	int i, j, x, y, alt;
	l_byte topline[MAG][320], bottomline[MAG][320];
	l_byte leftline[MAG][200], rightline[MAG][200];

	// Save border (restores after shake)
	for (x=0; x < 320; x++)
		for (i=0; i < MAG; i++) {
			topline[i][x] = vpix (x, i);
			bottomline[i][x] = vpix (x, (319-i));
		}
	for (y=0; y < 200; y++)
		for (i=0; i < MAG; i++) {
			leftline[i][y] = vpix (i, y);
			rightline[i][y] = vpix ((199-i), y);
		}

	alt = -MAG;
	for (j=0; j < (8*times); j++) {
		alt = -alt;

		for (x=0; x < 320; x++)
			drop (x, MAG, 199-MAG, alt);
		driver->blit (video_buffer);
		usleep (20 * 1000);

		for (y=0; y < 200; y++)
			push (y, MAG, 319-MAG, alt);
		driver->blit (video_buffer);
		usleep (50 * 1000);
	}

	// Restore border
	for (x=0; x < 320; x++)
		for (i=0; i < MAG; i++) {
			vpix (x, i) = topline[i][x];
			vpix (x, (319-i)) = bottomline[i][x];
		}
	for (y=0; y < 200; y++)
		for (i=0; i < MAG; i++) {
			vpix (i, y) = leftline[i][y];
			vpix ((199-i), y) = rightline[i][y];
		}

	driver->blit (video_buffer);
}

// If (x,y) is a control pixel, search down (+y)
int priority_of_pixel (int x, int y)
{

	while (pixel (priority_buffer, x, y) < 4) {
		y++;
		if (y > 167)
			break;
	}

	return (pixel (priority_buffer, x, y));
}
