/*
	object.h
*/

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "types.h"

//
// DEFINES
//

// Flags for field 'flags' (below)

// Should this object be animated (false == ignore me *always*)
#define F_ANIMATE	(1 << 0)
// Should I be redrawn each cycle?
#define F_UPDATE_ME	(1 << 1)
// Should I be drawn next cycle?
#define F_DRAW_ME	(1 << 2)
// Priority automatically calculated?
#define F_AUTO_PRI	(1 << 3)
// Pick loop # depending on direction?
#define F_AUTO_LOOP	(1 << 4)
// current_cel++ automatically?
#define F_AUTO_CYCLE	(1 << 5)
// Stay below the horizon? (agi_horizon)
#define F_OBSERVE_HORIZON	(1 << 6)
// Cycle through animations in reverse?
#define F_REVERSE	(1 << 7)
// Object is not allowed to touch water? (priority 3)
#define F_NO_WATER	(1 << 8)
// Object is not allowed to touch land? (everything not priority 3)
#define F_NO_LAND	(1 << 9)
// Should object treat other objects as obstacles?
#define F_OBSERVE_OBJS	(1 << 10)
// Is the object being moved to (tx, ty) automatically? ('move.obj')
#define F_AUTO_MOVE	(1 << 11)
// Chase the Ego? ('follow.ego') (if I catch him, set target_flag)
#define F_FOLLOW_EGO	(1 << 12)
// Wandering around aimlessly?
#define F_WANDER	(1 << 13)
// Observe blocks (priority==1, or blocks set by 'block')?
#define F_OBSERVE_BLOCKS	(1 << 14)


// Structure for VIEW objects (posn, obj.in.box, set.cel, etc.)
typedef struct {
	int flags;		// Bit-field (see above)

	int oldx, oldy;		// Position before possible move
	int x, y;		// Position on screen
	int xsize, ysize;	// Dimensions (in pixels)
	l_byte current_view;	// Current VIEW.x
	l_byte current_loop;	// Current loop in VIEW
	l_byte current_cel;	// Current cel in loop
	l_byte priority;	// Priority (3D visuals)

	// Numbers:
	l_byte num_loops;	// Number of loops in VIEW
	l_byte num_cels;	// Number of cels in loop

	// Movement
	boolean motion;		// Moving? ('start.motion')
	l_byte step_time;	// # of interpreter cycles per step
	l_byte step_size;	// Pixels to move per step
	l_byte tx, ty;		// TargetX, TargetY
	l_byte target_flag;	// Set this flag when target reached

	l_byte half_move;

	// Animation controls:
	l_byte do_once;		// Cycle once, then set this flag
	l_byte cycle_time;	// # of interpreter cycles per update
	l_byte direction;	// 0=stationary, clockwise from North
} agi_object_t;

// Structure for inventory objects (has, obj.in.room, get, drop, etc.)
typedef struct {
	l_byte room;		// Where the object is (255 == Ego has it)
	char *name;		// Name of object
} agi_inventory_object_t;

//
// FUNCTIONS
//

void object_init (void);
void object_recalc (agi_object_t *ob);
void object_update (agi_object_t *ob);
void object_recalc_direction (agi_object_t *ob);
void object_wander (agi_object_t *ob);
void object_follow_ego (agi_object_t *ob);
void object_move (agi_object_t *ob, int dx, int dy);

//
// EXTERNALS
//

extern agi_object_t agi_objects[256];
extern agi_inventory_object_t agi_inventory_objects[256];

#endif	// __OBJECT_H__
