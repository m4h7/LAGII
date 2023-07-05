/*
	events.h
*/

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "types.h"

//
// DEFINES
//

typedef enum {
	keypress,		// Key was pressed
	menuitem		// Menu item was selected
} event_type;

typedef struct agi_event_s agi_event_t;
struct agi_event_s {
	event_type type;		// Type of event
	l_byte code;			// The code of the event
	agi_event_t *prev, *next;	// The previous and next event
};

// special_key_t: Created with 'set.key'
typedef struct special_key_s special_key_t;
struct special_key_s {
	int key;
	l_byte code;			// Code to generate in event
	special_key_t *prev, *next;	// The previous and next key
};

//
// FUNCTIONS
//

void event_init (void);
void event_close (void);
void event_flush (void);
void event_key_handler (int scancode, int press);
boolean event_check_for_event (l_byte code);
void event_attach (agi_event_t *event);
void event_new_special_key (int scancode, l_byte code);
void event_update (void);
special_key_t *is_special (int scancode);
int event_key_pressed (void);
int event_wait_for_key (void);
int get_next_number (void);
void event_movement_key (int code);
char *event_data_entry (char *prompt, int x, int y, int max, boolean num);

//
// EXTERNALS
//

extern boolean menu_up;
char menu_key;

#endif	// __EVENTS_H__
