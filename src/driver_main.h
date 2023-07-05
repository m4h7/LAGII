/*
	driver_main.h
*/

#ifndef __DRIVER_MAIN_H__
#define __DRIVER_MAIN_H__

#include "types.h"

//
// DEFINES
//

typedef struct {
	char *name;

	// Graphics:
	int (*init) (void);
	void (*close) (void);
	void (*blit) (l_byte *buf);

	// Keyboard:
	void (*key_init) (void);
	void (*key_close) (void);
	void (*key_flush) (void);
	void (*key_update) (void);
} driver_t;

//
// FUNCTIONS
//

void driver_init (int reserved);
void driver_close (void);
void driver_title_screen (void);

//
// EXTERNALS
//

extern driver_t *driver;

// Declared inside graphics.c
extern l_byte *video_buffer, *priority_buffer;
extern l_byte *background_buffer, *tmp_buffer;

#endif	// __DRIVER_MAIN_H__
