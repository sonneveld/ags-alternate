#include "allegro.h"
#include "winalleg.h"

// use to get an idea of what's being used so we can port it to SDL or Allegro3

// ignore stuff in the graphics drivers since we'd probably have to write from scratch
// ignore external libraries, they'd have to be rewritten anyway.

// init
#define ALW_ALLEGRO_DATE ALLEGRO_DATE
#define alw_allegro_exit allegro_exit
#define alw_set_window_title set_window_title
#define alw_get_desktop_resolution get_desktop_resolution
#define alw_install_allegro install_allegro
#define alw_set_close_button_callback set_close_button_callback
#define alw_allegro_error allegro_error
#define ALW_END_OF_MAIN END_OF_MAIN

// UNICODE

// used in ac_game for save games
#define alw_uconvert uconvert
// used in ac.cpp 
#define alw_set_uformat set_uformat

// MOUSE
#define alw_mouse_z mouse_z
#define alw_mouse_x mouse_x
#define alw_mouse_y mouse_y
#define alw_mouse_b mouse_b
#define alw_poll_mouse poll_mouse
#define alw_set_mouse_range set_mouse_range
#define alw_position_mouse position_mouse
#define alw_install_mouse install_mouse


// TIMER
#define ALW_END_OF_FUNCTION END_OF_FUNCTION
#define ALW_LOCK_FUNCTION LOCK_FUNCTION
#define ALW_LOCK_VARIABLE LOCK_VARIABLE
#define alw_install_int_ex install_int_ex
#define alw_install_timer install_timer
#define alw_rest rest

// KEYBOARD
#define alw_keypressed keypressed
#define alw_readkey readkey
#define alw_install_keyboard install_keyboard
#define alw_keyboard_needs_poll keyboard_needs_poll
#define alw_poll_keyboard poll_keyboard
#define alw_key_shifts key_shifts
#define alw_set_leds set_leds


// GRAPHICS MODES
#define alw_set_color_depth set_color_depth
#define alw_set_display_switch_callback set_display_switch_callback
#define alw_set_display_switch_mode set_display_switch_mode
#define alw_set_gfx_mode set_gfx_mode
#define alw_get_gfx_mode_list get_gfx_mode_list
#define alw_destroy_gfx_mode_list destroy_gfx_mode_list
#define alw_vsync vsync



// BITMAP
#define alw_bitmap_color_depth bitmap_color_depth
#define alw_destroy_bitmap destroy_bitmap
#define alw_create_bitmap_ex create_bitmap_ex
#define alw_acquire_bitmap acquire_bitmap
#define alw_release_bitmap release_bitmap
#define alw_acquire_screen acquire_screen
#define alw_is_video_bitmap is_video_bitmap
#define alw_release_screen release_screen
#define alw_is_linear_bitmap is_linear_bitmap
#define alw_bitmap_mask_color bitmap_mask_color
#define alw_set_clip_rect set_clip_rect
#define alw_set_clip_state set_clip_state
#define alw_create_sub_bitmap create_sub_bitmap
#define alw_is_memory_bitmap is_memory_bitmap
#define alw_create_bitmap create_bitmap
#define alw_screen screen

// IMAGE FILES
#define alw_save_bitmap save_bitmap
#define alw_load_bitmap load_bitmap
#define alw_load_pcx load_pcx
#define alw_set_color_conversion set_color_conversion

// PALETTE
#define alw_fade_interpolate fade_interpolate
#define alw_set_palette_range set_palette_range
#define alw_get_palette_range get_palette_range
#define alw_set_palette set_palette
#define alw_get_palette get_palette
#define alw_select_palette select_palette
#define alw_unselect_palette unselect_palette
#define alw_black_palette black_palette

// TRUE COLOUR PIXELS
#define alw_makeacol32 makeacol32
#define alw_getr16 getr16
#define alw_getg16 getg16
#define alw_getb16 getb16
#define alw_makeacol_depth makeacol_depth
#define alw_geta32 geta32
#define alw_makecol_depth makecol_depth
#define alw_geta_depth geta_depth
#define alw_getr_depth getr_depth
#define alw_getg_depth getg_depth
#define alw_getb_depth getb_depth
#define alw_getr15 getr15
#define alw_getg15 getg15
#define alw_getb15 getb15
#define alw_makecol15 makecol15
#define alw_makecol16 makecol16
#define alw_getr32 getr32
#define alw_getg32 getg32
#define alw_getb32 getb32
#define alw_makecol8 makecol8
#define ALW_MASK_COLOR_16 MASK_COLOR_16
#define ALW_MASK_COLOR_32 MASK_COLOR_32

