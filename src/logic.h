/*
	logic.h
*/

#ifndef __LOGIC_H__
#define __LOGIC_H__

#include "resource.h"

//
// DEFINES
//

// VM stack definition
typedef struct stack_s stack_t;
struct stack_s {
	l_byte data;
	stack_t *next;
};

// VM condition definition
typedef struct condition_s condition_t;
struct condition_s {
	int startIP;		// IP of first byte *inside* construct
	int length;		// Number of bytes *inside* construct
	int depth;		// Nesting depth (0 => normal level)
	condition_t *next;
};

// Virtual Machine definition
typedef struct {
	agi_resource_t *res;		// Resource of this LOGIC
	int num_messages;		// Number of private messages
	char **private_messages;	// Messages of this LOGIC
	int IP;				// Instruction Pointer
	int CS;				// Code Size (bytes of LOGIC code)
	l_byte *code;			// Should point to &res->data[2]
	stack_t *stack;			// Top of LIFO stack
	condition_t *condition;		// Top of LIFO condition stack
} logic_vm_t;

#define last_decomp_char	(decomp_buffer[decomp_size - 1])

//
// FUNCTIONS
//

void run_logic (agi_resource_t *res);
void fix_version (void);
int number_of_messages (agi_resource_t *res);
void decrypt_messages (agi_resource_t *res);
void unload_messages (agi_resource_t *res, char **dst);
char **load_messages (agi_resource_t *res);


// Virtual Machine:
void stack_push (logic_vm_t *vm, l_byte x);
l_byte stack_pop (logic_vm_t *vm);
void stack_flush (logic_vm_t *vm);
void condition_push (logic_vm_t *vm, int startIP, int length);
void condition_pop (logic_vm_t *vm);
void condition_flush (logic_vm_t *vm);

void decomp (const char *fmt, ...);
void decomp_flush (void);
void run_virtual_machine (logic_vm_t *vm);

boolean logical_branch (logic_vm_t *vm);
boolean logical_not (logic_vm_t *vm);
boolean logical_and (logic_vm_t *vm);
boolean logical_or (logic_vm_t *vm);
boolean evaluate_next_condition (logic_vm_t *vm);
boolean handle_said (logic_vm_t *vm);


//
// EXTERNALS
//

extern char ***logic_messages;
extern l_byte vm_variables[256];
extern boolean vm_flags[256];
extern char vm_strings[24][40];
extern int agi_horizon;
extern int foreground_color, background_color;
extern boolean text_mode;

extern boolean text_mode, status_line_on, accept_input;
extern boolean dialogue_open;
extern int min_line, user_input_line, status_line;

extern char decomp_buffer[];
extern int decomp_size;

#endif	// __LOGIC_H__
