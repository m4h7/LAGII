/*
	virtual_machine.h
*/

#ifndef __VIRTUAL_MACHINE_H__
#define __VIRTUAL_MACHINE_H__

#include "logic.h"
#include "types.h"

//
// DEFINES
//

typedef struct {
	l_byte id;		// e.g. 0x05
	char *name;		// e.g. "addn"
	int args;		// e.g. 2
	void (*handler) (logic_vm_t *vm);
} vm_command_t;

//
// FUNCTIONS
//

#define DHANDLER(x)	void x (logic_vm_t *vm)

// Test commands (vm_test.c)
DHANDLER (agi_equaln);
DHANDLER (agi_equalv);
DHANDLER (agi_lessn);
DHANDLER (agi_lessv);
DHANDLER (agi_greatern);
DHANDLER (agi_greaterv);
DHANDLER (agi_isset);
DHANDLER (agi_issetv);
DHANDLER (agi_has);
DHANDLER (agi_obj_in_room);
DHANDLER (agi_posn);
DHANDLER (agi_controller);
DHANDLER (agi_have_key);
DHANDLER (agi_said);
DHANDLER (agi_compare_strings);
DHANDLER (agi_obj_in_box);
DHANDLER (agi_center_posn);
DHANDLER (agi_right_posn);


