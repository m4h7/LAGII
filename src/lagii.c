/*
	lagii.c
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "driver_main.h"
#include "events.h"
#include "graphics.h"
#include "lagii.h"
#include "logic.h"
#include "mem.h"
#include "menu.h"
#include "object.h"
#include "resource.h"
#include "text.h"
#include "version.h"


// This saves me from including the whole of virtual_machine.h
void agi_version (logic_vm_t *vm);

sierra_game_t sierra_game[] = {

	// Space Quest series
	{0xDA3E, "Space Quest 1 (2.2)", "sq1", {2, 917, 0}},
	{0x8EFA, "Space Quest 2 (2.0F)", "sq2", {2, 936, 0}},

	// Police Quest series
	{0xB78F, "Police Quest 1 (2.0G)", "pq1", {2, 917, 0}},

	// Kings Quest series
	{0xB40A, "Kings Quest 1 (2.0F)", "kq1", {2, 917, 0}},
	{0xC1AF, "Kings Quest 2 (2.2)", "kq2", {2, 917, 0}},
	{0x7574, "Kings Quest 3 (1.01)", "kq3", {2, 272, 0}},

	// Manhunter series
	{0x8820, "Manhunter 2 (3.0?)", "mh2", {3, 2, 149}},

	// Other games
	{0xC574, "Black Cauldron (2.00)", "bc", {2, 439, 0}},
	{0x0F25, "Lesiure Suit Larry 1 (1.00)", "lsl1", {2, 440, 0}},
	{0xFA73, "Mixed Up Mother Goose (?.??)", "mg", {2, 915, 0}},

	// Demos
	{0xE5DB, "AGI Xmas (?.??)", "xmas", {2, 272, 0}},
	{0x2E0B, "Demo Pack 1", "dem1", {2, 915, 0}},
	{0xECD7, "Demo Pack 2", "dem2", {2, 915, 0}},
	{0x58C6, "Demo Pack 3", "dem3", {2, 917, 0}},

	// Fan-made games
	{0x7F38, "Acidopolis (?.??)", "acid", {2, 917, 0}},
	{0x51F7, "Ruby Cast Demo (0.2)", "ruby", {2, 936, 0}},

	//========================
	// Untested (personally):
	//========================

	// Christoph Ender <chrender@wetterau.de>
	// interpreter version unknown
	{0x6EA0, "Space Quest 1 (?.??)", "sq1", {2, 917, 0}},
};
#define sierra_games_total	\
	(sizeof (sierra_game) / sizeof (sierra_game[0]))


FILE *volumes[15];
char *data_dir;
int game_id;
boolean new_room;
boolean program_control, quitting;
int cycle_count;

boolean decompile_toggle;
FILE *decompile;
boolean turbo_toggle;
boolean obj_toggle;
boolean pri_toggle;

void syntax (void)
{
	printf ("usage: ");
	printf ("lagii [-v] [-h] [-t] [-d <dump-file>] [-p] "
						"<game-dir>\n\n");
	printf ("\t-v\tPrint version information\n");
	printf ("\t-h\tShow this screen\n");
	printf ("\t-t\tTurbo mode (quite silly, actually)\n");
	printf ("\t-o\tShow object information\n");
	printf ("\t-d\tDecompile LOGIC resources to <dump-file> as "
							"they are run\n");
	printf ("<game-dir>\tDirectory holding AGI files\n");
}

int main (int argc, char *argv[])
{
	int len, i, o;
	agi_object_t *ego;
	l_byte *tmp;

	// Print info
	printf ("\n----------\n");
	printf ("%s v%s - %s\n", LAGII_TITLE,
		LAGII_VERSION, LAGII_LONG_TITLE);
	printf ("\t(c) 1999 XoXus\n");
	printf ("----------\n\n");

	// Initialize memory manager
	MemInit ();

	// Setup default options
	decompile_toggle = false;
	decompile = NULL;
	turbo_toggle = false;
	obj_toggle = false;

	// Get command-line arguments
	o = getopt (argc, argv, "vhtod:");
	while (o != -1) {
		switch (o) {
			case 'v':
				// Ummm....it's printed anyway!
				exit (0);
				break;
			case 'h':
				syntax ();
				exit (0);
				break;
			case 't':
				turbo_toggle = true;
				break;
			case 'o':
				obj_toggle = true;
				break;
			case 'd':
				decompile_toggle = true;
				decompile = fopen (optarg, "w");
				if (!decompile) {
					decompile_toggle = false;
					DEBUG_MSG2 ("'%s': ", optarg);
					perror ("fopen");
					exit (1);
				} else
					printf ("Decompiling to '%s'\n",
							optarg);
				break;
			default:
				syntax ();
				exit (0);
		}

		o = getopt (argc, argv, "vhtod:");
	};

	// WARNING: I am assuming the next non-arg option is game-dir
	if (argv[optind] == NULL) {
		syntax ();
		exit (0);
	}

	// Trim a possible trailing slash
	len = strlen (argv[optind]);
	if (argv[optind][len - 1] == '/')
		len--;
	data_dir = (char *) MemAlloc (len + 1);
	strncpy (data_dir, argv[optind], len);

	// Try and detect game id
	game_id = get_id (data_dir);
	if (game_id < 0) {
		printf ("Couldn't determine game....."
			"Maybe not an AGI game?\n\n");
		printf ("If this *IS* an AGI game, please e-mail the "
			"author (xoxus@usa.net) with as much of the "
			"following information as possible:\n");
		printf ("\t* Game name\n"
			"\t* CRC reported above\n"
			"\t* Sierra interpreter version\n"
			"\t* Game version\n");
		return 1;
	} else {
		DEBUG_MSG2 ("Game name => %s\n",
				sierra_game[game_id].title);
		if (sierra_game[game_id].agi_version[2] == 0) {
			DEBUG_MSG3 ("AGI version => %i.%03i\n",
				sierra_game[game_id].agi_version[0],
				sierra_game[game_id].agi_version[1]);
		} else {
			DEBUG_MSG4 ("AGI version => %i.%03i.%03i\n",
				sierra_game[game_id].agi_version[0],
				sierra_game[game_id].agi_version[1],
				sierra_game[game_id].agi_version[2]);
		}
	}

	// No volumes should be open
	for (i=0; i < 10; i++)
		volumes[i] = NULL;

	// Initialize resource management
	resource_init ();

	// Initialize objects
	object_init ();

	// Initialize menu
	menu_init ();

	// Patch action_commands[] for some versions
	fix_version ();

	// Initialize virtual machine
	for (i=0; i < 256; i++) {
		vm_variables[i] = 0;
		vm_flags[i] = false;
	}
	for (i=0; i < 24; i++)
		memset (vm_strings[i], 0, 40);
	i = 256 * sizeof (char **);
	logic_messages = (char ***) MemAlloc (i);

	// Initialize vocabulary
	text_load_vocabulary ();

	// Initialize graphics
	driver_init (0);
	if (!driver)
		exit (1);
	driver->init ();

	// Initialize event handling
	event_init ();

	// Put up message box
	agi_version (NULL);

	//============================
	// START OF INTERPRETER CYCLE
	//============================

	vm_flags[5] = true;	// New room ON
	vm_flags[6] = false;	// Restart game OFF
	vm_flags[9] = true;	// Default to sound ON

	vm_variables[22] = 1;	// PC sound
	vm_variables[24] = 38;	// Max characters on a line
	vm_variables[26] = 3;	// EGA monitor

	new_room = false;
	agi_horizon = 36;
	program_control = false;	// Player control by default
	quitting = false;		// We wanna play, eh?
	cycle_count = -1;

	// Default text attributes
	foreground_color = 15;
	background_color = 0;

	// Handy pointer
	ego = &agi_objects[0];
	while (!quitting) {
		// STEP 1 - Increment cycle_count
		cycle_count++;
		if (vm_variables[10] > 0)
			if ((cycle_count % (20 / vm_variables[10])) == 0)
				add_to_timer (1);

		// STEP 2 - Clear the keyboard buffer
//		event_flush ();

		// 2(a)
		vm_flags[2] = false;
		vm_flags[4] = false;

		// STEP 3 - Poll keyboard (and joystick)
		event_update ();

		// 3(a)
		// Whether 'program.control' or 'player.control'.....
		if (program_control) {
			ego->direction = vm_variables[6];
		} else {
			vm_variables[6] = ego->direction;
		}

		// 3(b)
		// For all obj's for which 'animate.obj',
		//	'start.update' and 'draw' were done,
		//	recalculate direction of movement
		for (i=0; i < 256; i++) {
			agi_object_t *ob;
			ob = &agi_objects[i];
//			if (ob->flags & F_ANIMATE)
//			if ((ob->flags & F_ANIMATE) &&
//			    (ob->flags & F_UPDATE_ME) &&
//			    (ob->flags & F_DRAW_ME))
			if ((ob->flags & F_ANIMATE) &&
			    ((ob->flags & F_UPDATE_ME) ||
			     (ob->flags & F_DRAW_ME)))
				object_recalc_direction (ob);
		}

		// 3(c)
		// FIXME: If the score changed (Var(3)), or the sound
		//		toggled (Flag(9)), update status line

RunLogicZero:
		// STEP 4 - Run LOGIC.0
		run_logic (LOGIC[0]);

		// 4(a)
		// (i) Dir. of motion of Ego <-- Var(6)
//		ego->direction = vm_variables[6];
		// FIXME: Update status line (see 3(c))

		// 4(b)
		vm_variables[4] = 0;
		vm_variables[5] = 0;
//		vm_variables[9] = 0;	// <word-not-found>: Leave in?
		vm_flags[5] = false;
		vm_flags[6] = false;
		vm_flags[12] = false;

		// 4(c) Update sprites
		memcpy (video_buffer, background_buffer, 320 * 200);
		memcpy (tmp_buffer, priority_buffer, 320 * 200);
		tmp = priority_buffer;
		priority_buffer = tmp_buffer;

		for (i=255; i >= 0; i--) {
			if (agi_objects[i].flags & F_ANIMATE) {
				object_update (&agi_objects[i]);
			}
		}
		if (pri_toggle)
			memcpy (video_buffer, priority_buffer, 320 * 200);

		priority_buffer = tmp;

		// Show text input line
		if (accept_input) {
			write_char (video_buffer, 0, user_input_line*8,
							']', 15, 0);
			for (i=0; i < input_pos; i++)
				write_char (video_buffer, i*8+8,
					user_input_line*8, input_txt[i],
								15, 0);
			write_char (video_buffer, input_pos*8+8,
					user_input_line*8, input_cursor,
								15, 0);
		}

		// Show status line
		if (status_line_on) {
			char st[40];

			sprintf (st, "        %i of %i",
				vm_variables[3], vm_variables[7]);
			memset (video_buffer, 15, 320 * 8);
			write_string (video_buffer, 0, status_line * 8,
							st, 0, 15);
		}

		driver->blit (video_buffer);

		if (!turbo_toggle)
			usleep (vm_variables[10] * 50 * 1000);

		// 4(d)
		if (new_room) {
			new_room = false;
			vm_flags[2] = false;
			vm_flags[5] = true;
			goto RunLogicZero;
		}
	}

	// Clean-up
	if (decompile_toggle) {
		decomp_flush ();
		fclose (decompile);
	}
	driver->close ();
	driver_close ();

	return 0;
}

// Returns an index number of the ID of AGI game in directory 'dir'
// (see table at top of lagii.c (this file) for more info)
int get_id (char *dir)
{
	l_byte ch;
	FILE *f;
	int crc, id;
	char *fname, *v3[] = {"mh1", "mh2", 0};

	// Compute game checksum from VOL.0
	fname = MemAlloc (strlen (dir) + 15);
	sprintf (fname, "%s/vol.0", dir);
	f = fopen (fname, "rb");
	if (!f) {
		// Try anything in the form *vol.0 (AGI v3)
		for (id=0; v3[id]; id++) {
			sprintf (fname, "%s/%svol.0", dir, v3[id]);
			f = fopen (fname, "rb");
			if (f)
				break;
		}
		if (!f)
			return -1;
	}

	crc = 0;
	while (!feof (f)) {
		ch = fgetc (f);
		crc += ch;
		crc %= 0xFFFF;
	}

	// Search table
	for (id=0; id < sierra_games_total; id++)
		if (sierra_game[id].crc == crc)
			return id;

DEBUG_MSG2 ("CRC = 0x%04X\n", crc);

	return -1;
}

void add_to_timer (int t)
{
	l_byte *d, *h, *m, *s;

	s = &vm_variables[11];
	m = &vm_variables[12];
	h = &vm_variables[13];
	d = &vm_variables[14];

	*s += t;
	if (*s < 60)
		return;

	*m += (*s / 60);
	*s %= 60;
	if (*m < 60)
		return;

	*h += (*m / 60);
	*m %= 60;
	if (*h < 24)
		return;

	*d += (*h / 24);
	*h %= 24;
}
