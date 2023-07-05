/*
	vm_action.c
*/

//#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "driver_main.h"
#include "events.h"
#include "lagii.h"
#include "logic.h"
#include "mem.h"
#include "menu.h"
#include "object.h"
#include "picture.h"
#include "resource.h"
#include "text.h"
#include "types.h"
#include "utility.h"
#include "version.h"
#include "view.h"
#include "virtual_machine.h"

void agi_return (logic_vm_t *vm)
{
	// Do nothing
}

void agi_increment (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 18;
		decomp ("v%i++;\n", arg1);
	}

	if (vm_variables[arg1] < 0xFF)
		vm_variables[arg1]++;
}

void agi_decrement (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 18;
		decomp ("v%i--;\n", arg1);
	}

	if (vm_variables[arg1] > 0)
		vm_variables[arg1]--;
}

void agi_assignn (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 22;
		decomp ("v%i = %i;\n", arg1, arg2);
	}

	vm_variables[arg1] = arg2;
}

void agi_assignv (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 22;
		decomp ("v%i = v%i;\n", arg1, arg2);
	}

	vm_variables[arg1] = vm_variables[arg2];
}

void agi_addn (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 19;
		decomp ("v%i += %i;\n", arg1, arg2);
	}

	vm_variables[arg1] += arg2;
}

void agi_addv (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 19;
		decomp ("v%i += v%i;\n", arg1, arg2);
	}

	vm_variables[arg1] += vm_variables[arg2];
}

void agi_subn (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 19;
		decomp ("v%i -= %i;\n", arg1, arg2);
	}

	vm_variables[arg1] -= arg2;
}

void agi_subv (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 19;
		decomp ("v%i -= v%i;\n", arg1, arg2);
	}

	vm_variables[arg1] -= vm_variables[arg2];
}

void agi_lindirectv (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 25;
		decomp ("v(v%i) = v%i;\n", arg1, arg2);
	}

	vm_variables[vm_variables[arg1]] = vm_variables[arg2];
}

void agi_rindirect (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 24;
		decomp ("v%i = v(v%i);\n", arg1, arg2);
	}

	vm_variables[arg1] = vm_variables[vm_variables[arg2]];
}

void agi_lindirectn (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 25;
		decomp ("v(v%i) = %i;\n", arg1, arg2);
	}

	vm_variables[vm_variables[arg1]] = arg2;
}

void agi_set (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 12;
		decomp ("f%i = true;\n", arg1);
	}

	vm_flags[arg1] = true;
}

void agi_reset (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("f%i = false;\n", arg1);
	}

	vm_flags[arg1] = false;
}

void agi_toggle (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 15;
		decomp ("f%i = !f%i;\n", arg1, arg1);
	}

	vm_flags[arg1] = !vm_flags[arg1];
}

void agi_set_v (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("f(v%i) = true;\n", arg1);
	}

	vm_flags[vm_variables[arg1]] = true;
}

void agi_reset_v (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 16;
		decomp ("f(v%i) = false;\n", arg1);
	}

	vm_flags[vm_variables[arg1]] = false;
}

void agi_toggle_v (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 17;
		decomp ("f(v%i) = !f(v%i);\n", arg1, arg1);
	}

	vm_flags[vm_variables[arg1]] = !vm_flags[vm_variables[arg1]];
}

void agi_new_room (logic_vm_t *vm)
{
	l_byte arg1;
	int i;
	agi_object_t *ego;

	// Notify the main interpreter loop
	new_room = true;

	ego = &agi_objects[0];

	// Get the arg
	arg1 = stack_pop (vm);

	for (i=0; i < 256; i++) {
		stack_push (vm, i);
		agi_stop_update (vm);
	}
	agi_unanimate_all (vm);

	// Unload all resources
	for (i=0; i < 256; i++) {
		resource_unload (PICTURE[i]);
		resource_unload (VIEW[i]);
		resource_unload (SOUND[i]);
		if ((i != 0) && (i != vm->res->id))
			resource_unload (LOGIC[i]);
	}

	// Init. for new room
	agi_player_control (vm);
	agi_unblock (vm);
	stack_push (vm, 36);
	agi_set_horizon (vm);

	// Set variables
	vm_variables[1] = vm_variables[0];
	vm_variables[0] = arg1;
	vm_variables[4] = 0;
	vm_variables[5] = 0;
	vm_variables[16] = ego->current_view;

	// Load resource
	resource_load (LOGIC[arg1]);

	// Move Ego to opposite edge
	if (vm_variables[2] == 1)
		ego->y = 167;
	else if (vm_variables[2] == 2)
		ego->x = 1;
	else if (vm_variables[2] == 3)
		ego->y = (agi_horizon + 1);
	else if (vm_variables[2] == 4)
		ego->x = 160 - ego->xsize - 2;

	vm_variables[2] = 0;
	vm_flags[5] = true;	// Don't forget to reset this!!

	// Clear background + priority screen
	memset (background_buffer, 0, 320 * 200);
	memset (priority_buffer, 0, 320 * 200);
	memset (tmp_buffer, 0, 320 * 200);
	memset (video_buffer, 0, 320 * 200);

	event_flush ();
}

