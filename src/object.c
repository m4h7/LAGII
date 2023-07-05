/*
	object.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "graphics.h"
#include "lagii.h"
#include "logic.h"
#include "mem.h"
#include "object.h"
#include "resource.h"
#include "types.h"
#include "utility.h"
#include "view.h"


agi_object_t agi_objects[256];
agi_inventory_object_t agi_inventory_objects[256];


// 'data_dir' is directory of AGI files, NO TRAILING SLASHES!!
// (Doesn't return on failure)
void object_init (void)
{
	int i, count, size, name_offset;
	l_byte *data, *this;
	char *fname;
	FILE *f;

	for (i=0; i < 256; i++) {
		memset (&agi_objects[i], 0, sizeof (agi_object_t));
		agi_objects[i].flags = 0;
	}

	fname = (char *) MemAlloc (strlen (data_dir) + 14);
	sprintf (fname, "%s/object", data_dir);
	f = fopen (fname, "rb");
	if (!f) {
		DEBUG_MSG2 ("FATAL - Can't open '%s'.\n", fname);
		exit (1);
	}

	// Load file into memory
	fseek (f, 0, SEEK_END);
	size = ftell (f);
	fseek (f, 0, SEEK_SET);
	data = (l_byte *) MemAlloc (size);
	fread (data, 1, size, f);
	fclose (f);

	// Is file encrypted?
	count = (data[1] << 8) + data[0];
	if (count > size) {
		// Decrypt whole file with "Avis Durgan"
		l_byte *avis = (l_byte *) "Avis Durgan";
		for (i=0; i < size; i++)
			data[i] ^= avis[i % 11];
	}

	// Each entry is 3 bytes, so:
	count = ((data[1] << 8) + data[0]) / 3;

	// Process each entry
	for (i=0; i < count; i++) {
		this = &data[3 * i + 3];
		name_offset = (this[1] << 8) + this[0] + 3;
		agi_inventory_objects[i].room = this[2];
		agi_inventory_objects[i].name =
				l_strdup ((char *) &data[name_offset]);
	}

	// Clean-up
	MemFree (data);
	MemFree (fname);
}

// Update num_loops, num_cels, xsize, ysize
void object_recalc (agi_object_t *ob)
{
	agi_resource_t *res;
	l_byte *data;
	int offset;

	res = VIEW[ob->current_view];

	if (!res)
		return;

	resource_load (res);    // extra check
	data = res->data;
	if (!data)
		return;

	// Skip first two bytes
	data += 2;

	ob->num_loops = *data++;

	if (ob->current_loop >= ob->num_loops)
		ob->current_loop = 0;

	data += 2;              // skip description offset
	data += (2 * ob->current_loop);
	offset = (data[1] << 8) + data[0];
	data = &res->data[offset];
	ob->num_cels = *data++;

	if (ob->current_cel >= ob->num_cels)
		ob->current_cel = 0;

	data += (2 * ob->current_cel);
	offset += ((data[1] << 8) + data[0]);
	if ((offset + 2) > res->length) {
		// Erg -- somethings gone wrong (like in PQ1)
		ob->xsize = 0;
		ob->ysize = 0;
		return;
	}
	data = &res->data[offset];
	ob->xsize = *data++;
	ob->ysize = *data++;
}

void object_update (agi_object_t *ob)
{
	int priority;

	if (!(ob->flags & F_ANIMATE))
		return;
	if (!VIEW[ob->current_view])
		return;

	if (ob->step_time < 1)
		ob->step_time = 1;
	if (ob->cycle_time < 1)
		ob->cycle_time = 1;

	// Update object
	object_recalc (ob);

	// (1) Am I auto-picking the loop?
	if (ob->flags & F_AUTO_LOOP) {
		if ((ob->num_loops > 1) && (ob->num_loops < 4)) {
			switch (ob->direction) {
				case 2: case 3: case 4:
					ob->current_loop = 0;
					break;
				case 6: case 7: case 8:
					ob->current_loop = 1;
					break;
			}
		} else if (ob->num_loops >= 4) {
			switch (ob->direction) {
				case 1:
					ob->current_loop = 3;
					break;
				case 2: case 3: case 4:
					ob->current_loop = 0;
					break;
				case 5:
					ob->current_loop = 2;
					break;
				case 6: case 7: case 8:
					ob->current_loop = 1;
					break;
			}
		}
		object_recalc (ob);
	}

	// (2) Have I finished the one cycle?
	if ((ob->do_once > 0) &&
	    ((cycle_count % ob->cycle_time) == 0)) {
		boolean stop = false;

//DEBUG_MSG4 ("* 1xCycle %s-- cel: %i/%i\n",
//	((ob->flags & F_REVERSE) ? "Reverse " : ""),
//	ob->current_cel, ob->num_cels);

		if (ob->flags & F_REVERSE) {
			if (ob->current_cel < 1)
				stop = true;
		} else {
			if ((ob->current_cel + 1) >= ob->num_cels)
				stop = true;
		}
		if (stop) {
//DEBUG_MSG1 ("  %% stopped.\n");
			ob->flags &= ~(F_AUTO_CYCLE | F_REVERSE);
			vm_flags[ob->do_once] = true;
			ob->do_once = 0;
		}
	}

	// (3) Am I auto-cycling?
	if ((ob->flags & F_AUTO_CYCLE) &&
	    ((cycle_count % ob->cycle_time) == 0)) {

//DEBUG_MSG2 ("* Auto-cycling object %i.....\n", (ob - agi_objects));
//DEBUG_MSG5 ("  loop: %i/%i   cel: %i/%i\n", ob->current_loop,
//			ob->num_loops, ob->current_cel, ob->num_cels);
//DEBUG_MSG3 ("  cycle_count: %i  cycle_time: %i\n",
//			cycle_count, ob->cycle_time);

		if (ob->flags & F_REVERSE) {
			if ((ob->current_cel == 0) && (ob->num_cels > 0))
				ob->current_cel = ob->num_cels;
			ob->current_cel--;
		} else
			ob->current_cel++;
//		ob->current_cel %= ob->num_cels;
		if (ob->current_cel >= ob->num_cels)
			ob->current_cel = 0;
		object_recalc (ob);
	}

	// (4) Auto-move object?
	if ((ob->flags & F_AUTO_MOVE) &&
	    (ob->motion) &&
	    ((cycle_count % ob->step_time) == 0)) {

//DEBUG_MSG2 ("* Auto-moving object %i.....\n", (ob - agi_objects));
//DEBUG_MSG5 ("  now: {%i,%i}  target: {%i,%i}\n", ob->x, ob->y,
//							ob->tx, ob->ty);
//DEBUG_MSG4 ("  cycle_count: %i  step_size: %i  step_time: %i\n",
//			cycle_count, ob->step_size, ob->step_time);

		// Calculate direction, towards (ob->tx, ob->ty)
		object_recalc_direction (ob);
		if (ob->step_size < 1)
			ob->step_size = 1;
	}

	// Wandering?
	if ((ob->flags & F_WANDER) &&
	    ((cycle_count % ob->step_time) == 0))
		object_wander (ob);

	// Following Ego?
	if ((ob->flags & F_FOLLOW_EGO) &&
	    ((cycle_count % ob->step_time) == 0))
		object_follow_ego (ob);

	if ((ob->step_size > 0) &&
	    (ob->motion) &&
	    ((cycle_count % ob->step_time) == 0)) {
		int full, half;

		// FIXME: Is diagonal movement correct here?

		full = ob->step_size;
		half = full / 2;
		// Round up half every second update
		if (full == 1)
			half = 1;
		if ((half * 2) < full) {
			ob->half_move++;
			ob->half_move %= 2;
			if (ob->half_move == 0)
				half++;
		}
		ob->oldx = ob->x;
		ob->oldy = ob->y;
		switch (ob->direction) {
			case 1:
				object_move (ob, 0, -full);
				break;
			case 2:
				object_move (ob, half, -half);
				break;
			case 3:
				object_move (ob, full, 0);
				break;
			case 4:
				object_move (ob, half, half);
				break;
			case 5:
				object_move (ob, 0, full);
				break;
			case 6:
				object_move (ob, -half, half);
				break;
			case 7:
				object_move (ob, -full, 0);
				break;
			case 8:
				object_move (ob, -half, -half);
				break;
		}
	}

	// Wandering object hit border?
	if ((ob->flags & F_WANDER) &&
	    (ob->x == ob->oldx) && (ob->y == ob->oldy)) {
		ob->direction = 0;
	}

	// Ego walked into barrier?
	if (((ob - agi_objects) == 0) && !(ob->flags & F_AUTO_MOVE)) {
		if ((ob->x == ob->oldx) && (ob->y == ob->oldy))
			// Stop moving
			ob->direction = 0;
	}

	if (ob->flags & F_AUTO_MOVE) {
		// Check for over-shoot, and automatically quantize
		if (ob->x > ob->tx)
			if (ob->direction < 6)
				ob->x = ob->tx;
		if (ob->x < ob->tx)
			if ((ob->direction < 2) || (ob->direction > 4))
				ob->x = ob->tx;
		if (ob->y > ob->ty)
			if ((ob->direction > 2) && (ob->direction < 8))
				ob->y = ob->ty;
		if (ob->y < ob->ty)
			if ((ob->direction < 4) || (ob->direction > 6))
				ob->y = ob->ty;

		// Got to target, yet?
		if ((ob->x == ob->tx) && (ob->y == ob->ty)) {
			ob->flags &= ~F_AUTO_MOVE;
			ob->motion = false;
if (ob->step_size > 1)
ob->step_size = 1;
			vm_flags[ob->target_flag] = true;
		}
	}

	// (5) Draw the object
	if (ob->priority < 4)
		ob->priority = 4;
	if (ob->flags & F_AUTO_PRI)
		ob->priority = (ob->y / 12) + 1;
	priority = ob->priority;

	if (ob->flags & (F_UPDATE_ME | F_DRAW_ME)) {
//DEBUG_MSG4 ("Object %i -- priority = %i [flags=0x%04X]\n",
//(ob - agi_objects), priority, ob->flags);
		show_view (VIEW[ob->current_view], ob->x, ob->y,
			ob->current_loop, ob->current_cel, priority);

		if (obj_toggle) {
			int num = (ob - agi_objects), i;
			char buf[40];

			sprintf (buf, "%i (0x%02X,%02i,%02i)\n",
					num, ob->current_view,
					ob->current_loop,
					ob->current_cel);
			for (i=0; i < strlen (buf); i++)
				write_char (video_buffer, (ob->x*2) +
					(i*8), ob->y, buf[i], 15, 0);

//			if (num == 0) {
{
				sprintf (buf, "f=0x%04X,d=%i,p=%i",
					ob->flags, ob->direction,
							ob->priority);
				for (i=0; i < strlen (buf); i++)
					write_char (video_buffer,
						(ob->x*2) + (i*8),
						(ob->y+8), buf[i], 15, 0);
			}
//DEBUG_MSG4 ("Object %i -- @ {%i,%i}\n", (ob - agi_objects), ob->x, ob->y);
		}
	}
}


#ifndef sign
#define sign(x)		((x) < 0 ? -1 : ((x) > 0 ? 1: 0))
#endif

void object_recalc_direction (agi_object_t *ob)
{
	int dx, dy;
	int map_table[3][3] = {
		{8, 1, 2}, {7, 0, 3}, {6, 5, 4}
	};

	if (ob->flags & F_AUTO_MOVE) {
		dx = sign (ob->tx - ob->x) + 1;
		dy = sign (ob->ty - ob->y) + 1;
		ob->direction = map_table[dy][dx];
	}
}

void object_wander (agi_object_t *ob)
{
	if (ob->direction == 0)
		ob->direction = (random () % 8) + 1;
}

void object_follow_ego (agi_object_t *ob)
{
	ob->tx = agi_objects[0].x;
	ob->ty = agi_objects[0].y;
	object_recalc_direction (ob);
}

#define stop_pt(x,y)	\
	((ob->flags & F_OBSERVE_BLOCKS) ?	\
	 (pixel (real_priority_buffer, x*2, y) < 2) :	\
	 (pixel (real_priority_buffer, x*2, y) == 0))

#define alarm_pt(x,y)	\
	((id == 0) ?	\
	 (pixel (real_priority_buffer, x*2, y) == 2) : 0)

#define test_pt(x,y)					\
	{						\
		if (stop_pt (x, y))			\
			return;				\
		if (alarm_pt (x, y))			\
			vm_flags[3] = true;		\
	}

void object_move (agi_object_t *ob, int dx, int dy)
{
	int x, y, rx, ry, dir, id;
	int map_table[3][3] = {
		{8, 1, 2}, {7, 0, 3}, {6, 5, 4}
	};

	// Get proper direction
	dir = map_table[sign (dy) + 1][sign (dx) + 1];

	if (dir == 0)
		return;		// Not moving?!?

	// ID of object
	id = (ob - agi_objects);

	// Determine point to examine
	if (dir < 5)
		rx = (ob->x + ob->xsize - 1);
	else
		rx = ob->x;
	ry = ob->y;

	// First, clip to horizon
	if (ob->flags & F_OBSERVE_HORIZON) {
		if ((ry + dy) <= agi_horizon) {
			if (id == 0)
				vm_variables[2] = 1;
			return;
		}
	}

	// Second, clip to bottom of screen
	if ((ry + dy) > 167) {
		if (id == 0)
			vm_variables[2] = 3;
		return;
	}

	// Third, check for contact with either side of screen
	if ((rx + dx) < 1) {
		// Left edge
		if (id == 0)
			vm_variables[2] = 4;
		return;
	}
	if ((rx + dx) > 159) {
		// Right edge
		if (id == 0)
			vm_variables[2] = 2;
		return;
	}

	// Fourth, check for control lines
	// (black == unconditional, etc.)
	if (dir == 1) {
		for (y=ry; y >= (ry + dy); y--)
			test_pt (rx, y);
	} else if (dir == 3) {
		for (x=rx; x <= (rx + dx); x++)
			test_pt (x, ry);
	} else if (dir == 5) {
		for (y=ry; y <= (ry + dy); y++)
			test_pt (rx, y);
	} else if (dir == 7) {
		for (x=rx; x >= (rx + dx); x--)
			test_pt (x, ry);
	} else if ((dir == 2) || (dir == 4)) {
		for (x=rx; x <= (rx + dx); x++) {
			y = ((x - rx) * dy) / dx + ry;
			test_pt (x, y)
		}
	} else if ((dir == 6) || (dir == 8)) {
		for (x=rx; x >= (rx + dx); x--) {
			y = ((x - rx) * dy) / dx + ry;
			test_pt (x, y)
		}
	}

	// If there's no problems, move the object
	ob->x += dx;
	ob->y += dy;
}
