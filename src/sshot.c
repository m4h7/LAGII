/*
	sshot.c
*/

#include "config.h"

#ifndef NO_GIFLIB
#include <gif_lib.h>
#endif

#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "graphics.h"
#include "mem.h"
#include "sshot.h"

#ifndef NO_GIFLIB
static GifColorType ega_palette[] = {
	{0, 0, 0}, {0, 0, 170}, {0, 170, 0}, {0, 170, 170},
	{170, 0, 0}, {170, 0, 170}, {170, 85, 0}, {170, 170, 170},
	{85, 85, 85}, {85, 85, 255}, {85, 255, 85}, {85, 255, 255},
	{255, 85, 85}, {255, 85, 255}, {255, 255, 85}, {255, 255, 255}
};
#endif

#ifndef NO_GIFLIB
void take_screenshot (char *fname)
{
	GifFileType *gif;
	int err, i;
	ColorMapObject *color_map;
	GifPixelType *scan_line;

	gif = EGifOpenFileName (fname, 0);
	if (gif == NULL) {
		DEBUG_MSG1 ("EGifOpenFileName failed.\n");
		return;
	}

	color_map = MakeMapObject (16, ega_palette);

	err = EGifPutScreenDesc (gif, 320, 200, color_map->BitsPerPixel,
					0, color_map);
	if (err == GIF_ERROR) {
		DEBUG_MSG1 ("EGifPutScreenDesc failed.\n");
		return;
	}

	// parameter 6 ("0") can be changed to "1", for interlacing
	err = EGifPutImageDesc (gif, 0, 0, 320, 200, 0, color_map);
	if (err == GIF_ERROR) {
		DEBUG_MSG1 ("EGifPutImageDesc failed.\n");
		return;
	}

	scan_line = (GifPixelType *) MemAlloc (320);

	for (i=0; i < 200; i++) {
		// Read in the line
		memcpy (scan_line, &video_buffer[320 * i], 320);

		// Dump to file
		err = EGifPutLine (gif, scan_line, 320);
		if (err == GIF_ERROR) {
			DEBUG_MSG1 ("EGifPutLine failed.\n");
			MemFree (scan_line);
			return;
		}
	}

	MemFree (scan_line);

	// Close file
	err = EGifCloseFile (gif);
	if (err == GIF_ERROR) {
		DEBUG_MSG1 ("EGifCloseFile failed.\n");
		return;
	}

	// Clean-up
	FreeMapObject (color_map);
}
#else
void take_screenshot (char *fname)
{
	DEBUG_MSG1 ("Can't take screenshot -- no libungif.\n");
}
#endif	// NO_GIFLIB
