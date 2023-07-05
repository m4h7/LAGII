/*
	mem.h
*/

#ifndef __MEM_H__
#define __MEM_H__

//
// DEFINES
//

// A block of allocated memory
typedef struct mem_s mem_t;
struct mem_s {
        mem_t *prev;
        mem_t *next;
        void *item;
	int size;
};

typedef struct {
	unsigned int smallest;
	unsigned int largest;
	unsigned int total;
} mem_stats_t;

//
// FUNCTIONS
//

void MemInit (void);
void CleanUp (void);
void *MemAlloc (int bytes);
void MemFree (void *it);

#endif	// __MEM_H__
