/*
	virtual_machine.c
*/

#include <stdio.h>

#include "types.h"
#include "virtual_machine.h"

vm_command_t test_commands[] = {
	{0x00, "",		0,	NULL},
	{0x01, "equaln",	2,	agi_equaln},
	{0x02, "equalv",	2,	agi_equalv},
	{0x03, "lessn",		2,	agi_lessn},
	{0x04, "lessv",		2,	agi_lessv},
	{0x05, "greatern",	2,	agi_greatern},
	{0x06, "greaterv",	2,	agi_greaterv},
	{0x07, "isset",		1,	agi_isset},
	{0x08, "issetv",	1,	agi_issetv},
	{0x09, "has",		1,	agi_has},
	{0x0A, "obj.in.room",	2,	agi_obj_in_room},
	{0x0B, "posn",		5,	agi_posn},
	{0x0C, "controller",	1,	agi_controller},
	{0x0D, "have.key",	0,	agi_have_key},
	{0x0E, "said",		0,	NULL},
	{0x0F, "compare.strings",2,	agi_compare_strings},
	{0x10, "obj.in.box",	5,	agi_obj_in_box},
	{0x11, "center.posn",	5,	agi_center_posn},
	{0x12, "right.posn",	5,	agi_right_posn}
};

int total_test_commands =
	(sizeof (test_commands) / sizeof (test_commands[0]));