void agi_new_room_v (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(v%i);\n", arg1);
	}

	// Redirect through 'new.room'
	stack_push (vm, vm_variables[arg1]);
	agi_new_room (vm);
}

void agi_load_logic (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(%i);\n", arg1);
	}

	resource_load (LOGIC[arg1]);
}

void agi_load_logic_v (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(v%i);\n", arg1);
	}

	resource_load (LOGIC[vm_variables[arg1]]);
}

void agi_call (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	// Run the LOGIC
	run_logic (LOGIC[arg1]);
}

void agi_call_v (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(v%i);\n", arg1);
	}

	// Run the LOGIC
	run_logic (LOGIC[vm_variables[arg1]]);
}

void agi_load_pic (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(v%i);\n", arg1);
	}

	// Load the PICTURE
	resource_load (PICTURE[vm_variables[arg1]]);
}

void agi_draw_pic (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(v%i);\n", arg1);
	}

	draw_picture (PICTURE[vm_variables[arg1]], true);
}

void agi_show_pic (logic_vm_t *vm)
{
	int i;
	l_byte *tmp;

	memcpy (video_buffer, background_buffer, 320 * 200);

	memcpy (tmp_buffer, priority_buffer, 320 * 200);
	tmp = priority_buffer;
	priority_buffer = tmp_buffer;

	for (i=0; i < 256; i++) {
		if (agi_objects[i].flags & F_ANIMATE) {
			object_update (&agi_objects[i]);
		}
	}

	priority_buffer = tmp;
	driver->blit (video_buffer);
}

void agi_discard_pic (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(v%i);\n", arg1);
	}

	resource_unload (PICTURE[vm_variables[arg1]]);
}

void agi_overlay_pic (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(v%i);\n", arg1);
	}

	draw_picture (PICTURE[vm_variables[arg1]], false);
}

void agi_show_pri_screen (logic_vm_t *vm)
{
	// FIXME: Should this be such a permanent change?
	memcpy (video_buffer, priority_buffer, 320 * 200);
	driver->blit (video_buffer);
}

void agi_load_view (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	resource_load (VIEW[arg1]);
}

void agi_load_view_v (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(v%i);\n", arg1);
	}

	resource_load (VIEW[vm_variables[arg1]]);
}

void agi_discard_view (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	resource_unload (VIEW[arg1]);
}

void agi_animate_obj (logic_vm_t *vm)
{
	l_byte arg1;
	agi_object_t *ob;

	// Get the arg
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];

	// Reset everything (in case this object is being re-used)
	if (arg1 != 0) {
		ob->flags = 0;
		ob->motion = false;
		ob->step_size = 0;
		ob->do_once = 0;
		ob->direction = 0;
	}

	// FIXME: Should this imply 'draw'? (obj.draw_me = true?)
	ob->flags |= F_ANIMATE;

	if (arg1 == 0)
		ob->flags |= (F_AUTO_LOOP | F_AUTO_PRI);

	// FIXME: Are you sure about this?
	ob->flags |= (F_AUTO_CYCLE);

	// Experimental
	ob->flags |= (F_OBSERVE_BLOCKS | F_OBSERVE_HORIZON);
}

void agi_unanimate_all (logic_vm_t *vm)
{
	int i;

	for (i=0; i < 256; i++) {
//		agi_objects[i].flags &= ~(F_ANIMATE | F_DRAW_ME);
		agi_objects[i].flags = 0;
	}
}

void agi_draw (logic_vm_t *vm)
{
	l_byte arg1;
	agi_object_t *ob;

	// Get the arg
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];
	ob->flags |= F_DRAW_ME;

	// Experimental
	ob->flags |= F_AUTO_CYCLE;
}

void agi_erase (logic_vm_t *vm)
{
	l_byte arg1;
	agi_object_t *ob;

	// Get the arg
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];
	ob->flags &= ~F_DRAW_ME;
}

void agi_position (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;
	agi_object_t *ob;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];
	ob->x = arg2;
	ob->y = arg3;
}

void agi_position_v (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;
	agi_object_t *ob;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 20;
		decomp ("(%i, v%i, v%i);\n", arg1, arg2, arg3);
	}

	ob = &agi_objects[arg1];
	ob->x = vm_variables[arg2];
	ob->y = vm_variables[arg3];
}

void agi_get_posn (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;
	agi_object_t *ob;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 20;
		decomp ("(%i, v%i, v%i);\n", arg1, arg2, arg3);
	}

	ob = &agi_objects[arg1];
	vm_variables[arg2] = ob->x;
	vm_variables[arg3] = ob->y;
}

