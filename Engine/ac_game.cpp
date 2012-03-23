#include "ac_game.h"

#include "ac.h"
#include "ac_types.h"
#include "ac_context.h"
#include "bmp.h"
#include "ac_string.h"
#include "misc.h"
#include "clib32.h"
#include "sprcache.h"
#include "acgfx.h"
#include "ali3d.h"
#include "acgui.h"
#include "ac_text.h"
#include "acdialog.h"
#include "ac_mouse.h"
#include "ac_file.h"
#include "ac_hotspot.h"
#include "acsound.h"
#include "acchars.h"
#include "ac_obj.h"
#include "ac_script_gui.h"
#include "ac_exescr.h"
#include "ac_keyinput.h"

#include "agsplugin.h"

// forward declarations
void SetSpeechFont (int fontnum);
void SetNormalFont (int fontnum);
void save_game(int slotn, const char*descript);
void SetMultitasking (int mode);


/* *** SCRIPT SYMBOL: [Game] RestartGame *** */
void restart_game() {
  can_run_delayed_command();
  if (inside_script) {
    curscript->queue_action(ePSARestartGame, 0, "RestartGame");
    return;
  }
  int errcod;
  if ((errcod = load_game(RESTART_POINT_SAVE_GAME_NUMBER, NULL, NULL))!=0)
    quitprintf("unable to restart game (error:%s)", load_game_errors[-errcod]);

}



// Binary tree structure for holding translations, allows fast
// access
struct TreeMap {
  TreeMap *left, *right;
  char *text;
  char *translation;

  TreeMap() {
    left = NULL;
    right = NULL;
    text = NULL;
    translation = NULL;
  }
  char* findValue (const char* key) {
    if (text == NULL)
      return NULL;

    if (strcmp(key, text) == 0)
      return translation;
    //debug_log("Compare: '%s' with '%s'", key, text);

    if (strcmp (key, text) < 0) {
      if (left == NULL)
        return NULL;
      return left->findValue (key);
    }
    else {
      if (right == NULL)
        return NULL;
      return right->findValue (key);
    }
  }
  void addText (const char* ntx, char *trans) {
    if ((ntx == NULL) || (ntx[0] == 0) ||
        ((text != NULL) && (strcmp(ntx, text) == 0)))
      // don't add if it's an empty string or if it's already here
      return;

    if (text == NULL) {
      text = (char*)malloc(strlen(ntx)+1);
      translation = (char*)malloc(strlen(trans)+1);
      if (translation == NULL)
        quit("load_translation: out of memory");
      strcpy(text, ntx);
      strcpy(translation, trans);
    }
    else if (strcmp(ntx, text) < 0) {
      // Earlier in alphabet, add to left
      if (left == NULL)
        left = new TreeMap();

      left->addText (ntx, trans);
    }
    else if (strcmp(ntx, text) > 0) {
      // Later in alphabet, add to right
      if (right == NULL)
        right = new TreeMap();

      right->addText (ntx, trans);
    }
  }
  void clear() {
    if (left) {
      left->clear();
      delete left;
    }
    if (right) {
      right->clear();
      delete right;
    }
    if (text)
      free(text);
    if (translation)
      free(translation);
    left = NULL;
    right = NULL;
    text = NULL;
    translation = NULL;
  }
  ~TreeMap() {
    clear();
  }
};

TreeMap *transtree = NULL;
long lang_offs_start = 0;
char transFileName[MAX_PATH] = "\0";

void close_translation () {
  if (transtree != NULL) {
    delete transtree;
    transtree = NULL;
  }
}

bool init_translation (const char *lang) {
  char *transFileLoc;

  if (lang == NULL) {
    sprintf(transFileName, "default.tra");
  }
  else {
    sprintf(transFileName, "%s.tra", lang);
  }

  transFileLoc = ci_find_file(usetup.data_files_dir, transFileName);

  FILE *language_file = clibfopen(transFileLoc, "rb");
  free(transFileLoc);

  if (language_file == NULL) 
  {
    if (lang != NULL)
    {
      // Just in case they're running in Debug, try compiled folder
      sprintf(transFileName, "Compiled\\%s.tra", lang);
      language_file = clibfopen(transFileName, "rb");
    }
    if (language_file == NULL)
      return false;
  }
  // in case it's inside a library file, record the offset
  lang_offs_start = ftell(language_file);

  char transsig[16];
  fread(transsig, 15, 1, language_file);
  if (strcmp(transsig, "AGSTranslation") != 0) {
    fclose(language_file);
    return false;
  }

  if (transtree != NULL)
  {
    close_translation();
  }
  transtree = new TreeMap();

  while (!feof (language_file)) {
    int blockType = getw(language_file);
    if (blockType == -1)
      break;
    // MACPORT FIX 9/6/5: remove warning
    /* int blockSize = */ getw(language_file);

    if (blockType == 1) {
      char original[STD_BUFFER_SIZE], translation[STD_BUFFER_SIZE];
      while (1) {
        read_string_decrypt (language_file, original);
        read_string_decrypt (language_file, translation);
        if ((strlen (original) < 1) && (strlen(translation) < 1))
          break;
        if (feof (language_file))
          quit("!Language file is corrupt");
        transtree->addText (original, translation);
      }

    }
    else if (blockType == 2) {
      int uidfrom;
      char wasgamename[100];
      fread (&uidfrom, 4, 1, language_file);
      read_string_decrypt (language_file, wasgamename);
      if ((uidfrom != game.uniqueid) || (strcmp (wasgamename, game.gamename) != 0)) {
        char quitmess[250];
        sprintf(quitmess,
          "!The translation file you have selected is not compatible with this game. "
          "The translation is designed for '%s'. Make sure the translation was compiled by the original game author.",
          wasgamename);
        quit(quitmess);
      }
    }
    else if (blockType == 3) {
      // game settings
      int temp = getw(language_file);
      // normal font
      if (temp >= 0)
        SetNormalFont (temp);
      temp = getw(language_file);
      // speech font
      if (temp >= 0)
        SetSpeechFont (temp);
      temp = getw(language_file);
      // text direction
      if (temp == 1) {
        play.text_align = SCALIGN_LEFT;
        game.options[OPT_RIGHTLEFTWRITE] = 0;
      }
      else if (temp == 2) {
        play.text_align = SCALIGN_RIGHT;
        game.options[OPT_RIGHTLEFTWRITE] = 1;
      }
    }
    else
      quit("Unknown block type in translation file.");
  }

  fclose (language_file);

  if (transtree->text == NULL)
    quit("!The selected translation file was empty. The translation source may have been translated incorrectly or you may have generated a blank file.");

  return true;
}

/* *** SCRIPT SYMBOL: [Game] GetTranslation *** */
char *get_translation (const char *text) {
  if (text == NULL)
    quit("!Null string supplied to CheckForTranslations");

  source_text_length = strlen(text);
  if ((text[0] == '&') && (play.unfactor_speech_from_textlength != 0)) {
    // if there's an "&12 text" type line, remove "&12 " from the source
    // length
    int j = 0;
    while ((text[j] != ' ') && (text[j] != 0))
      j++;
    j++;
    source_text_length -= j;
  }

  // check if a plugin wants to translate it - if so, return that
  char *plResult = (char*)platform->RunPluginHooks(AGSE_TRANSLATETEXT, (int)text);
  if (plResult) {
    if (((int)plResult >= -1) && ((int)plResult < 10000))
      quit("!Plugin did not return a string for text translation");
    return plResult;
  }

  if (transtree != NULL) {
    // translate the text using the translation file
    char * transl = transtree->findValue (text);
    if (transl != NULL)
      return transl;
  }
  // return the original text
  return (char*)text;
}

