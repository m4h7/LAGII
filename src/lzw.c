/*
	lzw.c
*/

#include <stdio.h>

#include "debug.h"
#include "lzw.h"
#include "mem.h"
#include "resource.h"
#include "types.h"
#include "utility.h"


#define BUF_SIZE	32
l_byte bit_buffer[BUF_SIZE];
int bit_buffer_size;

lzw_entry *string_table;
int string_table_size;

void lzw_init (void)
{
	int i;

	string_table = MemAlloc (MAX_ENTRIES * sizeof (lzw_entry));

	// Starting codes
	for (i=0; i < 256; i++) {
		string_table[i].len = 1;
		string_table[i].string = MemAlloc (1);
		string_table[i].string[0] = i;
	}

	// Special codes: 256 = restart, 257 = EOF
	string_table_size = 258;
}

void lzw_close (void)
{
	int i;

	for (i=0; i < string_table_size; i++) {
		if ((i != 256) && (i != 257))
			MemFree (string_table[i].string);
	}
	MemFree (string_table);
}

void lzw_new_entry (l_byte *string, int len)
{
	int i;

	i = string_table_size;
//DEBUG_MSG3 ("Adding 0x%03X [len=%i]\n", i, len);
	string_table[i].len = len;
	string_table[i].string = MemAlloc (len);
	memcpy (string_table[i].string, string, len);

	string_table_size++;
}

void lzw_read_more_bits (FILE *f)
{
	l_byte c;
	int i;

	c = fgetc (f);
//DEBUG_MSG2 ("Read character 0x%02X\n", c);

//	for (i=7; i >= 0; i--) {
	for (i=0; i < 8; i++) {
		if (c & (1 << i))
			bit_buffer[bit_buffer_size] = 1;
		else
			bit_buffer[bit_buffer_size] = 0;
		bit_buffer_size++;
	}
}

int lzw_get_next_code (FILE *f, int bit_size)
{
	int ret, i, j;

	while (bit_size > bit_buffer_size)
		lzw_read_more_bits (f);

	ret = 0;
//	for (i=(bit_size - 1); i >= 0; i--) {
	for (i=0; i < bit_size; i++) {
		// Read next bit
		ret += (bit_buffer[0] << i);

		// Shrink bit_buffer
		bit_buffer_size--;

		// Shift bit_buffer
		for (j=0; j < bit_buffer_size; j++)
			bit_buffer[j] = bit_buffer[j + 1];
	}

	return ret;
}

void lzw_restart (void)
{
	int i;

	for (i=258; i < string_table_size; i++)
		MemFree (string_table[i].string);

	string_table_size = 258;
}

void lzw_output_code (int code, l_byte **dst)
{
	memcpy (*dst, string_table[code].string, string_table[code].len);
#if 0
{
int i;
DEBUG_MSG2 ("Outputting (0x%03X): ", code);
for (i=0; i < string_table[code].len; i++)
	fprintf (stderr, "<%02X>", string_table[code].string[i]);
fprintf (stderr, "\n");
}
#endif
	*dst += string_table[code].len;
}

void lzw_decompress (FILE *f, l_byte *dst, int num)
{
	int current_code, old_code, bit_size;
	l_byte *st, *new_string;

	// Step 1 - Init.
	lzw_init ();
	bit_size = 9;
	bit_buffer_size = 0;
	st = dst;
	old_code = 0;

	// Step 2 - Get first code
//	current_code = lzw_get_next_code (f, bit_size);
//DEBUG_MSG2 ("Step 2: current_code = 0x%03X\n", current_code);

	// Step 3 - output first code
//	lzw_output_code (current_code, &dst);

	// Step 4 - old_code = current_code
//	old_code = current_code;
//DEBUG_MSG2 ("Step 4: old_code = 0x%03X\n", old_code);

	while (1) {
		// Check for overrun
		if ((st + num) <= dst)
			break;

		if ((string_table_size) > (1 << bit_size)) {
			// Bump bit_size up
			bit_size++;
//DEBUG_MSG2 ("Shifting bit_size to %i\n", bit_size);
		}

		if (bit_size > 12) {
			bit_size = 12;
//DEBUG_MSG2 ("Restraining bit_size to %i\n", bit_size);
		}

		// Step 5 - Get next code
		current_code = lzw_get_next_code (f, bit_size);
//DEBUG_MSG2 ("Step 5: current_code = 0x%03X\n", current_code);
		if (current_code == 257)
			break;
		if (current_code == 256) {
			lzw_restart ();
			bit_size = 8;
//DEBUG_MSG1 ("Restarting.\n");
			continue;
		}

		// Step 6 - Handle code
		if (current_code < string_table_size) {
			lzw_output_code (current_code, &dst);

//DEBUG_MSG3 ("(1)string_table[%03X].len = %i\n", old_code,
//string_table[old_code].len);

			new_string = MemAlloc
				(string_table[old_code].len + 1);
			memcpy (new_string,
				string_table[old_code].string,
				string_table[old_code].len);
			new_string[string_table[old_code].len] =
				string_table[current_code].string[0];
			lzw_new_entry (new_string,
					string_table[old_code].len + 1);
			MemFree (new_string);
		} else {

//DEBUG_MSG3 ("(2)string_table[%03X].len = %i\n", old_code,
//string_table[old_code].len);

			new_string = MemAlloc
				(string_table[old_code].len + 1);
			memcpy (new_string,
				string_table[old_code].string,
				string_table[old_code].len);
			new_string[string_table[old_code].len] =
					string_table[old_code].string[0];
			lzw_new_entry (new_string,
					string_table[old_code].len + 1);
			MemFree (new_string);
			lzw_output_code (string_table_size - 1, &dst);
		}

		// Step 7 - old_code = current_code
		old_code = current_code;
	}

	// Step 8 - Clean-up
	lzw_close ();
}
