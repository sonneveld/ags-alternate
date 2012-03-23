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
extern int my_readkey();
extern int is_key_pressed (int keycode);

#endif
