/*
	resource.h
*/

#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include "types.h"

//
// DEFINES
//

typedef enum {
	picture,
	view,
	logic,
	sound
} resource_type;


typedef struct {

	resource_type type;	// Type of resource
	int id;			// Entry number (in *dir)

	int vol;		// Which volume file (vol.x)
	int offset;		// Offset from start of volume file

	boolean cached;		// In memory? (pointed to by 'data')
	l_byte *data;		// Cached copy of this resource
	int length;		// Length of data (in bytes)

	int ssIP;		// LOGIC-only, for 'set.scan.start', etc.
} agi_resource_t;


// This sets everything, EXCEPT 'type', 'vol' and 'offset'
#define allocate_resource_mem(array,ix)	{			\
	array[ix] = (agi_resource_t *) MemAlloc			\
			(sizeof (agi_resource_t));		\
	array[ix]->id = ix;					\
	array[ix]->cached = false;				\
	array[ix]->data = NULL;					\
	array[ix]->length = 0;					\
	}



//
// FUNCTIONS
//

void resource_init (void);
void resource_load (agi_resource_t *res);
void resource_unload (agi_resource_t *res);

//
// EXTERNALS
//

extern agi_resource_t *PICTURE[256], *VIEW[256], *LOGIC[256], *SOUND[256];

#endif	// __RESOURCE_H__
