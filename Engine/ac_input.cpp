#include "ac_input.h"

#include "allegro_wrapper.h"


#include "allegro.h"
#include "ac.h"
#include "ac_context.h"

#include "acsound.h"
#include "acgfx.h"
#include "ac_game.h"
#include "cscomp.h"


// KEYBOARD HANDLER
// ============================================================================

int my_readkey() {
  int gott=alw_readkey();
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
    if ((alw_key_shifts & KB_NUMLOCK_FLAG) == 0)
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

void keyboard_input_initialise() {
#ifdef ALLEGRO_KEYBOARD_HANDLER
  write_log_debug("Initializing keyboard");
  alw_install_keyboard();
#endif
}

//#define getch() my_readkey()
//#undef kbhit
//#define kbhit alw_keypressed
// END KEYBOARD HANDLER



// Record input
// ============================================================================


char playback_keystate[KEY_MAX];
int recbutstate[4] = {-1, -1, -1, -1};
int pluginSimulatedClick = NONE;


void write_record_event (int evnt, int dlen, short *dbuf) {

  recordbuffer[recsize] = play.gamestep;
  recordbuffer[recsize+1] = evnt;

  for (int i = 0; i < dlen; i++)
    recordbuffer[recsize + i + 2] = dbuf[i];
  recsize += dlen + 2;

  if (recsize >= recbuffersize - 100) {
    recbuffersize += 10000;
    recordbuffer = (short*)realloc (recordbuffer, recbuffersize * sizeof(short));
  }

  play.gamestep++;
}

void disable_replay_playback () {
  play.playback = 0;
  if (recordbuffer)
    free (recordbuffer);
  recordbuffer = NULL;
  disable_mgetgraphpos = 0;
}

void done_playback_event (int size) {
  recsize += size;
  play.gamestep++;
  if ((recsize >= recbuffersize) || (recordbuffer[recsize+1] == REC_ENDOFFILE))
    disable_replay_playback();
}

int rec_getch () {
  if (play.playback) {
    if ((recordbuffer[recsize] == play.gamestep) && (recordbuffer[recsize + 1] == REC_GETCH)) {
      int toret = recordbuffer[recsize + 2];
      done_playback_event (3);
      return toret;
    }
    // Since getch() waits for a key to be pressed, if we have no
    // record for it we're out of sync
    quit("out of sync in playback in getch");
  }
  int result = my_readkey();
  if (play.recording) {
    short buff[1] = {result};
    write_record_event (REC_GETCH, 1, buff);
  }

  return result;  
}

int rec_kbhit () {
  if ((play.playback) && (recordbuffer != NULL)) {
    // check for real keypresses to abort the replay
    if (alw_keypressed()) {
      if (my_readkey() == 27) {
        disable_replay_playback();
        return 0;
      }
    }
    // now simulate the keypresses
    if ((recordbuffer[recsize] == play.gamestep) && (recordbuffer[recsize + 1] == REC_KBHIT)) {
      done_playback_event (2);
      return 1;
    }
    return 0;
  }
  int result = alw_keypressed();
  if ((result) && (globalTimerCounter < play.ignore_user_input_until_time))
  {
    // ignoring user input
    my_readkey();
    result = 0;
  }
  if ((result) && (play.recording)) {
    write_record_event (REC_KBHIT, 0, NULL);
  }
  return result;  
}


int rec_iskeypressed (int keycode) {

  if (play.playback) {
    if ((recordbuffer[recsize] == play.gamestep)
     && (recordbuffer[recsize + 1] == REC_KEYDOWN)
     && (recordbuffer[recsize + 2] == keycode)) {
      playback_keystate[keycode] = recordbuffer[recsize + 3];
      done_playback_event (4);
    }
    return playback_keystate[keycode];
  }

  int toret = key[keycode];

  if (play.recording) {
    if (toret != playback_keystate[keycode]) {
      short buff[2] = {keycode, toret};
      write_record_event (REC_KEYDOWN, 2, buff);
      playback_keystate[keycode] = toret;
    }
  }

  return toret;
}

int rec_isSpeechFinished () {
  if (play.playback) {
    if ((recordbuffer[recsize] == play.gamestep) && (recordbuffer[recsize + 1] == REC_SPEECHFINISHED)) {
      done_playback_event (2);
      return 1;
    }
    return 0;
  }

  if (!channels[SCHAN_SPEECH]->done) {
    return 0;
  }
  if (play.recording)
    write_record_event (REC_SPEECHFINISHED, 0, NULL);
  return 1;
}


int rec_misbuttondown (int but) {
  if (play.playback) {
    if ((recordbuffer[recsize] == play.gamestep)
     && (recordbuffer[recsize + 1] == REC_MOUSEDOWN)
     && (recordbuffer[recsize + 2] == but)) {
      recbutstate[but] = recordbuffer[recsize + 3];
      done_playback_event (4);
    }
    return recbutstate[but];
  }
  int result = misbuttondown (but);
  if (play.recording) {
    if (result != recbutstate[but]) {
      short buff[2] = {but, result};
      write_record_event (REC_MOUSEDOWN, 2, buff);
      recbutstate[but] = result;
    }
  }
  return result;
}


void PluginSimulateMouseClick(int pluginButtonID) {
  pluginSimulatedClick = pluginButtonID - 1;
}

int rec_mgetbutton() {

  if ((play.playback) && (recordbuffer != NULL)) {
    if ((recordbuffer[recsize] < play.gamestep) && (play.gamestep < 32766))
      quit("Playback error: out of sync");
    if (loopcounter >= replay_last_second + 40) {
      replay_time ++;
      replay_last_second += 40;
    }
    if ((recordbuffer[recsize] == play.gamestep) && (recordbuffer[recsize + 1] == REC_MOUSECLICK)) {
      filter->SetMousePosition(recordbuffer[recsize+3], recordbuffer[recsize+4]);
      disable_mgetgraphpos = 0;
      mgetgraphpos ();
      disable_mgetgraphpos = 1;
      int toret = recordbuffer[recsize + 2];
      done_playback_event (5);
      return toret;
    }
    return NONE;
  }

  int result;

  if (pluginSimulatedClick > NONE) {
    result = pluginSimulatedClick;
    pluginSimulatedClick = NONE;
  }
  else {
    result = mgetbutton();
  }

  if ((result >= 0) && (globalTimerCounter < play.ignore_user_input_until_time))
  {
    // ignoring user input
    result = NONE;
  }

  if (play.recording) {
    if (result >= 0) {
      short buff[3] = {result, mousex, mousey};
      write_record_event (REC_MOUSECLICK, 3, buff);
    }
    if (loopcounter >= replay_last_second + 40) {
      replay_time ++;
      replay_last_second += 40;
    }
  }
  return result;
}

void rec_domouse (int what) {
  
  if (play.recording) {
    int mxwas = mousex, mywas = mousey;
    if (what == DOMOUSE_NOCURSOR)
      mgetgraphpos();
    else
      domouse(what);

    if ((mxwas != mousex) || (mywas != mousey)) {
      // don't divide down the co-ordinates, because we lose
      // the precision, and it might click the wrong thing
      // if eg. hi-res 71 -> 35 in record file -> 70 in playback
      short buff[2] = {mousex, mousey};
      write_record_event (REC_MOUSEMOVE, 2, buff);
    }
    return;
  }
  else if ((play.playback) && (recordbuffer != NULL)) {
    if ((recordbuffer[recsize] == play.gamestep) && (recordbuffer[recsize + 1] == REC_MOUSEMOVE)) {
      filter->SetMousePosition(recordbuffer[recsize+2], recordbuffer[recsize+3]);
      disable_mgetgraphpos = 0;
      if (what == DOMOUSE_NOCURSOR)
        mgetgraphpos();
      else
        domouse(what);
      disable_mgetgraphpos = 1;
      done_playback_event (4);
      return;
    }
  }
  if (what == DOMOUSE_NOCURSOR)
    mgetgraphpos();
  else
    domouse(what);
}
int check_mouse_wheel () {
  if ((play.playback) && (recordbuffer != NULL)) {
    if ((recordbuffer[recsize] == play.gamestep) && (recordbuffer[recsize + 1] == REC_MOUSEWHEEL)) {
      int toret = recordbuffer[recsize+2];
      done_playback_event (3);
      return toret;
    }
    return 0;
  }

  int result = 0;
  if ((alw_mouse_z != mouse_z_was) && (game.options[OPT_MOUSEWHEEL] != 0)) {
    if (alw_mouse_z > mouse_z_was)
      result = 1;
    else
      result = -1;
    mouse_z_was = alw_mouse_z;
  }

  if ((play.recording) && (result)) {
    short buff[1] = {result};
    write_record_event (REC_MOUSEWHEEL, 1, buff);
  }

  return result;
}


// Friendly keyboard input layer
// ============================================================================

int ac_mgetbutton() {
  return rec_mgetbutton();
}

void ac_domouse(int str) {
  return rec_domouse(str);
}

int ac_misbuttondown(int buno) {
  return rec_misbuttondown(buno);
}

int ac_kbhit() {
  return rec_kbhit();
}

int ac_getch() {
  return rec_getch();
}

int is_key_pressed (int keycode)
{

#ifndef ALLEGRO_KEYBOARD_HANDLER
  // old allegro version
  quit("allegro keyboard handler not in use??");
#endif

  if (alw_keyboard_needs_poll())
    alw_poll_keyboard();

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


// Recording Script
// ============================================================================

void start_recording() {
  if (play.playback) {
    play.recording = 0;  // stop quit() crashing
    play.playback = 0;
    quit("!playback and recording of replay selected simultaneously");
  }

  srand (play.randseed);
  play.gamestep = 0;

  recbuffersize = 10000;
  recordbuffer = (short*)malloc (recbuffersize * sizeof(short));
  recsize = 0;
  memset (playback_keystate, -1, KEY_MAX);
  replay_last_second = loopcounter;
  replay_time = 0;
  strcpy (replayfile, "New.agr");
}

void start_replay_record () {
  FILE *ott = fopen(replayTempFile, "wb");
  save_game_data (ott, NULL);
  fclose (ott);
  start_recording();
  play.recording = 1;
}

/* *** SCRIPT SYMBOL: [??] StartRecording *** */
static void scStartRecording (int keyToStop) {
  quit("!StartRecording: not et suppotreD");
}

void stop_recording() {
  if (!play.recording)
    return;

  write_record_event (REC_ENDOFFILE, 0, NULL);

  play.recording = 0;
  char replaydesc[100] = "";
  sc_inputbox ("Enter replay description:", replaydesc);
  sc_inputbox ("Enter replay filename:", replayfile);
  if (replayfile[0] == 0)
    strcpy (replayfile, "Untitled");
  if (strchr (replayfile, '.') != NULL)
    strchr (replayfile, '.')[0] = 0;
  strcat (replayfile, ".agr");

  FILE *ooo = fopen(replayfile, "wb");
  fwrite ("AGSRecording", 12, 1, ooo);
  fputstring (ACI_VERSION_TEXT, ooo);
  int write_version = 2;
  FILE *fsr = fopen(replayTempFile, "rb");
  if (fsr != NULL) {
    // There was a save file created
    write_version = 3;
  }
  putw (write_version, ooo);

  fputstring (game.gamename, ooo);
  putw (game.uniqueid, ooo);
  putw (replay_time, ooo);
  fputstring (replaydesc, ooo);  // replay description, maybe we'll use this later
  putw (play.randseed, ooo);
  if (write_version >= 3)
    putw (recsize, ooo);
  fwrite (recordbuffer, recsize, sizeof(short), ooo);
  if (fsr != NULL) {
    putw (1, ooo);  // yes there is a save present
    int lenno = filelength(fileno(fsr));
    char *tbufr = (char*)malloc (lenno);
    fread (tbufr, lenno, 1, fsr);
    fwrite (tbufr, lenno, 1, ooo);
    free (tbufr);
    fclose (fsr);
    unlink (replayTempFile);
  }
  else if (write_version >= 3) {
    putw (0, ooo);
  }
  fclose (ooo);

  free (recordbuffer);
  recordbuffer = NULL;
}


void register_recording_script_functions() {
  scAdd_External_Symbol("StartRecording", (void *)scStartRecording);
}