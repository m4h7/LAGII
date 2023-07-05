/*
	driver_svgalib.h
*/

#ifndef __DRIVER_SVGALIB_H__
#define __DRIVER_SVGALIB_H__

#include "types.h"

//
// FUNCTIONS
//

int svgalib_init (void);
void svgalib_close (void);
void svgalib_blit (l_byte *buf);
void svgalib_key_init (void);
void svgalib_key_close (void);
void svgalib_key_flush (void);
void svgalib_key_update (void);
void svgalib_key_handler (int scancode, int press);

#endif	// __DRIVER_SVGALIB_H__