// Action commands (vm_action.c)
DHANDLER (agi_return);
DHANDLER (agi_increment);
DHANDLER (agi_decrement);
DHANDLER (agi_assignn);
DHANDLER (agi_assignv);
DHANDLER (agi_addn);
DHANDLER (agi_addv);
DHANDLER (agi_subn);
DHANDLER (agi_subv);
DHANDLER (agi_lindirectv);
DHANDLER (agi_rindirect);
DHANDLER (agi_lindirectn);
DHANDLER (agi_set);
DHANDLER (agi_reset);
DHANDLER (agi_toggle);
DHANDLER (agi_set_v);
DHANDLER (agi_reset_v);
DHANDLER (agi_toggle_v);
DHANDLER (agi_new_room);
DHANDLER (agi_new_room_v);
DHANDLER (agi_load_logic);
DHANDLER (agi_load_logic_v);
DHANDLER (agi_call);
DHANDLER (agi_call_v);
DHANDLER (agi_load_pic);
DHANDLER (agi_draw_pic);
DHANDLER (agi_show_pic);
DHANDLER (agi_discard_pic);
DHANDLER (agi_overlay_pic);
DHANDLER (agi_show_pri_screen);
DHANDLER (agi_load_view);
DHANDLER (agi_load_view_v);
DHANDLER (agi_discard_view);
DHANDLER (agi_animate_obj);
DHANDLER (agi_unanimate_all);
DHANDLER (agi_draw);
DHANDLER (agi_erase);
DHANDLER (agi_position);
DHANDLER (agi_position_v);
DHANDLER (agi_get_posn);
DHANDLER (agi_reposition);
DHANDLER (agi_set_view);
DHANDLER (agi_set_view_v);
DHANDLER (agi_set_loop);
DHANDLER (agi_set_loop_v);
DHANDLER (agi_fix_loop);
DHANDLER (agi_release_loop);
DHANDLER (agi_set_cel);
DHANDLER (agi_set_cel_v);
DHANDLER (agi_last_cel);
DHANDLER (agi_current_cel);
DHANDLER (agi_current_loop);
DHANDLER (agi_current_view);
DHANDLER (agi_number_of_loops);
DHANDLER (agi_set_priority);
DHANDLER (agi_set_priority_v);
DHANDLER (agi_release_priority);
DHANDLER (agi_get_priority);
DHANDLER (agi_stop_update);
DHANDLER (agi_start_update);
DHANDLER (agi_force_update);
DHANDLER (agi_ignore_horizon);
DHANDLER (agi_observe_horizon);
DHANDLER (agi_set_horizon);
DHANDLER (agi_object_on_water);
DHANDLER (agi_object_on_land);
DHANDLER (agi_object_on_anything);
DHANDLER (agi_ignore_objs);
DHANDLER (agi_observe_objs);
DHANDLER (agi_distance);
DHANDLER (agi_stop_cycling);
DHANDLER (agi_start_cycling);
DHANDLER (agi_normal_cycle);
DHANDLER (agi_end_of_loop);
DHANDLER (agi_reverse_cycle);
DHANDLER (agi_reverse_loop);
DHANDLER (agi_cycle_time);
DHANDLER (agi_stop_motion);
DHANDLER (agi_start_motion);
DHANDLER (agi_step_size);
DHANDLER (agi_step_time);
DHANDLER (agi_move_obj);
DHANDLER (agi_move_obj_v);
DHANDLER (agi_follow_ego);
DHANDLER (agi_wander);
DHANDLER (agi_normal_motion);
DHANDLER (agi_set_dir);
DHANDLER (agi_get_dir);
DHANDLER (agi_ignore_blocks);
DHANDLER (agi_observe_blocks);
DHANDLER (agi_block);
DHANDLER (agi_unblock);
DHANDLER (agi_get);
DHANDLER (agi_get_v);
DHANDLER (agi_drop);
DHANDLER (agi_put);
DHANDLER (agi_put_v);
DHANDLER (agi_get_room_v);
DHANDLER (agi_load_sound);
DHANDLER (agi_sound);
DHANDLER (agi_stop_sound);
DHANDLER (agi_print);
DHANDLER (agi_print_v);
DHANDLER (agi_display);
DHANDLER (agi_display_v);
DHANDLER (agi_clear_lines);
DHANDLER (agi_text_screen);
DHANDLER (agi_graphics);
DHANDLER (agi_set_cursor_char);
DHANDLER (agi_set_text_attribute);
DHANDLER (agi_shake_screen);
DHANDLER (agi_configure_screen);
DHANDLER (agi_status_line_on);
DHANDLER (agi_status_line_off);
DHANDLER (agi_set_string);
DHANDLER (agi_get_string);
DHANDLER (agi_word_to_string);
DHANDLER (agi_parse);
DHANDLER (agi_get_num);
DHANDLER (agi_prevent_input);
DHANDLER (agi_accept_input);
DHANDLER (agi_set_key);
DHANDLER (agi_add_to_pic);
DHANDLER (agi_add_to_pic_v);
DHANDLER (agi_status);
DHANDLER (agi_save_game);
DHANDLER (agi_restore_game);
DHANDLER (agi_init_disk);
DHANDLER (agi_restart_game);
DHANDLER (agi_show_obj);
DHANDLER (agi_random);
DHANDLER (agi_program_control);
DHANDLER (agi_player_control);
DHANDLER (agi_obj_status_v);
DHANDLER (agi_quit);
DHANDLER (agi_show_mem);
DHANDLER (agi_pause);
DHANDLER (agi_echo_line);
DHANDLER (agi_cancel_line);
DHANDLER (agi_init_joy);
DHANDLER (agi_toggle_monitor);
DHANDLER (agi_version);
DHANDLER (agi_script_size);
DHANDLER (agi_set_game_id);
DHANDLER (agi_log);
DHANDLER (agi_set_scan_start);
DHANDLER (agi_reset_scan_start);
DHANDLER (agi_reposition_to);
DHANDLER (agi_reposition_to_v);
DHANDLER (agi_trace_on);
DHANDLER (agi_trace_info);
DHANDLER (agi_print_at);
DHANDLER (agi_print_at_v);
DHANDLER (agi_discard_view_v);
DHANDLER (agi_clear_text_rect);
DHANDLER (agi_set_upper_left);
DHANDLER (agi_set_menu);
DHANDLER (agi_set_menu_item);
DHANDLER (agi_submit_menu);
DHANDLER (agi_enable_item);
DHANDLER (agi_disable_item);
DHANDLER (agi_menu_input);
DHANDLER (agi_show_obj_v);
DHANDLER (agi_open_dialogue);
DHANDLER (agi_close_dialogue);
DHANDLER (agi_mul_n);
DHANDLER (agi_mul_v);
DHANDLER (agi_div_n);
DHANDLER (agi_div_v);
DHANDLER (agi_close_window);
DHANDLER (agi_unknown170);
DHANDLER (agi_unknown171);
DHANDLER (agi_unknown172);
DHANDLER (agi_unknown173);
DHANDLER (agi_unknown174);
DHANDLER (agi_unknown175);
DHANDLER (agi_unknown176);
DHANDLER (agi_unknown177);
DHANDLER (agi_unknown178);
DHANDLER (agi_unknown179);
DHANDLER (agi_unknown180);
DHANDLER (agi_unknown181);


//
// EXTERNALS
//

extern vm_command_t test_commands[];
extern vm_command_t action_commands[];

extern int total_test_commands;
extern int total_action_commands;

#endif	// __VIRTUAL_MACHINE_H__
