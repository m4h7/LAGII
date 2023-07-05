/*
	driver_svgalib.c
*/

#include <stdio.h>
#include <vga.h>
#include <vgagl.h>
#include <vgakeyboard.h>

#include "driver_main.h"
#include "driver_svgalib.h"
#include "events.h"
#include "types.h"

l_byte ega_palette[16][3] = {
	{0, 0, 0}, {0, 0, 42}, {0, 42, 0}, {0, 42, 42},
	{42, 0, 0}, {42, 0, 42}, {42, 21, 0}, {42, 42, 42},
	{21, 21, 21}, {21, 21, 63}, {21, 63, 21}, {21, 63, 63},
	{63, 21, 21}, {63, 21, 63}, {63, 63, 21}, {63, 63, 63}
};

int svgalib_init (void)
{
	int err, i;

	err = vga_init ();
	if (err) {
		printf ("Unable to initialize svgalib.\n");
		return -1;
	}

	err = vga_hasmode (G320x200x256);
	if (err == 0) {
		printf ("Video mode unavailable (320x200x256).\n");
		return -1;
	}

	vga_setmode (G320x200x256);
	gl_setcontextvga (G320x200x256);

	// Set up EGA colors
	for (i=0; i < 16; i++)
		gl_setpalettecolor (i, ega_palette[i][0],
			ega_palette[i][1], ega_palette[i][2]);

	return 0;
}

void svgalib_close (void)
{
	vga_setmode (TEXT);
}

void svgalib_blit (l_byte *buf)
{
	gl_putbox (0, 0, 320, 200, buf);
}

void svgalib_key_init (void)
{
	int err;

	err = keyboard_init ();
	if (err) {
		printf ("Couldn't init. keyboard!\n");
		exit (1);
	}

	// Unify enter keys
	keyboard_translatekeys (TRANSLATE_KEYPADENTER);

	keyboard_seteventhandler (svgalib_key_handler);
	keyboard_clearstate ();
}

void svgalib_key_close (void)
{
	keyboard_setdefaulteventhandler ();
	keyboard_clearstate ();
	keyboard_translatekeys (0);
	keyboard_close ();
}

void svgalib_key_flush (void)
{
	keyboard_update ();
	keyboard_clearstate ();
}

void svgalib_key_update (void)
{
	keyboard_update ();
}

void svgalib_key_handler (int scancode, int press)
{
	event_key_handler (scancode, press);
}
