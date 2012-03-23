#include "ac_keyinput.h"

#include "allegro.h"
#include "ac.h"
#include "ac_context.h"


// KEYBOARD HANDLER

int my_readkey() {
  int gott=readkey();
  int scancode = ((gott >> 8) & 0x00ff);

  if (gott == READKEY_CODE_ALT_TAB)
  {
    // Alt+Tab, it gets stuck down unless we do this
    return AGS_KEYCODE_ALT_TAB;
  }

/*  char message[200];
  sprintf(message, "Scancode: %04X", gott);
  OutputDebugString(message);*/

  /*if ((scancode >= KEY_0_PAD) && (scancode <= KEY_9_PAD)) {
    // fix numeric pad keys if numlock is off (allegro 4.2 changed this behaviour)
    if ((key_shifts & KB_NUMLOCK_FLAG) == 0)
      gott = (gott & 0xff00) | EXTENDED_KEY_CODE;
  }*/

  if ((gott & 0x00ff) == EXTENDED_KEY_CODE) {
    gott = scancode + 300;

    // convert Allegro KEY_* numbers to scan codes
    // (for backwards compatibility we can't just use the
    // KEY_* constants now, it's too late)
    if ((gott>=347) & (gott<=356)) gott+=12;
    // F11-F12
    else if ((gott==357) || (gott==358)) gott+=76;
    // insert / numpad insert
    else if ((scancode == KEY_0_PAD) || (scancode == KEY_INSERT))
      gott = AGS_KEYCODE_INSERT;
    // delete / numpad delete
    else if ((scancode == KEY_DEL_PAD) || (scancode == KEY_DEL))
      gott = AGS_KEYCODE_DELETE;
    // Home
    else if (gott == 378) gott = 371;
    // End
    else if (gott == 379) gott = 379;
    // PgUp
    else if (gott == 380) gott = 373;
    // PgDn
    else if (gott == 381) gott = 381;
    // left arrow
    else if (gott==382) gott=375;
    // right arrow
    else if (gott==383) gott=377;
    // up arrow
    else if (gott==384) gott=372;
    // down arrow
    else if (gott==385) gott=380;
    // numeric keypad
    else if (gott==338) gott=379;
    else if (gott==339) gott=380;
    else if (gott==340) gott=381;
    else if (gott==341) gott=375;
    else if (gott==342) gott=376;
    else if (gott==343) gott=377;
    else if (gott==344) gott=371;
    else if (gott==345) gott=372;
    else if (gott==346) gott=373;
  }
  else
    gott = gott & 0x00ff;

  // Alt+X, abort (but only once game is loaded)
  if (gott == play.abort_key) {
    if (displayed_room >= 0) {
      check_dynamic_sprites_at_exit = 0;
      quit("!|");
    }
  }

  //sprintf(message, "Keypress: %d", gott);
  //OutputDebugString(message);

  return gott;
}