void agi_reposition (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;
	agi_object_t *ob;
	int dx, dy;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 20;
		decomp ("(%i, v%i, v%i);\n", arg1, arg2, arg3);
	}

	dx = vm_variables[arg2];
	dy = vm_variables[arg3];

	// Signed?
	if (dx & 0x80)
		dx = -((~dx & 0xFF) + 1);
	if (dy & 0x80)
		dy = -((~dy & 0xFF) + 1);

	ob = &agi_objects[arg1];
	ob->x += dx;
	ob->y += dy;
}

void agi_set_view (logic_vm_t *vm)
{
	l_byte arg1, arg2;
	agi_object_t *ob;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];

	ob->current_view = arg2;
	ob->current_loop = 0;
	ob->current_cel = 0;

	// Find out dimensions
	object_recalc (ob);

//	ob->flags |= F_AUTO_CYCLE;
}

void agi_set_view_v (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	// Redirect through 'set.view'
	stack_push (vm, arg1);
	stack_push (vm, vm_variables[arg2]);
	agi_set_view (vm);
}

void agi_set_loop (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	agi_objects[arg1].current_loop = arg2;
}

void agi_set_loop_v (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	agi_objects[arg1].current_loop = vm_variables[arg2];
}

void agi_fix_loop (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags &= ~F_AUTO_LOOP;
}

void agi_release_loop (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags |= F_AUTO_LOOP;
}

void agi_set_cel (logic_vm_t *vm)
{
	l_byte arg1, arg2;
	agi_object_t *ob;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];
	ob->current_cel = arg2;

	// Experimental
	ob->flags &= ~F_AUTO_CYCLE;
}

void agi_set_cel_v (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	agi_objects[arg1].current_cel = vm_variables[arg2];
}

void agi_last_cel (logic_vm_t *vm)
{
	l_byte arg1, arg2;
	agi_object_t *ob;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	ob = &agi_objects[arg1];
	vm_variables[arg2] = ob->num_cels - 1;
}

void agi_current_cel (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	vm_variables[arg2] = agi_objects[arg1].current_cel;
}

void agi_current_loop (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	vm_variables[arg2] = agi_objects[arg1].current_loop;
}

void agi_current_view (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	vm_variables[arg2] = agi_objects[arg1].current_view;
}

void agi_number_of_loops (logic_vm_t *vm)
{
	l_byte arg1, arg2;
	agi_object_t *ob;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	ob = &agi_objects[arg1];
	vm_variables[arg2] = ob->num_loops;
}

void agi_set_priority (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	agi_objects[arg1].priority = arg2;
}

void agi_set_priority_v (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	agi_objects[arg1].priority = vm_variables[arg2];
}

void agi_release_priority (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags |= F_AUTO_PRI;
}

void agi_get_priority (logic_vm_t *vm)
{
	l_byte arg1, arg2;
	agi_object_t *ob;
	int pri;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	ob = &agi_objects[arg1];
	if (ob->flags & F_AUTO_PRI) {
		pri = (ob->y / 12) + 1;
	} else
		pri = ob->priority;

	vm_variables[arg2] = pri;
}

void agi_stop_update (logic_vm_t *vm)
{
	l_byte arg1;
	agi_object_t *ob;

	// Get the arg
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];
	ob->flags &= ~(F_UPDATE_ME | F_AUTO_CYCLE);
}

void agi_start_update (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags |= F_UPDATE_ME;
}

void agi_force_update (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	// FIXME: Should this set (ob.update_me = true)?
	no_pri_draw = true;
	object_update (&agi_objects[arg1]);
	no_pri_draw = false;
}

void agi_ignore_horizon (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags &= ~F_OBSERVE_HORIZON;
}

void agi_observe_horizon (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags |= F_OBSERVE_HORIZON;
}

void agi_set_horizon (logic_vm_t *vm)
{
	l_byte arg1;
	agi_object_t *ego;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_horizon = arg1;

	// Experimental
	ego = &agi_objects[0];
	if (ego->y < agi_horizon)
		ego->y = agi_horizon + 1;
}

void agi_object_on_water (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags &= ~F_NO_WATER;
	agi_objects[arg1].flags |= F_NO_LAND;
}

void agi_object_on_land (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags &= ~F_NO_LAND;
	agi_objects[arg1].flags |= F_NO_WATER;
}

void agi_object_on_anything (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags &= ~(F_NO_WATER | F_NO_LAND);
}

void agi_ignore_objs (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags &= ~F_OBSERVE_OBJS;
}

void agi_observe_objs (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags |= F_OBSERVE_OBJS;
}

void agi_distance (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;
	agi_object_t *ob1, *ob2;
	int d;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 20;
		decomp ("(%i, %i, v%i);\n", arg1, arg2, arg3);
	}

	ob1 = &agi_objects[arg1];
	ob2 = &agi_objects[arg2];

	if ((ob1->flags & ob2->flags) & F_ANIMATE)
		d = abs (ob1->x - ob2->x) + abs (ob1->y - ob2->y);
	else
		d = 255;

	vm_variables[arg3] = d;
}

