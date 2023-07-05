/*
	lzw.h
*/

#ifndef __LZW_H__
#define __LZW_H__

#include <stdio.h>
#include "types.h"

//
// DEFINES
//

#define MIN_BITS	9
#define MAX_BITS	12
#define MAX_ENTRIES	(1 << MAX_BITS)

typedef struct {
	l_byte len;		// Length of string
	l_byte *string;
} lzw_entry;

//
// FUNCTIONS
//

void lzw_init (void);
void lzw_close (void);
void lzw_new_entry (l_byte *string, int len);
void lzw_read_more_bits (FILE *f);
int lzw_get_next_code (FILE *f, int bit_size);
void lzw_restart (void);
void lzw_output_code (int code, l_byte **dst);
void lzw_decompress (FILE *f, l_byte *dst, int num);

#endif	// __LZW_H__
