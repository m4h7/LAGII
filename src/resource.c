/*
	resource.c
*/

#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "lagii.h"
#include "logic.h"
#include "lzw.h"
#include "mem.h"
#include "resource.h"


agi_resource_t *PICTURE[256], *VIEW[256], *LOGIC[256], *SOUND[256];


// 'data_dir' is directory of AGI files, NO TRAILING SLASHES!!
// (Doesn't return on failure)
void resource_init (void)
{
	l_byte buf[3];
	char *fname;
	FILE *f;
	int count, off;

	// NULLify all resources
	for (count=0; count < 256; count++) {
		PICTURE[count] = NULL;
		VIEW[count] = NULL;
		LOGIC[count] = NULL;
		SOUND[count] = NULL;
	}

	fname = (char *) MemAlloc (strlen (data_dir) + 15);

	// (1) Process picdir
	if (is_v3) {
		sprintf (fname, "%s/%sdir", data_dir, v3_short);
	} else {
		sprintf (fname, "%s/picdir", data_dir);
	}
	f = fopen (fname, "rb");
	if (!f) {
		DEBUG_MSG2 ("FATAL - Can't open '%s'.\n", fname);
		exit (1);
	}

	if (is_v3) {
		fseek (f, 2, SEEK_SET);
		fread (buf, 1, 2, f);
		off = (buf[1] << 8) + buf[0];
		fseek (f, off, SEEK_SET);
	}

	count = -1;
	while (!feof (f)) {
		fread (buf, 1, 3, f);
		count++;
		PICTURE[count] = NULL;

		// We should check all 3 bytes, but is there a VOL.15?!?
		if (buf[0] == 0xFF)
			continue;

		// Allocate mem.
		allocate_resource_mem (PICTURE, count);
		PICTURE[count]->type = picture;
		PICTURE[count]->vol = (buf[0] & 0xF0) >> 4;
		PICTURE[count]->offset = ((buf[0] & 0x0F) << 16)
					+ (buf[1] << 8) + buf[2];
	}
//DEBUG_MSG2 ("*-> %i PICTURE's\n", count+1);
	fclose (f);

	// (2) Process viewdir
	if (is_v3) {
		sprintf (fname, "%s/%sdir", data_dir, v3_short);
	} else {
		sprintf (fname, "%s/viewdir", data_dir);
	}
	f = fopen (fname, "rb");
	if (!f) {
		DEBUG_MSG2 ("FATAL - Can't open '%s'.\n", fname);
		exit (1);
	}

	if (is_v3) {
		fseek (f, 4, SEEK_SET);
		fread (buf, 1, 2, f);
		off = (buf[1] << 8) + buf[0];
		fseek (f, off, SEEK_SET);
	}

	count = -1;
	while (!feof (f)) {
		fread (buf, 1, 3, f);
		count++;
		VIEW[count] = NULL;

		// We should check all 3 bytes, but is there a VOL.15?!?
		if (buf[0] == 0xFF)
			continue;

		// Allocate mem.
		allocate_resource_mem (VIEW, count);
		VIEW[count]->type = view;
		VIEW[count]->vol = (buf[0] & 0xF0) >> 4;
		VIEW[count]->offset = ((buf[0] & 0x0F) << 16)
					+ (buf[1] << 8) + buf[2];
	}
//DEBUG_MSG2 ("*-> %i VIEW's\n", count+1);
	fclose (f);

	// (3) Process logdir
	if (is_v3) {
		sprintf (fname, "%s/%sdir", data_dir, v3_short);
	} else {
		sprintf (fname, "%s/logdir", data_dir);
	}
	f = fopen (fname, "rb");
	if (!f) {
		DEBUG_MSG2 ("FATAL - Can't open '%s'.\n", fname);
		exit (1);
	}

	if (is_v3) {
		fseek (f, 0, SEEK_SET);
		fread (buf, 1, 2, f);
		off = (buf[1] << 8) + buf[0];
		fseek (f, off, SEEK_SET);
	}

	count = -1;
	while (!feof (f)) {
		fread (buf, 1, 3, f);
		count++;
		LOGIC[count] = NULL;

		// We should check all 3 bytes, but is there a VOL.15?!?
		if (buf[0] == 0xFF)
			continue;

		// Allocate mem.
		allocate_resource_mem (LOGIC, count);
		LOGIC[count]->type = logic;
		LOGIC[count]->vol = (buf[0] & 0xF0) >> 4;
		LOGIC[count]->offset = ((buf[0] & 0x0F) << 16)
					+ (buf[1] << 8) + buf[2];
		LOGIC[count]->ssIP = 0;
	}
//DEBUG_MSG2 ("*-> %i LOGIC's\n", count+1);
	fclose (f);

	// (4) Process snddir
	if (is_v3) {
		sprintf (fname, "%s/%sdir", data_dir, v3_short);
	} else {
		sprintf (fname, "%s/snddir", data_dir);
	}
	f = fopen (fname, "rb");
	if (!f) {
		DEBUG_MSG2 ("FATAL - Can't open '%s'.\n", fname);
		exit (1);
	}

	if (is_v3) {
		fseek (f, 6, SEEK_SET);
		fread (buf, 1, 2, f);
		off = (buf[1] << 8) + buf[0];
		fseek (f, off, SEEK_SET);
	}

	count = -1;
	while (!feof (f)) {
		fread (buf, 1, 3, f);
		count++;
		SOUND[count] = NULL;

		// We should check all 3 bytes, but is there a VOL.15?!?
		if (buf[0] == 0xFF)
			continue;

		// Allocate mem.
		allocate_resource_mem (SOUND, count);
		SOUND[count]->type = sound;
		SOUND[count]->vol = (buf[0] & 0xF0) >> 4;
		SOUND[count]->offset = ((buf[0] & 0x0F) << 16)
					+ (buf[1] << 8) + buf[2];
	}
//DEBUG_MSG2 ("*-> %i SOUND's\n", count+1);
	fclose (f);

	// Clean up
	MemFree (fname);

	// Return
	return;
}

