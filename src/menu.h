/*
	menu.h
*/

#ifndef __MENU_H__
#define __MENU_H__

#include "types.h"

//
// DEFINES
//

// For menu headers AND menu items
typedef struct menu_item_s menu_item_t;
struct menu_item_s {
	char *name;
	l_byte code;
	boolean enabled;
	menu_item_t *prev, *next;

	// Only used in menu headers:
	menu_item_t *current;		// Currently selected menuitem
	int maxlen;			// Max. characters of a menuitem
};

//
// FUNCTIONS
//

void menu_init (void);
void menu_new_menu (char *name);
void menu_new_menu_item (char *name, l_byte code);
void menu_change_item (l_byte code, boolean enable);
void menu_do (void);
void menu_draw (int i);
void menu_draw_item (menu_item_t *it, boolean selected);
void menu_get_position (menu_item_t *it, int *x, int *y);

#endif	// __MENU_H__
