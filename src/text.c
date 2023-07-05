/*
	text.c
*/

#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "lagii.h"
#include "logic.h"
#include "mem.h"
#include "object.h"
#include "text.h"
#include "utility.h"

l_byte *vocab;
int vocab_len, vocab_offset[26];

int input_line[20], input_num;
char input_words[20][40];

char input_last_line[50];
char input_txt[50], input_cursor;
int input_pos;

char *text_concat (char *one, char *two)
{
	int a = 0, b = 0;
	char *new;

	if (one)
		a = strlen (one);
	if (two)
		b = strlen (two);
	new = MemAlloc (a + b + 1);
	if (one)
		memcpy (new, one, a);
	if (two)
		memcpy (&new[a], two, b);
	return new;
}

char *text_interpret (logic_vm_t *vm, char *msg)
{
	char *ret, *old, ch, buf[30];
	int i, param, num;

	ret = NULL;
	for (i=0; i < strlen (msg); i++) {
		ch = msg[i];
		if (ch != '%') {
			buf[0] = ch;
			buf[1] = 0;
			old = ret;
			ret = text_concat (old, buf);
			MemFree (old);
			continue;
		}
		i++;
		ch = msg[i++];
		sscanf (&msg[i], "%u%n", &param, &num);
		i += num;
		buf[0] = 0;
		if (ch == 'v') {
			char format[20];

			// Check for formatting info.
			if (msg[i] == '|') {
				int width;

				i++;
				sscanf (&msg[i], "%u%n", &width, &num);
				i += num;
				sprintf (format, "%%0%ii", width);
			} else
				sprintf (format, "%%i");
			sprintf (buf, format, vm_variables[param]);
		} else if (ch == 'm') {
			old = ret;
			ret = text_concat (old,
				logic_messages[vm->res->id][param - 1]);
			MemFree (old);
		} else if (ch == '0') {
			old = ret;
			ret = text_concat (old,
				agi_inventory_objects[param].name);
			MemFree (old);
		} else if (ch == 'w') {
if (0) {
int i;
for (i=0; i < input_num; i++) {
DEBUG_MSG3 ("input_words[%i] = '%s'\n", i, input_words[i]);
}
}
			old = ret;
			ret = text_concat (old, input_words[param - 1]);
			MemFree (old);
		} else if (ch == 's') {
			old = ret;
			ret = text_concat (old, vm_strings[param]);
			MemFree (old);
		} else if (ch == 'g') {
			old = ret;
			ret = text_concat (old,
					logic_messages[0][param - 1]);
			MemFree (old);
		} else {
			sprintf (buf, "%%%c%u", ch, param);
		}

		if (buf[0] != 0) {
			old = ret;
			ret = text_concat (old, buf);
			MemFree (old);
		}
		i--;
	}
//DEBUG_MSG3 ("\"%s\" => \"%s\"\n", msg, ret);

	return ret;
}

void text_load_vocabulary (void)
{
	char *fname;
	FILE *f;
	int i;
	l_byte buf[2];

	// Some small things to init.
	input_num = 0;
	input_pos = 0;
	input_cursor = '#';

	fname = (char *) MemAlloc (strlen (data_dir) + 12);
	sprintf (fname, "%s/words.tok", data_dir);

	f = fopen (fname, "rb");
	if (!f) {
		DEBUG_MSG2 ("FATAL - Can't open '%s'.\n", fname);
		exit (1);
	}

	fseek (f, 0, SEEK_END);
	vocab_len = ftell (f) - 52;
	fseek (f, 0, SEEK_SET);

	// Read in offsets
	for (i=0; i < 26; i++) {
		fread (buf, 1, 2, f);
		vocab_offset[i] = (buf[0] << 8) + buf[1];	// MSB!
		if (vocab_offset[i] > 0)
			vocab_offset[i] -= 52;
		else
			vocab_offset[i] = -1;
	}

	// Allocate mem, and store entire file
	vocab = (l_byte *) MemAlloc (vocab_len);
	fread (vocab, 1, vocab_len, f);

	// Clean-up
	MemFree (fname);
	fclose (f);
}

int text_match_phrase (char *txt, int *len)
{
	char cur[40];
	l_byte *p, ch;
	int i, off, best_len, best_id;

	// String OK?
	if ((txt[0] < 'a') || (txt[0] > 'z'))
		return -1;

	// Init. for search
	off = vocab_offset[txt[0] - 'a'];
	if (off < 0)
		return -1;
	p = &vocab[off];
	memset (cur, 0, 40);
	best_len = 0;
	best_id = -1;

	while (p < &vocab[vocab_len]) {
		i = *p++;
		for (ch=0; !(ch & 0x80); i++) {
			if (p >= &vocab[vocab_len]) {
				i = 0;
				break;
			}
			ch = *p++;
			cur[i] = (ch & 0x7F) ^ 0x7F;
		}
		cur[i] = 0;

		// EOF?
		if (i == 0)
			break;

		// Finished words beginning with the same letter?
		if (txt[0] != cur[0])
			break;

		// Too long?
		if (strlen (cur) > strlen (txt)) {
			p += 2;
			continue;
		}

		// Mismatch?
		if (strncmp (txt, cur, i)) {
			p += 2;
			continue;
		}

		// Not a whole word (with a space after it)
		if (strlen (txt) > i) {
			if (txt[i] != ' ') {
				p += 2;
				continue;
			}
		}

		if (i > best_len) {
			best_len = i;
			best_id = (*p++ << 8);
			best_id += *p++;
		} else
			p += 2;
	}

	if (len)
		*len = best_len;
	return best_id;
}