void resource_load (agi_resource_t *res)
{
	FILE *f;
	char *fname;
	l_byte buf[5];
	int comp_size;

	// Already cached?
	if (res->cached)
		return;

	// Open file
	fname = (char *) MemAlloc (strlen (data_dir) + 12);

	if (is_v3)
		sprintf (fname, "%s/%svol.%i", data_dir, v3_short,
							res->vol);
	else
		sprintf (fname, "%s/vol.%i", data_dir, res->vol);

	if (!volumes[res->vol])
		volumes[res->vol] = fopen (fname, "rb");
	f = volumes[res->vol];
	if (!f) {
		DEBUG_MSG2 ("Can't open '%s'.\n", fname);
		exit (1);
	}

	// Seek to position
	fseek (f, res->offset, SEEK_SET);

	// Read header
	fread (buf, 1, 5, f);
	res->length = (buf[4] << 8) + buf[3];

	// Quick check
	if (res->length < 1) {
		res->length = 0;
		res->data = (l_byte *) MemAlloc (2);
		res->cached = true;
		fclose (f);
		return;
	}

	// Allocate memory for it
	res->data = (l_byte *) MemAlloc (res->length);

	// Read whole file in
	if (is_v3) {
		// Get compressed size
		fread (buf, 1, 2, f);
		comp_size = (buf[1] << 8) + buf[0];
DEBUG_MSG3 ("uncomp = %i, comp = %i\n", res->length, comp_size);
		if (comp_size == res->length) {
			// Uncompressed
			fread (res->data, 1, res->length, f);
		} else if (res->type == picture) {
			// PICTURE compression (colors in 1 nybble)
			DEBUG_MSG1 ("PICTURE compressed resource ");
			fprintf (stderr, "PICTURE.%i\n", res->id);
			exit (1);
		} else {
			// LZW compressed
			char *t;
			DEBUG_MSG1 ("LZW compressed resource ");
			switch (res->type) {
				case logic: t = "LOGIC"; break;
				case picture: t = "PICTURE"; break;
				case view: t = "VIEW"; break;
				case sound: t = "SOUND"; break;
				default: t = "UNKNOWN";
			}
			fprintf (stderr, "%s.%i\n", t, res->id);
			lzw_decompress (f, res->data, res->length);
		}
	} else
		fread (res->data, 1, res->length, f);

	// Toggle cached flag
	res->cached = true;

	// If this is a LOGIC resource, automatically decrypt the messages
	if ((res->type == logic) && (!is_v3))
		decrypt_messages (res);

	return;
}

void resource_unload (agi_resource_t *res)
{

	if (!res)
		return;

	// Cached?
	if (!res->cached)
		return;

	// Free mem
	MemFree (res->data);

	// Update fields
	res->data = NULL;
	res->length = 0;
	res->cached = false;
}
