/*
	lagii.h
*/

#ifndef __LAGII_H__
#define __LAGII_H__

#include <stdio.h>
#include "types.h"

//
// DEFINES
//

typedef struct {
	int crc;		// Sum of bytes in VOL.0, mod 0xFFFF
	char *title;		// e.g. "Space Quest 1"
	char *short_title;	// e.g. "SQ1"
	int agi_version[3];	// e.g. 2.917 => {2, 917, 0}
				// e.g. 3.002.149 => {3, 2, 149}
} sierra_game_t;

#define is_v3	((sierra_game[game_id].agi_version[0] == 3) ? 1 : 0)
#define v3_short	(sierra_game[game_id].short_title)

//
// FUNCTIONS
//

void syntax (void);
int get_id (char *dir);
void add_to_timer (int t);

//
// EXTERNALS
//

extern sierra_game_t sierra_game[];
extern FILE *volumes[15];
extern char *data_dir;
extern int game_id;
extern boolean new_room;
extern boolean program_control, quitting;
extern int cycle_count;

extern boolean decompile_toggle;
extern FILE *decompile;
extern boolean turbo_toggle, obj_toggle, pri_toggle;

#endif	// __LAGII_H__
