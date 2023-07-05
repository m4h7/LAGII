/*
	vm_test.c
*/

//#include <stdio.h>
#include <string.h>

#include "driver_main.h"
#include "events.h"
#include "lagii.h"
#include "logic.h"
#include "object.h"
#include "types.h"
#include "virtual_machine.h"

void agi_equaln (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 19;
		if (last_decomp_char == '!') {
			decomp_size--;
			decomp ("(v%i != %i)", arg1, arg2);
		} else
			decomp ("(v%i == %i)", arg1, arg2);
	}

	if (vm_variables[arg1] == arg2)
		stack_push (vm, true);
	else
		stack_push (vm, false);
}

void agi_equalv (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 19;
		if (last_decomp_char == '!') {
			decomp_size--;
			decomp ("(v%i != v%i)", arg1, arg2);
		} else
			decomp ("(v%i == v%i)", arg1, arg2);
	}

	if (vm_variables[arg1] == vm_variables[arg2])
		stack_push (vm, true);
	else
		stack_push (vm, false);
}

void agi_lessn (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 18;
		if (last_decomp_char == '!') {
			decomp_size--;
			decomp ("(v%i >= %i)", arg1, arg2);
		} else
			decomp ("(v%i < %i)", arg1, arg2);
	}

	if (vm_variables[arg1] < arg2)
		stack_push (vm, true);
	else
		stack_push (vm, false);
}

void agi_lessv (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 18;
		if (last_decomp_char == '!') {
			decomp_size--;
			decomp ("(v%i >= v%i)", arg1, arg2);
		} else
			decomp ("(v%i < v%i)", arg1, arg2);
	}

	if (vm_variables[arg1] < vm_variables[arg2])
		stack_push (vm, true);
	else
		stack_push (vm, false);
}

void agi_greatern (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 21;
		if (last_decomp_char == '!') {
			decomp_size--;
			decomp ("(v%i <= %i)", arg1, arg2);
		} else
			decomp ("(v%i > %i)", arg1, arg2);
	}

	if (vm_variables[arg1] > arg2)
		stack_push (vm, true);
	else
		stack_push (vm, false);
}

void agi_greaterv (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 21;
		if (last_decomp_char == '!') {
			decomp_size--;
			decomp ("(v%i <= v%i)", arg1, arg2);
		} else
			decomp ("(v%i > v%i)", arg1, arg2);
	}

	if (vm_variables[arg1] > vm_variables[arg2])
		stack_push (vm, true);
	else
		stack_push (vm, false);
}

void agi_isset (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 12;
		decomp ("f%i", arg1);
	}

	stack_push (vm, vm_flags[arg1]);
}

void agi_issetv (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 13;
		decomp ("f(v%i)", arg1);
	}

	stack_push (vm, vm_flags[vm_variables[arg1]]);
}

void agi_has (logic_vm_t *vm)
{
	l_byte arg1;

	// Get the arg
	arg1 = stack_pop (vm);

	if (decompile_toggle) {
		decomp_size -= 10;
		decomp ("has (\"%s\")", agi_inventory_objects[arg1].name);
	}

	if (agi_inventory_objects[arg1].room == 0xFF)
		stack_push (vm, true);
	else
		stack_push (vm, false);
}

void agi_obj_in_room (logic_vm_t *vm)
{
	l_byte arg1, arg2;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	if (agi_inventory_objects[arg1].room == arg2)
		stack_push (vm, true);
	else
		stack_push (vm, false);
}

void agi_posn (logic_vm_t *vm)
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
	if ((arg2 <= ob->x) && (ob->x <= arg4)) {
		if ((arg3 <= ob->y) && (ob->y <= arg5))
			stack_push (vm, true);
		else
			stack_push (vm, false);
	} else
		stack_push (vm, false);
}

void agi_controller (logic_vm_t *vm)
{
	l_byte arg1;
	boolean ret;

	// Get the arg
	arg1 = stack_pop (vm);

	ret = event_check_for_event (arg1);
	stack_push (vm, ret);
}

void agi_have_key (logic_vm_t *vm)
{
	boolean ret;

	driver->key_update ();
	ret = event_key_pressed ();
	if (ret > 0)
		stack_push (vm, true);
	else
		stack_push (vm, false);
}

void agi_said (logic_vm_t *vm)
{
	// Do nothing. This routine is never called, since 'said' commands
	// are intercepted in run_virtual_machine
}

void agi_compare_strings (logic_vm_t *vm)
{
	l_byte arg1, arg2;
	int ret;

	// Get the two args (reverse order)
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	ret = strcmp (vm_strings[arg1], vm_strings[arg2]);
	if (ret == 0)
		stack_push (vm, true);
	else
		stack_push (vm, false);
}

void agi_obj_in_box (logic_vm_t *vm)
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
	if ((arg3 <= ob->y) && (ob->y <= arg5)) {
		if ((arg2 <= ob->x) && ((ob->x + ob->xsize) < arg4))
			stack_push (vm, true);
		else
			stack_push (vm, false);
	} else
		stack_push (vm, false);
}

void agi_center_posn (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3, arg4, arg5;
	agi_object_t *ob;
	int center;

	// Get the five args (reverse order)
	arg5 = stack_pop (vm);
	arg4 = stack_pop (vm);
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];
	center = ob->x + (ob->xsize / 2);
	if ((arg2 <= center) && (center <= arg4)) {
		if ((arg3 <= ob->y) && (ob->y <= arg5))
			stack_push (vm, true);
		else
			stack_push (vm, false);
	} else
		stack_push (vm, false);
}

void agi_right_posn (logic_vm_t *vm)
{
	l_byte arg1, arg2, arg3, arg4, arg5;
	agi_object_t *ob;
	int right;

	// Get the five args (reverse order)
	arg5 = stack_pop (vm);
	arg4 = stack_pop (vm);
	arg3 = stack_pop (vm);
	arg2 = stack_pop (vm);
	arg1 = stack_pop (vm);

	ob = &agi_objects[arg1];
	right = ob->x + ob->xsize - 1;
	if ((arg2 <= right) && (right <= arg4)) {
		if ((arg3 <= ob->y) && (ob->y <= arg5))
			stack_push (vm, true);
		else
			stack_push (vm, false);
	} else
		stack_push (vm, false);
}
