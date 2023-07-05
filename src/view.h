/*
	view.h
*/

#ifndef __VIEW_H__
#define __VIEW_H__

#include "logic.h"
#include "resource.h"
#include "types.h"

//
// DEFINES
//

//
// FUNCTIONS
//

char *get_view_description (agi_resource_t *res);
void show_view (agi_resource_t *res, int x, int y, int loop, int cel,
								int pri);
void show_inventory_view (agi_resource_t *res, logic_vm_t *vm);

//
// EXTERNALS
//

extern boolean no_pri_draw;

#endif	// __VIEW_H__