void agi_stop_cycling (logic_vm_t *vm)
{
	l_byte arg1;
	agi_object_t *ob;

	// Get the arg
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];

	if ((arg1 == 0) && (ob->flags & F_AUTO_MOVE))
		return;

	ob->flags &= ~F_AUTO_CYCLE;
}

void agi_start_cycling (logic_vm_t *vm)
{
	l_byte arg1;
	agi_object_t *ob;

	// Get the arg
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];
	ob->flags |= F_AUTO_CYCLE;

	// Experimental:
	if (ob->flags & F_AUTO_MOVE) {
		ob->flags |= (F_AUTO_LOOP | F_AUTO_PRI);
	}
}

void agi_normal_cycle (logic_vm_t *vm)
{
	l_byte arg1;
	agi_object_t *ob;

	// Get the arg
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];
	ob->flags |= F_AUTO_CYCLE;
	ob->flags &= ~F_REVERSE;
}

void agi_end_of_loop (logic_vm_t *vm)
{
	l_byte arg1, arg2;
	agi_object_t *ob;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, f%i);\n", arg1, arg2);
	}

	ob = &agi_objects[arg1];
	ob->do_once = arg2;
	ob->flags |= F_AUTO_CYCLE;
	ob->flags &= ~F_REVERSE;

	// Kludge to work around a small problem
	if ((ob->current_cel + 1) == ob->num_cels)
		ob->current_cel = 0;
}

void agi_reverse_cycle (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags |= (F_REVERSE | F_AUTO_CYCLE);
}

void agi_reverse_loop (logic_vm_t *vm)
{
	l_byte arg1, arg2;
	agi_object_t *ob;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, f%i);\n", arg1, arg2);
	}

	ob = &agi_objects[arg1];
	ob->do_once = arg2;
	ob->flags |= (F_AUTO_CYCLE | F_REVERSE);
}

void agi_cycle_time (logic_vm_t *vm)
{
	l_byte arg1, arg2;
	agi_object_t *ob;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	ob = &agi_objects[arg1];
	ob->cycle_time = vm_variables[arg2];
//	ob->flags |= F_AUTO_CYCLE;
}

void agi_stop_motion (logic_vm_t *vm)
{
	l_byte arg1;
	agi_object_t *ob;

	// Get the arg
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];
	ob->motion = false;
	ob->direction = 0;
	ob->flags &= ~(F_AUTO_CYCLE | F_AUTO_MOVE);
	if (arg1 == 0)
		agi_program_control (vm);
}

void agi_start_motion (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].motion = true;
	if (arg1 == 0)
		agi_player_control (vm);
}

void agi_step_size (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	agi_objects[arg1].step_size = vm_variables[arg2];
}

void agi_step_time (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	agi_objects[arg1].step_time = vm_variables[arg2];
}

void agi_move_obj (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3, arg4, arg5;
	agi_object_t *ob;

	// Get the five args (reverse order)
	arg5 = stack_pop (vm);
	arg4 = stack_pop (vm);
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];
	ob->flags |= F_AUTO_MOVE;
	ob->motion = true;
	ob->tx = arg2;
	ob->ty = arg3;
	if (arg4 > 0)
		ob->step_size = arg4;
	ob->target_flag = arg5;
	if (arg1 == 0)
		agi_program_control (vm);
}

void agi_move_obj_v (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3, arg4, arg5;

	// Get the five args (reverse order)
	arg5 = stack_pop (vm);
	arg4 = stack_pop (vm);
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 32;
		decomp ("(%i, v%i, v%i, %i, %i);\n",
				arg1, arg2, arg3, arg4, arg5);
	}

	// Redirect through 'move.obj'
	stack_push (vm, arg1);
	stack_push (vm, vm_variables[arg2]);
	stack_push (vm, vm_variables[arg3]);
	stack_push (vm, arg4);
	stack_push (vm, arg5);
	agi_move_obj (vm);
}

void agi_follow_ego (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;
	agi_object_t *ob;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 20;
		decomp ("(%i, %i, f%i);\n", arg1, arg2);
	}

	ob = &agi_objects[arg1];
	ob->flags |= F_FOLLOW_EGO;	// | F_AUTO_LOOP
	ob->flags &= ~F_WANDER;
	ob->step_size = arg2;
	ob->target_flag = arg3;
	ob->motion = true;
}

void agi_wander (logic_vm_t *vm)
{
	l_byte arg1;
	agi_object_t *ob;

	// Get the arg
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];
	ob->flags &= ~F_FOLLOW_EGO;
	ob->flags |= (F_WANDER | F_AUTO_PRI | F_AUTO_LOOP | F_UPDATE_ME |
				F_AUTO_CYCLE | F_OBSERVE_BLOCKS);
	ob->motion = true;
	if (ob->step_size < 1)
		ob->step_size = 1;
	if (arg1 == 0)
		agi_program_control (vm);
}

