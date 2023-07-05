/*
	events.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "driver_main.h"
#include "events.h"
#include "graphics.h"
#include "keyboard.h"
#include "lagii.h"
#include "mem.h"
#include "object.h"
#include "sshot.h"
#include "text.h"
#include "types.h"
#include "utility.h"

agi_event_t *first_agi_event;
special_key_t *first_special_key;
char key_state[128];
boolean menu_up;
char menu_key;

struct mapping {
	int scancode, ascii;
} key_map[] = {
	{SCANCODE_ESCAPE, 0x1B}, {SCANCODE_ENTER, '\n'},
	{SCANCODE_BACKSPACE, '\b'}, {SCANCODE_TAB, '\t'},
	{SCANCODE_KEYPADENTER, '\n'},

	{SCANCODE_1, '1'}, {SCANCODE_2, '2'}, {SCANCODE_3, '3'},
	{SCANCODE_4, '4'}, {SCANCODE_5, '5'}, {SCANCODE_6, '6'},
	{SCANCODE_7, '7'}, {SCANCODE_8, '8'}, {SCANCODE_9, '9'},
	{SCANCODE_0, '0'},

	{SCANCODE_A, 'a'}, {SCANCODE_B, 'b'}, {SCANCODE_C, 'c'},
	{SCANCODE_D, 'd'}, {SCANCODE_E, 'e'}, {SCANCODE_F, 'f'},
	{SCANCODE_G, 'g'}, {SCANCODE_H, 'h'}, {SCANCODE_I, 'i'},
	{SCANCODE_J, 'j'}, {SCANCODE_K, 'k'}, {SCANCODE_L, 'l'},
	{SCANCODE_M, 'm'}, {SCANCODE_N, 'n'}, {SCANCODE_O, 'o'},
	{SCANCODE_P, 'p'}, {SCANCODE_Q, 'q'}, {SCANCODE_R, 'r'},
	{SCANCODE_S, 's'}, {SCANCODE_T, 't'}, {SCANCODE_U, 'u'},
	{SCANCODE_V, 'v'}, {SCANCODE_W, 'w'}, {SCANCODE_X, 'x'},
	{SCANCODE_Y, 'y'}, {SCANCODE_Z, 'z'},

	{SCANCODE_BRACKET_LEFT, '['}, {SCANCODE_BRACKET_RIGHT, ']'},
	{SCANCODE_MINUS, '-'}, {SCANCODE_EQUAL, '='},
	{SCANCODE_SEMICOLON, ';'}, {SCANCODE_APOSTROPHE, '\''},
	{SCANCODE_GRAVE, '`'}, {SCANCODE_BACKSLASH, '\\'},
	{SCANCODE_COMMA, ','}, {SCANCODE_PERIOD, '.'},
	{SCANCODE_SLASH, '/'}, {SCANCODE_SPACE, ' '},

	// Translate cursor block to keypad block
	{SCANCODE_CURSORBLOCKUP, SCANCODE_CURSORUP},
	{SCANCODE_CURSORBLOCKDOWN, SCANCODE_CURSORDOWN},
	{SCANCODE_CURSORBLOCKLEFT, SCANCODE_CURSORLEFT},
	{SCANCODE_CURSORBLOCKRIGHT, SCANCODE_CURSORRIGHT},

	// Standard keypad self-translations
	{SCANCODE_CURSORUP, SCANCODE_CURSORUP},
	{SCANCODE_CURSORDOWN, SCANCODE_CURSORDOWN},
	{SCANCODE_CURSORLEFT, SCANCODE_CURSORLEFT},
	{SCANCODE_CURSORRIGHT, SCANCODE_CURSORRIGHT},
	{SCANCODE_CURSORUPRIGHT, SCANCODE_CURSORUPRIGHT},
	{SCANCODE_CURSORDOWNRIGHT, SCANCODE_CURSORDOWNRIGHT},
	{SCANCODE_CURSORUPLEFT, SCANCODE_CURSORUPLEFT},
	{SCANCODE_CURSORDOWNLEFT, SCANCODE_CURSORDOWNLEFT},

	{0, 0}
};

void event_init (void)
{
	int err;

	first_agi_event = NULL;
	first_special_key = NULL;
	memset (key_state, 0, 128);

	// Auto-return to XLATE mode
	err = atexit (event_close);
	if (err) {
		perror ("atexit");
		exit (1);
	}

	driver->key_init ();
}

void event_close (void)
{
	driver->key_close ();
}

void event_flush (void)
{
	driver->key_flush ();
	memset (key_state, 0, 128);
}

void event_key_handler (int scancode, int press)
{
	agi_event_t *new;
	special_key_t *sp;
	boolean set_high;
	int i;

	// Trim high bytes (for safety)
	scancode &= 0x7F;
	press &= 0x7F;

	if (key_state[scancode] == press)
		return;			// Key held down

	key_state[scancode] = press;

	// Ignore releases
	if (press == KEY_EVENTRELEASE)
		return;

	// Hotkey: F11 == take screenshot
	if (scancode == SCANCODE_F11) {
		char fname[20];

		i = get_next_number ();
		sprintf (fname, "%s-%i.gif",
			sierra_game[game_id].short_title, i);
		take_screenshot (fname);
		return;
	}

	// Hotkey: F12 == instant exit
	if (scancode == SCANCODE_F12) {
		exit (0);
	}

	// Hotkey: ScrollLock == Toggle showing of object information
	if (scancode == SCANCODE_SCROLLLOCK) {
		obj_toggle = !obj_toggle;
		return;
	}

	// Hotkey: Pause/Break == Toggle turbo mode
	if ((scancode == SCANCODE_BREAK) ||
	    (scancode == SCANCODE_BREAK_ALTERNATIVE)) {
		turbo_toggle = !turbo_toggle;
		return;
	}

	// Hotkey: PrintScreen == Toggle showing of priority screen
	if (scancode == SCANCODE_PRINTSCREEN) {
		pri_toggle = !pri_toggle;
		return;
	}

	// Hotkey: Page Up == "Gimme Gimme Gimme!"
	if (scancode == SCANCODE_PAGEUP) {
//		agi_inventory_objects[8].room = 0xFF;
//		agi_inventory_objects[9].room = 0xFF;
//		for (i=0; i < 20; i++)
//			agi_inventory_objects[i].room = 0xFF;
	}

	// Alt-* and F* keys should be in the high byte (of the low word)
	set_high = false;
	if (key_state[SCANCODE_LEFTALT] ||
	    key_state[SCANCODE_RIGHTALT])
		set_high = true;
	if ((scancode >= SCANCODE_F1) && (scancode <= SCANCODE_F10))
		set_high = true;
	if (set_high)
		scancode <<= 8;
	else {
		// Map scancode to ASCII key
		boolean flag = false;
		for (i=0; ; i++) {
			if (key_map[i].scancode == 0)
				break;
			if (key_map[i].scancode == scancode) {
				scancode = key_map[i].ascii;
				flag = true;
				break;
			}
		}
		if (!flag)
			return;
	}

	// Menu running?
	if (menu_up) {
		menu_key = scancode;
		return;
	}

	// Special exception: DOS used to generate an 0x0D scancode for
	// the enter key. If this is a special key, map 0x0A's to it
	if ((is_special (0x0D)) && (scancode == '\n'))
		scancode = 0x0D;

	// Movement key?
	if ((scancode == SCANCODE_CURSORUP) ||
	    (scancode == SCANCODE_CURSORUPRIGHT) ||
	    (scancode == SCANCODE_CURSORRIGHT) ||
	    (scancode == SCANCODE_CURSORDOWNRIGHT) ||
	    (scancode == SCANCODE_CURSORDOWN) ||
	    (scancode == SCANCODE_CURSORDOWNLEFT) ||
	    (scancode == SCANCODE_CURSORLEFT) ||
	    (scancode == SCANCODE_CURSORUPLEFT)) {
		event_movement_key (scancode);
		return;
	}

	// Catch keys that the input line wants
	if ((!set_high) &&
	    (key_state[SCANCODE_LEFTCONTROL] == 0) &&
	    (key_state[SCANCODE_RIGHTCONTROL] == 0)) {
		// Alpha keys
		if ((scancode >= 'a') && (scancode <= 'z')) {
			if ((key_state[SCANCODE_LEFTSHIFT]) ||
			    (key_state[SCANCODE_RIGHTSHIFT]))
				scancode -= ('a' - 'A');
			text_new_key (scancode);
			return;
		}

		// Numeric keys
//		if ((scancode >= '0') && (scancode <= '9')) {
//			text_new_key (scancode);
//			return;
//		}

		// Enter and Backspace
		if ((scancode == '\n') || (scancode == '\b')) {
			text_new_key (scancode);
			return;
		}
	}

	// If Control key is held down, scancodes of alpha keys will be
	// offset by ('a' - 1)
	if ((key_state[SCANCODE_LEFTCONTROL]) ||
	    (key_state[SCANCODE_RIGHTCONTROL])) {
		scancode -= ('a' - 1);
	}

	// Compare with special keys set with 'set.key'
	sp = is_special (scancode);
	if (sp) {
		new = MemAlloc (sizeof (agi_event_t));
		new->type = keypress;
		new->code = sp->code;
		event_attach (new);
DEBUG_MSG3 ("Scancode 0x%04X triggered event %i\n", scancode, sp->code);
	} else {
		// Assume this key is input
		text_new_key (scancode);
	}
}

boolean event_check_for_event (l_byte code)
{
	agi_event_t *ev;

	driver->key_update ();
	ev = first_agi_event;
	while (ev) {
		if (ev->code == code) {
			// Match!
			if (ev->prev)
				ev->prev->next = ev->next;
			if (ev->next)
				ev->next->prev = ev->prev;
			if (ev == first_agi_event)
				first_agi_event = ev->next;
			MemFree (ev);
			return true;
		}
		ev = ev->next;
	}
	return false;
}

void event_attach (agi_event_t *event)
{
	agi_event_t *ev;

	event->prev = NULL;
	event->next = NULL;

	if (!first_agi_event) {
		first_agi_event = event;
		return;
	}

	// Find last event on chain
	ev = first_agi_event;
	while (ev->next)
		ev = ev->next;

	// Attach
	ev->next = event;
	event->prev = ev;
}

void event_new_special_key (int scancode, l_byte code)
{
	special_key_t *end, *new;

	new = MemAlloc (sizeof (special_key_t));
	new->key = scancode;
	new->code = code;
	new->prev = NULL;
	new->next = NULL;

	if (!first_special_key) {
		first_special_key = new;
		return;
	}

	// Find last special key on chain
	end = first_special_key;
	while (end->next)
		end = end->next;

	// Attach
	end->next = new;
	new->prev = end;
}

void event_update (void)
{
	driver->key_update ();
}

special_key_t *is_special (int scancode)
{
	special_key_t *sp;

	// Compare with special keys set with 'set.key'
	sp = first_special_key;
	while (sp) {
		if (sp->key == scancode)
			break;
		sp = sp->next;
	}

	return sp;
}

// Returns non-zero if a key is being held
int event_key_pressed (void)
{
	int i;

	for (i=1; i < 128; i++) {
		// Ignore modifier keys
		if ((i == SCANCODE_LEFTCONTROL) ||
		    (i == SCANCODE_LEFTSHIFT) ||
		    (i == SCANCODE_RIGHTSHIFT) ||
		    (i == SCANCODE_LEFTALT) ||
		    (i == SCANCODE_CAPSLOCK) ||
		    (i == SCANCODE_NUMLOCK) ||
		    (i == SCANCODE_SCROLLLOCK) ||
		    (i == SCANCODE_RIGHTCONTROL) ||
		    (i == SCANCODE_RIGHTALT))
			continue;

		// Ignore hotkeys
		if ((i == SCANCODE_F11) ||
		    (i == SCANCODE_F12) ||
		    (i == SCANCODE_SCROLLLOCK) ||
		    (i == SCANCODE_BREAK) ||
		    (i == SCANCODE_BREAK_ALTERNATIVE))
			continue;

		// Ignore special keys
		if (is_special (i) || is_special (i << 8))
			continue;

		if (key_state[i]) {
			int j;

//			key_state[i] = 0;
			// Map scancode to ASCII key
			for (j=0; ; j++) {
				if (key_map[j].scancode == 0)
					break;
				if (key_map[j].scancode == i)
					return (key_map[i].ascii);
			}
			return 1;
		}
	}
	return 0;
}

int event_wait_for_key (void)
{
	int ret;

	menu_up = true;

	while (menu_key == 0)
		driver->key_update ();

	ret = menu_key;
	menu_key = 0;
	menu_up = false;

	return ret;
}

int get_next_number (void)
{
	int i;
	char fname[20];

	for (i=0; i < 1000; i++) {
		sprintf (fname, "%s-%i.gif",
			sierra_game[game_id].short_title, i);
		if (l_fileexists (fname))
			continue;
		return i;
	}
	return 0;
}

void event_movement_key (int code)
{
	agi_object_t *ego;
	int newdir;

//	if (program_control)
//		return;

	ego = &agi_objects[0];

	switch (code) {
		case SCANCODE_CURSORUP:
			newdir = 1;
			break;
		case SCANCODE_CURSORUPRIGHT:
			newdir = 2;
			break;
		case SCANCODE_CURSORRIGHT:
			newdir = 3;
			break;
		case SCANCODE_CURSORDOWNRIGHT:
			newdir = 4;
			break;
		case SCANCODE_CURSORDOWN:
			newdir = 5;
			break;
		case SCANCODE_CURSORDOWNLEFT:
			newdir = 6;
			break;
		case SCANCODE_CURSORLEFT:
			newdir = 7;
			break;
		case SCANCODE_CURSORUPLEFT:
			newdir = 8;
			break;
		default:
			newdir = 0;
	}

	if (newdir == ego->direction) {
		ego->direction = 0;
		ego->motion = false;
		ego->flags &= ~F_AUTO_CYCLE;
	} else {
		ego->direction = newdir;
		ego->motion = true;
//		ego->flags |= (F_AUTO_CYCLE | F_UPDATE_ME);
		if (ego->step_size < 1)
			ego->step_size = 1;
	}

	vm_variables[6] = ego->direction;
}

char *event_data_entry (char *prompt, int x, int y, int max, boolean num)
{
	char *ret, ch;
	int len, i;

	x *= 16;
	y *= 8;

	write_string (video_buffer, x, y, prompt, foreground_color,
							background_color);
	driver->blit (video_buffer);
	x += 8 * strlen (prompt);

	ret = MemAlloc (max + 1);

	menu_key = 0;
	menu_up = true;
	len = 0;
	while (1) {
		// Update display
		write_string (video_buffer, x, y, ret, foreground_color,
							background_color);
		write_char (video_buffer, (x + 8 * len), y, input_cursor,
			foreground_color, background_color);
		for (i=len+1; i < max; i++)
			write_char (video_buffer, (x + 8 * i), y, ' ',
				foreground_color, background_color);
		driver->blit (video_buffer);

		while (menu_key == 0)
			driver->key_update ();
		ch = menu_key;
		menu_key = 0;

		if (ch == '\b') {
			ret[len] = 0;
			if (len > 0)
				len--;
		} else if (ch == '\n') {
			ret[len] = 0;
			break;
		} else {
			ret[len] = ch;
			len++;
		}

		if (len >= max) {
			len--;
			ret[len] = 0;
		}
	}
	menu_up = false;
	menu_key = 0;
	driver->key_flush ();

	return ret;
}