/* *** SCRIPT SYMBOL: [Game] IsTranslationAvailable *** */
int IsTranslationAvailable () {
  if (transtree != NULL)
    return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [Game] GetTranslationName *** */
int GetTranslationName (char* buffer) {
  VALIDATE_STRING (buffer);
  const char *copyFrom = transFileName;

  while (strchr(copyFrom, '\\') != NULL)
  {
    copyFrom = strchr(copyFrom, '\\') + 1;
  }
  while (strchr(copyFrom, '/') != NULL)
  {
    copyFrom = strchr(copyFrom, '/') + 1;
  }

  strcpy (buffer, copyFrom);
  // remove the ".tra" from the end of the filename
  if (strstr (buffer, ".tra") != NULL)
    strstr (buffer, ".tra")[0] = 0;

  return IsTranslationAvailable();
}

/* *** SCRIPT SYMBOL: [Game] Game::get_TranslationFilename *** */
const char* Game_GetTranslationFilename() {
  char buffer[STD_BUFFER_SIZE];
  GetTranslationName(buffer);
  return CreateNewScriptString(buffer);
}

/* *** SCRIPT SYMBOL: [Game] Game::ChangeTranslation^1 *** */
int Game_ChangeTranslation(const char *newFilename)
{
  if ((newFilename == NULL) || (newFilename[0] == 0))
  {
    close_translation();
    strcpy(transFileName, "");
    return 1;
  }

  char oldTransFileName[MAX_PATH];
  strcpy(oldTransFileName, transFileName);

  if (!init_translation(newFilename))
  {
    strcpy(transFileName, oldTransFileName);
    return 0;
  }

  return 1;
}

// End translation functions



/* *** SCRIPT SYMBOL: [Game] SetAmbientTint *** */
void SetAmbientTint (int red, int green, int blue, int opacity, int luminance) {
  if ((red < 0) || (green < 0) || (blue < 0) ||
      (red > 255) || (green > 255) || (blue > 255) ||
      (opacity < 0) || (opacity > 100) ||
      (luminance < 0) || (luminance > 100))
    quit("!SetTint: invalid parameter. R,G,B must be 0-255, opacity & luminance 0-100");

  DEBUG_CONSOLE("Set ambient tint RGB(%d,%d,%d) %d%%", red, green, blue, opacity);

  play.rtint_red = red;
  play.rtint_green = green;
  play.rtint_blue = blue;
  play.rtint_level = opacity;
  play.rtint_light = (luminance * 25) / 10;
}


/* *** SCRIPT SYMBOL: [Game] RestoreGameSlot *** */
void RestoreGameSlot(int slnum) {
  if (displayed_room < 0)
    quit("!RestoreGameSlot: a game cannot be restored from within game_start");

  can_run_delayed_command();
  if (inside_script) {
    curscript->queue_action(ePSARestoreGame, slnum, "RestoreGameSlot");
    return;
  }
  load_game(slnum, NULL, NULL);
}


/* *** SCRIPT SYMBOL: [Game] DeleteSaveSlot *** */
void DeleteSaveSlot (int slnum) {
  char nametouse[260];
  get_save_game_path(slnum, nametouse);
  unlink (nametouse);
  if ((slnum >= 1) && (slnum <= MAXSAVEGAMES)) {
    char thisname[260];
    for (int i = MAXSAVEGAMES; i > slnum; i--) {
      get_save_game_path(i, thisname);
      FILE *fin = fopen (thisname, "rb");
      if (fin != NULL) {
        fclose (fin);
        // Rename the highest save game to fill in the gap
        rename (thisname, nametouse);
        break;
      }
    }

  }
}

/* *** SCRIPT SYMBOL: [Game] Game::SetSaveGameDirectory^1 *** */
int Game_SetSaveGameDirectory(const char *newFolder) {

  // don't allow them to go to another folder
  if ((newFolder[0] == '/') || (newFolder[0] == '\\') ||
    (newFolder[0] == ' ') ||
    ((newFolder[0] != 0) && (newFolder[1] == ':')))
    return 0;

  char newSaveGameDir[260];
  platform->ReplaceSpecialPaths(newFolder, newSaveGameDir);
  fix_filename_slashes(newSaveGameDir);

#ifdef LINUX_VERSION
  mkdir(newSaveGameDir, 0);
#else
  mkdir(newSaveGameDir);
#endif

  put_backslash(newSaveGameDir);

  char newFolderTempFile[260];
  strcpy(newFolderTempFile, newSaveGameDir);
  strcat(newFolderTempFile, "agstmp.tmp");

  FILE *testTemp = fopen(newFolderTempFile, "wb");
  if (testTemp == NULL) {
    return 0;
  }
  fclose(testTemp);
  unlink(newFolderTempFile);

  // copy the Restart Game file, if applicable
  char restartGamePath[260];
  sprintf(restartGamePath, "%s""agssave.%d%s", saveGameDirectory, RESTART_POINT_SAVE_GAME_NUMBER, saveGameSuffix);
  FILE *restartGameFile = fopen(restartGamePath, "rb");
  if (restartGameFile != NULL) {
    long fileSize = filelength(fileno(restartGameFile));
    char *mbuffer = (char*)malloc(fileSize);
    fread(mbuffer, fileSize, 1, restartGameFile);
    fclose(restartGameFile);

    sprintf(restartGamePath, "%s""agssave.%d%s", newSaveGameDir, RESTART_POINT_SAVE_GAME_NUMBER, saveGameSuffix);
    restartGameFile = fopen(restartGamePath, "wb");
    fwrite(mbuffer, fileSize, 1, restartGameFile);
    fclose(restartGameFile);
    free(mbuffer);
  }

  strcpy(saveGameDirectory, newSaveGameDir);
  return 1;
}

/* *** SCRIPT SYMBOL: [Game] GetSaveSlotDescription *** */
int GetSaveSlotDescription(int slnum,char*desbuf) {
  VALIDATE_STRING(desbuf);
  if (load_game(slnum, desbuf, NULL) == 0)
    return 1;
  sprintf(desbuf,"INVALID SLOT %d", slnum);
  return 0;
}

/* *** SCRIPT SYMBOL: [Game] Game::GetSaveSlotDescription^1 *** */
const char* Game_GetSaveSlotDescription(int slnum) {
  char buffer[STD_BUFFER_SIZE];
  if (load_game(slnum, buffer, NULL) == 0)
    return CreateNewScriptString(buffer);
  return NULL;
}

int oldmouse;
void setup_for_dialog() {
  cbuttfont = play.normal_font;
  acdialog_font = play.normal_font;
  wsetscreen(virtual_screen);
  if (!play.mouse_cursor_hidden)
    ac_domouse(1);
  oldmouse=cur_cursor; set_mouse_cursor(CURS_ARROW);
}
void restore_after_dialog() {
  set_mouse_cursor(oldmouse);
  if (!play.mouse_cursor_hidden)
    ac_domouse(2);
  construct_virtual_screen(true);
}



/* *** SCRIPT SYMBOL: [Game] RestoreGameDialog *** */
void restore_game_dialog() {
  can_run_delayed_command();
  if (thisroom.options[ST_SAVELOAD] == 1) {
    DisplayMessage (983);
    return;
  }
  if (inside_script) {
    curscript->queue_action(ePSARestoreGameDialog, 0, "RestoreGameDialog");
    return;
  }
  setup_for_dialog();
  int toload=loadgamedialog();
  restore_after_dialog();
  if (toload>=0) {
    load_game_and_print_error(toload);
  }
}

/* *** SCRIPT SYMBOL: [Game] SaveGameDialog *** */
void save_game_dialog() {
  if (thisroom.options[ST_SAVELOAD] == 1) {
    DisplayMessage (983);
    return;
  }
  if (inside_script) {
    curscript->queue_action(ePSASaveGameDialog, 0, "SaveGameDialog");
    return;
  }
  setup_for_dialog();
  int toload=savegamedialog();
  restore_after_dialog();
  if (toload>=0)
    save_game(toload,buffer2);
  }



/* *** SCRIPT SYMBOL: [Game] PauseGame *** */
void PauseGame() {
  game_paused++;
  DEBUG_CONSOLE("Game paused");
}
/* *** SCRIPT SYMBOL: [Game] UnPauseGame *** */
void UnPauseGame() {
  if (game_paused > 0)
    game_paused--;
  DEBUG_CONSOLE("Game UnPaused, pause level now %d", game_paused);
}



/* *** SCRIPT SYMBOL: [Game] IsGamePaused *** */
int IsGamePaused() {
  return game_paused > 0;
}


/* *** SCRIPT SYMBOL: [Game] IsKeyPressed *** */
int IsKeyPressed (int keycode) {
  return is_key_pressed(keycode);
}


/* *** SCRIPT SYMBOL: [Game] UpdateInventory *** */
void update_invorder() {
  for (int cc = 0; cc < game.numcharacters; cc++) {
    charextra[cc].invorder_count = 0;
    int ff, howmany;
    // Iterate through all inv items, adding them once (or multiple
    // times if requested) to the list.
    for (ff=0;ff < game.numinvitems;ff++) {
      howmany = game.chars[cc].inv[ff];
      if ((game.options[OPT_DUPLICATEINV] == 0) && (howmany > 1))
        howmany = 1;

      for (int ts = 0; ts < howmany; ts++) {
        if (charextra[cc].invorder_count >= MAX_INVORDER)
          quit("!Too many inventory items to display: 500 max");

        charextra[cc].invorder[charextra[cc].invorder_count] = ff;
        charextra[cc].invorder_count++;
      }
    }
  }
  // backwards compatibility
  play.obsolete_inv_numorder = charextra[game.playercharacter].invorder_count;

  guis_need_update = 1;
}


/* *** SCRIPT SYMBOL: [Game] Game::GetColorFromRGB^3 *** */
int Game_GetColorFromRGB(int red, int grn, int blu) {
  if ((red < 0) || (red > 255) || (grn < 0) || (grn > 255) ||
      (blu < 0) || (blu > 255))
    quit("!GetColorFromRGB: colour values must be 0-255");

  if (game.color_depth == 1)
  {
    return makecol8(red, grn, blu);
  }

  int agscolor = ((blu >> 3) & 0x1f);
  agscolor += ((grn >> 2) & 0x3f) << 5;
  agscolor += ((red >> 3) & 0x1f) << 11;
  return agscolor;
}


/* *** SCRIPT SYMBOL: [Game] ClaimEvent *** */
void ClaimEvent() {
  if (eventClaimed == EVENT_NONE)
    quit("!ClaimEvent: no event to claim");

  eventClaimed = EVENT_CLAIMED;
}


/* *** SCRIPT SYMBOL: [Game] SetTextWindowGUI *** */
void SetTextWindowGUI (int guinum) {
  if ((guinum < -1) | (guinum >= game.numgui))
    quit("!SetTextWindowGUI: invalid GUI number");

  if (guinum < 0) ;  // disable it
  else if (!guis[guinum].is_textwindow())
    quit("!SetTextWindowGUI: specified GUI is not a text window");

  if (play.speech_textwindow_gui == game.options[OPT_TWCUSTOM])
    play.speech_textwindow_gui = guinum;
  game.options[OPT_TWCUSTOM] = guinum;
}



/* *** SCRIPT SYMBOL: [Game] Game::set_SpeechFont *** */
/* *** SCRIPT SYMBOL: [Game] SetSpeechFont *** */
void SetSpeechFont (int fontnum) {
  if ((fontnum < 0) || (fontnum >= game.numfonts))
    quit("!SetSpeechFont: invalid font number.");
  play.speech_font = fontnum;
  }
/* *** SCRIPT SYMBOL: [Game] Game::set_NormalFont *** */
/* *** SCRIPT SYMBOL: [Game] SetNormalFont *** */
void SetNormalFont (int fontnum) {
  if ((fontnum < 0) || (fontnum >= game.numfonts))
    quit("!SetNormalFont: invalid font number.");
  play.normal_font = fontnum;
  }
/* *** SCRIPT SYMBOL: [Game] Game::get_SpeechFont *** */
int Game_GetSpeechFont() {
  return play.speech_font;
}
/* *** SCRIPT SYMBOL: [Game] Game::get_NormalFont *** */
int Game_GetNormalFont() {
  return play.normal_font;
}



/* *** SCRIPT SYMBOL: [Game] SetGlobalInt *** */
void SetGlobalInt(int index,int valu) {
  if ((index<0) | (index>=MAXGSVALUES))
    quit("!SetGlobalInt: invalid index");

  if (play.globalscriptvars[index] != valu) {
    DEBUG_CONSOLE("GlobalInt %d set to %d", index, valu);
  }

  play.globalscriptvars[index]=valu;
}
/* *** SCRIPT SYMBOL: [Game] GetGlobalInt *** */
int GetGlobalInt(int index) {
  if ((index<0) | (index>=MAXGSVALUES))
    quit("!GetGlobalInt: invalid index");
  return play.globalscriptvars[index];
}

/* *** SCRIPT SYMBOL: [Game] Game::seti_GlobalStrings *** */
/* *** SCRIPT SYMBOL: [Game] SetGlobalString *** */
void SetGlobalString (int index, char *newval) {
  if ((index<0) | (index >= MAXGLOBALSTRINGS))
    quit("!SetGlobalString: invalid index");
  DEBUG_CONSOLE("GlobalString %d set to '%s'", index, newval);
  strncpy(play.globalstrings[index], newval, MAX_MAXSTRLEN);
  // truncate it to 200 chars, to be sure
  play.globalstrings[index][MAX_MAXSTRLEN - 1] = 0;
}

/* *** SCRIPT SYMBOL: [Game] GetGlobalString *** */
void GetGlobalString (int index, char *strval) {
  if ((index<0) | (index >= MAXGLOBALSTRINGS))
    quit("!GetGlobalString: invalid index");
  strcpy (strval, play.globalstrings[index]);
}

/* *** SCRIPT SYMBOL: [Game] Game::geti_GlobalStrings *** */
const char* Game_GetGlobalStrings(int index) {
  if ((index < 0) || (index >= MAXGLOBALSTRINGS))
    quit("!Game.GlobalStrings: invalid index");

  return CreateNewScriptString(play.globalstrings[index]);
}




/* *** SCRIPT SYMBOL: [Game] SaveScreenShot *** */
int SaveScreenShot(char*namm) {
  char fileName[MAX_PATH];

  if (strchr(namm,'.') == NULL)
    sprintf(fileName, "%s%s.bmp", saveGameDirectory, namm);
  else
    sprintf(fileName, "%s%s", saveGameDirectory, namm);

  if (gfxDriver->RequiresFullRedrawEachFrame()) 
  {
    BITMAP *buffer = create_bitmap_ex(32, scrnwid, scrnhit);
    gfxDriver->GetCopyOfScreenIntoBitmap(buffer);

    if (save_bitmap(fileName, buffer, palette)!=0)
    {
      destroy_bitmap(buffer);
      return 0;
    }
    destroy_bitmap(buffer);
  }
  else if (save_bitmap(fileName, virtual_screen, palette)!=0)
    return 0; // failed

  return 1;  // successful
}




/* *** SCRIPT SYMBOL: [Game] Wait *** */
void scrWait(int nloops) {
  if (nloops < 1)
    quit("!Wait: must wait at least 1 loop");

  play.wait_counter = nloops;
  play.key_skip_wait = 0;
  do_main_cycle(UNTIL_MOVEEND,(int)&play.wait_counter);
  }

/* *** SCRIPT SYMBOL: [Game] WaitKey *** */
int WaitKey(int nloops) {
  if (nloops < 1)
    quit("!WaitKey: must wait at least 1 loop");

  play.wait_counter = nloops;
  play.key_skip_wait = 1;
  do_main_cycle(UNTIL_MOVEEND,(int)&play.wait_counter);
  if (play.wait_counter < 0)
    return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [Game] WaitMouseKey *** */
int WaitMouseKey(int nloops) {
  if (nloops < 1)
    quit("!WaitMouseKey: must wait at least 1 loop");

  play.wait_counter = nloops;
  play.key_skip_wait = 3;
  do_main_cycle(UNTIL_MOVEEND,(int)&play.wait_counter);
  if (play.wait_counter < 0)
    return 1;
  return 0;
}



char gamefilenamebuf[200];
#define RAGMODE_PRESERVEGLOBALINT 1
#define RAGMODE_LOADNOW 0x8000000  // just to make sure it's non-zero
/* *** SCRIPT SYMBOL: [Game] RunAGSGame *** */
int RunAGSGame (char *newgame, unsigned int mode, int data) {

  can_run_delayed_command();

  int AllowedModes = RAGMODE_PRESERVEGLOBALINT | RAGMODE_LOADNOW;

  if ((mode & (~AllowedModes)) != 0)
    quit("!RunAGSGame: mode value unknown");

  if (use_compiled_folder_as_current_dir || editor_debugging_enabled)
  {
    quit("!RunAGSGame cannot be used while running the game from within the AGS Editor. You must build the game EXE and run it from there to use this function.");
  }

  if ((mode & RAGMODE_LOADNOW) == 0) {
    // need to copy, since the script gets destroyed
    get_current_dir_path(gamefilenamebuf, newgame);
    game_file_name = &gamefilenamebuf[0];
    usetup.main_data_filename = game_file_name;
    play.takeover_data = data;
    load_new_game_restore = -1;

    if (inside_script) {
      curscript->queue_action(ePSARunAGSGame, mode | RAGMODE_LOADNOW, "RunAGSGame");
      ccAbortInstance (ccGetCurrentInstance ());
    }
    else
      load_new_game = mode | RAGMODE_LOADNOW;

    return 0;
  }

  int result, ee;

  unload_old_room();
  displayed_room = -10;

  unload_game_file();

  if (csetlib(game_file_name,""))
    quitprintf("!RunAGSGame: unable to load new game file '%s'", game_file_name);

  clear(abuf);
  show_preload();

  if ((result = load_game_file ()) != 0) {
    quitprintf("!RunAGSGame: error %d loading new game file", result);
  }

  spriteset.reset();
  if (spriteset.initFile ("acsprset.spr"))
    quit("!RunAGSGame: error loading new sprites");

  if ((mode & RAGMODE_PRESERVEGLOBALINT) == 0) {
    // reset GlobalInts
    for (ee = 0; ee < MAXGSVALUES; ee++)
      play.globalscriptvars[ee] = 0;  
  }

  init_game_settings();
  play.screen_is_faded_out = 1;

  if (load_new_game_restore >= 0) {
    load_game (load_new_game_restore, NULL, NULL);
    load_new_game_restore = -1;
  }
  else
    start_game();

  return 0;
}



static void display_switch_out() {
  // this is only called if in SWITCH_PAUSE mode
  //debug_log("display_switch_out");

  switching_away_from_game++;

  platform->DisplaySwitchOut();

  // allow background running temporarily to halt the sound
  if (set_display_switch_mode(SWITCH_BACKGROUND) == -1)
    set_display_switch_mode(SWITCH_BACKAMNESIA);

  // stop the sound stuttering
  for (int i = 0; i <= MAX_SOUND_CHANNELS; i++) {
    if ((channels[i] != NULL) && (channels[i]->done == 0)) {
      channels[i]->pause();
    }
  }

  rest(1000);

  // restore the callbacks
  SetMultitasking(0);

  switching_away_from_game--;
}

static void display_switch_in() {
  for (int i = 0; i <= MAX_SOUND_CHANNELS; i++) {
    if ((channels[i] != NULL) && (channels[i]->done == 0)) {
      channels[i]->resume();
    }
  }

  if (gfxDriver->UsesMemoryBackBuffer())  // make sure all borders are cleared
    gfxDriver->ClearRectangle(0, 0, final_scrn_wid - 1, final_scrn_hit - 1, NULL);

  platform->DisplaySwitchIn();
}


/* *** SCRIPT SYMBOL: [Game] SetMultitaskingMode *** */
void SetMultitasking (int mode) {
  if ((mode < 0) | (mode > 1))
    quit("!SetMultitasking: invalid mode parameter");

  // Don't allow background running if full screen
  if ((mode == 1) && (usetup.windowed == 0))
    mode = 0;

  if (mode == 0) {
    if (set_display_switch_mode(SWITCH_PAUSE) == -1)
      set_display_switch_mode(SWITCH_AMNESIA);
    // install callbacks to stop the sound when switching away
    set_display_switch_callback(SWITCH_IN, display_switch_in);
    set_display_switch_callback(SWITCH_OUT, display_switch_out);
  }
  else {
    if (set_display_switch_mode (SWITCH_BACKGROUND) == -1)
      set_display_switch_mode(SWITCH_BACKAMNESIA);
  }
}



/* *** SCRIPT SYMBOL: [Game] ProcessClick *** */
void ProcessClick(int xx,int yy,int mood) {
  getloctype_throughgui = 1;
  int loctype = GetLocationType (xx, yy);
  xx += divide_down_coordinate(offsetx); 
  yy += divide_down_coordinate(offsety);

  if ((mood==MODE_WALK) && (game.options[OPT_NOWALKMODE]==0)) {
    int hsnum=get_hotspot_at(xx,yy);
    if (hsnum<1) ;
    else if (thisroom.hswalkto[hsnum].x<1) ;
    else if (play.auto_use_walkto_points == 0) ;
    else {
      xx=thisroom.hswalkto[hsnum].x;
      yy=thisroom.hswalkto[hsnum].y;
      DEBUG_CONSOLE("Move to walk-to point hotspot %d", hsnum);
    }
    walk_character(game.playercharacter,xx,yy,0, true);
    return;
  }
  play.usedmode=mood;

  if (loctype == 0) {
    // click on nothing -> hotspot 0
    getloctype_index = 0;
    loctype = LOCTYPE_HOTSPOT;
  }
  
  if (loctype == LOCTYPE_CHAR) {
    if (check_click_on_character(xx,yy,mood)) return;
  }
  else if (loctype == LOCTYPE_OBJ) {
    if (check_click_on_object(xx,yy,mood)) return;
  }
  else if (loctype == LOCTYPE_HOTSPOT) 
    RunHotspotInteraction (getloctype_index, mood);
}

// ** GetGameParameter replacement functions

/* *** SCRIPT SYMBOL: [Game] Game::get_InventoryItemCount *** */
int Game_GetInventoryItemCount() {
  // because of the dummy item 0, this is always one higher than it should be
  return game.numinvitems - 1;
}

/* *** SCRIPT SYMBOL: [Game] Game::get_FontCount *** */
int Game_GetFontCount() {
  return game.numfonts;
}

/* *** SCRIPT SYMBOL: [Game] Game::get_MouseCursorCount *** */
int Game_GetMouseCursorCount() {
  return game.numcursors;
}

/* *** SCRIPT SYMBOL: [Game] Game::get_CharacterCount *** */
int Game_GetCharacterCount() {
  return game.numcharacters;
}

/* *** SCRIPT SYMBOL: [Game] Game::get_GUICount *** */
int Game_GetGUICount() {
  return game.numgui;
}

/* *** SCRIPT SYMBOL: [Game] Game::get_ViewCount *** */
int Game_GetViewCount() {
  return game.numviews;
}

/* *** SCRIPT SYMBOL: [Game] Game::get_UseNativeCoordinates *** */
int Game_GetUseNativeCoordinates()
{
  if (game.options[OPT_NATIVECOORDINATES] != 0)
  {
    return 1;
  }
  return 0;
}

/* *** SCRIPT SYMBOL: [Game] Game::geti_SpriteWidth *** */
int Game_GetSpriteWidth(int spriteNum) {
  if ((spriteNum < 0) || (spriteNum >= MAX_SPRITES))
    return 0;

  if (!spriteset.doesSpriteExist(spriteNum))
    return 0;

  return divide_down_coordinate(spritewidth[spriteNum]);
}

/* *** SCRIPT SYMBOL: [Game] Game::geti_SpriteHeight *** */
int Game_GetSpriteHeight(int spriteNum) {
  if ((spriteNum < 0) || (spriteNum >= MAX_SPRITES))
    return 0;

  if (!spriteset.doesSpriteExist(spriteNum))
    return 0;

  return divide_down_coordinate(spriteheight[spriteNum]);
}

/* *** SCRIPT SYMBOL: [Game] Game::GetLoopCountForView^1 *** */
int Game_GetLoopCountForView(int viewNumber) {
  if ((viewNumber < 1) || (viewNumber > game.numviews))
    quit("!GetGameParameter: invalid view specified");

  return views[viewNumber - 1].numLoops;
}

/* *** SCRIPT SYMBOL: [Game] Game::GetRunNextSettingForLoop^2 *** */
int Game_GetRunNextSettingForLoop(int viewNumber, int loopNumber) {
  if ((viewNumber < 1) || (viewNumber > game.numviews))
    quit("!GetGameParameter: invalid view specified");
  if ((loopNumber < 0) || (loopNumber >= views[viewNumber - 1].numLoops))
    quit("!GetGameParameter: invalid loop specified");

  return (views[viewNumber - 1].loops[loopNumber].RunNextLoop()) ? 1 : 0;
}

/* *** SCRIPT SYMBOL: [Game] Game::GetFrameCountForLoop^2 *** */
int Game_GetFrameCountForLoop(int viewNumber, int loopNumber) {
  if ((viewNumber < 1) || (viewNumber > game.numviews))
    quit("!GetGameParameter: invalid view specified");
  if ((loopNumber < 0) || (loopNumber >= views[viewNumber - 1].numLoops))
    quit("!GetGameParameter: invalid loop specified");

  return views[viewNumber - 1].loops[loopNumber].numFrames;
}

/* *** SCRIPT SYMBOL: [Game] Game::GetViewFrame^3 *** */
ScriptViewFrame* Game_GetViewFrame(int viewNumber, int loopNumber, int frame) {
  if ((viewNumber < 1) || (viewNumber > game.numviews))
    quit("!GetGameParameter: invalid view specified");
  if ((loopNumber < 0) || (loopNumber >= views[viewNumber - 1].numLoops))
    quit("!GetGameParameter: invalid loop specified");
  if ((frame < 0) || (frame >= views[viewNumber - 1].loops[loopNumber].numFrames))
    quit("!GetGameParameter: invalid frame specified");

  ScriptViewFrame *sdt = new ScriptViewFrame(viewNumber - 1, loopNumber, frame);
  ccRegisterManagedObject(sdt, sdt);
  return sdt;
}

/* *** SCRIPT SYMBOL: [Game] Game::DoOnceOnly^1 *** */
int Game_DoOnceOnly(const char *token) 
{
  if (strlen(token) > 199)
    quit("!Game.DoOnceOnly: token length cannot be more than 200 chars");

  for (int i = 0; i < play.num_do_once_tokens; i++)
  {
    if (strcmp(play.do_once_tokens[i], token) == 0)
    {
      return 0;
    }
  }
  play.do_once_tokens = (char**)realloc(play.do_once_tokens, sizeof(char*) * (play.num_do_once_tokens + 1));
  play.do_once_tokens[play.num_do_once_tokens] = (char*)malloc(strlen(token) + 1);
  strcpy(play.do_once_tokens[play.num_do_once_tokens], token);
  play.num_do_once_tokens++;
  return 1;
}



#define GP_SPRITEWIDTH   1
#define GP_SPRITEHEIGHT  2
#define GP_NUMLOOPS      3
#define GP_NUMFRAMES     4
#define GP_ISRUNNEXTLOOP 5
#define GP_FRAMESPEED    6
#define GP_FRAMEIMAGE    7
#define GP_FRAMESOUND    8
#define GP_NUMGUIS       9
#define GP_NUMOBJECTS    10
#define GP_NUMCHARACTERS 11
#define GP_NUMINVITEMS   12
#define GP_ISFRAMEFLIPPED 13

/* *** SCRIPT SYMBOL: [Game] GetGameParameter *** */
int GetGameParameter (int parm, int data1, int data2, int data3) {
  switch (parm) {
   case GP_SPRITEWIDTH:
     return Game_GetSpriteWidth(data1);
   case GP_SPRITEHEIGHT:
     return Game_GetSpriteHeight(data1);
   case GP_NUMLOOPS:
     return Game_GetLoopCountForView(data1);
   case GP_NUMFRAMES:
     return Game_GetFrameCountForLoop(data1, data2);
   case GP_FRAMESPEED:
   case GP_FRAMEIMAGE:
   case GP_FRAMESOUND:
   case GP_ISFRAMEFLIPPED:
     {
     if ((data1 < 1) || (data1 > game.numviews))
       quit("!GetGameParameter: invalid view specified");
     if ((data2 < 0) || (data2 >= views[data1 - 1].numLoops))
       quit("!GetGameParameter: invalid loop specified");
     if ((data3 < 0) || (data3 >= views[data1 - 1].loops[data2].numFrames))
       quit("!GetGameParameter: invalid frame specified");

     ViewFrame *pvf = &views[data1 - 1].loops[data2].frames[data3];

     if (parm == GP_FRAMESPEED)
       return pvf->speed;
     else if (parm == GP_FRAMEIMAGE)
       return pvf->pic;
     else if (parm == GP_FRAMESOUND)
       return pvf->sound;
     else if (parm == GP_ISFRAMEFLIPPED)
       return (pvf->flags & VFLG_FLIPSPRITE) ? 1 : 0;
     else
       quit("GetGameParameter internal error");
     }
   case GP_ISRUNNEXTLOOP:
     return Game_GetRunNextSettingForLoop(data1, data2);
   case GP_NUMGUIS:
     return game.numgui;
   case GP_NUMOBJECTS:
     return croom->numobj;
   case GP_NUMCHARACTERS:
     return game.numcharacters;
   case GP_NUMINVITEMS:
     return game.numinvitems;
   default:
     quit("!GetGameParameter: unknown parameter specified");
  }
  return 0;
}



/* *** SCRIPT SYMBOL: [Game] Game::get_TextReadingSpeed *** */
int Game_GetTextReadingSpeed()
{
  return play.text_speed;
}

/* *** SCRIPT SYMBOL: [Game] Game::set_TextReadingSpeed *** */
void Game_SetTextReadingSpeed(int newTextSpeed)
{
  if (newTextSpeed < 1)
    quitprintf("!Game.TextReadingSpeed: %d is an invalid speed", newTextSpeed);

  play.text_speed = newTextSpeed;
}

/* *** SCRIPT SYMBOL: [Game] Game::get_MinimumTextDisplayTimeMs *** */
int Game_GetMinimumTextDisplayTimeMs()
{
  return play.text_min_display_time_ms;
}

/* *** SCRIPT SYMBOL: [Game] Game::set_MinimumTextDisplayTimeMs *** */
void Game_SetMinimumTextDisplayTimeMs(int newTextMinTime)
{
  play.text_min_display_time_ms = newTextMinTime;
}

/* *** SCRIPT SYMBOL: [Game] Game::get_IgnoreUserInputAfterTextTimeoutMs *** */
int Game_GetIgnoreUserInputAfterTextTimeoutMs()
{
  return play.ignore_user_input_after_text_timeout_ms;
}

/* *** SCRIPT SYMBOL: [Game] Game::set_IgnoreUserInputAfterTextTimeoutMs *** */
void Game_SetIgnoreUserInputAfterTextTimeoutMs(int newValueMs)
{
  play.ignore_user_input_after_text_timeout_ms = newValueMs;
}

/* *** SCRIPT SYMBOL: [Game] Game::get_FileName *** */
const char *Game_GetFileName() {
  return CreateNewScriptString(usetup.main_data_filename);
}

/* *** SCRIPT SYMBOL: [Game] Game::get_Name *** */
const char *Game_GetName() {
  return CreateNewScriptString(play.game_name);
}

/* *** SCRIPT SYMBOL: [Game] Game::set_Name *** */
void Game_SetName(const char *newName) {
  strncpy(play.game_name, newName, 99);
  play.game_name[99] = 0;

#if (ALLEGRO_DATE > 19990103)
  set_window_title(play.game_name);
#endif
}


/* *** SCRIPT SYMBOL: [Game] IsInteractionAvailable *** */
int IsInteractionAvailable (int xx,int yy,int mood) {
  getloctype_throughgui = 1;
  int loctype = GetLocationType (xx, yy);
  xx += divide_down_coordinate(offsetx); 
  yy += divide_down_coordinate(offsety);

  // You can always walk places
  if ((mood==MODE_WALK) && (game.options[OPT_NOWALKMODE]==0))
    return 1;

  play.check_interaction_only = 1;

  if (loctype == 0) {
    // click on nothing -> hotspot 0
    getloctype_index = 0;
    loctype = LOCTYPE_HOTSPOT;
  }
  
  if (loctype == LOCTYPE_CHAR) {
    check_click_on_character(xx,yy,mood);
  }
  else if (loctype == LOCTYPE_OBJ) {
    check_click_on_object(xx,yy,mood);
  }
  else if (loctype == LOCTYPE_HOTSPOT)
    RunHotspotInteraction (getloctype_index, mood);

  int ciwas = play.check_interaction_only;
  play.check_interaction_only = 0;

  if (ciwas == 2)
    return 1;

  return 0;
}


/* *** SCRIPT SYMBOL: [Game] QuitGame *** */
void QuitGame(int dialog) {
  if (dialog) {
    int rcode;
    setup_for_dialog();
    rcode=quitdialog();
    restore_after_dialog();
    if (rcode==0) return;
    }
  quit("|You have exited.");
  }

/* *** SCRIPT SYMBOL: [Game] InputBox *** */
void sc_inputbox(const char*msg,char*bufr) {
  VALIDATE_STRING(bufr);
  setup_for_dialog();
  enterstringwindow(get_translation(msg),bufr);
  restore_after_dialog();
  }

/* *** SCRIPT SYMBOL: [Game] Game::InputBox^1 *** */
const char* Game_InputBox(const char *msg) {
  char buffer[STD_BUFFER_SIZE];
  sc_inputbox(msg, buffer);
  return CreateNewScriptString(buffer);
}



/* *** SCRIPT SYMBOL: [Game] GetTextWidth *** */
int GetTextWidth(char *text, int fontnum) {
  VALIDATE_STRING(text);
  if ((fontnum < 0) || (fontnum >= game.numfonts))
    quit("!GetTextWidth: invalid font number.");

  return divide_down_coordinate(wgettextwidth_compensate(text, fontnum));
}

/* *** SCRIPT SYMBOL: [Game] GetTextHeight *** */
int GetTextHeight(char *text, int fontnum, int width) {
  VALIDATE_STRING(text);
  if ((fontnum < 0) || (fontnum >= game.numfonts))
    quit("!GetTextHeight: invalid font number.");

  int texthit = wgetfontheight(fontnum);

  break_up_text_into_lines(multiply_up_coordinate(width), fontnum, text);

  return divide_down_coordinate(texthit * numlines);
}


/* *** SCRIPT SYMBOL: [Game] DisableInterface *** */
void DisableInterface() {
  play.disabled_user_interface++;
  guis_need_update = 1;
  set_mouse_cursor(CURS_WAIT);
  }
/* *** SCRIPT SYMBOL: [Game] EnableInterface *** */
void EnableInterface() {
  guis_need_update = 1;
  play.disabled_user_interface--;
  if (play.disabled_user_interface<1) {
    play.disabled_user_interface=0;
    set_default_cursor();
    }
  }
// Returns 1 if user interface is enabled, 0 if disabled
/* *** SCRIPT SYMBOL: [Game] IsInterfaceEnabled *** */
int IsInterfaceEnabled() {
  return (play.disabled_user_interface > 0) ? 0 : 1;
}


/* *** SCRIPT SYMBOL: [Game] SetRestartPoint *** */
void SetRestartPoint() {
  save_game(RESTART_POINT_SAVE_GAME_NUMBER, "Restart Game Auto-Save");
}

/* *** SCRIPT SYMBOL: [Game] CallRoomScript *** */
void CallRoomScript (int value) {
  can_run_delayed_command();

  if (!inside_script)
    quit("!CallRoomScript: not inside a script???");

  play.roomscript_finished = 0;
  curscript->run_another("$on_call", value, 0);
}

/* *** SCRIPT SYMBOL: [Game] SetGameSpeed *** */
void SetGameSpeed(int newspd) {
  // if Ctrl+E has been used to max out frame rate, lock it there
  if ((frames_per_second == 1000) && (display_fps == 2))
    return;

  newspd += play.game_speed_modifier;
  if (newspd>1000) newspd=1000;
  if (newspd<10) newspd=10;
  set_game_speed(newspd);
  DEBUG_CONSOLE("Game speed set to %d", newspd);
}

/* *** SCRIPT SYMBOL: [Game] GetGameSpeed *** */
int GetGameSpeed() {
  return frames_per_second - play.game_speed_modifier;
}

/* *** SCRIPT SYMBOL: [Game] SetGameOption *** */
int SetGameOption (int opt, int setting) {
  if (((opt < 1) || (opt > OPT_HIGHESTOPTION)) && (opt != OPT_LIPSYNCTEXT))
    quit("!SetGameOption: invalid option specified");

  if (opt == OPT_ANTIGLIDE)
  {
    for (int i = 0; i < game.numcharacters; i++)
    {
      if (setting)
        game.chars[i].flags |= CHF_ANTIGLIDE;
      else
        game.chars[i].flags &= ~CHF_ANTIGLIDE;
    }
  }

  if ((opt == OPT_CROSSFADEMUSIC) && (game.audioClipTypeCount > AUDIOTYPE_LEGACY_MUSIC))
  {
    // legacy compatibility -- changing crossfade speed here also
    // updates the new audio clip type style
    game.audioClipTypes[AUDIOTYPE_LEGACY_MUSIC].crossfadeSpeed = setting;
  }
  
  int oldval = game.options[opt];
  game.options[opt] = setting;

  if (opt == OPT_DUPLICATEINV)
    update_invorder();
  else if (opt == OPT_DISABLEOFF)
    gui_disabled_style = convert_gui_disabled_style(game.options[OPT_DISABLEOFF]);
  else if (opt == OPT_PORTRAITSIDE) {
    if (setting == 0)  // set back to Left
      play.swap_portrait_side = 0;
  }

  return oldval;
}

/* *** SCRIPT SYMBOL: [Game] GetGameOption *** */
int GetGameOption (int opt) {
  if (((opt < 1) || (opt > OPT_HIGHESTOPTION)) && (opt != OPT_LIPSYNCTEXT))
    quit("!GetGameOption: invalid option specified");

  return game.options[opt];
}



/* *** SCRIPT SYMBOL: [Game] Random *** */
int __Rand(int upto) {
  upto++;
  if (upto < 1)
    quit("!Random: invalid parameter passed -- must be at least 0.");
  return rand()%upto;
  }


/* *** SCRIPT SYMBOL: [Game] GiveScore *** */
void GiveScore(int amnt) 
{
  guis_need_update = 1;
  play.score += amnt;

  if ((amnt > 0) && (play.score_sound >= 0))
    play_audio_clip_by_index(play.score_sound);

  run_on_event (GE_GOT_SCORE, amnt);
}


/* *** SCRIPT SYMBOL: [Game] GetLocationName *** */
void GetLocationName(int xxx,int yyy,char*tempo) {
  if (displayed_room < 0)
    quit("!GetLocationName: no room has been loaded");

  VALIDATE_STRING(tempo);
  
  if (GetGUIAt(xxx, yyy) >= 0) {
    tempo[0]=0;
    int mover = GetInvAt (xxx, yyy);
    if (mover > 0) {
      if (play.get_loc_name_last_time != 1000 + mover)
        guis_need_update = 1;
      play.get_loc_name_last_time = 1000 + mover;
      strcpy(tempo,get_translation(game.invinfo[mover].name));
    }
    else if ((play.get_loc_name_last_time > 1000) && (play.get_loc_name_last_time < 1000 + MAX_INV)) {
      // no longer selecting an item
      guis_need_update = 1;
      play.get_loc_name_last_time = -1;
    }
    return;
  }
  int loctype = GetLocationType (xxx, yyy);
  xxx += divide_down_coordinate(offsetx); 
  yyy += divide_down_coordinate(offsety);
  tempo[0]=0;
  if ((xxx>=thisroom.width) | (xxx<0) | (yyy<0) | (yyy>=thisroom.height))
    return;

  int onhs,aa;
  if (loctype == 0) {
    if (play.get_loc_name_last_time != 0) {
      play.get_loc_name_last_time = 0;
      guis_need_update = 1;
    }
    return;
  }

  // on character
  if (loctype == LOCTYPE_CHAR) {
    onhs = getloctype_index;
    strcpy(tempo,get_translation(game.chars[onhs].name));
    if (play.get_loc_name_last_time != 2000+onhs)
      guis_need_update = 1;
    play.get_loc_name_last_time = 2000+onhs;
    return;
  }
  // on object
  if (loctype == LOCTYPE_OBJ) {
    aa = getloctype_index;
    strcpy(tempo,get_translation(thisroom.objectnames[aa]));
    if (play.get_loc_name_last_time != 3000+aa)
      guis_need_update = 1;
    play.get_loc_name_last_time = 3000+aa;
    return;
  }
  onhs = getloctype_index;
  if (onhs>0) strcpy(tempo,get_translation(thisroom.hotspotnames[onhs]));
  if (play.get_loc_name_last_time != onhs)
    guis_need_update = 1;
  play.get_loc_name_last_time = onhs;
}

/* *** SCRIPT SYMBOL: [Game] Game::GetLocationName^2 *** */
const char* Game_GetLocationName(int x, int y) {
  char buffer[STD_BUFFER_SIZE];
  GetLocationName(x, y, buffer);
  return CreateNewScriptString(buffer);
}



/* *** SCRIPT SYMBOL: [Game] Debug *** */
void script_debug(int cmdd,int dataa) {
  if (play.debug_mode==0) return;
  int rr;
  if (cmdd==0) {
    for (rr=1;rr<game.numinvitems;rr++)
      playerchar->inv[rr]=1;
    update_invorder();
//    Display("invorder decided there are %d items[display %d",play.inv_numorder,play.inv_numdisp);
    }
  else if (cmdd==1) {
    char toDisplay[STD_BUFFER_SIZE];
    const char *filterName = filter->GetVersionBoxText();
    sprintf(toDisplay,"Adventure Game Studio run-time engine[ACI version " ACI_VERSION_TEXT
      "[Running %d x %d at %d-bit %s[GFX: %s[%s" "Sprite cache size: %ld KB (limit %ld KB; %ld locked)",
      final_scrn_wid,final_scrn_hit,final_col_dep, (convert_16bit_bgr) ? "BGR" : "",
      gfxDriver->GetDriverName(), filterName,
      spriteset.cachesize / 1024, spriteset.maxCacheSize / 1024, spriteset.lockedSize / 1024);
    if (play.seperate_music_lib)
      strcat(toDisplay,"[AUDIO.VOX enabled");
    if (play.want_speech >= 1)
      strcat(toDisplay,"[SPEECH.VOX enabled");
    if (transtree != NULL) {
      strcat(toDisplay,"[Using translation ");
      strcat(toDisplay, transFileName);
    }
    if (opts.mod_player == 0)
      strcat(toDisplay,"[(mod/xm player discarded)");
    Display(toDisplay);
//    Display("shftR: %d  shftG: %d  shftB: %d", _rgb_r_shift_16, _rgb_g_shift_16, _rgb_b_shift_16);
//    Display("Remaining memory: %d kb",_go32_dpmi_remaining_virtual_memory()/1024);
//Display("Play char bcd: %d",bitmap_color_depth(spriteset[views[playerchar->view].frames[playerchar->loop][playerchar->frame].pic]));
    }
  else if (cmdd==2) 
  {  // show walkable areas from here
    block tempw=create_bitmap(BMP_W(thisroom.walls),BMP_H(thisroom.walls));
    blit(prepare_walkable_areas(-1),tempw,0,0,0,0,BMP_W(tempw),BMP_H(tempw));
    block stretched = create_bitmap(scrnwid, scrnhit);
    stretch_sprite(stretched, tempw, -offsetx, -offsety, get_fixed_pixel_size(BMP_W(tempw)), get_fixed_pixel_size(BMP_H(tempw)));

    IDriverDependantBitmap *ddb = gfxDriver->CreateDDBFromBitmap(stretched, false, true);
    render_graphics(ddb, 0, 0);

    destroy_bitmap(tempw);
    destroy_bitmap(stretched);
    gfxDriver->DestroyDDB(ddb);
    while (!ac_kbhit()) ;
    ac_getch();
    invalidate_screen();
  }
  else if (cmdd==3) 
  {
    int goToRoom = -1;
    if (game.roomCount == 0)
    {
      char inroomtex[80];
      sprintf(inroomtex, "!Enter new room: (in room %d)", displayed_room);
      setup_for_dialog();
      goToRoom = enternumberwindow(inroomtex);
      restore_after_dialog();
    }
    else
    {
      setup_for_dialog();
      goToRoom = roomSelectorWindow(displayed_room, game.roomCount, game.roomNumbers, game.roomNames);
      restore_after_dialog();
    }
    if (goToRoom >= 0) 
      NewRoom(goToRoom);
  }
  else if (cmdd == 4) {
    if (display_fps != 2)
      display_fps = dataa;
  }
  else if (cmdd == 5) {
    if (dataa == 0) dataa = game.playercharacter;
    if (game.chars[dataa].walking < 1) {
      Display("Not currently moving.");
      return;
    }
    block tempw=create_bitmap(BMP_W(thisroom.walls),BMP_H(thisroom.walls));
    int mlsnum = game.chars[dataa].walking;
    if (game.chars[dataa].walking >= TURNING_AROUND)
      mlsnum %= TURNING_AROUND;
    MoveList*cmls = &mls[mlsnum];
    clear_to_color(tempw, bitmap_mask_color(tempw));
    for (int i = 0; i < cmls->numstage-1; i++) {
      short srcx=short((cmls->pos[i] >> 16) & 0x00ffff);
      short srcy=short(cmls->pos[i] & 0x00ffff);
      short targetx=short((cmls->pos[i+1] >> 16) & 0x00ffff);
      short targety=short(cmls->pos[i+1] & 0x00ffff);
      line (tempw, srcx, srcy, targetx, targety, get_col8_lookup(i+1));
    }
    stretch_sprite(screen, tempw, -offsetx, -offsety, multiply_up_coordinate(BMP_W(tempw)), multiply_up_coordinate(BMP_H(tempw)));
    render_to_screen(screen, 0, 0);
    wfreeblock(tempw);
    while (!ac_kbhit()) ;
    ac_getch();
  }
  else if (cmdd == 99)
    ccSetOption(SCOPT_DEBUGRUN, dataa);
  else quit("!Debug: unknown command code");
}



/* *** SCRIPT SYMBOL: [Game] SetTimer *** */
void script_SetTimer(int tnum,int timeout) {
  if ((tnum < 1) || (tnum >= MAX_TIMERS))
    quit("!StartTimer: invalid timer number");
  play.script_timers[tnum] = timeout;
  }

/* *** SCRIPT SYMBOL: [Game] IsTimerExpired *** */
int IsTimerExpired(int tnum) {
  if ((tnum < 1) || (tnum >= MAX_TIMERS))
    quit("!IsTimerExpired: invalid timer number");
  if (play.script_timers[tnum] == 1) {
    play.script_timers[tnum] = 0;
    return 1;
    }
  return 0;
  }

/* *** SCRIPT SYMBOL: [Game] AbortGame *** */
void _sc_AbortGame(char*texx, ...) {
  char displbuf[STD_BUFFER_SIZE] = "!?";
  va_list ap;
  va_start(ap,texx);
  my_sprintf(&displbuf[2], get_translation(texx), ap);
  va_end(ap);

  quit(displbuf);
}




/* *** SCRIPT SYMBOL: [Game] GetGraphicalVariable *** */
int GetGraphicalVariable (const char *varName) {
  InteractionVariable *theVar = FindGraphicalVariable(varName);
  if (theVar == NULL) {
    char quitmessage[120];
    sprintf (quitmessage, "!GetGraphicalVariable: interaction variable '%s' not found", varName);
    quit(quitmessage);
    return 0;
  }
  return theVar->value;
}

/* *** SCRIPT SYMBOL: [Game] SetGraphicalVariable *** */
void SetGraphicalVariable (const char *varName, int p_value) {
  InteractionVariable *theVar = FindGraphicalVariable(varName);
  if (theVar == NULL) {
    char quitmessage[120];
    sprintf (quitmessage, "!SetGraphicalVariable: interaction variable '%s' not found", varName);
    quit(quitmessage);
  }
  else
    theVar->value = p_value;
}



/* *** SCRIPT SYMBOL: [Game] SaveGameSlot *** */
void save_game(int slotn, const char*descript) {
  
  // dont allow save in rep_exec_always, because we dont save
  // the state of blocked scripts
  can_run_delayed_command();

  if (inside_script) {
    strcpy(curscript->postScriptSaveSlotDescription[curscript->queue_action(ePSASaveGame, slotn, "SaveGameSlot")], descript);
    return;
  }

  if (platform->GetDiskFreeSpaceMB() < 2) {
    Display("ERROR: There is not enough disk space free to save the game. Clear some disk space and try again.");
    return;
  }

  VALIDATE_STRING(descript);
  char nametouse[260];
  get_save_game_path(slotn, nametouse);

  FILE *ooo = fopen(nametouse, "wb");
  if (ooo == NULL)
    quit("save_game: unable to open savegame file for writing");

  // Initialize and write Vista header
  RICH_GAME_MEDIA_HEADER vistaHeader;
  memset(&vistaHeader, 0, sizeof(RICH_GAME_MEDIA_HEADER));
  memcpy(&vistaHeader.dwMagicNumber, RM_MAGICNUMBER, sizeof(long));
  vistaHeader.dwHeaderVersion = 1;
  vistaHeader.dwHeaderSize = sizeof(RICH_GAME_MEDIA_HEADER);
  vistaHeader.dwThumbnailOffsetHigherDword = 0;
  vistaHeader.dwThumbnailOffsetLowerDword = 0;
  vistaHeader.dwThumbnailSize = 0;
  convert_guid_from_text_to_binary(game.guid, &vistaHeader.guidGameId[0]);
  uconvert(game.gamename, U_ASCII, (char*)&vistaHeader.szGameName[0], U_UNICODE, RM_MAXLENGTH);
  uconvert(descript, U_ASCII, (char*)&vistaHeader.szSaveName[0], U_UNICODE, RM_MAXLENGTH);
  vistaHeader.szLevelName[0] = 0;
  vistaHeader.szComments[0] = 0;

  fwrite(&vistaHeader, sizeof(RICH_GAME_MEDIA_HEADER), 1, ooo);

  fwrite(sgsig,sgsiglen,1,ooo);

  safeguard_string ((unsigned char*)descript);

  fputstring((char*)descript,ooo);

  block screenShot = NULL;

  if (game.options[OPT_SAVESCREENSHOT]) {
    int usewid = multiply_up_coordinate(play.screenshot_width);
    int usehit = multiply_up_coordinate(play.screenshot_height);
    if (usewid > BMP_W(virtual_screen))
      usewid = BMP_W(virtual_screen);
    if (usehit > BMP_H(virtual_screen))
      usehit = BMP_H(virtual_screen);

    if ((play.screenshot_width < 16) || (play.screenshot_height < 16))
      quit("!Invalid game.screenshot_width/height, must be from 16x16 to screen res");

    if (gfxDriver->UsesMemoryBackBuffer())
    {
      screenShot = create_bitmap_ex(bitmap_color_depth(virtual_screen), usewid, usehit);

      stretch_blit(virtual_screen, screenShot, 0, 0,
        BMP_W(virtual_screen), BMP_H(virtual_screen), 0, 0,
        BMP_W(screenShot), BMP_H(screenShot));
    }
    else
    {
      block tempBlock = create_bitmap_ex(final_col_dep, BMP_W(virtual_screen), BMP_H(virtual_screen));
      gfxDriver->GetCopyOfScreenIntoBitmap(tempBlock);

      screenShot = create_bitmap_ex(final_col_dep, usewid, usehit);
      stretch_blit(tempBlock, screenShot, 0, 0,
        BMP_W(tempBlock), BMP_H(tempBlock), 0, 0,
        BMP_W(screenShot), BMP_H(screenShot));

      destroy_bitmap(tempBlock);
    }
  }

  update_polled_stuff();

  save_game_data(ooo, screenShot);

  if (screenShot != NULL)
  {
    long screenShotOffset = ftell(ooo) - sizeof(RICH_GAME_MEDIA_HEADER);
    long screenShotSize = write_screen_shot_for_vista(ooo, screenShot);
    fclose(ooo);

    update_polled_stuff();

    ooo = fopen(nametouse, "r+b");
    fseek(ooo, 12, SEEK_SET);
    putw(screenShotOffset, ooo);
    fseek(ooo, 4, SEEK_CUR);
    putw(screenShotSize, ooo);
  }

  if (screenShot != NULL)
    free(screenShot);

  fclose(ooo);
}



/* *** SCRIPT SYMBOL: [Game] SkipUntilCharacterStops *** */
void SkipUntilCharacterStops(int cc) {
  if (!is_valid_character(cc))
    quit("!SkipUntilCharacterStops: invalid character specified");
  if (game.chars[cc].room!=displayed_room)
    quit("!SkipUntilCharacterStops: specified character not in current room");

  // if they are not currently moving, do nothing
  if (!game.chars[cc].walking)
    return;

  if (play.in_cutscene)
    quit("!SkipUntilCharacterStops: cannot be used within a cutscene");

  initialize_skippable_cutscene();
  play.fast_forward = 2;
  play.skip_until_char_stops = cc;
}



// skipwith decides how it can be skipped:
// 1 = ESC only
// 2 = any key
// 3 = mouse button
// 4 = mouse button or any key
// 5 = right click or ESC only
/* *** SCRIPT SYMBOL: [Game] StartCutscene *** */
void StartCutscene (int skipwith) {
  if (play.in_cutscene)
    quit("!StartCutscene: already in a cutscene");

  if ((skipwith < 1) || (skipwith > 5))
    quit("!StartCutscene: invalid argument, must be 1 to 5.");

  // make sure they can't be skipping and cutsceneing at the same time
  EndSkippingUntilCharStops();

  play.in_cutscene = skipwith;
  initialize_skippable_cutscene();
}

/* *** SCRIPT SYMBOL: [Game] EndCutscene *** */
int EndCutscene () {
  if (play.in_cutscene == 0)
    quit("!EndCutscene: not in a cutscene");

  int retval = play.fast_forward;
  play.in_cutscene = 0;
  // Stop it fast-forwarding
  stop_fast_forwarding();

  // make sure that the screen redraws
  invalidate_screen();

  // Return whether the player skipped it
  return retval;
}

/* *** SCRIPT SYMBOL: [Game] Game::get_SkippingCutscene *** */
int Game_GetSkippingCutscene()
{
  if (play.fast_forward)
  {
    return 1;
  }
  return 0;
}

/* *** SCRIPT SYMBOL: [Game] Game::get_InSkippableCutscene *** */
int Game_GetInSkippableCutscene()
{
  if (play.in_cutscene)
  {
    return 1;
  }
  return 0;
}





void register_game_script_functions() {

  scAdd_External_Symbol("Game::ChangeTranslation^1", (void *)Game_ChangeTranslation);
  scAdd_External_Symbol("Game::DoOnceOnly^1", (void *)Game_DoOnceOnly);
  scAdd_External_Symbol("Game::GetColorFromRGB^3", (void *)Game_GetColorFromRGB);
  scAdd_External_Symbol("Game::GetFrameCountForLoop^2", (void *)Game_GetFrameCountForLoop);
  scAdd_External_Symbol("Game::GetLocationName^2",(void *)Game_GetLocationName);
  scAdd_External_Symbol("Game::GetLoopCountForView^1", (void *)Game_GetLoopCountForView);
  scAdd_External_Symbol("Game::GetRunNextSettingForLoop^2", (void *)Game_GetRunNextSettingForLoop);
  scAdd_External_Symbol("Game::GetSaveSlotDescription^1",(void *)Game_GetSaveSlotDescription);
  scAdd_External_Symbol("Game::GetViewFrame^3",(void *)Game_GetViewFrame);
  scAdd_External_Symbol("Game::InputBox^1",(void *)Game_InputBox);
  scAdd_External_Symbol("Game::SetSaveGameDirectory^1", (void *)Game_SetSaveGameDirectory);
  scAdd_External_Symbol("Game::get_CharacterCount", (void *)Game_GetCharacterCount);

  scAdd_External_Symbol("Game::get_FileName", (void *)Game_GetFileName);
  scAdd_External_Symbol("Game::get_FontCount", (void *)Game_GetFontCount);
  scAdd_External_Symbol("Game::geti_GlobalStrings",(void *)Game_GetGlobalStrings);
  scAdd_External_Symbol("Game::seti_GlobalStrings",(void *)SetGlobalString);
  scAdd_External_Symbol("Game::get_GUICount", (void *)Game_GetGUICount);
  scAdd_External_Symbol("Game::get_IgnoreUserInputAfterTextTimeoutMs", (void *)Game_GetIgnoreUserInputAfterTextTimeoutMs);
  scAdd_External_Symbol("Game::set_IgnoreUserInputAfterTextTimeoutMs", (void *)Game_SetIgnoreUserInputAfterTextTimeoutMs);
  scAdd_External_Symbol("Game::get_InSkippableCutscene", (void *)Game_GetInSkippableCutscene);
  scAdd_External_Symbol("Game::get_InventoryItemCount", (void *)Game_GetInventoryItemCount);
  scAdd_External_Symbol("Game::get_MinimumTextDisplayTimeMs", (void *)Game_GetMinimumTextDisplayTimeMs);
  scAdd_External_Symbol("Game::set_MinimumTextDisplayTimeMs", (void *)Game_SetMinimumTextDisplayTimeMs);
  scAdd_External_Symbol("Game::get_MouseCursorCount", (void *)Game_GetMouseCursorCount);
  scAdd_External_Symbol("Game::get_Name", (void *)Game_GetName);
  scAdd_External_Symbol("Game::set_Name", (void *)Game_SetName);
  scAdd_External_Symbol("Game::get_NormalFont", (void *)Game_GetNormalFont);
  scAdd_External_Symbol("Game::set_NormalFont", (void *)SetNormalFont);
  scAdd_External_Symbol("Game::get_SkippingCutscene", (void *)Game_GetSkippingCutscene);
  scAdd_External_Symbol("Game::get_SpeechFont", (void *)Game_GetSpeechFont);
  scAdd_External_Symbol("Game::set_SpeechFont", (void *)SetSpeechFont);
  scAdd_External_Symbol("Game::geti_SpriteWidth", (void *)Game_GetSpriteWidth);
  scAdd_External_Symbol("Game::geti_SpriteHeight", (void *)Game_GetSpriteHeight);
  scAdd_External_Symbol("Game::get_TextReadingSpeed", (void *)Game_GetTextReadingSpeed);
  scAdd_External_Symbol("Game::set_TextReadingSpeed", (void *)Game_SetTextReadingSpeed);
  scAdd_External_Symbol("Game::get_TranslationFilename",(void *)Game_GetTranslationFilename);
  scAdd_External_Symbol("Game::get_UseNativeCoordinates", (void *)Game_GetUseNativeCoordinates);
  scAdd_External_Symbol("Game::get_ViewCount", (void *)Game_GetViewCount);

  scAdd_External_Symbol("AbortGame",(void *)_sc_AbortGame);
  scAdd_External_Symbol("CallRoomScript",(void *)CallRoomScript);
  scAdd_External_Symbol("ClaimEvent",(void *)ClaimEvent);
  scAdd_External_Symbol("Debug",(void *)script_debug);
  scAdd_External_Symbol("DeleteSaveSlot",(void *)DeleteSaveSlot);
  scAdd_External_Symbol("DisableInterface",(void *)DisableInterface);
  scAdd_External_Symbol("EnableInterface",(void *)EnableInterface);
  scAdd_External_Symbol("EndCutscene", (void *)EndCutscene);
  scAdd_External_Symbol("GetGameOption",(void *)GetGameOption);
  scAdd_External_Symbol("GetGameParameter",(void *)GetGameParameter);
  scAdd_External_Symbol("GetGameSpeed",(void *)GetGameSpeed);
  scAdd_External_Symbol("GetGlobalInt",(void *)GetGlobalInt);
  scAdd_External_Symbol("GetGlobalString",(void *)GetGlobalString);
  scAdd_External_Symbol("GetGraphicalVariable",(void *)GetGraphicalVariable);
  scAdd_External_Symbol("GetLocationName",(void *)GetLocationName);
  scAdd_External_Symbol("GetSaveSlotDescription",(void *)GetSaveSlotDescription);
  scAdd_External_Symbol("GetTextHeight",(void *)GetTextHeight);
  scAdd_External_Symbol("GetTextWidth",(void *)GetTextWidth);
  scAdd_External_Symbol("GetTranslation", (void *)get_translation);
  scAdd_External_Symbol("GetTranslationName", (void *)GetTranslationName);
  scAdd_External_Symbol("GiveScore",(void *)GiveScore);
  scAdd_External_Symbol("InputBox",(void *)sc_inputbox);
  scAdd_External_Symbol("IsGamePaused",(void *)IsGamePaused);
  scAdd_External_Symbol("IsInteractionAvailable", (void *)IsInteractionAvailable);
  scAdd_External_Symbol("IsInterfaceEnabled", (void *)IsInterfaceEnabled);
  scAdd_External_Symbol("IsKeyPressed",(void *)IsKeyPressed);
  scAdd_External_Symbol("IsTimerExpired",(void *)IsTimerExpired);
  scAdd_External_Symbol("IsTranslationAvailable", (void *)IsTranslationAvailable);
  scAdd_External_Symbol("PauseGame",(void *)PauseGame);
  scAdd_External_Symbol("ProcessClick",(void *)ProcessClick);
  scAdd_External_Symbol("QuitGame",(void *)QuitGame);
  scAdd_External_Symbol("Random",(void *)__Rand);
  scAdd_External_Symbol("RestartGame",(void *)restart_game);
  scAdd_External_Symbol("RestoreGameDialog",(void *)restore_game_dialog);
  scAdd_External_Symbol("RestoreGameSlot",(void *)RestoreGameSlot);
  scAdd_External_Symbol("RunAGSGame", (void *)RunAGSGame);
  scAdd_External_Symbol("SaveGameDialog",(void *)save_game_dialog);
  scAdd_External_Symbol("SaveGameSlot",(void *)save_game);
  scAdd_External_Symbol("SaveScreenShot",(void *)SaveScreenShot);
  scAdd_External_Symbol("SetAmbientTint",(void *)SetAmbientTint);
  scAdd_External_Symbol("SetGameOption",(void *)SetGameOption);
  scAdd_External_Symbol("SetGameSpeed",(void *)SetGameSpeed);
  scAdd_External_Symbol("SetGlobalInt",(void *)SetGlobalInt);
  scAdd_External_Symbol("SetGlobalString",(void *)SetGlobalString);
  scAdd_External_Symbol("SetGraphicalVariable",(void *)SetGraphicalVariable);
  scAdd_External_Symbol("SetMultitaskingMode",(void *)SetMultitasking);
  scAdd_External_Symbol("SetNormalFont", (void *)SetNormalFont);
  scAdd_External_Symbol("SetRestartPoint",(void *)SetRestartPoint);
  scAdd_External_Symbol("SetSpeechFont", (void *)SetSpeechFont);
  scAdd_External_Symbol("SetTextWindowGUI",(void *)SetTextWindowGUI);
  scAdd_External_Symbol("SetTimer",(void *)script_SetTimer);
  scAdd_External_Symbol("SkipUntilCharacterStops",(void *)SkipUntilCharacterStops);
  scAdd_External_Symbol("StartCutscene", (void *)StartCutscene);
  scAdd_External_Symbol("UnPauseGame",(void *)UnPauseGame);
  scAdd_External_Symbol("UpdateInventory", (void *)update_invorder);
  scAdd_External_Symbol("Wait",(void *)scrWait);
  scAdd_External_Symbol("WaitKey",(void *)WaitKey);
  scAdd_External_Symbol("WaitMouseKey",(void *)WaitMouseKey);
}