void agi_normal_motion (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags &=
			~(F_AUTO_MOVE | F_FOLLOW_EGO | F_WANDER);
}

void agi_set_dir (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	agi_objects[arg1].direction = vm_variables[arg2];
}

void agi_get_dir (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	vm_variables[arg2] = agi_objects[arg1].direction;
}

void agi_ignore_blocks (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags &= ~F_OBSERVE_BLOCKS;
}

void agi_observe_blocks (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_objects[arg1].flags |= F_OBSERVE_BLOCKS;
}

void agi_block (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3, arg4;

	// Get the four args (reverse order)
	arg4 = stack_pop (vm);
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	// FIXME: Set block from (arg1, arg2) => (arg3, arg4)
}

void agi_unblock (logic_vm_t *vm)
{
	// FIXME: Remove previously set block
}

void agi_get (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_inventory_objects[arg1].room = 0xFF;
}

void agi_get_v (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(v%i);\n", arg1);
	}

	agi_inventory_objects[vm_variables[arg1]].room = 0xFF;
}

void agi_drop (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	agi_inventory_objects[arg1].room = 0;
}

void agi_put (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	agi_inventory_objects[arg1].room = arg2;
}

void agi_put_v (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, v%i);\n", arg1, arg2);
	}

	agi_inventory_objects[arg1].room = vm_variables[arg2];
}

void agi_get_room_v (logic_vm_t *vm)
{
	l_byte arg1, arg2;
	agi_inventory_object_t *inv;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(v%i, v%i);\n", arg1, arg2);
	}

	inv = &agi_inventory_objects[vm_variables[arg1]];
	vm_variables[arg2] = inv->room;
}

void agi_load_sound (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	resource_load (SOUND[arg1]);
}

void agi_sound (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(%i, f%i);\n", arg1, arg2);
	}

	// FIXME: Play SOUND[arg1] (in background), then set flag arg2
	vm_flags[arg2] = true;
}

void agi_stop_sound (logic_vm_t *vm)
{
	// FIXME: Stop any sound playback
}

void agi_print (logic_vm_t *vm)
{
	l_byte arg1;
	char *msg;

	// Get the arg
	arg1 = stack_pop (vm);

	msg = text_interpret (vm, logic_messages[vm->res->id][arg1 - 1]);
{
	char *t;

	t = msg;
	msg = text_interpret (vm, t);
	MemFree (t);
}
	message_box (msg, false);
	MemFree (msg);
}

void agi_print_v (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(v%i);\n", arg1);
	}

	// Redirect through 'print'
	stack_push (vm, vm_variables[arg1]);
	agi_print (vm);
}

void agi_display (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;
	int i, j;
	char *msg;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	msg = text_interpret (vm, logic_messages[vm->res->id][arg3 - 1]);
	if (!msg)
		return;

	for (i=0, j=(8 * arg2); i < strlen (msg); i++, j += 8) {
		write_char (background_buffer, j, (arg1 * 8), msg[i],
				foreground_color, background_color);
		if (msg[i] == '\n') {
			arg1++;
			j = (8 * arg2) - 8;
		}
	}

	if (text_mode || dialogue_open) {
		memcpy (video_buffer, background_buffer, 320 * 200);
		driver->blit (video_buffer);
	}

//DEBUG_MSG4 ("Message => \"%s\" [%i,%i]\n", msg, foreground_color,
//						background_color);
	MemFree (msg);
}

void agi_display_v (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 20;
		decomp ("(v%i, v%i, v%i);\n", arg1, arg2, arg3);
	}

	// Redirect through 'display'
	stack_push (vm, vm_variables[arg1]);
	stack_push (vm, vm_variables[arg2]);
	stack_push (vm, vm_variables[arg3]);
}

void agi_clear_lines (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	// FIXME: Clear text lines arg1 => arg2 with color arg3
}

void agi_text_screen (logic_vm_t *vm)
{
	foreground_color = 15;
	background_color = 0;
	if (!text_mode) {
		memcpy (tmp_buffer, background_buffer, 320 * 200);
		text_mode = true;
	}
	memset (background_buffer, 0, 320 * 200);
	memset (video_buffer, 0, 320 * 200);
	driver->blit (video_buffer);
}

void agi_graphics (logic_vm_t *vm)
{
	memcpy (background_buffer, tmp_buffer, 320 * 200);
	text_mode = false;
}

void agi_set_cursor_char (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(m%i);\n", arg1);
	}

	input_cursor = logic_messages[vm->res->id][arg1 - 1][0];
}

void agi_set_text_attribute (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	foreground_color = arg1;
	background_color = arg2;
}

void agi_shake_screen (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	shake_screen (arg1);
}

void agi_configure_screen (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	min_line = arg1;
	user_input_line = arg2;
	status_line = arg3;
}

void agi_status_line_on (logic_vm_t *vm)
{
	status_line_on = true;
}

void agi_status_line_off (logic_vm_t *vm)
{
	status_line_on = false;
}

