/*
	menu.c
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "driver_main.h"
#include "events.h"
#include "graphics.h"
#include "keyboard.h"
#include "mem.h"
#include "menu.h"
#include "types.h"
#include "utility.h"

static menu_item_t menus[10];
static int num_menus, current_menu;

void menu_init (void)
{
	num_menus = 0;
	current_menu = 0;
}

void menu_new_menu (char *name)
{
	menu_item_t *mh;

	mh = &menus[num_menus];
	num_menus++;

	mh->name = l_strdup (name);
	mh->code = 0;
	mh->enabled = true;
	mh->prev = NULL;
	mh->next = NULL;
	mh->current = NULL;
	mh->maxlen = 0;
}

void menu_new_menu_item (char *name, l_byte code)
{
	menu_item_t *end, *new;

	end = &menus[num_menus - 1];
	while (end->next)
		end = end->next;

	if (strlen (name) > menus[num_menus - 1].maxlen)
		menus[num_menus - 1].maxlen = strlen (name);

	new = (menu_item_t *) MemAlloc (sizeof (menu_item_t));
	new->name = l_strdup (name);
	new->code = code;
	new->enabled = true;
	new->next = NULL;

	new->prev = end;
	end->next = new;
}

void menu_change_item (l_byte code, boolean enable)
{
	int i;
	menu_item_t *m;

	for (i=0; i < num_menus; i++) {
		m = &menus[i];
		while (m) {
			if (m->code == code)
				m->enabled = enable;
			m = m->next;
		}
	}
}

void menu_do (void)
{
	int ch, cur;
	menu_item_t *cmi;
	agi_event_t *ev;

	menu_up = true;

	// Save screen
	memcpy (tmp_buffer, video_buffer, 320 * 200);

	cur = current_menu;
	while (1) {
		if (!menus[cur].current)
			menus[cur].current = menus[cur].next;
		cmi = menus[cur].current;

		memset (video_buffer, 15, 320 * 8);
		menu_draw (cur);
		driver->blit (video_buffer);

		// Get key
		while (menu_key == 0)
			driver->key_update ();
		ch = menu_key;
		menu_key = 0;

		if (ch == 0x1B)
			break;
		if ((ch == '\n') && (cmi->enabled)) {
			ev = MemAlloc (sizeof (agi_event_t));
			ev->type = menuitem;
			ev->code = cmi->code;
			event_attach (ev);
			break;
		}
		if (ch == SCANCODE_CURSORUP) {
			if (cmi->prev->prev)
				cmi = cmi->prev;
			else {
				while (cmi->next)
					cmi = cmi->next;
			}
		}
		if (ch == SCANCODE_CURSORDOWN) {
			if (cmi->next)
				cmi = cmi->next;
			else
				cmi = menus[cur].next;
		}
		if (ch == SCANCODE_CURSORLEFT) {
			cur--;
			cur += num_menus;
			cur %= num_menus;
			memcpy (video_buffer, tmp_buffer, 320 * 200);
			continue;
		}
		if (ch == SCANCODE_CURSORRIGHT) {
			cur++;
			cur %= num_menus;
			memcpy (video_buffer, tmp_buffer, 320 * 200);
			continue;
		}

		menus[cur].current = cmi;
	}

	// Restore screen
	memcpy (video_buffer, tmp_buffer, 320 * 200);

	current_menu = cur;
	menu_up = false;
	driver->key_flush ();
}

// Parameter 'i' is the # of menu that is selected
void menu_draw (int i)
{
	menu_item_t *t;
	int num, x, y, j, maxlen;

	// Count menu items
	t = &menus[i];
	num = -1;
	while (t) {
		num++;
		t = t->next;
	}

	maxlen = menus[i].maxlen * 8;

	// Draw white background
	menu_get_position (menus[i].next, &x, &y);
	x -= 8;
	y -= 8;
	for (j=0; j < (8 * num + 16); j++, y++) {
		memset (&pixel (video_buffer, x, y), 15, maxlen + 16);
	}

	// Draw borders
	// top & bottom
	menu_get_position (menus[i].next, &x, &y);
	x -= 4;
	y -= 5;
	memset (&pixel (video_buffer, x, y), 0, maxlen + 8);
	y += (8 * num + 10);
	memset (&pixel (video_buffer, x, y), 0, maxlen + 8);
	// left & right
	menu_get_position (menus[i].next, &x, &y);
	x -= 6;
	y -= 5;
	maxlen += x + 10;
	for (j=0; j <= (8 * num + 10); j++, y++) {
		memset (&pixel (video_buffer, x, y), 0, 2);
		memset (&pixel (video_buffer, maxlen, y), 0, 2);
	}

	// Draw menuitems
	t = &menus[i];
	while (t) {
		// Draw this menu item
		if ((t == menus[i].current) ||
		    (t == &menus[i]))
			menu_draw_item (t, true);
		else
			menu_draw_item (t, false);

		// Next menu item
		t = t->next;
	}

	// Draw other menu headers
	for (j=0; j < num_menus; j++) {
		if (j != i)
			menu_draw_item (&menus[j], false);
	}
}

void menu_draw_item (menu_item_t *it, boolean selected)
{
	int x, y, fg, bg, i, len, nx, ny;

	if (selected) {
		fg = 15;
		bg = 0;
	} else {
		fg = 0;
		bg = 15;
	}

	// Get position
	menu_get_position (it, &x, &y);

	if ((!it->enabled) && selected) {
		// Grey out disabled items
		menu_get_position (it, &nx, &ny);
		len = 8 * strlen (it->name);
		x = nx;
		y = ny;
		for (i=0; i < (32 * strlen (it->name)); i++) {
			pixel (video_buffer, x, y) = 0;
			x += 2;
			if (x > (nx + len - 2)) {
				x -= (len - 1);
				y++;
			}
		}
	} else {
		for (i=0; i < strlen (it->name); i++) {
			write_char (video_buffer, x, y, it->name[i], fg,
								bg);
			x += 8;
		}
	}
}

void menu_get_position (menu_item_t *it, int *x, int *y)
{
	int i;
	menu_item_t *mh, *t;

	// Determine header of this item
	mh = it;
	while (mh->prev)
		mh = mh->prev;

	*x = 8;
	for (i=0; i < num_menus; i++) {
		if (mh == &menus[i])
			break;
		*x += (8 * strlen (menus[i].name) + 8);
	}

	// If not a menuheader, stop falling off screen
	if (it != mh) {
		i = menus[i].maxlen * 8;
		// Falling off screen?
		if ((*x + i) >= 312)
			*x = 312 - i;
	}

	*y = 0;
	if (it != mh)
		*y = 8;
	t = it;
	while (t != mh) {
		*y += 8;
		t = t->prev;
	}
}
