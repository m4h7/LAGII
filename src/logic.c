/*
	logic.c
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "driver_main.h"
#include "mem.h"
#include "lagii.h"
#include "logic.h"
#include "resource.h"
#include "text.h"
#include "types.h"
#include "utility.h"
#include "virtual_machine.h"


// Confusing, eh? Think of it this way:
//
// logic_messages[x][y][z]:
//	is the zth character of the yth message of LOGIC.x
// (N.B. The first message is in logic_messages[x][0],
//		*NOT* logic_messages[x][1]!!)
// WARNING: The each _individual_ message is allocated with malloc, not
// 		with MemAlloc, to improve SPEED!

char ***logic_messages;

l_byte vm_variables[256];
boolean vm_flags[256];
char vm_strings[24][40];
int agi_horizon;
int foreground_color, background_color;

boolean text_mode = false, status_line_on = false, accept_input = false;
boolean dialogue_open = false;
int min_line = 1, user_input_line = 24, status_line = 0;

void run_logic (agi_resource_t *res)
{
	logic_vm_t vm;
	boolean lal;

	// Load resource
	lal = res->cached;
	resource_load (res);

	// Load messages into logic_messages[i] (for LOGIC.i)
	logic_messages[res->id] = load_messages (res);

	// Initialise virtual machine
	vm.res = res;
	vm.num_messages = number_of_messages (res);
	vm.private_messages = logic_messages[res->id];
	vm.IP = res->ssIP;
	vm.CS = res->length;
	vm.code = &res->data[2];
	vm.stack = NULL;
	vm.condition = NULL;

	// FIXME: Reset res->ssIP to zero?
	//res->ssIP = 0;

	// Start virtual machine
	run_virtual_machine (&vm);

	// Clean-up virtual machine

	// Unload messages (for LOGIC.i)
	unload_messages (res, logic_messages[res->id]);

	// Unload resource
	if ((!lal) && (res->id > 0))
		resource_unload (res);
}

// Modifies the action_commands[] array for some versions
void fix_version (void)
{
	sierra_game_t *game;
	int ver[3];

	game = &sierra_game[game_id];
	ver[0] = game->agi_version[0];
	ver[1] = game->agi_version[1];
	ver[2] = game->agi_version[2];

	// 'quit' takes 0 args for 2.089
	if ((ver[0] == 2) && (ver[1] == 89)) {
		action_commands[0x86].args = 0;
	}

	// 'print.at' and 'print.at.v' take 3 args before 2.400
	if ((ver[0] == 2) && (ver[1] < 400)) {
		action_commands[0x97].args = 3;
		action_commands[0x98].args = 3;
	}

	// 'unknown176' takes 1 arg for 3.002.086
	if ((ver[0] == 3) && (ver[1] == 2) && (ver[2] == 86)) {
		action_commands[0xB0].args = 1;
	}
}

// Returns the number of resources in 'res'
// (Assumes resource is already cached)
int number_of_messages (agi_resource_t *res)
{
	l_byte *data;
	int text_offset, msg_num;

	// Start of data
	data = res->data;

	// Read text offset
	text_offset = (data[1] << 8) + data[0] + 2;

	// Seek to text
	data = &res->data[text_offset];

	// Read number of messages
	msg_num = *data++;

	return msg_num;
}

// Assumes resource is already cached
void decrypt_messages (agi_resource_t *res)
{
	l_byte *data, t[2], *avis = (l_byte *) "Avis Durgan", *p;
	int text_offset, msg_num, cycle;

	// Start of data
	data = res->data;

	// Read text offset
	memcpy (t, data, 2);
	text_offset = (t[1] << 8) + t[0] + 2;

	// Seek to text
	data = &res->data[text_offset];

	// Read number of messages
	msg_num = *data++;

	// Rapidly decrypt *all* messages
	p = &res->data[text_offset + (msg_num * 2) + 3];
	cycle = 0;
	while (p < &res->data[res->length]) {
		*p ^= avis[cycle % 11];
		p++;
		cycle++;
	}
}

void unload_messages (agi_resource_t *res, char **dst)
{
	int msg_num, i;

	if (!dst)
		return;

	// Read number of messages
	msg_num = number_of_messages (res);

	for (i=0; i < msg_num; i++) {
		if (dst[i])
			free (dst[i]);
	}

	MemFree (dst);
	dst = NULL;
}

// Assumes resource is already cached
char **load_messages (agi_resource_t *res)
{
	int msg_num, i, text_offset, msg_offset, len;
	l_byte *data, *msg;
	char **dst;

	// Read number of messages
	msg_num = number_of_messages (res);

	// No messages?
	if (msg_num < 1)
		return (char **) NULL;

	// Fast-forward to text section
	data = res->data;
	text_offset = (data[1] << 8) + data[0] + 3;

	// Position pointers
	data = &res->data[text_offset];
	msg = &data[2];

	// Allocate memory
	dst = (char **) MemAlloc ((msg_num * sizeof (char *)));

	for (i=0; i < msg_num; i++, msg += 2) {
		msg_offset = (msg[1] << 8) + msg[0];
		len = strlen ((char *) &data[msg_offset]);
		dst[i] = (char *) malloc (len + 1);
		memset (dst[i], 0, (len + 1));
//		memcpy (dst[i], &data[msg_offset], len);
		strncpy (dst[i], (char *) &data[msg_offset], len);
	}

	return dst;
}

// WARNING: These stack functions bypass memory manager for SPEED!
void stack_push (logic_vm_t *vm, l_byte x)
{
	stack_t *new;

	new = (stack_t *) malloc (sizeof (stack_t));
	if (!new) {
		DEBUG_MSG1 ("FATAL - couldn't malloc for stack\n");
		stack_flush (vm);
		condition_flush (vm);
		exit (1);
	}

	new->data = x;
	new->next = vm->stack;
	vm->stack = new;
}

l_byte stack_pop (logic_vm_t *vm)
{
	l_byte x;
	stack_t *old;

	// Empty stack?
	if (!vm->stack) {
		DEBUG_MSG3 ("FATAL - empty stack [IP=%i CS=%i]\n",
					vm->IP, vm->CS);
		condition_flush (vm);
		exit (1);
	}

	x = vm->stack->data;
	old = vm->stack;
	vm->stack = vm->stack->next;
	free (old);

	return x;
}

void stack_flush (logic_vm_t *vm)
{
	stack_t *old;

	while (vm->stack) {
		old = vm->stack;
		vm->stack = vm->stack->next;
		free (old);
	}
}

// WARNING: These condition functions bypass memory manager for SPEED!
void condition_push (logic_vm_t *vm, int startIP, int length)
{
	condition_t *new;

	new = (condition_t *) malloc (sizeof (condition_t));

	if (!new) {
		DEBUG_MSG1 ("FATAL - couldn't malloc for condition\n");
		stack_flush (vm);
		condition_flush (vm);
		exit (1);
	}

	new->startIP = startIP;
	new->length = length;

	if (vm->condition)
		new->depth = vm->condition->depth + 1;
	else
		new->depth = 0;

	new->next = vm->condition;
	vm->condition = new;
}

void condition_pop (logic_vm_t *vm)
{
	condition_t *old;

	// Empty stack?
	if (!vm->condition) {
		DEBUG_MSG1 ("FATAL - empty condition stack\n");
		stack_flush (vm);
		exit (1);
	}

	old = vm->condition;
	vm->condition = vm->condition->next;
	free (old);
}

void condition_flush (logic_vm_t *vm)
{
	condition_t *old;

	while (vm->condition) {
		old = vm->condition;
		vm->condition = vm->condition->next;
		free (old);
	}
}


#define BUF_SIZE	2048
char decomp_buffer[BUF_SIZE];
int decomp_size = 0, decomp_nl = 0;

void decomp (const char *fmt, ...)
{
	va_list ap;
	char buf[201], ch;
	int i, len;

	if (!decompile_toggle) {
		decomp_size = 0;
		return;
	}

	va_start (ap, fmt);
	vsnprintf (buf, 200, fmt, ap);
	va_end (ap);

	// Scan through, dealing with '\b'
	len = strlen (buf);
	for (i=0; i < len; i++) {
		ch = buf[i];
		if (ch == '\b') {
			decomp_size--;
		} else {
			decomp_buffer[decomp_size] = ch;
			decomp_size++;
		}
		if (ch == '\n')
			decomp_nl++;
	}

	if (decomp_nl > 3)
		decomp_flush ();
}

void decomp_flush (void)
{
	if (!decompile_toggle)
		return;

	if (decomp_size > 0)
		fwrite (decomp_buffer, 1, decomp_size, decompile);
	fflush (decompile);
	decomp_size = 0;
	decomp_nl = 0;
}

void run_virtual_machine (logic_vm_t *vm)
{
	l_byte op, num_args, arg, t[2];
	int i, len;
	const vm_command_t *comm;
	boolean test;

	while (vm->IP < vm->CS) {
		// Flush decompilation buffer
		if (decompile_toggle)
			decomp_flush ();

		// Get next command
		op = vm->code[vm->IP++];

		// End of conditional construct?
		if (vm->condition) {
			if ((vm->condition->startIP + vm->condition->length)
								< vm->IP)
			{
				condition_pop (vm);
if (decompile_toggle) {
	if (vm->condition) {
		for (i=0; i <= vm->condition->depth; i++)
			decomp ("\t");
	}
	decomp ("}\n");
}
			}
		}

// Tab in for conditional constructs
if (decompile_toggle) {
	if (vm->condition) {
		for (i=0; i <= vm->condition->depth; i++)
			decomp ("\t");
	}
}

		// Conditional start? ('if' construct)
		if (op == 0xFF) {
if (decompile_toggle) {
	decomp ("if (");
}
			vm->IP--;
			test = logical_and (vm);

			// Read next two bytes for jump-information
			t[0] = vm->code[vm->IP++];
			t[1] = vm->code[vm->IP++];
			len = (t[1] << 8) + t[0];

			// Push this new conditional structure onto stack
			condition_push (vm, vm->IP, len);

if (decompile_toggle) {
	decomp (") {\t\t[ %i bytes\n", len);
}
			if (test)
				continue;

			// Since the 'if' tested false, jump the braces
			vm->IP += vm->condition->length;

if (decompile_toggle) {
	if (vm->condition) {
		for (i=0; i <= vm->condition->depth; i++)
			decomp ("\t");
	}
	decomp ("[ false\n");
}

			continue;
		}

		// Unconditional jump? ('else', 'goto', etc.)
		if (op == 0xFE) {
			t[0] = vm->code[vm->IP++];
			t[1] = vm->code[vm->IP++];
			len = (t[1] << 8) + t[0];
			if (t[1] & 0x80) {
				// Find two's complement
				t[0] = ~t[0];
				t[1] = ~t[1];
				len = (t[1] << 8) + t[0] + 1;
				len = -len;
			}

if (decompile_toggle) {
	decomp ("JMP %i;\n", len);
}

			// If used as an 'else', end an 'if' bracket
			if (vm->condition) {
				condition_pop (vm);
if (decompile_toggle) {
	if (vm->condition) {
		for (i=0; i <= vm->condition->depth; i++)
			decomp ("\t");
	}
	decomp ("}\n");
}
			}

			vm->IP += len;
			continue;
		}

		// Valid command?
		if (op >= total_action_commands) {
			DEBUG_MSG4 ("CRITICAL - Unrecognised op 0x%02X "
				"[IP=%i CS=%i]\n", op, vm->IP-1, vm->CS);
			break;
		}

		// Get arguments
		comm = &action_commands[op];
		num_args = comm->args;

if (decompile_toggle) {
	decomp ("%s (", comm->name);
}
		for (i=0; i < num_args; i++) {
			arg = vm->code[vm->IP++];
			stack_push (vm, arg);
if (decompile_toggle) {
	decomp ("0x%02X, ", arg);
}
		}

if (decompile_toggle) {
	if (num_args > 0)
		decomp ("\b\b);\n");
	else
		decomp (");\n");
}

		// Return command?
		if (op == 0x00) {
			break;
		}

		// Call interpreter
		if (comm->handler == NULL) {
			// Pop arguments off stack
			for (i=0; i < num_args; i++)
				stack_pop (vm);
			DEBUG_MSG5 ("CRITICAL - Unhandled op 0x%02X (%s)"
				"[IP=%i CS=%i]\n", op, comm->name,
					vm->IP-num_args-1, vm->CS);
			continue;
		}
		comm->handler (vm);

		// If message box came down, check keys
		if ((op == 0x65) || (op == 0x66) ||
		    (op == 0x97) || (op == 0x98))
			driver->key_update ();
	}

	stack_flush (vm);
	condition_flush (vm);
	return;
}

boolean logical_branch (logic_vm_t *vm)
{
	boolean test;

if (decompile_toggle) {
	decomp ("(");
}

	switch (vm->code[vm->IP]) {
		case 0xFF:
			test = logical_and (vm);
			break;
		case 0xFD:
if (decompile_toggle) {
	decomp ("\b");
}
			test = logical_not (vm);
			return test;
			break;
		case 0xFC:
			test = logical_or (vm);
			break;
		default:
			test = false;
	}

if (decompile_toggle) {
	decomp (")");
}

	return test;
}

boolean logical_not (logic_vm_t *vm)
{
	boolean test;

if (decompile_toggle) {
	decomp ("!");
}

	vm->IP++;
	test = evaluate_next_condition (vm);
	test = !test;

	return test;
}

boolean logical_and (logic_vm_t *vm)
{
	l_byte op;
	boolean test;
	int result;

	// Evaluate conditions until I reach the next 0xFF
	vm->IP++;

	// If result > 0 (at end), condition is TRUE
	result = 1;

	do {
		test = false;
		op = vm->code[vm->IP];
		if (op >= total_test_commands) {
			// Nested test command
			test = logical_branch (vm);
		} else {
			test = evaluate_next_condition (vm);
		}
if (decompile_toggle) {
	decomp (" && ");
}

		if (!test)
			result = 0;
	} while (vm->code[vm->IP] != 0xFF);
	vm->IP++;

if (decompile_toggle) {
	decomp ("\b\b\b\b");
}

	if (result > 0)
		return true;
	return false;
}

boolean logical_or (logic_vm_t *vm)
{
	l_byte op;
	boolean test;
	int result;

	// Evaluate conditions until I reach the next 0xFC
	vm->IP++;

	// If result > 0 (at end), condition is TRUE
	result = 0;

	do {
		test = false;
		op = vm->code[vm->IP];
		if (op >= total_test_commands) {
			// Nested test command
			test = logical_branch (vm);
		} else {
			test = evaluate_next_condition (vm);
		}
if (decompile_toggle) {
	decomp (" || ");
}

		if (test)
			result++;
	} while (vm->code[vm->IP] != 0xFC);
	vm->IP++;

if (decompile_toggle) {
	decomp ("\b\b\b\b");
}

	if (result > 0)
		return true;
	return false;
}

boolean evaluate_next_condition (logic_vm_t *vm)
{
	l_byte op, num_args, arg;
	int i;
	const vm_command_t *comm;
	boolean test;

	// Read in next op
	op = vm->code[vm->IP++];

	// Valid command?
	if (op >= total_test_commands) {
		DEBUG_MSG4 ("CRITICAL - Unrecognised op 0x%02X "
			"[IP=%i CS=%i]\n", op, vm->IP-1, vm->CS);
		return false;
	}

	// Get arguments
	comm = &test_commands[op];
	num_args = comm->args;

	// Kludge for 'said' (variable # of args)
	if (op == 0x0E) {
		test = handle_said (vm);
		return test;
	}

if (decompile_toggle) {
	decomp ("%s (", comm->name);
}
	for (i=0; i < num_args; i++) {
		arg = vm->code[vm->IP++];
		stack_push (vm, arg);
if (decompile_toggle) {
	decomp ("0x%02X, ", arg);
}
	}

if (decompile_toggle) {
	if (num_args > 0)
		decomp ("\b\b");
	decomp (")");
}

	// Handler available?
	if (comm->handler == NULL) {
		// Pop arguments off stack
		for (i=0; i < num_args; i++)
			stack_pop (vm);
		DEBUG_MSG5 ("CRITICAL - Unhandled op 0x%02X (%s) "
			"[IP=%i CS=%i]\n", op, comm->name,
				vm->IP-num_args-1, vm->CS);
		return false;
	}

	// Call handler
	comm->handler (vm);

	// Pop result off stack
	test = (boolean) stack_pop (vm);

	return test;
}

boolean handle_said (logic_vm_t *vm)
{
	l_byte t[2];
	int num_args, i, args[10];	// Hope there's less than 10 args

if (decompile_toggle) {
	decomp ("said (");
}
	num_args = vm->code[vm->IP++];
	for (i=0; i < num_args; i++) {
		t[0] = vm->code[vm->IP++];
		t[1] = vm->code[vm->IP++];
		args[i] = (t[1] << 8) + t[0];
if (decompile_toggle) {
	char *s;
	s = text_get_word (args[i]);
	decomp ("\"%s\", ", s);
	MemFree (s);
}
	}

if (decompile_toggle) {
	if (num_args > 0)
		decomp ("\b\b");
	decomp (")");
}

	// Line inputted yet?
	if (!vm_flags[2])
		return false;

	// Already matched?
	if (vm_flags[4])
		return false;

	// Inputted line has too many words?
	if (input_num > num_args)
		return false;

	// Compare args
	for (i=0; i < num_args; i++) {
		if (args[i] == 1)		// "anyword"
			continue;
		if (args[i] == 9999)		// "rol"
			break;
		if (input_num <= i)		// not enough args
			return false;
		if (args[i] != input_line[i])	// mismatch
			return false;
	}

	vm_flags[2] = false;
	vm_flags[4] = true;
	return true;
}