void agi_set_string (logic_vm_t *vm)
{
	l_byte arg1, arg2;
	int len;
	char *msg;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(s%i, m%i);\n", arg1, arg2);
	}

	msg = logic_messages[vm->res->id][arg2 - 1];
	len = strlen (msg);

	// Protection against super-long strings
	if (len >= 40)
		len = 39;

	strncpy (vm_strings[arg1], msg, len);
}

#include <unistd.h>
void agi_get_string (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3, arg4, arg5;
	char *s;

	// Get the five args (reverse order)
	arg5 = stack_pop (vm);
	arg4 = stack_pop (vm);
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 32;
		decomp ("(s%i, m%i, %i, %i, %i);\n",
				arg1, arg2, arg3, arg4, arg5);
	}

	s = event_data_entry (logic_messages[vm->res->id][arg2 - 1],
				arg4, arg3, arg5, false);
	strcpy (vm_strings[arg1], s);
	MemFree (s);
}

void agi_word_to_string (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(w%i, s%i);\n", arg1, arg2);
	}

	if (arg2 <= input_num)
		strncpy (vm_strings[arg1], input_words[arg2 - 1], 40);
}

void agi_parse (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(s%i);\n", arg1);
	}

	// FIXME: Strip out naughty characters!!
//	text_process_input (vm_strings[arg1]);

	input_num = 1;
	strcpy (input_words[0], vm_strings[arg1]);
}

void agi_get_num (logic_vm_t *vm)
{
	l_byte arg1, arg2;
	char *s;
	int i;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(m%i, v%i);\n", arg1, arg2);
	}

	s = event_data_entry (logic_messages[vm->res->id][arg1 - 1],
				3/* x */, 3/* y */, 3, true);
	sscanf (s, "%i", &i);
	vm_variables[arg2] = (i & 0xFF);
}

void agi_prevent_input (logic_vm_t *vm)
{
	accept_input = false;
}

void agi_accept_input (logic_vm_t *vm)
{
	accept_input = true;
}

void agi_set_key (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;
	int scancode;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	scancode = (arg2 << 8) + arg1;
	event_new_special_key (scancode, arg3);
}

void agi_add_to_pic (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3, arg4, arg5, arg6, arg7;
	l_byte *tmp;

	// Get the seven args (reverse order)
	arg7 = stack_pop (vm);
	arg6 = stack_pop (vm);
	arg5 = stack_pop (vm);
	arg4 = stack_pop (vm);
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	// FIXME: If arg7 (margin) is < 4, base of cel is surrounded with
	//	a rectangle of priority arg7

	// Trick 1
	tmp = video_buffer;
	video_buffer = background_buffer;

	show_view (VIEW[arg1], arg4, arg5, arg2, arg3, arg6);

	// Restore pointers
	video_buffer = tmp;
}

void agi_add_to_pic_v (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3, arg4, arg5, arg6, arg7;

	// Get the seven args (reverse order)
	arg7 = stack_pop (vm);
	arg6 = stack_pop (vm);
	arg5 = stack_pop (vm);
	arg4 = stack_pop (vm);
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 44;
		decomp ("(v%i, v%i, v%i, v%i, v%i, v%i, v%i);\n",
				arg1, arg2, arg3, arg4, arg5, arg6, arg7);
	}

	// Redirect through 'add.to.pic'
	stack_push (vm, vm_variables[arg1]);
	stack_push (vm, vm_variables[arg2]);
	stack_push (vm, vm_variables[arg3]);
	stack_push (vm, vm_variables[arg4]);
	stack_push (vm, vm_variables[arg5]);
	stack_push (vm, vm_variables[arg6]);
	stack_push (vm, vm_variables[arg7]);
	agi_add_to_pic (vm);
}

void agi_status (logic_vm_t *vm)
{
	// FIXME: Show inventory screen

{
	int i, num;
	char *old, *txt, buf[50];

	txt = l_strdup ("INVENTORY\n=========");
	num = 0;
	for (i=0; i < 256; i++) {
		if (agi_inventory_objects[i].room == 0xFF) {
			num++;
			sprintf (buf, "\n* %s",
				agi_inventory_objects[i].name);
			old = txt;
			txt = text_concat (old, buf);
			MemFree (old);
		}
	}
	if (num == 0) {
		old = txt;
		txt = text_concat (old, "\n(nothing)");
		MemFree (old);
	}
	message_box (txt, true);
}

//	message_box ("[Insert inventory screen here]", true);
	if (vm_flags[13]) {
		// Select object
		// (defaulting to pressing Esc)
		vm_variables[25] = 0xFF;
	}
}

void agi_save_game (logic_vm_t *vm)
{
	// FIXME: Save game here.....
	message_box ("Saving yet to be implemented.\n"
			"Watch this space.....", true);
}

void agi_restore_game (logic_vm_t *vm)
{
	// FIXME: Restore game here.....
	message_box ("Restoring yet to be implemented.\n"
			"Watch this space.....", true);
}

