/*
	mem.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "mem.h"

static mem_t *mem_root, *last_mem;
mem_stats_t mem_stats;

void MemInit (void)
{
	if (mem_root != NULL)		// Already initialised?!?
		return;

	mem_root = (mem_t *) malloc (sizeof (mem_t));
	mem_root->next = NULL;
	mem_root->prev = NULL;
	mem_root->item = NULL;
	last_mem = mem_root;

	if (atexit (CleanUp) == -1) {		// failed?
		printf ("LAGII Panic: Unable to register CleanUp() with atexit()!!\n");
		exit (1);
	}

	mem_stats.smallest = 0;
	mem_stats.largest = 0;
	mem_stats.total = 0;
}

void CleanUp (void)
{
	mem_t *i;

	if (mem_root == NULL)
		return;

	while (1) {
		if (mem_root->next == NULL)
			break;
		i = mem_root->next;
		if (i->item)
			free (i->item);
		mem_root->next = i->next;
		free (i);
	}

	free (mem_root);
	mem_root = NULL;

//	printf ("\n|--------- MEMORY STATS ---------|\n");
//	printf (" Smallest alloc: %i bytes\n", mem_stats.smallest);
//	printf ("  Largest alloc: %i bytes\n", mem_stats.largest);
//	printf ("    Total alloc: %i bytes\n", mem_stats.total);
//	printf ("|--------------------------------|\n\n");
}

void *MemAlloc (int bytes)
{
	void *new;
	mem_t *this;
	int rb;

	if (mem_root == NULL)
		MemInit ();

	if (bytes < 1) {
		printf ("\n\nLAGII Panic: MemAlloc called with 0 bytes!!\n\n");
		exit (1);
	}

	if (bytes > (500 * 1024)) {
		printf ("\n\nLAGII Panic: "
			"Attempting to malloc > 500K (%i)\n", bytes);
		exit (1);
	}

	rb = bytes;
	if (bytes < 3)
		// Insulating buffer against buggy free()
		rb = 3;

	new = malloc (rb);

	if (!new) {
		printf ("\n\nLAGII Panic: Can't allocate memory!!\n\n");
		exit (1);
	}

	// Zero the allocated memory
	memset (new, '\0', rb);

	this = (mem_t *) malloc (sizeof (mem_t));
	if (!this) {
		perror ("malloc");
		exit (1);
	}
	this->next = NULL;
	this->prev = last_mem;
	last_mem->next = this;
	this->item = new;
	this->size = rb;
	last_mem = this;

//	printf ("MemAlloc (): Just alloc'd %i bytes\n", rb);

	mem_stats.total += rb;
	if (mem_stats.smallest == 0) {
		mem_stats.smallest = rb;
		mem_stats.largest = rb;
	}
	if (rb < mem_stats.smallest)
		mem_stats.smallest = rb;
	if (rb > mem_stats.largest)
		mem_stats.largest = rb;

	return new;
}

void MemFree (void *it)
{
	mem_t *tmp;

	if (it == NULL)
		return;
	if (!mem_root)
		return;

	tmp = mem_root->next;
	while (tmp) {
		if (tmp->item == it) {
			if (tmp->next && tmp->prev) {
				tmp->next->prev = tmp->prev;
				tmp->prev->next = tmp->next;
			} else if (tmp->next) {
				tmp->next->prev = NULL;
			} else if (tmp->prev) {
				tmp->prev->next = NULL;
			}
			if (last_mem == tmp)
				last_mem = tmp->prev;
			free (tmp->item);
			free (tmp);
			return;
		}
		tmp = tmp->next;
	}
DEBUG_MSG2 ("Couldn't find alloc. record for %X\n", (unsigned int) it);
DEBUG_MSG1 ("Dumping records available:\n");
	tmp = mem_root->next;
	while (tmp) {
		fprintf (stderr, "\t%X\n", (unsigned int) tmp->item);
		tmp = tmp->next;
	}
	exit (-1);
}
