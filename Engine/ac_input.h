#ifndef _AC_KEYINPUT_H_HEADER
#define _AC_KEYINPUT_H_HEADER

#define ALLEGRO_KEYBOARD_HANDLER

#ifdef MAC_VERSION
#define EXTENDED_KEY_CODE 0x3f
#else
#define EXTENDED_KEY_CODE 0
#endif

#define AGS_KEYCODE_INSERT 382
#define AGS_KEYCODE_DELETE 383
#define AGS_KEYCODE_ALT_TAB 399
#define READKEY_CODE_ALT_TAB 0x4000

extern void keyboard_input_initialise();

extern void write_record_event (int evnt, int dlen, short *dbuf);
extern void disable_replay_playback ();
extern void done_playback_event (int size);
extern int rec_getch ();
extern int rec_kbhit () ;
extern int rec_iskeypressed (int keycode);
extern int rec_isSpeechFinished () ;
extern int rec_misbuttondown (int but);
extern void PluginSimulateMouseClick(int pluginButtonID);
extern int rec_mgetbutton() ;
extern void rec_domouse (int what);
extern int check_mouse_wheel ();

extern int ac_mgetbutton();
extern void ac_domouse(int str);
extern int ac_misbuttondown(int buno);
extern int ac_kbhit(); 
extern int ac_getch();
extern int is_key_pressed (int keycode);

extern void start_replay_record ();
extern void stop_recording();
extern void start_recording();

extern void register_recording_script_functions();

#endif
