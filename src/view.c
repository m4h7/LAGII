/*
	view.c
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "driver_main.h"
#include "graphics.h"
#include "logic.h"
#include "mem.h"
#include "resource.h"
#include "text.h"
#include "types.h"
#include "view.h"

boolean no_pri_draw = false;

char *get_view_description (agi_resource_t *res)
{
	l_byte t[2], *data;
	char *ret;
	int pos;

	// Valid resource?
	if (!res) {
DEBUG_MSG1 ("FATAL - NULL resource.\n");
		return NULL;
	}

	// Load resource
	resource_load (res);
	data = res->data;

	// Read offset
	data = &res->data[3];
	memcpy (t, data, 2);
	pos = (t[1] << 8) + t[0];

	if (pos == 0) {
		// No description
		resource_unload (res);
		return NULL;
	}

	// Seek to description
	data = &res->data[pos];

	ret = (char *) MemAlloc (strlen ((char *) data) + 1);
	strcpy (ret, (char *) data);

	// Unload resource
	resource_unload (res);

	return ret;
}

// (x == y == -1) => Align centered at bottom (e.g. inventory item)
// NOTE: 'loop' and 'cel' count from zero!!!
// NOTE: If 'pri' == -1, priority is automatically calculated
void show_view (agi_resource_t *res, int x, int y, int loop, int cel,
								int pri)
{
	int loop_pos, cel_pos;
	l_byte *data, t[2];
	int number_of_loops, number_of_cels, bottom;
	int width, height, transparent, priority;
	l_byte *frame;
	int rx, ry, idx, xoff, yoff, xtrim;
	boolean mirrored;

	// Valid resource?
	if (!res) {
DEBUG_MSG1 ("FATAL - NULL resource.\n");
		return;
	}

	// Load resource
	resource_load (res);
	data = res->data;

	// Skip first two bytes
	data += 2;

	// Number of loops
	number_of_loops = *data++;
//DEBUG_MSG2 ("number_of_loops = %i\n", number_of_loops);
	if (number_of_loops <= loop) {
DEBUG_MSG1 ("FATAL - too few loops.\n");
		resource_unload (res);
		return;
	}

	// Skip description location (2 bytes)
	data += 2;

	// Skip irrelevant loops (2 bytes each)
	data += (2 * loop);

	// Position of nth loop (the one we want)
	t[0] = *data++;
	t[1] = *data++;
	loop_pos = (t[1] << 8) + t[0];

	// Seek to loop
	data = &res->data[loop_pos];

	// Number of cels in this loop
	number_of_cels = *data++;
//DEBUG_MSG2 ("number_of_cels = %i\n", number_of_cels);
	if (number_of_cels <= cel) {
DEBUG_MSG3 ("FATAL - too few cels. [got=%i, want=%i]\n",
					number_of_cels, cel);
		resource_unload (res);
		return;
	}

	// Skip irrelevant cels
	data += (2 * cel);

	// Read position of the cel we want
	t[0] = *data++;
	t[1] = *data++;
	cel_pos = (t[1] << 8) + t[0];

	// Seek to the cel
	data = &res->data[loop_pos + cel_pos];

	// Dimensions of cel (width, height);
	width = *data++;
	height = *data++;

	// Catch naughty cels
	if ((width * height) == 0) {
DEBUG_MSG2 ("FATAL - VIEW.%i: width * height == 0.\n", res->id);
		resource_unload (res);
		return;
	}

	// Center view?
	if ((x == -1) && (y == -1)) {
		x = (160 - width) / 2;
		y = 169 - height;
	} else {
		// Translate bottom-left coordinates to top-left
		y -= (height - 1);
	}

	// Calculate priority
	if (pri > -1)
		priority = pri;
	else {
		bottom = y + height - 1;
		priority = (bottom / 12) + 1;
	}

	// Allocate frame data
	frame = (l_byte *) MemAlloc (width * height);

	// Read transparency and mirroring info
	t[0] = *data++;
//DEBUG_MSG2 ("mirror/trans. byte = 0x%02X\n", t[0]);
	transparent = (t[0] & 0x0F);
	mirrored = false;
	if (t[0] & 0x80) {
		int norm_loop;
		norm_loop = ((t[0] & 0x70) >> 4);
		if (norm_loop != loop)
			mirrored = true;
	}

	// Start reading info
	memset (frame, transparent, (width * height));
//DEBUG_MSG4 ("width=%i height=%i trans=%i\n", width, height, transparent);
	for (ry=0; ry < height; ry++) {
		l_byte *line = &frame[ry * width], chunk;
		int col, len;

		while (1) {
			// Read in next chunk
			chunk = *data++;
			if (chunk == 0)
				break;

			// Decode, and fill
			col = (chunk & 0xF0) >> 4;
			len = (chunk & 0x0F);
			memset (line, col, len);
			line += len;
		}
	}

	// If mirrored, flip around vertical axis
#define normal_bit	frame[ry * width + rx]
#define opposite_bit	frame[ry * width + (width - rx - 1)]
	if (mirrored) {
		int t;
		for (ry=0; ry < height; ry++) {
			for (rx=0; rx < (width/2); rx++) {
				t = normal_bit;
				normal_bit = opposite_bit;
				opposite_bit = t;
			}
		}
	}

	// Clip to screen border
	xoff = 0;
	yoff = 0;
	xtrim = 0;
	// (1) Top
	if (y < 0)
		yoff = -y;
	// (2) Bottom
	if ((y + height) > 168)
		height = (168 - y);
	// (3) Left
	if (x < 0)
		xoff = -x;
	// (4) Right
	if ((x + width) > 160)
		xtrim = ((x + width) - 160);

	// Display it (finally!)
	idx = 0;

#define vid_addr	&pixel (video_buffer, (rx * 2), ry)
#define pri_addr	&pixel (priority_buffer, (rx * 2), ry)
#define pri_pix		(priority_of_pixel ((rx * 2), ry))

	for (ry=y+yoff; ry < (y + height); ry++) {
		idx = (ry - y) * width + xoff;
		for (rx=x+xoff; rx < (x + width - xtrim); rx++, idx++) {
			if (frame[idx] != transparent) {
				if (pri_pix <= priority) {
					memset (vid_addr, frame[idx], 2);
					if (!no_pri_draw)
						memset (pri_addr, priority, 2);
				}
// Xray
#if 0
else if (pri < 0)
	memset (vid_addr, 7, 2);
#endif
			}
		}
	}

	// Clean up memory
	MemFree (frame);

	// Unload resource
	resource_unload (res);
}

void show_inventory_view (agi_resource_t *res, logic_vm_t *vm)
{
	char *desc, *msg;

	// Get description
	desc = get_view_description (res);
	if (!desc)
		return;

	msg = text_interpret (vm, desc);
	MemFree (desc);

	// Save current screen
	memcpy (tmp_buffer, video_buffer, 320 * 200);

	// Put view onto screen
	no_pri_draw = true;
	show_view (res, -1, -1, 0, 0, -1);
	no_pri_draw = false;

	// Put message description up
	message_box (msg, true);

	// Restore view (updated in message_box)
	memcpy (video_buffer, tmp_buffer, 320 * 200);

	// Update graphics
	driver->blit (video_buffer);

	MemFree (msg);
}