int is_key_pressed (int keycode)
{

#ifndef ALLEGRO_KEYBOARD_HANDLER
  // old allegro version
  quit("allegro keyboard handler not in use??");
#endif

  if (keyboard_needs_poll())
    poll_keyboard();

  if (keycode >= 300) {
    // function keys are 12 lower in allegro 4
    if ((keycode>=359) & (keycode<=368)) keycode-=12;
    // F11-F12
    else if ((keycode==433) || (keycode==434)) keycode-=76;
    // left arrow
    else if (keycode==375) keycode=382;
    // right arrow
    else if (keycode==377) keycode=383;
    // up arrow
    else if (keycode==372) keycode=384;
    // down arrow
    else if (keycode==380) keycode=385;
    // numeric keypad
    else if (keycode==379) keycode=338;
    else if (keycode==380) keycode=339;
    else if (keycode==381) keycode=340;
    else if (keycode==375) keycode=341;
    else if (keycode==376) keycode=342;
    else if (keycode==377) keycode=343;
    else if (keycode==371) keycode=344;
    else if (keycode==372) keycode=345;
    else if (keycode==373) keycode=346;
    // insert
    else if (keycode == AGS_KEYCODE_INSERT) keycode = KEY_INSERT + 300;
    // delete
    else if (keycode == AGS_KEYCODE_DELETE) keycode = KEY_DEL + 300;

    // deal with shift/ctrl/alt
    if (keycode == 403) keycode = KEY_LSHIFT;
    else if (keycode == 404) keycode = KEY_RSHIFT;
    else if (keycode == 405) keycode = KEY_LCONTROL;
    else if (keycode == 406) keycode = KEY_RCONTROL;
    else if (keycode == 407) keycode = KEY_ALT;
    else keycode -= 300;

    if (rec_iskeypressed(keycode))
      return 1;
    // deal with numeric pad keys having different codes to arrow keys
    if ((keycode == KEY_LEFT) && (rec_iskeypressed(KEY_4_PAD) != 0))
      return 1;
    if ((keycode == KEY_RIGHT) && (rec_iskeypressed(KEY_6_PAD) != 0))
      return 1;
    if ((keycode == KEY_UP) && (rec_iskeypressed(KEY_8_PAD) != 0))
      return 1;
    if ((keycode == KEY_DOWN) && (rec_iskeypressed(KEY_2_PAD) != 0))
      return 1;
    // PgDn/PgUp are equivalent to 3 and 9 on numeric pad
    if ((keycode == KEY_9_PAD) && (rec_iskeypressed(KEY_PGUP) != 0))
      return 1;
    if ((keycode == KEY_3_PAD) && (rec_iskeypressed(KEY_PGDN) != 0))
      return 1;
    // Home/End are equivalent to 7 and 1
    if ((keycode == KEY_7_PAD) && (rec_iskeypressed(KEY_HOME) != 0))
      return 1;
    if ((keycode == KEY_1_PAD) && (rec_iskeypressed(KEY_END) != 0))
      return 1;
    // insert/delete have numpad equivalents
    if ((keycode == KEY_INSERT) && (rec_iskeypressed(KEY_0_PAD) != 0))
      return 1;
    if ((keycode == KEY_DEL) && (rec_iskeypressed(KEY_DEL_PAD) != 0))
      return 1;

    return 0;
  }
  // convert ascii to scancode
  else if ((keycode >= 'A') && (keycode <= 'Z'))
  {
    keycode = platform->ConvertKeycodeToScanCode(keycode);
  }
  else if ((keycode >= '0') && (keycode <= '9'))
    keycode -= ('0' - KEY_0);
  else if (keycode == 8)
    keycode = KEY_BACKSPACE;
  else if (keycode == 9)
    keycode = KEY_TAB;
  else if (keycode == 13) {
    // check both the main return key and the numeric pad enter
    if (rec_iskeypressed(KEY_ENTER))
      return 1;
    keycode = KEY_ENTER_PAD;
  }
  else if (keycode == ' ')
    keycode = KEY_SPACE;
  else if (keycode == 27)
    keycode = KEY_ESC;
  else if (keycode == '-') {
    // check both the main - key and the numeric pad
    if (rec_iskeypressed(KEY_MINUS))
      return 1;
    keycode = KEY_MINUS_PAD;
  }
  else if (keycode == '+') {
    // check both the main + key and the numeric pad
    if (rec_iskeypressed(KEY_EQUALS))
      return 1;
    keycode = KEY_PLUS_PAD;
  }
  else if (keycode == '/') {
    // check both the main / key and the numeric pad
    if (rec_iskeypressed(KEY_SLASH))
      return 1;
    keycode = KEY_SLASH_PAD;
  }
  else if (keycode == '=')
    keycode = KEY_EQUALS;
  else if (keycode == '[')
    keycode = KEY_OPENBRACE;
  else if (keycode == ']')
    keycode = KEY_CLOSEBRACE;
  else if (keycode == '\\')
    keycode = KEY_BACKSLASH;
  else if (keycode == ';')
    keycode = KEY_SEMICOLON;
  else if (keycode == '\'')
    keycode = KEY_QUOTE;
  else if (keycode == ',')
    keycode = KEY_COMMA;
  else if (keycode == '.')
    keycode = KEY_STOP;
  else {
    DEBUG_CONSOLE("IsKeyPressed: unsupported keycode %d", keycode);
    return 0;
  }

  return rec_iskeypressed(keycode) != 0;
}

void keyboard_input_initialise() {
#ifdef ALLEGRO_KEYBOARD_HANDLER
  write_log_debug("Initializing keyboard");
  install_keyboard();
#endif
}

//#define getch() my_readkey()
//#undef kbhit
//#define kbhit keypressed
// END KEYBOARD HANDLER