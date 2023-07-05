/*
	text.h
*/

#ifndef __TEXT_H__
#define __TEXT_H__

#include "logic.h"

//
// DEFINES
//

//
// FUNCTIONS
//

char *text_concat (char *one, char *two);
char *text_interpret (logic_vm_t *vm, char *msg);
void text_load_vocabulary (void);
int text_match_phrase (char *txt, int *len);
void text_process_input (char *in);
char *text_get_word (int id);
void text_new_key (char key);

//
// EXTERNALS
//

// Processed input
extern char input_words[20][40];
extern int input_line[20], input_num;

// Actual keyboard input
extern char input_last_line[50];
extern char input_txt[50], input_cursor;
extern int input_pos;

#endif	// __TEXT_H__