// DRAWING
#define alw_getpixel getpixel
#define alw_putpixel putpixel
#define alw_rectfill rectfill
#define alw_hline hline
#define alw_line line
#define alw_do_line do_line
#define alw_floodfill floodfill
#define alw_clear_to_color clear_to_color
#define alw_rect rect
#define alw_triangle triangle
#define alw_circlefill circlefill
#define alw__getpixel _getpixel
#define alw__putpixel _putpixel
#define alw_clear_bitmap clear_bitmap

// BLITTING AND SPRITES
#define alw_blit blit
#define alw_draw_sprite draw_sprite
#define alw_draw_trans_sprite draw_trans_sprite
#define alw_draw_lit_sprite draw_lit_sprite
#define alw_stretch_blit stretch_blit
#define alw_rotate_sprite rotate_sprite
#define alw_draw_sprite_h_flip draw_sprite_h_flip
#define alw_draw_sprite_v_flip draw_sprite_v_flip
#define alw_draw_sprite_vh_flip draw_sprite_vh_flip
#define alw_stretch_sprite stretch_sprite
#define alw_pivot_sprite pivot_sprite


// TRANSPARENCY
#define alw_color_map color_map
#define alw_set_alpha_blender set_alpha_blender
#define alw_set_trans_blender set_trans_blender
#define alw_set_blender_mode set_blender_mode
#define alw_create_light_table create_light_table


// COLOUR FORMATS
#define alw_bestfit_color bestfit_color
#define alw_rgb_to_hsv rgb_to_hsv
#define alw_hsv_to_rgb hsv_to_rgb
#define alw_create_rgb_table create_rgb_table
#define alw_rgb_map rgb_map

// FLI
#define alw_play_fli play_fli

// SOUND INIT
#define alw_install_sound install_sound
#define alw_reserve_voices reserve_voices
#define alw_set_volume set_volume
#define alw_remove_sound remove_sound
#define alw_set_volume_per_voice set_volume_per_voice


// DIGIAL AUDIO
#define alw_voice_set_pan voice_set_pan
#define alw_voice_start voice_start
#define alw_voice_stop voice_stop
#define alw_voice_get_position voice_get_position
#define alw_voice_set_position voice_set_position
#define alw_voice_set_volume voice_set_volume
#define alw_destroy_sample destroy_sample
#define alw_voice_get_frequency voice_get_frequency
#define alw_play_sample play_sample
#define alw_stop_sample stop_sample
#define alw_load_sample load_sample



// MIDI
#define alw_midi_seek midi_seek
#define alw_midi_pos midi_pos
#define alw_stop_midi stop_midi
#define alw_destroy_midi destroy_midi
#define alw_play_midi play_midi
#define alw_midi_pause midi_pause
#define alw_midi_resume midi_resume
#define alw_get_midi_length get_midi_length
#define alw_load_midi load_midi


// AUDIO STREAMS
#define alw_stop_audio_stream stop_audio_stream

// FILE
#define alw_append_filename append_filename
#define alw_fix_filename_case fix_filename_case
#define alw_fix_filename_slashes fix_filename_slashes
#define alw_exists exists
#define alw_pack_fread pack_fread
#define alw_pack_fclose pack_fclose
#define alw_pack_fopen pack_fopen
#define alw_al_findclose al_findclose
#define alw_al_findfirst al_findfirst
#define alw_al_findnext al_findnext
#define alw_put_backslash put_backslash
#define alw_pack_fseek pack_fseek
#define alw_get_filename get_filename
#define alw_file_exists file_exists
#define alw_file_size_ex file_size_ex


// FIXED
#define alw_itofix itofix
#define alw_fixdiv fixdiv
#define alw_fixmul fixmul
#define alw_fixsin fixsin
#define alw_fixcos fixcos
#define alw_fixatan fixatan
#define alw_fixtof fixtof
