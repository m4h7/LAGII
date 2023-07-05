/*
	picture.c
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "driver_main.h"
	#include "graphics.h"
#include "picture.h"
#include "resource.h"
#include "types.h"


boolean picture_draw, priority_draw;
int picture_color, priority_color, brush;


// NOTE: This doubles the 'x' co-ordinates AT THE END!!
// (Sierra screen is 160x200, our screen is 320x200)
// (For flood fills, screen in 160x168)
void draw_picture (agi_resource_t *res, boolean clear_first)
{
	int ch, q;
	int x, y, dx, dy;
	l_byte *data;

	// Valid resource?
	if (!res)
		return;

	// Load resource
	resource_load (res);
	data = res->data;

	// Set up defaults
	picture_draw = false;
	picture_color = 0;
	priority_draw = false;
	priority_color = 0;
	brush = 0;

	// Set up screens right
	if (clear_first) {
		// Clear screen
		memset (background_buffer, 15, 320 * 200);
		memset (priority_buffer, 4, 320 * 200);
	} else {
		// Copy existing screen
		for (y=0; y < 168; y++) {
			q = y * 320;
			for (x=0; x < 160; x++) {
				dx = background_buffer[q + x*2];
				background_buffer[q + x] = dx;

				dy = priority_buffer[q + x*2];
				priority_buffer[q + x] = dy;
			}
		}
	}

	// Clean up strip down at the bottom
	memset (&background_buffer[168 * 320], 0, 32 * 320);

// Read through file
while (1) {
	// Find next action (ch >= 0xF0)
	ch = *data++;
	if (ch < 0xF0)
		continue;

	// Got an action, decipher

	if (ch == 0xFF) {
		// End of picture
//printf ("\n**End of picture **\n");
		break;
	}

	if (ch == 0xF0) {
		// Change picture color & enable picture draw
		picture_draw = true;
		picture_color = *data++;
		continue;
	}

	if (ch == 0xF1) {
		// Disable picture draw
		picture_draw = false;
		continue;
	}

	if (ch == 0xF2) {
		// Change priority color & enable priority draw
		priority_draw = true;
		priority_color = *data++;
		continue;
	}

	if (ch == 0xF3) {
		// Disable priority draw
		priority_draw = false;
		continue;
	}

	if (ch == 0xF4) {
		// Draw Y corner
		// Read start x & y
		x = *data++;
		y = *data++;
		while (1) {
			// Change Y first
			q = *data++;
			if (q >= 0xF0)
				break;
			dy = q;
			draw_line (x, y, x, q);
			y = q;
			// Change X next
			q = *data++;
			if (q >= 0xF0)
				break;
			dx = q;
			draw_line (x, y, q, y);
			x = q;
		}
		// Rewind
		data--;
		continue;
	}

	if (ch == 0xF5) {
		// Draw X corner
		// Read start x & y
		x = *data++;
		y = *data++;
		while (1) {
			// Change X first
			q = *data++;
			if (q >= 0xF0)
				break;
			dx = q;
			draw_line (x, y, q, y);
			x = q;
			// Change Y next
			q = *data++;
			if (q >= 0xF0)
				break;
			dy = q;
			draw_line (x, y, x, q);
			y = q;
		}
		// Rewind
		data--;
		continue;
	}

	if (ch == 0xF6) {
		// Absolute line (long lines)
		// Read starting x & y
		x = *data++;
		y = *data++;
		while (1) {
			// Get x
			dx = *data++;
			if (dx >= 0xF0)
				break;
			// Get y
			dy = *data++;
			// Draw line
			draw_line (x, y, dx, dy);
			x = dx;
			y = dy;
		}
		// Rewind
		data--;
		continue;
	}

	if (ch == 0xF7) {
		// Relative line (short lines)
		// Start position
		x = *data++;
		y = *data++;
		while (1) {
			q = *data++;
			if (q >= 0xF0)
				break;
			// First nybble is dX (signed)
			dx = (q & 0x70) >> 4;
			if (q & 0x80)
				dx = -dx;
			// Second nybble is dY (signed)
			dy = (q & 0x07);
			if (q & 0x08)
				dy = -dy;
			// Update
			dx += x;
			dy += y;
			draw_line (x, y, dx, dy);
			x = dx;
			y = dy;
		}
		// Rewind
		data--;
		continue;
	}

	if (ch == 0xF8) {
		// Flood fill
		while (1) {
			x = *data++;
			if (x >= 0xF0)
				break;
			y = *data++;
			flood_fill (x, y);
		}

		// Rewind
		data--;
		continue;
	}

	if (ch == 0xF9) {
		// Change pen size & style
		brush = *data++;
		continue;
	}

	if (ch == 0xFA) {
		// Plot with pen
//printf ("<PLOT>\n");
		while (1) {
			if (brush & 0x20) {
				// Splatter
//printf ("\b-SPLATTER>");
				q = *data++;
				if (q >= 0xF0)
					break;
				x = *data++;
				y = *data++;
				plot_pattern (x, y, q);
			} else {
				// Solid
				x = *data++;
				if (x >= 0xF0)
					break;
				y = *data++;
				plot_pattern (x, y, 0);
			}
		}

		// Rewind
		data--;
		continue;
	}

	printf ("\n## Skipping byte 0x%.2X\n", ch);

} // end of "while (1)"

	// Unload resource
	resource_unload (res);

	// To finish it off, expand each 160-pixel line to 320-pixels!
	// (rows 168 -> 200 are plain black!)
	for (y=0; y < 168; y++) {
		q = y * 320;
		for (x=159; x >= 0; x--) {
			dx = background_buffer[q + x];
			background_buffer[q + x*2] = dx;
			background_buffer[q + x*2 + 1] = dx;

			dy = priority_buffer[q + x];
			priority_buffer[q + x*2] = dy;
			priority_buffer[q + x*2 + 1] = dy;
		}
	}
}

static l_byte circle_data[][15] = {
	{0x01},
	{0x03, 0x03, 0x03},
	{0x02, 0x07, 0x07, 0x07, 0x02},
	{0x06, 0x06, 0x0F, 0x0F, 0x0F, 0x06, 0x06},
	{0x04, 0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x04},
	{0x0C, 0x1E, 0x1E, 0x1E, 0x3F, 0x3F, 0x3F, 0x1E, 0x1E, 0x1E,
							0x0C},
	{0x1C, 0x3E, 0x3E, 0x3E, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x3E,
						0x3E, 0x3E, 0x1C},
	{0x81, 0xC3, 0xE7, 0xE7, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
					0xE7, 0xE7, 0xE7, 0xC3, 0x81}
};

// FIXME: This ignores the pattern!!
void plot_pattern (int x, int y, int pattern)
{
	int size, x1, y1, x2, y2, rx, ry, h, w;

	size = (brush & 0x07);
	x1 = x - ((size + 1) / 2);
	x2 = x + (size / 2);
	y1 = y - size;
	y2 = y + size;

	if (brush & 0x10) {
		// Rectangle
		for (ry=y1; ry <= y2; ry++) {
			for (rx=x1; rx <= x2; rx++)
				draw_point (rx, ry);
		}
	} else {
		// Circle
		for (h=0, ry=y1; h < (2 * size + 1); h++, ry++) {
			int ch;
			ch = circle_data[size][h];
			for (w=0, rx=x1; w <= size; w++, rx++) {
				if (ch & (1 << w))
					draw_point (rx, ry);
			}
		}
	}
}