void agi_init_disk (logic_vm_t *vm)
{
	// FIXME: WTF does 'init.disk' do?!?
	message_box ("'init.disk' called.", true);
}

void agi_restart_game (logic_vm_t *vm)
{
	// FIXME: Restart game here
	message_box ("Game restarting yet to be implemented.\n"
			"Watch this space.....", true);
}

void agi_show_obj (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	show_inventory_view (VIEW[arg1], vm);
}

void agi_random (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;
	int i;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 20;
		decomp ("(%i, %i, v%i);\n", arg1, arg2, arg3);
	}

	i = (random () % (arg2 - arg1 + 1)) + arg1;
	i %= 256;
	vm_variables[arg3] = i;
}

void agi_program_control (logic_vm_t *vm)
{
	program_control = true;
}

void agi_player_control (logic_vm_t *vm)
{
	program_control = false;
}

void agi_obj_status_v (logic_vm_t *vm)
{
	l_byte arg1;
	agi_object_t *ob;
	char msg[100];
	int i;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(v%i);\n", arg1);
	}

	i = vm_variables[arg1];
	ob = &agi_objects[i];
	sprintf (msg, "Obj %i x:%i y:%i pri:%i stepsize:%i", i, ob->x,
				ob->y, ob->priority, ob->step_size);
	// FIXME: Message box, or raw print?
	message_box (msg, true);
}

void agi_quit (logic_vm_t *vm)
{
	l_byte arg1;
	int ret;

	// Get the arg
	if (action_commands[0x86].args == 1)
		arg1 = stack_pop (vm);
	else
		arg1 = 0;

	if (arg1 == 0)
		ret = message_box (" Press ENTER to quit.\n"
					"Press ESC to continue.", true);
	else
		ret = 0x0A;

	if (ret == 0x0A)
		quitting = true;
}

void agi_show_mem (logic_vm_t *vm)
{
	message_box ("Memory Status:\nMore than enough", true);
}

void agi_pause (logic_vm_t *vm)
{
	message_box ("       Game Paused.\nPress any key to continue.",
								true);
}

void agi_echo_line (logic_vm_t *vm)
{
	strcpy (input_txt, input_last_line);
	input_pos = strlen (input_last_line);
}

void agi_cancel_line (logic_vm_t *vm)
{
	input_pos = 0;
	input_txt[0] = 0;
}

void agi_init_joy (logic_vm_t *vm)
{
	message_box ("Joystick support yet to be implemented.\n"
				"Watch this space.....", true);
}

void agi_toggle_monitor (logic_vm_t *vm)
{
	// Do nothing -- already in graphics mode!
}

void agi_version (logic_vm_t *vm)
{
	char msg[100];
	int ver[2];

	ver[0] = sierra_game[game_id].agi_version[0];
	ver[1] = sierra_game[game_id].agi_version[1];
	ver[2] = sierra_game[game_id].agi_version[2];

	if (ver[2] == 0) {
		sprintf (msg, "    %s v%s\n\n%s\n\nEmulating AGI v%i.%i",
			LAGII_TITLE, LAGII_VERSION, LAGII_LONG_TITLE,
							ver[0], ver[1]);
	} else {
		sprintf (msg, "    %s v%s\n\n%s\n\n"
					"Emulating AGI v%i.%03i.%03i",
			LAGII_TITLE, LAGII_VERSION, LAGII_LONG_TITLE,
						ver[0], ver[1], ver[2]);
	}

	message_box (msg, true);
}

void agi_script_size (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	// FIXME: [Read AGIspec for more details]
}

void agi_set_game_id (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(m%i);\n", arg1);
	}

	DEBUG_MSG3 ("Game ID => message %i = '%s'\n", arg1,
				logic_messages[vm->res->id][arg1 - 1]);
}

void agi_log (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(m%i);\n", arg1);
	}

	// FIXME: Write "Room %i Input line %s (Message arg1)\n" to file
}

void agi_set_scan_start (logic_vm_t *vm)
{
	vm->res->ssIP = vm->IP;
}

void agi_reset_scan_start (logic_vm_t *vm)
{
	vm->res->ssIP = 0;
}

void agi_reposition_to (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;
	agi_object_t *ob;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];
	ob->x = arg2;
	ob->y = arg3;
}

void agi_reposition_to_v (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;
	agi_object_t *ob;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 20;
		decomp ("(%i, v%i, v%i);\n", arg1, arg2, arg3);
	}

	ob = &agi_objects[arg1];
	ob->x = vm_variables[arg2];
	ob->y = vm_variables[arg3];
}

void agi_trace_on (logic_vm_t *vm)
{
	// FIXME: Am I gonna bother with a debugger? Nah.....
}

void agi_trace_info (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3;

	// Get the three args (reverse order)
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	// FIXME: Debugger parameters.....
	//	arg1 == LOGIC resource # with command names
	//	arg2 == first line of debug window
	//	arg3 == height of debug window
}