void text_process_input (char *in)
{
	int i, cur, len, match, maxlen;
	char *c, ch, last;
	boolean flag;

	maxlen = strlen (in);

	// Strip out unwanted characters
	c = MemAlloc (maxlen + 1);
	cur = 0;
	last = ' ';
	for (i=0; i < maxlen; i++) {
		ch = in[i];
		// Lowercase it
		if ((ch >= 'A') && (ch <= 'Z'))
			ch += ('a' - 'A');
		// Skip anything but spaces, letters and numbers
		flag = false;
		if ((ch >= 'a') && (ch <= 'z'))
			flag = true;
		if ((ch >= '0') && (ch <= '9'))
			flag = true;
		if ((ch == ' ') && (last != ' '))
			flag = true;
		if (!flag)
			continue;
		c[cur++] = ch;
		last = ch;
	}

	maxlen = strlen (c);
	cur = 0;
	input_num = 0;
	vm_variables[9] = 0;

	if (maxlen < 1) {
		vm_flags[2] = false;
		vm_flags[4] = false;
		return;
	}

	while (cur < maxlen) {
		// Find next non-space
		if (c[cur] == ' ') {
			cur++;
			continue;
		}

		// Next match
		match = text_match_phrase (&c[cur], &len);

		// Store matched word
		if (match > 0) {
			memcpy (input_words[input_num], &c[cur], len);
			input_words[input_num][len] = 0;
		} else if (match == 0) {
			// Do nothing
		} else {
			char *sp = strchr (&c[cur], ' ');
			if (!sp) {
				strncpy (input_words[input_num], &c[cur],
								40);
				input_words[input_num][39] = 0;
			} else {
				int i = (sp - &c[cur]);
				memcpy (input_words[input_num], &c[cur],
								i);
				input_words[input_num][i] = 0;
			}
			vm_variables[9] = input_num + 1;
		}

		cur += len;
//DEBUG_MSG2 ("Got word %i\n", match);

		if (match == 0)
			continue;

		input_line[input_num] = match;
		input_num++;

		if (match < 0)
			break;
	}

{
int i;
DEBUG_MSG1 ("*\n");
for (i=0; i < input_num; i++)
	DEBUG_MSG3 ("Got word '%s' [%i]\n", input_words[i],
						input_line[i]);
}
	if (input_num > 0)
		vm_flags[2] = true;
	else
		vm_flags[2] = false;

	// 'said' hasn't processed this yet
	vm_flags[4] = false;

	MemFree (c);
}

char *text_get_word (int id)
{
	char *ret, cur[40];
	l_byte *p, ch;
	int i, this_id;

	// ID OK?
	if (id < 1) {
		ret = l_strdup ("0x0000");
		return ret;
	}
	if (id == 1) {
		ret = l_strdup ("<anyword>");
		return ret;
	}

	// Init. for search
	p = vocab;
	memset (cur, 0, 40);

	while (p < &vocab[vocab_len]) {
		i = *p++;
		for (ch=0; !(ch & 0x80); i++) {
			if (p >= &vocab[vocab_len]) {
				i = 0;
				break;
			}
			ch = *p++;
			cur[i] = (ch & 0x7F) ^ 0x7F;
		}
		cur[i] = 0;

		// EOF?
		if (i == 0)
			break;

		this_id = (*p++ << 8);
		this_id += *p++;
		if (this_id == id) {
			ret = l_strdup (cur);
			return ret;
		}
	}
	return NULL;
}

void text_new_key (char key)
{
	// Do I want this key?
	if (!accept_input)
		return;
	if ((key < ' ') && (key != '\n') && (key != '\b'))
		return;
	if (key > '~')
		return;

	if (key == '\b') {
		if (input_pos > 0) {
			input_txt[input_pos] = 0;
			input_pos--;
		}
		return;
	}
	if (key == '\n') {
		input_txt[input_pos] = 0;
		memcpy (input_last_line, input_txt, input_pos + 1);
		text_process_input (input_txt);
		input_pos = 0;
		return;
	}

	input_txt[input_pos] = key;
	input_pos++;
	input_txt[input_pos] = 0;
}