vm_command_t action_commands[] = {
	{0x00, "return", 	0,	agi_return},
	{0x01, "increment", 	1,	agi_increment},
	{0x02, "decrement", 	1,	agi_decrement},
	{0x03, "assignn", 	2,	agi_assignn},
	{0x04, "assignv", 	2,	agi_assignv},
	{0x05, "addn", 		2,	agi_addn},
	{0x06, "addv", 		2,	agi_addv},
	{0x07, "subn", 		2,	agi_subn},
	{0x08, "subv", 		2,	agi_subv},
	{0x09, "lindirectv",	2,	agi_lindirectv},
	{0x0A, "rindirect",	2,	agi_rindirect},
	{0x0B, "lindirectn",	2,	agi_lindirectn},
	{0x0C, "set",		1,	agi_set},
	{0x0D, "reset",		1,	agi_reset},
	{0x0E, "toggle",	1,	agi_toggle},
	{0x0F, "set.v",		1,	agi_set_v},
	{0x10, "reset.v",	1,	agi_reset_v},
	{0x11, "toggle.v",	1,	agi_toggle_v},
	{0x12, "new.room",	1,	agi_new_room},
	{0x13, "new.room.v",	1,	agi_new_room_v},
	{0x14, "load.logic",	1,	agi_load_logic},
	{0x15, "load.logic.v",	1,	agi_load_logic_v},
	{0x16, "call",		1,	agi_call},
	{0x17, "call.v",	1,	agi_call_v},
	{0x18, "load.pic",	1,	agi_load_pic},
	{0x19, "draw.pic",	1,	agi_draw_pic},
	{0x1A, "show.pic",	0,	agi_show_pic},
	{0x1B, "discard.pic",	1,	agi_discard_pic},
	{0x1C, "overlay.pic",	1,	agi_overlay_pic},
	{0x1D, "show.pri.screen",0,	agi_show_pri_screen},
	{0x1E, "load.view",	1,	agi_load_view},
	{0x1F, "load.view.v",	1,	agi_load_view_v},
	{0x20, "discard.view",	1,	agi_discard_view},
	{0x21, "animate.obj",	1,	agi_animate_obj},
	{0x22, "unanimate.all",	0,	agi_unanimate_all},
	{0x23, "draw",		1,	agi_draw},
	{0x24, "erase",		1,	agi_erase},
	{0x25, "position",	3,	agi_position},
	{0x26, "position.v",	3,	agi_position_v},
	{0x27, "get.posn",	3,	agi_get_posn},
	{0x28, "reposition",	3,	agi_reposition},
	{0x29, "set.view",	2,	agi_set_view},
	{0x2A, "set.view.v",	2,	agi_set_view_v},
	{0x2B, "set.loop",	2,	agi_set_loop},
	{0x2C, "set.loop.v",	2,	agi_set_loop_v},
	{0x2D, "fix.loop",	1,	agi_fix_loop},
	{0x2E, "release.loop",	1,	agi_release_loop},
	{0x2F, "set.cel",	2,	agi_set_cel},
	{0x30, "set.cel.v",	2,	agi_set_cel_v},
	{0x31, "last.cel",	2,	agi_last_cel},
	{0x32, "current.cel",	2,	agi_current_cel},
	{0x33, "current.loop",	2,	agi_current_loop},
	{0x34, "current.view",	2,	agi_current_view},
	{0x35, "number.of.loops",2,	agi_number_of_loops},
	{0x36, "set.priority",	2,	agi_set_priority},
	{0x37, "set.priority.v",2,	agi_set_priority_v},
	{0x38, "release.priority",1,	agi_release_priority},
	{0x39, "get.priority",	2,	agi_get_priority},
	{0x3A, "stop.update",	1,	agi_stop_update},
	{0x3B, "start.update",	1,	agi_start_update},
	{0x3C, "force.update",	1,	agi_force_update},
	{0x3D, "ignore.horizon",1,	agi_ignore_horizon},
	{0x3E, "observe.horizon",1,	agi_observe_horizon},
	{0x3F, "set.horizon",	1,	agi_set_horizon},
	{0x40, "object.on.water",1,	agi_object_on_water},
	{0x41, "object.on.land",1,	agi_object_on_land},
	{0x42, "object.on.anything",1,	agi_object_on_anything},
	{0x43, "ignore.objs",	1,	agi_ignore_objs},
	{0x44, "observe.objs",	1,	agi_observe_objs},
	{0x45, "distance",	3,	agi_distance},
	{0x46, "stop.cycling",	1,	agi_stop_cycling},
	{0x47, "start.cycling",	1,	agi_start_cycling},
	{0x48, "normal.cycle",	1,	agi_normal_cycle},
	{0x49, "end.of.loop",	2,	agi_end_of_loop},
	{0x4A, "reverse.cycle",	1,	agi_reverse_cycle},
	{0x4B, "reverse.loop",	2,	agi_reverse_loop},
	{0x4C, "cycle.time",	2,	agi_cycle_time},
	{0x4D, "stop.motion",	1,	agi_stop_motion},
	{0x4E, "start.motion",	1,	agi_start_motion},
	{0x4F, "step.size",	2,	agi_step_size},
	{0x50, "step.time",	2,	agi_step_time},
	{0x51, "move.obj",	5,	agi_move_obj},
	{0x52, "move.obj.v",	5,	agi_move_obj_v},
	{0x53, "follow.ego",	3,	agi_follow_ego},
	{0x54, "wander",	1,	agi_wander},
	{0x55, "normal.motion",	1,	agi_normal_motion},
	{0x56, "set.dir",	2,	agi_set_dir},
	{0x57, "get.dir",	2,	agi_get_dir},
	{0x58, "ignore.blocks",	1,	agi_ignore_blocks},
	{0x59, "observe.blocks",1,	agi_observe_blocks},
	{0x5A, "block",		4,	agi_block},	// FIXME
	{0x5B, "unblock",	0,	agi_unblock},	// FIXME
	{0x5C, "get",		1,	agi_get},
	{0x5D, "get.v",		1,	agi_get_v},
	{0x5E, "drop",		1,	agi_drop},
	{0x5F, "put",		2,	agi_put},
	{0x60, "put.v",		2,	agi_put_v},
	{0x61, "get.room.v",	2,	agi_get_room_v},
	{0x62, "load.sound",	1,	agi_load_sound},
	{0x63, "sound",		2,	agi_sound},	// FIXME
	{0x64, "stop.sound",	0,	agi_stop_sound},	// FIXME
	{0x65, "print",		1,	agi_print},	// FIXME
	{0x66, "print.v",	1,	agi_print_v},
	{0x67, "display",	3,	agi_display},	// FIXME
	{0x68, "display.v",	3,	agi_display_v},
	{0x69, "clear.lines",	3,	agi_clear_lines},	// FIXME
	{0x6A, "text.screen",	0,	agi_text_screen},	// FIXME
	{0x6B, "graphics",	0,	agi_graphics},		// FIXME
	{0x6C, "set.cursor.char",1,	agi_set_cursor_char},	// FIXME
	{0x6D, "set.text.attribute",2,	agi_set_text_attribute},
	{0x6E, "shake.screen",	1,	agi_shake_screen},
	{0x6F, "configure.screen",3,	agi_configure_screen},	// FIXME
	{0x70, "status.line.on",0,	agi_status_line_on},	// FIXME
	{0x71, "status.line.off",0,	agi_status_line_off},	// FIXME
	{0x72, "set.string",	2,	agi_set_string},
	{0x73, "get.string",	5,	agi_get_string},	// FIXME
	{0x74, "word.to.string",2,	agi_word_to_string},	// FIXME
	{0x75, "parse",		1,	agi_parse},	// FIXME
	{0x76, "get.num",	2,	agi_get_num},	// FIXME
	{0x77, "prevent.input",	0,	agi_prevent_input},	// FIXME
	{0x78, "accept.input",	0,	agi_accept_input},	// FIXME
	{0x79, "set.key",	3,	agi_set_key},
	{0x7A, "add.to.pic",	7,	agi_add_to_pic},
	{0x7B, "add.to.pic.v",	7,	agi_add_to_pic_v},
	{0x7C, "status",	0,	agi_status},	// FIXME
	{0x7D, "save.game",	0,	agi_save_game},	// FIXME
	{0x7E, "restore.game",	0,	agi_restore_game},	// FIXME
	{0x7F, "init.disk",	0,	agi_init_disk},	// FIXME
	{0x80, "restart.game",	0,	agi_restart_game},	// FIXME
	{0x81, "show.obj",	1,	agi_show_obj},
	{0x82, "random",	3,	agi_random},
	{0x83, "program.control",0,	agi_program_control},
	{0x84, "player.control",0,	agi_player_control},
	{0x85, "obj.status.v",	1,	agi_obj_status_v},	// FIXME
	{0x86, "quit",		1,	agi_quit},	// FIXME
	{0x87, "show.mem",	0,	agi_show_mem},
	{0x88, "pause",		0,	agi_pause},
	{0x89, "echo.line",	0,	agi_echo_line},	// FIXME
	{0x8A, "cancel.line",	0,	agi_cancel_line},	// FIXME
	{0x8B, "init.joy",	0,	agi_init_joy},
	{0x8C, "toggle.monitor",0,	agi_toggle_monitor},
	{0x8D, "version",	0,	agi_version},
	{0x8E, "script.size",	1,	agi_script_size},	// FIXME
	{0x8F, "set.game.id",	1,	agi_set_game_id},
	{0x90, "log",		1,	agi_log},	// FIXME
	{0x91, "set.scan.start",0,	agi_set_scan_start},
	{0x92, "reset.scan.start",0,	agi_reset_scan_start},
	{0x93, "reposition.to",	3,	agi_reposition_to},
	{0x94, "reposition.to.v",3,	agi_reposition_to_v},
	{0x95, "trace.on",	0,	agi_trace_on},
	{0x96, "trace.info",	3,	agi_trace_info},
	{0x97, "print.at",	4,	agi_print_at},	// FIXME
	{0x98, "print.at.v",	4,	agi_print_at_v},	// FIXME
	{0x99, "discard.view.v",1,	agi_discard_view_v},
	{0x9A, "clear.text.rect",5,	agi_clear_text_rect},	// FIXME
	{0x9B, "set.upper.left",2,	agi_set_upper_left},	// FIXME
	{0x9C, "set.menu",	1,	agi_set_menu},	// FIXME
	{0x9D, "set.menu.item",	2,	agi_set_menu_item},	// FIXME
	{0x9E, "submit.menu",	0,	agi_submit_menu},	// FIXME
	{0x9F, "enable.item",	1,	agi_enable_item},	// FIXME
	{0xA0, "disable.item",	1,	agi_disable_item},	// FIXME
	{0xA1, "menu.input",	0,	agi_menu_input},	// FIXME
	{0xA2, "show.obj.v",	1,	agi_show_obj_v},
	{0xA3, "open.dialogue",	0,	agi_open_dialogue},	// FIXME
	{0xA4, "close.dialogue",0,	agi_close_dialogue},	// FIXME
	{0xA5, "mul.n",		2,	agi_mul_n},
	{0xA6, "mul.v",		2,	agi_mul_v},
	{0xA7, "div.n",		2,	agi_div_n},
	{0xA8, "div.v",		2,	agi_div_v},
	{0xA9, "close.window",	0,	agi_close_window},	// FIXME
	{0xAA, "unknown170",	1,	agi_unknown170},
	{0xAB, "unknown171",	0,	agi_unknown171},
	{0xAC, "unknown172",	0,	agi_unknown172},
	{0xAD, "unknown173",	0,	agi_unknown173},
	{0xAE, "unknown174",	1,	agi_unknown174},
	{0xAF, "unknown175",	1,	agi_unknown175},
	{0xB0, "unknown176",	0,	agi_unknown176},
	{0xB1, "unknown177",	1,	agi_unknown177},
	{0xB2, "unknown178",	0,	agi_unknown178},
	{0xB3, "unknown179",	4,	agi_unknown179},
	{0xB4, "unknown180",	2,	agi_unknown180},
	{0xB5, "unknown181",	0,	agi_unknown181}
};

int total_action_commands =
	(sizeof (action_commands) / sizeof (action_commands[0]));