void agi_print_at (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3, arg4;
	char *msg;

	// Get the four args (reverse order)
	if (action_commands[0x97].args == 4)
		arg4 = stack_pop (vm);
	else
		arg4 = 0;
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	msg = text_interpret (vm, logic_messages[vm->res->id][arg1 - 1]);
	mbox_x = arg3 * 8;
	mbox_y = arg2 * 8;
	if (arg4 > 0)
		mbox_width = arg4 + 1;
	message_box (msg, false);
	MemFree (msg);
}

void agi_print_at_v (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3, arg4;

	// Get the four args (reverse order)
	if (action_commands[0x97].args == 4)
		arg4 = stack_pop (vm);
	else
		arg4 = 0;
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		if (action_commands[0x97].args == 4) {
			decomp_size -= 26;
			decomp ("(m%i, v%i, v%i, v%i);\n",
					arg1, arg2, arg3, arg4);
		} else {
			decomp_size -= 20;
			decomp ("(m%i, v%i, v%i);\n", arg1, arg2, arg3);
		}
	}

	// Redirect through 'print.at'
	stack_push (vm, arg1);
	stack_push (vm, vm_variables[arg2]);
	stack_push (vm, vm_variables[arg3]);
	if (action_commands[0x97].args == 4)
		stack_push (vm, vm_variables[arg4]);
	agi_print_at (vm);
}

void agi_discard_view_v (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(v%i);\n", arg1);
	}

	resource_unload (VIEW[vm_variables[arg1]]);
}

void agi_clear_text_rect (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3, arg4, arg5;
	int x, y;

	// Get the five args (reverse order)
	arg5 = stack_pop (vm);
	arg4 = stack_pop (vm);
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	// FIXME: Clear the background_ or video_buffer?
	for (x=arg1; x <= arg3; x++) {
		for (y=arg2; y <= arg4; y++)
			pixel (video_buffer, x, y) = arg5;
	}
}

void agi_set_upper_left (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	// FIXME: Change object tracking to monitor top-left corner
}

void agi_set_menu (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(m%i);\n", arg1);
	}

	menu_new_menu (logic_messages[vm->res->id][arg1 - 1]);
}

void agi_set_menu_item (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(m%i, %i);\n", arg1, arg2);
	}

	menu_new_menu_item (logic_messages[vm->res->id][arg1 - 1], arg2);
}

void agi_submit_menu (logic_vm_t *vm)
{

}

void agi_enable_item (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	menu_change_item (arg1, true);
}

void agi_disable_item (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	menu_change_item (arg1, false);
}

void agi_menu_input (logic_vm_t *vm)
{
	// FIXME: If (vm_flags[14] == true), show menu system
	if (vm_flags[14]) {
		menu_do ();
	} else {
DEBUG_MSG1 ("NO MENU POP-UP, Flag(14) = false\n");
	}
}

void agi_show_obj_v (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 8;
		decomp ("(v%i);\n", arg1);
	}

	// FIXME: Are you sure about going through variables?
	show_inventory_view (VIEW[vm_variables[arg1]], vm);
}

void agi_open_dialogue (logic_vm_t *vm)
{
	// FIXME: Enable 'get.string' and 'get.num' if 'prevent.input'
}

void agi_close_dialogue (logic_vm_t *vm)
{
	// FIXME: Disable 'get.string' and 'get.num' if 'prevent.input'
}

void agi_mul_n (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(v%i, %i);\n", arg1, arg2);
	}

	vm_variables[arg1] *= arg2;
}

void agi_mul_v (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(v%i, v%i);\n", arg1, arg2);
	}

	vm_variables[arg1] *= vm_variables[arg2];
}

void agi_div_n (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(v%i, %i);\n", arg1, arg2);
	}

	if (arg2 != 0)
		vm_variables[arg1] /= arg2;
}

void agi_div_v (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 14;
		decomp ("(v%i, v%i);\n", arg1, arg2);
	}

	if (vm_variables[arg2] != 0)
		vm_variables[arg1] /= vm_variables[arg2];
}

void agi_close_window (logic_vm_t *vm)
{
	// FIXME: If there is a text window on screen, remove it
}

void agi_unknown170 (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);
}

void agi_unknown171 (logic_vm_t *vm)
{

}

void agi_unknown172 (logic_vm_t *vm)
{

}

void agi_unknown173 (logic_vm_t *vm)
{

}

void agi_unknown174 (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);
}

void agi_unknown175 (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);
}

void agi_unknown176 (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	if (action_commands[0xB0].args == 1)
		arg1 = stack_pop (vm);
}

void agi_unknown177 (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);
}

void agi_unknown178 (logic_vm_t *vm)
{

}

void agi_unknown179 (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3, arg4;

	// Get the four args (reverse order)
	arg4 = stack_pop (vm);
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);
}

void agi_unknown180 (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);
}

void agi_unknown181 (logic_vm_t *vm)
{

}
