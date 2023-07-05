/*
	driver_main.c
*/

#include <stdio.h>
#include <stdlib.h>

#include "driver_main.h"
#include "driver_svgalib.h"

#include "graphics.h"
#include "mem.h"
#include "types.h"
#include "version.h"

//==================
// MAIN DRIVER LIST
//==================

driver_t driver_list[] = {

	// svgalib driver
	{
		"svgalib",
		svgalib_init,
		svgalib_close,
		svgalib_blit,
		svgalib_key_init,
		svgalib_key_close,
		svgalib_key_flush,
		svgalib_key_update
	},
};

driver_t *driver;

void driver_init (int reserved)
{
	driver = &driver_list[0];

	// Allocate video buffer
	video_buffer = (l_byte *) MemAlloc (320 * 200);
	priority_buffer = (l_byte *) MemAlloc (320 * 200);
	background_buffer = (l_byte *) MemAlloc (320 * 200);
	tmp_buffer = (l_byte *) MemAlloc (320 * 200);

	real_priority_buffer = priority_buffer;

	// Load font
	load_font ();

	driver_title_screen ();
}

void driver_close (void)
{
	MemFree (video_buffer);
	MemFree (priority_buffer);
	MemFree (background_buffer);
	MemFree (tmp_buffer);
}

void driver_title_screen (void)
{
	char *txt = LAGII_TITLE;
	int i, j;

	for (i=0; i < strlen (txt); i++)
		write_char (video_buffer, (i * 8), 0, txt[i], 15, 0);

	// Expand down (x8)
	for (j=63; j >= 0; j--) {
		for (i=0; i < 40; i++)
			pixel (video_buffer, i, j) =
				pixel (video_buffer, i, (j / 8));
	}

	// Expand right (x8)
	for (i=319; i >= 0; i--) {
		for (j=0; j < 64; j++)
			pixel (video_buffer, i, j) =
				pixel (video_buffer, (i / 8), j);
	}
}
