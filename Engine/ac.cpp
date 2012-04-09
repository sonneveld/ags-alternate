/* Adventure Creator v2 Run-time engine
   Started 27-May-99 (c) 1999-2011 Chris Jones

  Adventure Game Studio source code Copyright 1999-2011 Chris Jones.
  All rights reserved.

  The AGS Editor Source Code is provided under the Artistic License 2.0
  http://www.opensource.org/licenses/artistic-license-2.0.php

  You MAY NOT compile your own builds of the engine without making it EXPLICITLY
  CLEAR that the code has been altered from the Standard Version.

*/

#ifdef MAC_VERSION
#define MAC_HACK
#endif

#include "bmp.h"

//#define THIS_IS_THE_ENGINE   now defined in the VC Project so that it's defined in all files

#define UNICODE
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <time.h>

#ifdef DJGPP
#include <dir.h>
#endif

#if !defined(LINUX_VERSION) && !defined(MAC_VERSION)
#include <process.h>
#endif

// MACPORT FIX: endian support
#include "bigend.h"
#ifdef ALLEGRO_BIG_ENDIAN
struct DialogTopic;
void preprocess_dialog_script(DialogTopic *);
#endif

#ifdef MAC_VERSION
char dataDirectory[512];
char appDirectory[512];
extern "C"
{
   //int osx_sys_question(AL_CONST char *msg, AL_CONST char *but1, AL_CONST char *but2);
}
#endif

#include "misc.h"

#include "clib32.h"
/*
extern "C" {
extern void * memcpy_amd(void *dest, const void *src, size_t n);
}
#define memcpyfast memcpy_amd*/
#define memcpyfast memcpy

#define USE_CLIB

extern int our_eip;

#include "allegro.h"
#include "wgt2allg.h"
#include "sprcache.h"
#include "routefnd.h"
#include "ac_obj.h"
#include "ac_datetime.h"
#include "ac_string.h"
#include "ac_maths.h"
#include "ac_mouse.h"
#include "ac_script_room.h"


// Allegro 4 has switched 15-bit colour to BGR instead of RGB, so
// in this case we need to convert the graphics on load
#if ALW_ALLEGRO_DATE > 19991010
#define USE_15BIT_FIX
#endif

#ifdef WINDOWS_VERSION
#include <crtdbg.h>
//#include "winalleg.h"
#include <shlwapi.h>
#include <shellapi.h>
#elif defined(LINUX_VERSION) || defined(MAC_VERSION)
#define HWND long
#else   // it's DOS (DJGPP)
#include "sys/exceptn.h"
int sys_getch() {
  return getch();
}
#endif  // WINDOWS_VERSION

/*
#if defined(WINDOWS_VERSION) || defined(LINUX_VERSION) || defined(MAC_VERSION)
not needed now that allegro is being built with MSVC solution with no ASM
// The assembler stretch routine seems to GPF
extern "C" {
	void Cstretch_sprite(ALW_BITMAP *dst, ALW_BITMAP *src, int x, int y, int w, int h);
	void Cstretch_blit(ALW_BITMAP *src, ALW_BITMAP *dst, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh);
}

#define stretch_sprite Cstretch_sprite
#define stretch_blit Cstretch_blit
#endif  // WINDOWS_VERSION || LINUX_VERSION || MAC_VERSION
*/

// these 2 defines were applicable to acroom.h when it contained code.
#define NO_SAVE_FUNCTIONS
#define LOADROOM_DO_POLL
#include "acroom.h"

#include "cscomp.h"

#include "AGSEditorDebugger.h"
#include "ac_debug.h"

#define INI_READONLY
//#include <myini.H>

#define BITMAP ALW_BITMAP
#include "agsplugin.h"
#undef BITMAP
//#include <apeg.h>

#include "clib32.h"
#include "mousew32.h"
#include "routefnd.h"
#include "acdialog.h"
#include "csrun.h"
#include "cdplaylib.h"
#include "acchars.h"
#include "ac_parser.h"

#include "allegro.h"


// WOUTTEST_REVERSE used by acgui.h
#define WOUTTEXT_REVERSE wouttext_reverseifnecessary
#include "acgui.h"

#include "acruntim.h"
#include "acsound.h"
#include "ac.h"
#include "acgfx.h"
#include "ali3d.h"
#include "ac_dialog.h"
#include "ac_dialog_render.h"
#include "ac_file.h"
#include "ac_overlay.h"
#include "ac_invitem.h"
#include "ac_script_gui.h"
#include "ac_gui_control.h"
#include "ac_gui_label.h"
#include "ac_gui_button.h"
#include "ac_gui_slider.h"
#include "ac_gui_textbox.h"
#include "ac_gui_inv_window.h"
#include "ac_gui_list_box.h"
#include "ac_hotspot.h"
#include "ac_region.h"
#include "ac_drawsurf.h"
#include "ac_dynspr.h"
#include "ac_viewframe.h"
#include "ac_system.h"
#include "ac_multimedia.h"
#include "ac_text.h"
#include "ac_screen.h"
#include "ac_palette.h"
#include "ac_game.h"
#include "ac_exescr.h"
#include "ac_input.h"
#include "cc_gui_object.h"
#include "cc_character.h"
#include "cc_hotspot.h"
#include "cc_region.h"
#include "cc_inventory.h"
#include "cc_gui.h"
#include "cc_object.h"
#include "cc_dialog.h"
#include "script_string.h"
#include "script_dialog_options_rendering.h"
#include "script_drawing_surface.h"
#include "sc_file.h"
#include "script_overlay.h"
#include "script_date_time.h"
#include "script_view_frame.h"
#include "script_dynamic_sprite.h"
#include "strimpl.h"


#if defined(WINDOWS_VERSION) && !defined(_DEBUG)
#define USE_CUSTOM_EXCEPTION_HANDLER
#endif

//extern "C" HWND allegro_wnd;



#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifdef WINDOWS_VERSION
int wArgc;
LPWSTR *wArgv;
#else
#define wArgc argc
#define wArgv argv
#define LPWSTR char*
#define LPCWSTR const char*
#define WCHAR char
#define StrCpyW strcpy
#endif

#define MAX_SCRIPT_MODULES 50



// ============================================================================
// 000 EXTERNS
// ============================================================================

//extern char*get_language_text(int);
//extern void init_language_text(char*);
//extern void QGRegisterFunctions();  // let the QFG module register its own
int  sc_GetTime(int whatti);



// ============================================================================
// 000 TYPES
// ============================================================================

#include "ac_types.h"



// ============================================================================
// 000 GLOBALS
// ============================================================================

// TODO: commment and figure out what exactly for
// create GlobalContext object?  or individual contexts?

// SETUP
// ------------------------------------
GameSetup usetup;  // user preferences, setup.

// Startup flags, set from parameters to engine
int datafile_argv=0;  // datafile path as an offset in the commandline parameters
int help_required=0;  // when ? is provided on command line.
int change_to_game_dir = 0;  // set to 1 if -shelllaunch 
int force_window = 0;  // -windowed = 1, -fullscreen=2
int override_start_room = 0;  // --startr - override starting room
int force_16bit = 0;    // force to use hicolor mode.
bool justRegisterGame = false;  // register/unregister with game explorer then quit
bool justUnRegisterGame = false;
const char *loadSaveGameOnStartup = NULL;  // if set, load this save game on start. full path to savegame.
char return_to_roomedit[30] = "\0";  // roomedit? set with --testre but not used
char return_to_room[150] = "\0";  // roomedit? set with --testre but not used

// xref: read_config_file and acwsetup
//char datname[80]="ac.clb";
char ac_conf_file_defname[MAX_PATH] = "acsetup.cfg";  // buffer with default acsetup name
char *ac_config_file = &ac_conf_file_defname[0];  // actual ptr to config file.  will be set to full path (cwd or exe file + acsetup.cfg)
char conffilebuf[512];  // buf for parsing ini files.
char filetouse[MAX_PATH] = "nofile";  // ac_config_file copied into it


// ENGINE STATE
// ------------------------------------

AGSPlatformDriver *platform = NULL;   // cross platform wrapper.
int engineNeedsAsInt = 100;  // required engine version, only affects walkable areas.

char* game_file_name=NULL;  // path of the game's file
GameSetupStruct game;  // the actual game. options, arrays of data, etc
int loaded_game_file_version = 0;   // game file version, affects some functionality

GameState play;       // the game state.

int frames_per_second=40; // wanted game speed.

unsigned long loopcounter=0;   // incremented every frame
unsigned long lastcounter=0;   // the frame count at the start of the last second. (used for fps)
volatile unsigned long globalTimerCounter = 0;  // global tick.

int game_paused=0;// controllable from scripts.  prevents update_stuff() from running.

volatile char want_exit = 0;   // other other threads to ask for engine to quit.
volatile char abort_engine = 0;  // kill main loop if set.
int want_quit = 0;            // set by the close window hook.

unsigned int load_new_game = 0; // if not 0 then we should start a new game

// not actually used, checksum is generated against it.
char ac_engine_copyright[]="Adventure Game Studio engine & tools (c) 1999-2000 by Chris Jones.";



// DEBUG
// ------------------------------------
int our_eip=0;  // program counter for identifying program location for crashes

int eip_guinum;  // an inner counter for gui/gfx ops.. so as to not affect out_eip
int eip_guiobj;  // inner object counter

char pexbuf[STD_BUFFER_SIZE];  // str buffer for storing exit messages.
int proper_exit=0;  // if not set to 1 on exit, will print out debug error message.

#define DBG_NOIFACE       1
#define DBG_NODRAWSPRITES 2
#define DBG_NOOBJECTS     4
#define DBG_NOUPDATE      8
#define DBG_NOSFX      0x10
#define DBG_NOMUSIC    0x20
#define DBG_NOSCRIPT   0x40
#define DBG_DBGSCRIPT  0x80
#define DBG_DEBUGMODE 0x100
#define DBG_REGONLY   0x200
#define DBG_NOVIDEO   0x400

int debug_flags=0;    // set by command line arguments.


// DEBUG CONSOLE
// ------------------------------------

int display_console = 0;  // flag to display console ro not.
DebugConsoleText debug_line[DEBUG_CONSOLE_NUMLINES];  // debug console content
int first_debug_line = 0;  // offset of first line
int last_debug_line = 0;  // offset of last line
IDriverDependantBitmap *debugConsole = NULL;  // bitmap for console.
block debugConsoleBuffer = NULL;  // gfx buffer for console.


// GRAPHICS
// ------------------------------------

IGraphicsDriver *gfxDriver;  // graphics driver, to abstract software / 3d renderer.
int working_gfx_mode_status = -1;// set to 0 once successful

// actual dimensions/depth used when initing gfx
int final_scrn_wid=0;
int final_scrn_hit=0;
int final_col_dep=0;

block virtual_screen;  // secondary buffer whcih is blitted to screen.
int scrnwid;  // initialised screen dimensions
int scrnhit;
int current_screen_resolution_multiplier = 1;  // constant to convert from 320x200 to actual screen coords.

color palette[256];  // the game's pallete.
int bg_just_changed = 0;   // bg has changed so update palette when you can.

int offsetx = 0; // view port scrolling offset (eg scumm scrolling)
int offsety = 0;

long t1;  // timer for FPS, when changed, we recalculate fps. (we always assume it was 1 sec period)
int fps=0;  // last calc'd fls
int display_fps=0;   // if non-zero, display the fps. Something happens if == 2 too.

int force_letterbox = 0;  // if 1, allow borders to run in a different resolution
IDriverDependantBitmap *blankImage = NULL;  // used for drawing letterbox borders.
IDriverDependantBitmap *blankSidebarImage = NULL;   // used for drawing side borders for wide screens.

ALW_COLOR_MAP maincoltable;  // allegro lighting table, used for 256 lighting effects.
// created for allegro to speed up mapping 32bit colour to palette
ALW_RGB_MAP rgb_table;  // for 256-col antialiasing

// overlays
ScreenOverlay screenover[MAX_SCREEN_OVERLAYS];  // buffer of overlays
int numscreenover=0;  // number of screen overlays
int is_complete_overlay=0;  // is complete overlay being dispalyed?
int is_text_overlay=0;     // is text overlay being 

block raw_saved_screen = NULL;  // backup copy of the screen, created by CreateCopy/RawSaveScreen
// dynamically created surfaces = a list of copies of surfaces, similar to raw saved screen.
#define MAX_DYNAMIC_SURFACES 20
block dynamicallyCreatedSurfaces[MAX_DYNAMIC_SURFACES];

// crossfades
block temp_virtual = NULL;  // a copy of the screen, for crossfades
color old_palette[256];     // a copy of current palette, for crossfades

block screenstack[10];  // a stack of copies, for push/pop_screen()
int numOnStack = 0;

volatile int switching_away_from_game = 0;  // non-zero when alt-tabbing.  used by mp3 player to disable itself temporarily

// notify gfx system that background has changed and so has to redo walkbehind buffers or whatevs
bool current_background_is_dirty = false; // set by mark_current_background_dirty

// 'screen' is allegro's ptr to the video screen.
block _old_screen=NULL;  // the original screen
block _sub_screen=NULL;  // a sub screen

int debug_15bit_mode = 0; // for debugging, override to 15bit mode
int debug_24bit_mode = 0; // for debugging, override to 24bit mode

int convert_16bit_bgr = 0; // gfx card is lying about being 16bit, do something about it.

int screen_reset = 0; // UNUSED: set when screen is redrawn, never read.


// SPRITES and VIEWS
// ------------------------------------

int trans_mode=0; // alpha channel for drawing sprites using put_sprite_256

// sprites are just graphics, no animations
//block spriteset[MAX_SPRITES+1];
//SpriteCache spriteset (MAX_SPRITES+1);
// initially size 1, this will be increased by the initFile function
SpriteCache spriteset(1);  // cache of all the sprites
int spritewidth[MAX_SPRITES];  // an easy lookup of sprite sizes?
int spriteheight[MAX_SPRITES];
char check_dynamic_sprites_at_exit = 1;  // normally set but disabled on abnormal exits.

// views are collections of sprites to create an animation. with loops etc,
ViewStruct*views=NULL;

// the sprite list is an intermediate list used to order 
// objects and characters by their baselines before everything
// is added to the Thing To Draw List
#define MAX_SPRITES_ON_SCREEN 76
SpriteListEntry sprlist[MAX_SPRITES_ON_SCREEN];
int sprlistsize=0;

#define MAX_THINGS_TO_DRAW 125
SpriteListEntry thingsToDrawList[MAX_THINGS_TO_DRAW];
int thingsToDrawSize = 0;

// stretching sprites
// these vars are global to help with debugging
block tmpdbl; // used for drawing stretched sprite
block curspr; // last sprite stretched
int newwid;   // new width
int newhit;   // new height


// DIRTY RECTANGLES
// ------------------------------------
#define MAXDIRTYREGIONS 25
#define WHOLESCREENDIRTY (MAXDIRTYREGIONS + 5)
#define MAX_SPANS_PER_ROW 4

struct InvalidRect {
  int x1, y1, x2, y2;
};
struct IRSpan {
  int x1, x2;
  int mergeSpan(int tx1, int tx2);
};
struct IRRow {
  IRSpan span[MAX_SPANS_PER_ROW];
  int numSpans;
};

IRRow *dirtyRow = NULL;   // size = screen height
int _dirtyRowSize;    // == screen height
InvalidRect dirtyRegions[MAXDIRTYREGIONS];  // not used, commented out.
int numDirtyRegions = 0;  // ammount left.
int numDirtyBytes = 0;


// TEXT
// ------------------------------------
int source_text_length = -1;  // length of text displayed (orig, non-translated). == how long it is shown on screen for.

char *heightTestString = "ZHwypgfjqhkilIK";  // used by wgetfontheight

int longestline = 0;  // longest line of text, used to figure out dialog sizes.

// whether there are currently remnants of a DisplaySpeech
int screen_is_dirty = 0;  // set when displaying speech, will disable gui if you try to show text at same time.

int texthit;                // text height


// AUDIO
// ------------------------------------

SOUNDCLIP *channels[MAX_SOUND_CHANNELS+1];
int numSoundChannels = 8;

// predefined channel indexes.
enum {SCHAN_SPEECH=0, SCHAN_AMBIENT=1, SCHAN_MUSIC=2, SCHAN_NORMAL=3};

char *music_file;  // path to "audio.vox"
char *speech_file; // path to "speech.vox"

SOUNDCLIP *cachedQueuedMusic = NULL;  // next song to play when current finishes.

// predefined audio clip types.
enum {AUDIOTYPE_LEGACY_AMBIENT_SOUND=1, AUDIOTYPE_LEGACY_MUSIC=2, AUDIOTYPE_LEGACY_SOUND=3};

// crossFading is >0 (channel number of new track), or -1 (old
// track fading out, no new track)
int crossFading = 0;
int crossFadeVolumePerStep = 0;
int crossFadeStep = 0;
int crossFadeVolumeAtStart = 0;

int last_sound_played[MAX_SOUND_CHANNELS + 1];  // if new sound == last sound played, just replay that one.

int use_extra_sound_offset = 0;  // if 1, adjust for directx's extra offset.

int current_music_type = 0;  // current playig music type, eg, MIDI, MOD (MUS_*)
//int current_music=0;


// INPUT - KEYS
// ------------------------------------

// state of scroll-lock
int scrlockWasDown = 0;


// INPUT RECORDING
// ------------------------------------

// recording types (when reading from rec'd file)
#define REC_MOUSECLICK 1
#define REC_MOUSEMOVE  2
#define REC_MOUSEDOWN  3
#define REC_KBHIT      4
#define REC_GETCH      5
#define REC_KEYDOWN    6
#define REC_MOUSEWHEEL 7
#define REC_SPEECHFINISHED 8
#define REC_ENDOFFILE  0x6f

short *recordbuffer = NULL;  // buffer of recording playback
int  recbuffersize = 0;      // size of buff
int recsize = 0;             // current offset into buffer (for recording)

char replayfile[MAX_PATH] = "record.dat";  // replay file, default is 'record.dat' but can be changed
int replay_time = 0;                       // length of recording in seconds
unsigned long replay_last_second = 0;      // related to this and loopcounter and 40... 40fps?
int replay_start_this_time = 0;            // set to non-zero by replay hotkey, plays replay

const char *replayTempFile = "~replay.tmp"; // temp recording file.


// INPUT - MOUSE
// ------------------------------------

// global mouse state structure.
struct GlobalMouseState global_mouse_state;

int cur_mode;       // cursor mode, 
int cur_cursor;     // what is the current cursor (different from mode)

char alpha_blend_cursor = 0;  // if non-zero, use alphablend when drawing cursor.

// alw_mouse_z provided by allegro.
int mouse_z_was = 0;  // previous mouse_wheel value.


// FILESYS
// ------------------------------------

// intermediary buffer when creating game_file_name
WCHAR directoryPathBuffer[MAX_PATH]; // used only by initialise_game_file_name

int use_compiled_folder_as_current_dir = 0;  // only works when debugging


// SAVE GAMES
// ------------------------------------
// the save game number for "restart points", the save game to restore from for a restart
#define RESTART_POINT_SAVE_GAME_NUMBER 999

const char* sgnametemplate = "agssave.%03d";  // sprintf template for save game name
char saveGameSuffix[MAX_SG_EXT_LENGTH + 1]; // suffix to put at end of all save games.
char saveGameDirectory[260] = "./";       // where to save games.
int gameHasBeenRestored = 0;    // increment on every restore.  used by scripts to know when to rerun on reloaded game

#define SGVERSION 8  // current save version
char*sgsig="Adventure Game Studio saved game";  // sig to identify save games
int sgsiglen=32;  // len of above sig.

char rbuffer[200];  // restore/load buffer.

int load_new_game_restore = -1;  // slot number of the last restore?


// GUI
// ------------------------------------
#define MAX_ANIMATING_BUTTONS 15

GUIMain*guis=NULL;    // gui data, loaded by read_gui
//GUIMain dummygui;
//GUIButton dummyguicontrol;
block *guibg = NULL;  // list of bgs for guis
IDriverDependantBitmap **guibgbmp = NULL;    // list of bitmaps for gui

AnimatingGUIButton animbuts[MAX_ANIMATING_BUTTONS];  // list of gui buttons
int numAnimButs = 0;

int mouse_on_iface=-1;   // mouse cursor is over this interface
int mouse_on_iface_button=-1;
int mouse_pushed_iface=-1;  // this BUTTON on interface MOUSE_ON_IFACE is pushed
int mouse_ifacebut_xoffs=-1;
int mouse_ifacebut_yoffs=-1;

int ifacepopped=-1;  // currently displayed pop-up GUI (-1 if none)

int in_inv_screen = 0; // true if in inventory screen
int inv_screen_newroom = -1;  // go to a new room via inventory screen.


// GUI - TOPBAR
// ------------------------------------

// draw_text_window()
TopBarSettings topBar;    // settings for the top bar of text
block screenop = NULL;      // 
int wantFreeScreenop = 0;   // 


// ROOM
// ------------------------------------

#define NO_GAME_ID_IN_ROOM_FILE 16325

roomstruct thisroom;    // room data

RoomStatus *roomstats;  // status for each room
RoomStatus troom;       // used for non-saveable rooms, eg. intro
RoomStatus*croom=NULL;  // current room

int gs_to_newroom=-1;   // set but never read, graphscript
int displayed_room=-10; // index of displayed room
int starting_room = -1; // start room

IDriverDependantBitmap* roomBackgroundBmp = NULL;

int in_new_room=0;     
int new_room_was = 0;  // 1 in new room, 2 first time in new room, 3 loading saved game
int new_room_pos=0;  // 1000 for the left edge, 2000 for the right edge, 3000 for the bottom edge and 4000 for the top edge.
int new_room_x = SCR_NO_VALUE;
int new_room_y = SCR_NO_VALUE;

int new_room_flags=0; // never used



// CHARACTERS
// ------------------------------------

CharacterInfo*playerchar; // player char info
long _sc_PlayerCharPtr = 0;  // script symbol reference to player. use setup_player_character()

CharacterExtras *charextra;  // extra character info.

CharacterCache *charcache = NULL;  // cached info


// MOVING
// ------------------------------------

MoveList *mls = NULL;
/*extern int get_route_composition();
extern int routex1;*/


// WALK BEHIND
// ------------------------------------
char *walkBehindExists = NULL;  // whether a WB area is in this column
int *walkBehindStartY = NULL;
int *walkBehindEndY = NULL;
char noWalkBehindsAtAll = 0;
int walkBehindLeft[MAX_OBJ];
int walkBehindTop[MAX_OBJ];
int walkBehindRight[MAX_OBJ];
int walkBehindBottom[MAX_OBJ];
IDriverDependantBitmap *walkBehindBitmap[MAX_OBJ];
int walkBehindsCachedForBgNum = 0;
WalkBehindMethodEnum walkBehindMethod = DrawOverCharSprite;
int walk_behind_baselines_changed = 0;


// actsps is used for temporary storage of the bitamp image
// of the latest version of the sprite
int actSpsCount = 0;
block *actsps;
IDriverDependantBitmap* *actspsbmp;
// temporary cache of walk-behind for this actsps image
block *actspswb;
IDriverDependantBitmap* *actspswbbmp;
CachedActSpsData* actspswbcache;



// OBJECTS
// ------------------------------------
RoomObject*objs;
ObjectCache objcache[MAX_INIT_SPR];

// Used for deciding whether a char or obj was closer
int char_lowest_yp, obj_lowest_yp;


// SCRIPTS
// ------------------------------------

ccScript* gamescript=NULL;
ccScript* dialogScriptsScript = NULL;

ccInstance *gameinst = NULL;
ccInstance *roominst = NULL;
ccInstance *dialogScriptsInst = NULL;
ccInstance *gameinstFork = NULL;
ccInstance *roominstFork = NULL;
ScriptSystem scsystem;
int num_scripts=0;
int eventClaimed = EVENT_NONE;

char **characterScriptObjNames = NULL;
char objectScriptObjNames[MAX_INIT_SPR][MAX_SCRIPT_NAME_LEN + 5];
char **guiScriptObjNames = NULL;

int scaddr;  // unused

int in_enters_screen=0;
int done_es_error = 0;

int in_leaves_screen = -1;

#define MAXEVENTS 15
EventHappened event[MAXEVENTS+1];
int numevents=0;
#define EV_TEXTSCRIPT 1
#define EV_RUNEVBLOCK 2
#define EV_FADEIN     3
#define EV_IFACECLICK 4
#define EV_NEWROOM    5
#define TS_REPEAT   1
#define TS_KEYPRESS 2
#define TS_MCLICK   3
#define EVB_HOTSPOT 1
#define EVB_ROOM    2

#define REP_EXEC_ALWAYS_NAME "repeatedly_execute_always"
#define REP_EXEC_NAME "repeatedly_execute"
char*tsnames[4]={NULL, REP_EXEC_NAME, "on_key_press","on_mouse_click"};
const char*evblockbasename;
int evblocknum;

int inside_script=0;
int in_graph_script=0;
int no_blocking_functions = 0; // set to 1 while in rep_Exec_always

int post_script_cleanup_stack = 0;

ScriptObject scrObj[MAX_INIT_SPR];
ScriptGUI *scrGui = NULL;
ScriptHotspot scrHotspot[MAX_HOTSPOTS];
ScriptRegion scrRegion[MAX_REGIONS];
ScriptInvItem scrInv[MAX_INV];
ScriptDialog scrDialog[MAX_DIALOG];

NonBlockingScriptFunction repExecAlways(REP_EXEC_ALWAYS_NAME, 0);
NonBlockingScriptFunction getDialogOptionsDimensionsFunc("dialog_options_get_dimensions", 1);
NonBlockingScriptFunction renderDialogOptionsFunc("dialog_options_render", 1);
NonBlockingScriptFunction getDialogOptionUnderCursorFunc("dialog_options_get_active", 1);
NonBlockingScriptFunction runDialogOptionMouseClickHandlerFunc("dialog_options_mouse_click", 2);

char scfunctionname[30];

//char*ac_default_header=NULL,*temphdr=NULL;
char ac_default_header[15000],temphdr[10000];


// PLUGINS
// ------------------------------------
PluginObjectReader pluginReaders[MAX_PLUGIN_OBJECT_READERS];
int numPluginReaders = 0;
int pluginsWantingDebugHooks = 0;


// MODULES
// ------------------------------------
ccScript *scriptModules[MAX_SCRIPT_MODULES];
ccInstance *moduleInst[MAX_SCRIPT_MODULES];
ccInstance *moduleInstFork[MAX_SCRIPT_MODULES];
char *moduleRepExecAddr[MAX_SCRIPT_MODULES];
int numScriptModules = 0;
ExecutingScript scripts[MAX_SCRIPT_AT_ONCE];
ExecutingScript*curscript = NULL;


// WALKING
// ------------------------------------
block walkareabackup=NULL;
block walkable_areas_temp = NULL;


// DIALOG
// ------------------------------------
DialogTopic *dialog;

int said_speech_line; // used while in dialog to track whether screen needs updating

int said_text = 0;


// FACE TALK
// ------------------------------------
// Sierra-style speech settings
int face_talking=-1;
int facetalkview=0;
int facetalkwait=0;
int facetalkframe=0;
int facetalkloop=0;
int facetalkrepeat = 0;
int facetalkAllowBlink = 1;
int facetalkBlinkLoop = 0;
CharacterInfo *facetalkchar = NULL;


// LIP SYNC
// ------------------------------------
// lip-sync speech settings
int loops_per_character;
int text_lips_offset;
int char_speaking = -1;
char *text_lips_text = NULL;
SpeechLipSyncLine *splipsync = NULL;
int numLipLines = 0;
int curLipLine = -1;
int curLipLinePhenome = 0;


// HOTSPOTS
// ------------------------------------
int getloctype_index = 0;
int getloctype_throughgui = 0;

#define LOCTYPE_HOTSPOT 1
#define LOCTYPE_CHAR 2
#define LOCTYPE_OBJ  3


// USER DISABLED
// ------------------------------------
int user_disabled_for=0;
int user_disabled_data=0;
int user_disabled_data2=0;
int user_disabled_data3=0;


// ANIMATION?
// ------------------------------------
int restrict_until=0;


// EDITOR DEBUG
// ------------------------------------
int editor_debugging_enabled = 0;
int editor_debugging_initialized = 0;
char editor_debugger_instance_token[100];
IAGSEditorDebugger *editor_debugger = NULL;
int break_on_next_script_step = 0;
volatile int game_paused_in_debugger = 0;
HWND editor_window_handle = NULL;


// CD PLAYER
// ------------------------------------
int need_to_stop_cd=0;
int use_cdplayer=0;                   // set to 1 if enabled.
bool triedToUseCdAudioCommand = false;  // only written, never read


// TIMER
// ------------------------------------
// set by dj_timer_handler
volatile int timerloop=0;
volatile int mvolcounter = 0;
int update_music_at=0;
int time_between_timers=25;  // in milliseconds


// MISC
// ------------------------------------

//int abort_all_conditions=0;

// return codes for load_game()
const char *load_game_errors[9] =
  {"No error","File not found","Not an AGS save game",
  "Invalid save game version","Saved with different interpreter",
  "Saved under a different game", "Resolution mismatch",
  "Colour depth mismatch", ""};

// We need COLOR_DEPTH_24 to allow it to load the preload PCX in hi-col
//BEGIN_COLOR_DEPTH_LIST
//  COLOR_DEPTH_8
//  COLOR_DEPTH_15
//  COLOR_DEPTH_16
//  COLOR_DEPTH_24
//  COLOR_DEPTH_32
//END_COLOR_DEPTH_LIST



// ============================================================================
// 000 TYPE IMPL
// ============================================================================

  TempEip::TempEip (int newval) {
    oldval = our_eip;
    our_eip = newval;
  }
  TempEip::~TempEip () { our_eip = oldval; }


NonBlockingScriptFunction::NonBlockingScriptFunction(const char*funcName, int numParams)
  {
    this->functionName = funcName;
    this->numParameters = numParams;
    atLeastOneImplementationExists = false;
    roomHasFunction = true;
    globalScriptHasFunction = true;

    for (int i = 0; i < MAX_SCRIPT_MODULES; i++)
    {
      moduleHasFunction[i] = true;
    }
  }



// ============================================================================
// 000 FUNC FOWARD DECLARATIONS
// ============================================================================

//void draw_sprite_compensate(int,int,int,int);
//char *get_translation(const char*);

//void draw_sprite_compensate(int,int,int,int);
//char *get_translation(const char*);
//void quitprintf(char*texx, ...);
//void replace_macro_tokens(char*,char*);
//void wouttext_reverseifnecessary(int x, int y, int font, char *text);
void SetGameSpeed(int newspd);
void SetMultitasking(int mode);
//void put_sprite_256(int xxx,int yyy,block piccy);
int initialize_engine_with_exception_handling(int argc,char*argv[]);
int initialize_engine(int argc,char*argv[]);
//block recycle_bitmap(block bimp, int coldep, int wid, int hit);

//void do_main_cycle(int untilwhat,int daaa);
//int  Overlay_GetValid(ScriptOverlay *scover);
int  run_text_script(ccInstance*,char*);
int  run_text_script_2iparam(ccInstance*,char*,int,int);
int  run_text_script_iparam(ccInstance*,char*,int);
//void run_graph_script(int);
//void run_event_block(EventBlock*,int,int=-1, int=-1);

//void new_room(int,CharacterInfo*);
//void NewRoom(int);


//void GiveScore(int);
//void walk_character(int,int,int,int,bool);
//void StopMoving(int);
//void MoveCharacterToHotspot(int,int);
//int  GetCursorMode();
//void GetLocationName(int,int,char*);
void save_game(int,const char*);
//int  load_game(int,char*, int*);
//void update_music_volume();
//int  invscreen();
void process_interface_click(int,int,int);
//void DisplayMessage (int);
//void do_conversation(int);
void compile_room_script();
//int  CreateTextOverlay(int,int,int,int,int,char*,...);
//void RemoveOverlay(int);
//void stopmusic();
//void SetCharacterView(int,int);
//void ReleaseCharacterView(int);
//void update_events();
//void process_event(EventHappened*);
//int  GetLocationType(int,int);
//int  __GetLocationType(int,int,int);
//int  AreCharObjColliding(int charid,int objid);
//int  play_speech(int,int);
//void stop_speech();
//int  play_sound (int);
//int  play_sound_priority (int, int);
int  __Rand(int);
//void MergeObject(int);
void script_debug(int,int);
//void ParseText(char*);
//void FaceLocation(int,int,int);
//void check_debug_keys();
int  IsInterfaceEnabled();
//void break_up_text_into_lines(int,int,char*);
void start_game();
void init_game_settings();
void show_preload();
//void stop_recording ();
void save_game_data (FILE *, block screenshot);
void setup_script_exports ();
void SetSpeechFont (int);
void SetNormalFont (int);


//void render_graphics(IDriverDependantBitmap *extraBitmap = NULL, int extraX = 0, int extraY = 0);
int  wait_loop_still_valid();
//SOUNDCLIP *load_music_from_disk(int mnum, bool repeat);
//void play_new_music(int mnum, SOUNDCLIP *music);
int GetGameSpeed();
//int check_for_messages_from_editor();
//int show_dialog_options(int dlgnum, int sayChosenOption, bool runGameLoopsInBackground);


// MACPORT FIX 9/6/5: undef (was macro) and add prototype
#undef wouttext_outline
void wouttext_outline(int xxp, int yyp, int usingfont, char *texx);
  

// setup separate errno
/*#if defined(LINUX_VERSION) || defined(MAC_VERSION)
int myerrno;
#else
int errno;
#define myerrno errno
#endif*/

// for external modules to call
void next_iteration() {
  NEXT_ITERATION();
}


// ============================================================================
// DEBUG
// ============================================================================

void set_eip(int eip) {
  our_eip = eip;
}



// ============================================================================
// NEW INTERACTION CMD IMPLEMENTATION
// ============================================================================

void NewInteractionCommand::remove () {
  if (children != NULL) {
    children->reset();
    delete children;
  }
  children = NULL;
  parent = NULL;
  type = 0;
}



// ============================================================================
// NON_BLOCKING CHECK
// ============================================================================

// check and abort game if the script is currently
// inside the rep_exec_always function
void can_run_delayed_command() {
  if (no_blocking_functions)
    quit("!This command cannot be used within non-blocking events such as " REP_EXEC_ALWAYS_NAME);
}





// ============================================================================
// TIMER
// ============================================================================

// our timer, used to keep game running at same speed on all systems
#if defined(WINDOWS_VERSION) || defined(LINUX_VERSION) || defined(MAC_VERSION)
void __cdecl dj_timer_handler() {
#else
void dj_timer_handler(...) {
#endif
  timerloop++;
  globalTimerCounter++;
  if (mvolcounter > 0) mvolcounter++;
  }
ALW_END_OF_FUNCTION(dj_timer_handler);

void set_game_speed(int fps) {
  frames_per_second = fps;
  time_between_timers = 1000 / fps;
  alw_install_int_ex(dj_timer_handler,ALW_MSEC_TO_TIMER(time_between_timers));
}



// ============================================================================
// GRAPHICS - 15-BIT FIX
// ============================================================================

#ifdef USE_15BIT_FIX
extern "C" {
    extern ALW_GFX_VTABLE *_get_vtable(int color_depth);
}

// xref: load_new_room, initialize_sprite
// called if USE_15BIT_FIX is set.
block convert_16_to_15(block iii) {
//  int xx,yy,rpix;
  int iwid = BMP_W(iii), ihit = BMP_H(iii);
  int x,y;

  if (alw_bitmap_color_depth(iii) > 16) {
    // we want a 32-to-24 conversion
    block tempbl = alw_create_bitmap_ex(final_col_dep,iwid,ihit);

    if (final_col_dep < 24) {
      // 32-to-16
      alw_blit(iii, tempbl, 0, 0, 0, 0, iwid, ihit);
      return tempbl;
    }

	  GFX_VTABLE *vtable = _get_vtable(final_col_dep);
	  if (vtable == NULL) {
      quit("unable to get 24-bit bitmap vtable");
    }

    tempbl->vtable = vtable;

    for (y=0; y < BMP_H(tempbl); y++) {
      unsigned char*p32 = (unsigned char *)BMP_LINE(iii)[y];
      unsigned char*p24 = (unsigned char *)BMP_LINE(tempbl)[y];

      // strip out the alpha channel bit and copy the rest across
      for (x=0; x < BMP_W(tempbl); x++) {
        memcpy(&p24[x * 3], &p32[x * 4], 3);
		  }
    }

    return tempbl;
  }

  // we want a 16-to-15 converstion

  unsigned short c,r,g,b;
  // we do this process manually - no allegro color conversion
  // because we store the RGB in a particular order in the data files
  block tempbl = alw_create_bitmap_ex(15,iwid,ihit);

	GFX_VTABLE *vtable = _get_vtable(15);

	if (vtable == NULL) {
    quit("unable to get 15-bit bitmap vtable");
  }

  tempbl->vtable = vtable;

  for (y=0; y < BMP_H(tempbl); y++) {
    unsigned short*p16 = (unsigned short *)BMP_LINE(iii)[y];
    unsigned short*p15 = (unsigned short *)BMP_LINE(tempbl)[y];

    for (x=0; x < BMP_W(tempbl); x++) {
			c = p16[x];
			b = alw_get_rgb_scale_5(c & 0x1F);
			g = alw_get_rgb_scale_6((c >> 5) & 0x3F);
			r = alw_get_rgb_scale_5((c >> 11) & 0x1F);
			p15[x] = alw_makecol15(r, g, b);
		}
  }
/*
  // the auto color conversion doesn't seem to work
  for (xx=0;xx<iwid;xx++) {
    for (yy=0;yy<ihit;yy++) {
      rpix = alw__getpixel16(iii,xx,yy);
      rpix = (rpix & 0x001f) | ((rpix >> 1) & 0x7fe0);
      // again putpixel16 because the dest is actually 16bit
      alw__putpixel15(tempbl,xx,yy,rpix);
    }
  }*/

  return tempbl;
}

int _places_r = 3, _places_g = 2, _places_b = 3;

// convert RGB to BGR for strange graphics cards
// xref: load_new_room, initialize_sprite
// called if USE_15BIT_FIX is set.
block convert_16_to_16bgr(block tempbl) {

  int x,y;
  unsigned short c,r,g,b;

  for (y=0; y < BMP_H(tempbl); y++) {
    unsigned short*p16 = (unsigned short *)BMP_LINE(tempbl)[y];

    for (x=0; x < BMP_W(tempbl); x++) {
			c = p16[x];
      if (c != ALW_MASK_COLOR_16) {
        b = alw_get_rgb_scale_5(c & 0x1F);
		g = alw_get_rgb_scale_6((c >> 5) & 0x3F);
        r = alw_get_rgb_scale_5((c >> 11) & 0x1F);
        // allegro assumes 5-6-5 for 16-bit
        p16[x] = (((r >> _places_r) << alw_get_rgb_r_shift_16()) |
            ((g >> _places_g) << alw_get_rgb_g_shift_16()) |
            ((b >> _places_b) << alw_get_rgb_b_shift_16()));

      }
		}
  }

  return tempbl;
}
#endif



// ============================================================================
// GRAPHICS - RESOLUTION SYSTEM
// ============================================================================

// Begin resolution system functions

// Multiplies up the number of pixels depending on the current 
// resolution, to give a relatively fixed size at any game res
int get_fixed_pixel_size(int pixels)
{
  return pixels * current_screen_resolution_multiplier;
}

int convert_to_low_res(int coord)
{
  if (game.options[OPT_NATIVECOORDINATES] == 0)
    return coord;
  else
    return coord / current_screen_resolution_multiplier;
}

int convert_back_to_high_res(int coord)
{
  if (game.options[OPT_NATIVECOORDINATES] == 0)
    return coord;
  else
    return coord * current_screen_resolution_multiplier;
}

int multiply_up_coordinate(int coord)
{
  if (game.options[OPT_NATIVECOORDINATES] == 0)
    return coord * current_screen_resolution_multiplier;
  else
    return coord;
}

void multiply_up_coordinates(int *x, int *y)
{
  if (game.options[OPT_NATIVECOORDINATES] == 0)
  {
    x[0] *= current_screen_resolution_multiplier;
    y[0] *= current_screen_resolution_multiplier;
  }
}

void multiply_up_coordinates_round_up(int *x, int *y)
{
  if (game.options[OPT_NATIVECOORDINATES] == 0)
  {
    x[0] = x[0] * current_screen_resolution_multiplier + (current_screen_resolution_multiplier - 1);
    y[0] = y[0] * current_screen_resolution_multiplier + (current_screen_resolution_multiplier - 1);
  }
}

int divide_down_coordinate(int coord)
{
  if (game.options[OPT_NATIVECOORDINATES] == 0)
    return coord / current_screen_resolution_multiplier;
  else
    return coord;
}

int divide_down_coordinate_round_up(int coord)
{
  if (game.options[OPT_NATIVECOORDINATES] == 0)
    return (coord / current_screen_resolution_multiplier) + (current_screen_resolution_multiplier - 1);
  else
    return coord;
}

// End resolution system functions



// ============================================================================
// TEXT - FONT/TEXT SIZES
// ============================================================================

int wgetfontheight(int font) {
  int htof = wgettextheight(heightTestString, font);

  // automatic outline fonts are 2 pixels taller
  if (game.fontoutline[font] == FONT_OUTLINE_AUTO) {
    // scaled up SCI font, push outline further out
    if ((game.options[OPT_NOSCALEFNT] == 0) && (!fontRenderers[font]->SupportsExtendedCharacters(font)))
      htof += get_fixed_pixel_size(2);
    // otherwise, just push outline by 1 pixel
    else
      htof += 2;
  }

  return htof;
}

int wgettextwidth_compensate(const char *tex, int font) {
  int wdof = wgettextwidth(tex, font);

  if (game.fontoutline[font] == FONT_OUTLINE_AUTO) {
    // scaled up SCI font, push outline further out
    if ((game.options[OPT_NOSCALEFNT] == 0) && (!fontRenderers[font]->SupportsExtendedCharacters(font)))
      wdof += get_fixed_pixel_size(2);
    // otherwise, just push outline by 1 pixel
    else
      wdof += get_fixed_pixel_size(1);
  }

  return wdof;
}



// ============================================================================
// GRAPHICS - DIRTY RECTANGLES
// ============================================================================



int IRSpan::mergeSpan(int tx1, int tx2) {
  if ((tx1 > x2) || (tx2 < x1))
    return 0;
  // overlapping, increase the span
  if (tx1 < x1)
    x1 = tx1;
  if (tx2 > x2)
    x2 = tx2;
  return 1;
}

void init_invalid_regions(int scrnHit) {
  numDirtyRegions = WHOLESCREENDIRTY;
  dirtyRow = (IRRow*)malloc(sizeof(IRRow) * scrnHit);

  for (int e = 0; e < scrnHit; e++)
    dirtyRow[e].numSpans = 0;
  _dirtyRowSize = scrnHit;
}

void update_invalid_region(int x, int y, block src, block dest) {
  int i;

  // convert the offsets for the destination into
  // offsets into the source
  x = -x;
  y = -y;

  if (numDirtyRegions == WHOLESCREENDIRTY) {
    alw_blit(src, dest, x, y, 0, 0, BMP_W(dest), BMP_H(dest));
  }
  else {
    int k, tx1, tx2, srcdepth = alw_bitmap_color_depth(src);
    if ((srcdepth == alw_bitmap_color_depth(dest)) && (alw_is_memory_bitmap(dest))) {
      int bypp = bmp_bpp(src);
      // do the fast copy
      for (i = 0; i < scrnhit; i++) {
        for (k = 0; k < dirtyRow[i].numSpans; k++) {
          tx1 = dirtyRow[i].span[k].x1;
          tx2 = dirtyRow[i].span[k].x2;
          memcpyfast(&BMP_LINE(dest)[i][tx1 * bypp], &BMP_LINE(src)[i + y][(tx1 + x) * bypp], ((tx2 - tx1) + 1) * bypp);
        }
      }
    }
    else {
      // do the fast copy
      int rowsInOne;
      for (i = 0; i < scrnhit; i++) {
        rowsInOne = 1;
        
        // if there are rows with identical masks, do them all in one go
        while ((i+rowsInOne < scrnhit) && (memcmp(&dirtyRow[i], &dirtyRow[i+rowsInOne], sizeof(IRRow)) == 0))
          rowsInOne++;

        for (k = 0; k < dirtyRow[i].numSpans; k++) {
          tx1 = dirtyRow[i].span[k].x1;
          tx2 = dirtyRow[i].span[k].x2;
          alw_blit(src, dest, tx1 + x, i + y, tx1, i, (tx2 - tx1) + 1, rowsInOne);
        }
        
        i += (rowsInOne - 1);
      }
    }
   /* else {
      // update the dirty regions
      for (i = 0; i < numDirtyRegions; i++) {
        alw_blit(src, dest, x + dirtyRegions[i].x1, y + dirtyRegions[i].y1,
           dirtyRegions[i].x1, dirtyRegions[i].y1,
           (dirtyRegions[i].x2 - dirtyRegions[i].x1) + 1,
           (dirtyRegions[i].y2 - dirtyRegions[i].y1) + 1);
      }
    }*/
  }
}


void update_invalid_region_and_reset(int x, int y, block src, block dest) {

  int i;

  update_invalid_region(x, y, src, dest);

  // screen has been updated, no longer dirty
  numDirtyRegions = 0;
  numDirtyBytes = 0;

  for (i = 0; i < _dirtyRowSize; i++)
    dirtyRow[i].numSpans = 0;

}

int combine_new_rect(InvalidRect *r1, InvalidRect *r2) {

  // check if new rect is within old rect X-wise
  if ((r2->x1 >= r1->x1) && (r2->x2 <= r1->x2)) {
    if ((r2->y1 >= r1->y1) && (r2->y2 <= r1->y2)) {
      // Y is also within the old one - scrap the new rect
      return 1;
    }
  }

  return 0;
}

void invalidate_rect(int x1, int y1, int x2, int y2) {
  if (numDirtyRegions >= MAXDIRTYREGIONS) {
    // too many invalid rectangles, just mark the whole thing dirty
    numDirtyRegions = WHOLESCREENDIRTY;
    return;
  }

  int a;

  if (x1 >= scrnwid) x1 = scrnwid-1;
  if (y1 >= scrnhit) y1 = scrnhit-1;
  if (x2 >= scrnwid) x2 = scrnwid-1;
  if (y2 >= scrnhit) y2 = scrnhit-1;
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 < 0) x2 = 0;
  if (y2 < 0) y2 = 0;
/*
  dirtyRegions[numDirtyRegions].x1 = x1;
  dirtyRegions[numDirtyRegions].y1 = y1;
  dirtyRegions[numDirtyRegions].x2 = x2;
  dirtyRegions[numDirtyRegions].y2 = y2;

  for (a = 0; a < numDirtyRegions; a++) {
    // see if we can merge it into any other regions
    if (combine_new_rect(&dirtyRegions[a], &dirtyRegions[numDirtyRegions]))
      return;
  }

  numDirtyBytes += (x2 - x1) * (y2 - y1);

  if (numDirtyBytes > (scrnwid * scrnhit) / 2)
    numDirtyRegions = WHOLESCREENDIRTY;
  else {*/
    numDirtyRegions++;

    // ** Span code
    int s, foundOne;
    // add this rect to the list for this row
    for (a = y1; a <= y2; a++) {
      foundOne = 0;
      for (s = 0; s < dirtyRow[a].numSpans; s++) {
        if (dirtyRow[a].span[s].mergeSpan(x1, x2)) {
          foundOne = 1;
          break;
        }
      }
      if (foundOne) {
        // we were merged into a span, so we're ok
        int t;
        // check whether now two of the spans overlap each other
        // in which case merge them
        for (s = 0; s < dirtyRow[a].numSpans; s++) {
          for (t = s + 1; t < dirtyRow[a].numSpans; t++) {
            if (dirtyRow[a].span[s].mergeSpan(dirtyRow[a].span[t].x1, dirtyRow[a].span[t].x2)) {
              dirtyRow[a].numSpans--;
              for (int u = t; u < dirtyRow[a].numSpans; u++)
                dirtyRow[a].span[u] = dirtyRow[a].span[u + 1];
              break;
            }
          }
        }
      }
      else if (dirtyRow[a].numSpans < MAX_SPANS_PER_ROW) {
        dirtyRow[a].span[dirtyRow[a].numSpans].x1 = x1;
        dirtyRow[a].span[dirtyRow[a].numSpans].x2 = x2;
        dirtyRow[a].numSpans++;
      }
      else {
        // didn't fit in an existing span, and there are none spare
        int nearestDist = 99999, nearestWas = -1, extendLeft;
        int tleft, tright;
        // find the nearest span, and enlarge that to include this rect
        for (s = 0; s < dirtyRow[a].numSpans; s++) {
          tleft = dirtyRow[a].span[s].x1 - x2;
          if ((tleft > 0) && (tleft < nearestDist)) {
            nearestDist = tleft;
            nearestWas = s;
            extendLeft = 1;
          }
          tright = x1 - dirtyRow[a].span[s].x2;
          if ((tright > 0) && (tright < nearestDist)) {
            nearestDist = tright;
            nearestWas = s;
            extendLeft = 0;
          }
        }
        if (extendLeft)
          dirtyRow[a].span[nearestWas].x1 = x1;
        else
          dirtyRow[a].span[nearestWas].x2 = x2;
      }
    }
    // ** End span code
  //}
}

void invalidate_sprite(int x1, int y1, IDriverDependantBitmap *pic) {
  invalidate_rect(x1, y1, x1 + pic->GetWidth(), y1 + pic->GetHeight());
}

void draw_and_invalidate_text(int x1, int y1, int font, const char *text) {
  wouttext_outline(x1, y1, font, (char*)text);
  invalidate_rect(x1, y1, x1 + wgettextwidth_compensate(text, font), y1 + wgetfontheight(font) + get_fixed_pixel_size(1));
}

void invalidate_screen() {
  // mark the whole screen dirty
  numDirtyRegions = WHOLESCREENDIRTY;
}

// ** dirty rectangle system end **

void mark_current_background_dirty()
{
  current_background_is_dirty = true;
}



// ============================================================================
// OBJECTS
// ============================================================================

int is_valid_object(int obtest) {
  if ((obtest < 0) || (obtest >= croom->numobj)) return 0;
  return 1;
}



// ============================================================================
// SCREEN RENDERING
// ============================================================================

int get_screen_y_adjustment(ALW_BITMAP *checkFor) {

  if ((alw_screen == _sub_screen) && (BMP_H(checkFor) < final_scrn_hit))
    return get_fixed_pixel_size(20);

  return 0;
}

int get_screen_x_adjustment(ALW_BITMAP *checkFor) {

  if (((alw_screen) == _sub_screen) && (BMP_W(checkFor) < final_scrn_wid))
    return (final_scrn_wid - BMP_W(checkFor)) / 2;

  return 0;
}

void render_black_borders(int atx, int aty)
{
  if (!gfxDriver->UsesMemoryBackBuffer())
  {
    if (aty > 0)
    {
      // letterbox borders
      blankImage->SetStretch(scrnwid, aty);
      gfxDriver->DrawSprite(0, -aty, blankImage);
      gfxDriver->DrawSprite(0, scrnhit, blankImage);
    }
    if (atx > 0)
    {
      // sidebar borders for widescreen
      blankSidebarImage->SetStretch(atx, scrnhit);
      gfxDriver->DrawSprite(-atx, 0, blankSidebarImage);
      gfxDriver->DrawSprite(scrnwid, 0, blankSidebarImage);
    }
  }
}

void render_to_screen(ALW_BITMAP *toRender, int atx, int aty) {

  atx += get_screen_x_adjustment(toRender);
  aty += get_screen_y_adjustment(toRender);
  gfxDriver->SetRenderOffset(atx, aty);

  render_black_borders(atx, aty);

  gfxDriver->DrawSprite(AGSE_FINALSCREENDRAW, 0, NULL);

  if (play.screen_is_faded_out)
  {
    if (gfxDriver->UsesMemoryBackBuffer())
      gfxDriver->RenderToBackBuffer();
    gfxDriver->ClearDrawList();
    return;
  }

  // only vsync in full screen mode, it makes things worse
  // in a window
  //gfxDriver->EnableVsyncBeforeRender((scsystem.vsync > 0) && (usetup.windowed == 0));
  // always vsync
  gfxDriver->EnableVsyncBeforeRender(1);

  bool succeeded = false;
  while (!succeeded)
  {
    try
    {
      gfxDriver->Render((GlobalFlipType)play.screen_flipped);
      succeeded = true;
    }
    catch (Ali3DFullscreenLostException) 
    { 
      platform->Delay(500);
    }
  }
}

void clear_letterbox_borders() {

  if (multiply_up_coordinate(thisroom.height) < final_scrn_hit) {
    // blank out any traces in borders left by a full-screen room
    gfxDriver->ClearRectangle(0, 0, BMP_W(_old_screen) - 1, get_fixed_pixel_size(20) - 1, NULL);
    gfxDriver->ClearRectangle(0, final_scrn_hit - get_fixed_pixel_size(20), BMP_W(_old_screen) - 1, final_scrn_hit - 1, NULL);
  }

}

// writes the virtual screen to the screen, converting colours if
// necessary
void write_screen() {

  if (play.fast_forward)
    return;

  static int wasShakingScreen = 0;
  bool clearScreenBorders = false;
  int at_yp = 0;

  if (play.shakesc_length > 0) {
    wasShakingScreen = 1;
    if ( (loopcounter % play.shakesc_delay) < (play.shakesc_delay / 2) )
      at_yp = multiply_up_coordinate(play.shakesc_amount);
    invalidate_screen();
  }
  else if (wasShakingScreen) {
    wasShakingScreen = 0;

    if (!gfxDriver->RequiresFullRedrawEachFrame())
    {
      clear_letterbox_borders();
    }
  }

  if (play.screen_tint < 1)
    gfxDriver->SetScreenTint(0, 0, 0);
  else
    gfxDriver->SetScreenTint(play.screen_tint & 0xff, (play.screen_tint >> 8) & 0xff, (play.screen_tint >> 16) & 0xff);

  render_to_screen(virtual_screen, 0, at_yp);
}



// ============================================================================
// SAVE GAMES
// ============================================================================

void get_save_game_path(int slotNum, char *buffer) {
  strcpy(buffer, saveGameDirectory);
  sprintf(&buffer[strlen(buffer)], sgnametemplate, slotNum);
  strcat(buffer, saveGameSuffix);
}


int load_game_and_print_error(int toload) {
  int ecret = load_game(toload, NULL, NULL);
  if (ecret < 0) {
    // disable speech in case there are dynamic graphics that
    // have been freed
    int oldalways = game.options[OPT_ALWAYSSPCH];
    game.options[OPT_ALWAYSSPCH] = 0;
    Display("Unable to load game (error: %s).",load_game_errors[-ecret]);
    game.options[OPT_ALWAYSSPCH] = oldalways;
  }
  return ecret;
}



// ============================================================================
// SCRIPTS
// ============================================================================

int prepare_text_script(ccInstance*sci,char**tsname) {
  ccError=0;
  if (sci==NULL) return -1;
  if (ccGetSymbolAddr(sci,tsname[0]) == NULL) {
    strcpy (ccErrorString, "no such function in script");
    return -2;
  }
  if (sci->pc!=0) {
    strcpy(ccErrorString,"script is already in execution");
    return -3;
  }
  scripts[num_scripts].init();
  scripts[num_scripts].inst = sci;
/*  char tempb[300];
  sprintf(tempb,"Creating script instance for '%s' room %d",tsname[0],displayed_room);
  write_log(tempb);*/
  if (sci->pc != 0) {
//    write_log("Forking instance");
    scripts[num_scripts].inst = ccForkInstance(sci);
    if (scripts[num_scripts].inst == NULL)
      quit("unable to fork instance for secondary script");
    scripts[num_scripts].forked = 1;
    }
  curscript = &scripts[num_scripts];
  num_scripts++;
  if (num_scripts >= MAX_SCRIPT_AT_ONCE)
    quit("too many nested text script instances created");
  // in case script_run_another is the function name, take a backup
  strcpy(scfunctionname,tsname[0]);
  tsname[0]=&scfunctionname[0];
  update_script_mouse_coords();
  inside_script++;
//  aborted_ip=0;
//  abort_executor=0;
  return 0;
  }

void cancel_all_scripts() {
  int aa;

  for (aa = 0; aa < num_scripts; aa++) {
    if (scripts[aa].forked)
      ccAbortAndDestroyInstance(scripts[aa].inst);
    else
      ccAbortInstance(scripts[aa].inst);
    scripts[aa].numanother = 0;
    }
  num_scripts = 0;
/*  if (gameinst!=NULL) ccAbortInstance(gameinst);
  if (roominst!=NULL) ccAbortInstance(roominst);*/
  }

void post_script_cleanup() {
  // should do any post-script stuff here, like go to new room
  if (ccError) quit(ccErrorString);
  ExecutingScript copyof = scripts[num_scripts-1];
//  write_log("Instance finished.");
  if (scripts[num_scripts-1].forked)
    ccFreeInstance(scripts[num_scripts-1].inst);
  num_scripts--;
  inside_script--;

  if (num_scripts > 0)
    curscript = &scripts[num_scripts-1];
  else {
    curscript = NULL;
  }
//  if (abort_executor) user_disabled_data2=aborted_ip;

  int old_room_number = displayed_room;

  // run the queued post-script actions
  for (int ii = 0; ii < copyof.numPostScriptActions; ii++) {
    int thisData = copyof.postScriptActionData[ii];

    switch (copyof.postScriptActions[ii]) {
    case ePSANewRoom:
      // only change rooms when all scripts are done
      if (num_scripts == 0) {
        new_room(thisData, playerchar);
        // don't allow any pending room scripts from the old room
        // in run_another to be executed
        return;
      }
      else
        curscript->queue_action(ePSANewRoom, thisData, "NewRoom");
      break;
    case ePSAInvScreen:
      invscreen();
      break;
    case ePSARestoreGame:
      cancel_all_scripts();
      load_game_and_print_error(thisData);
      return;
    case ePSARestoreGameDialog:
      restore_game_dialog();
      return;
    case ePSARunAGSGame:
      cancel_all_scripts();
      load_new_game = thisData;
      return;
    case ePSARunDialog:
      do_conversation(thisData);
      break;
    case ePSARestartGame:
      cancel_all_scripts();
      restart_game();
      return;
    case ePSASaveGame:
      save_game(thisData, copyof.postScriptSaveSlotDescription[ii]);
      break;
    case ePSASaveGameDialog:
      save_game_dialog();
      break;
    default:
      quitprintf("undefined post script action found: %d", copyof.postScriptActions[ii]);
    }
    // if the room changed in a conversation, for example, abort
    if (old_room_number != displayed_room) {
      return;
    }
  }


  int jj;
  for (jj = 0; jj < copyof.numanother; jj++) {
    old_room_number = displayed_room;
    char runnext[40];
    strcpy(runnext,copyof.script_run_another[jj]);
    copyof.script_run_another[jj][0]=0;
    if (runnext[0]=='#')
      run_text_script_2iparam(gameinst,&runnext[1],copyof.run_another_p1[jj],copyof.run_another_p2[jj]);
    else if (runnext[0]=='!')
      run_text_script_iparam(gameinst,&runnext[1],copyof.run_another_p1[jj]);
    else if (runnext[0]=='|')
      run_text_script(roominst,&runnext[1]);
    else if (runnext[0]=='%')
      run_text_script_2iparam(roominst, &runnext[1], copyof.run_another_p1[jj], copyof.run_another_p2[jj]);
    else if (runnext[0]=='$') {
      run_text_script_iparam(roominst,&runnext[1],copyof.run_another_p1[jj]);
      play.roomscript_finished = 1;
    }
    else
      run_text_script(gameinst,runnext);

    // if they've changed rooms, cancel any further pending scripts
    if ((displayed_room != old_room_number) || (load_new_game))
      break;
  }
  copyof.numanother = 0;

}

void quit_with_script_error(const char *functionName)
{
  quitprintf("%sError running function '%s':\n%s", (ccErrorIsUserError ? "!" : ""), functionName, ccErrorString);
}

void _do_run_script_func_cant_block(ccInstance *forkedinst, NonBlockingScriptFunction* funcToRun, bool *hasTheFunc) {
  if (!hasTheFunc[0])
    return;

  no_blocking_functions++;
  int result;

  if (funcToRun->numParameters == 0)
    result = ccCallInstance(forkedinst, (char*)funcToRun->functionName, 0);
  else if (funcToRun->numParameters == 1)
    result = ccCallInstance(forkedinst, (char*)funcToRun->functionName, 1, funcToRun->param1);
  else if (funcToRun->numParameters == 2)
    result = ccCallInstance(forkedinst, (char*)funcToRun->functionName, 2, funcToRun->param1, funcToRun->param2);
  else
    quit("_do_run_script_func_cant_block called with too many parameters");

  if (result == -2) {
    // the function doens't exist, so don't try and run it again
    hasTheFunc[0] = false;
  }
  else if ((result != 0) && (result != 100)) {
    quit_with_script_error(funcToRun->functionName);
  }
  else
  {
    funcToRun->atLeastOneImplementationExists = true;
  }
  // this might be nested, so don't disrupt blocked scripts
  ccErrorString[0] = 0;
  ccError = 0;
  no_blocking_functions--;
}

void run_function_on_non_blocking_thread(NonBlockingScriptFunction* funcToRun) {

  update_script_mouse_coords();

  int room_changes_was = play.room_changes;
  funcToRun->atLeastOneImplementationExists = false;

  // run modules
  // modules need a forkedinst for this to work
  for (int kk = 0; kk < numScriptModules; kk++) {
    _do_run_script_func_cant_block(moduleInstFork[kk], funcToRun, &funcToRun->moduleHasFunction[kk]);

    if (room_changes_was != play.room_changes)
      return;
  }

  _do_run_script_func_cant_block(gameinstFork, funcToRun, &funcToRun->globalScriptHasFunction);

  if (room_changes_was != play.room_changes)
    return;

  _do_run_script_func_cant_block(roominstFork, funcToRun, &funcToRun->roomHasFunction);
}

int run_script_function_if_exist(ccInstance*sci,char*tsname,int numParam, int iparam, int iparam2, int iparam3) {
  int oldRestoreCount = gameHasBeenRestored;
  // First, save the current ccError state
  // This is necessary because we might be attempting
  // to run Script B, while Script A is still running in the
  // background.
  // If CallInstance here has an error, it would otherwise
  // also abort Script A because ccError is a global variable.
  int cachedCcError = ccError;
  ccError = 0;

  int toret = prepare_text_script(sci,&tsname);
  if (toret) {
    ccError = cachedCcError;
    return -18;
  }

  // Clear the error message
  ccErrorString[0] = 0;

  if (numParam == 0) 
    toret = ccCallInstance(curscript->inst,tsname,numParam);
  else if (numParam == 1)
    toret = ccCallInstance(curscript->inst,tsname,numParam, iparam);
  else if (numParam == 2)
    toret = ccCallInstance(curscript->inst,tsname,numParam,iparam, iparam2);
  else if (numParam == 3)
    toret = ccCallInstance(curscript->inst,tsname,numParam,iparam, iparam2, iparam3);
  else
    quit("Too many parameters to run_script_function_if_exist");

  // 100 is if Aborted (eg. because we are LoadAGSGame'ing)
  if ((toret != 0) && (toret != -2) && (toret != 100)) {
    quit_with_script_error(tsname);
  }

  post_script_cleanup_stack++;

  if (post_script_cleanup_stack > 50)
    quitprintf("!post_script_cleanup call stack exceeded: possible recursive function call? running %s", tsname);

  post_script_cleanup();

  post_script_cleanup_stack--;

  // restore cached error state
  ccError = cachedCcError;

  // if the game has been restored, ensure that any further scripts are not run
  if ((oldRestoreCount != gameHasBeenRestored) && (eventClaimed == EVENT_INPROGRESS))
    eventClaimed = EVENT_CLAIMED;

  return toret;
}

int run_text_script(ccInstance*sci,char*tsname) {
  if (strcmp(tsname, REP_EXEC_NAME) == 0) {
    // run module rep_execs
    int room_changes_was = play.room_changes;
    int restore_game_count_was = gameHasBeenRestored;

    for (int kk = 0; kk < numScriptModules; kk++) {
      if (moduleRepExecAddr[kk] != NULL)
        run_script_function_if_exist(moduleInst[kk], tsname, 0, 0, 0);

      if ((room_changes_was != play.room_changes) ||
          (restore_game_count_was != gameHasBeenRestored))
        return 0;
    }
  }

  int toret = run_script_function_if_exist(sci, tsname, 0, 0, 0);
  if ((toret == -18) && (sci == roominst)) {
    // functions in room script must exist
    quitprintf("prepare_script: error %d (%s) trying to run '%s'   (Room %d)",toret,ccErrorString,tsname, displayed_room);
  }
  return toret;
}

int run_claimable_event(char *tsname, bool includeRoom, int numParams, int param1, int param2, bool *eventWasClaimed) {
  *eventWasClaimed = true;
  // Run the room script function, and if it is not claimed,
  // then run the main one
  // We need to remember the eventClaimed variable's state, in case
  // this is a nested event
  int eventClaimedOldValue = eventClaimed;
  eventClaimed = EVENT_INPROGRESS;
  int toret;

  if (includeRoom) {
    toret = run_script_function_if_exist(roominst, tsname, numParams, param1, param2);

    if (eventClaimed == EVENT_CLAIMED) {
      eventClaimed = eventClaimedOldValue;
      return toret;
    }
  }

  // run script modules
  for (int kk = 0; kk < numScriptModules; kk++) {
    toret = run_script_function_if_exist(moduleInst[kk], tsname, numParams, param1, param2);

    if (eventClaimed == EVENT_CLAIMED) {
      eventClaimed = eventClaimedOldValue;
      return toret;
    }
  }

  eventClaimed = eventClaimedOldValue;
  *eventWasClaimed = false;
  return 0;
}

int run_text_script_iparam(ccInstance*sci,char*tsname,int iparam) {
  if ((strcmp(tsname, "on_key_press") == 0) || (strcmp(tsname, "on_mouse_click") == 0)) {
    bool eventWasClaimed;
    int toret = run_claimable_event(tsname, true, 1, iparam, 0, &eventWasClaimed);

    if (eventWasClaimed)
      return toret;
  }

  return run_script_function_if_exist(sci, tsname, 1, iparam, 0);
}

int run_text_script_2iparam(ccInstance*sci,char*tsname,int iparam,int param2) {
  if (strcmp(tsname, "on_event") == 0) {
    bool eventWasClaimed;
    int toret = run_claimable_event(tsname, true, 2, iparam, param2, &eventWasClaimed);

    if (eventWasClaimed)
      return toret;
  }

  // response to a button click, better update guis
  if (ac_strnicmp(tsname, "interface_click", 15) == 0)
    guis_need_update = 1;

  int toret = run_script_function_if_exist(sci, tsname, 2, iparam, param2);

  // tsname is no longer valid, because run_script_function_if_exist might
  // have restored a save game and freed the memory. Therefore don't 
  // attempt any strcmp's here
  tsname = NULL;

  return toret;
}



// ============================================================================
// SCREEN / BITMAPS
// ============================================================================

int GetMaxScreenHeight () {
  int maxhit = BASEHEIGHT;
  if ((maxhit == 200) || (maxhit == 400))
  {
    // uh ... BASEHEIGHT depends on Native Coordinates setting so be careful
    if ((usetup.want_letterbox) && (thisroom.height > maxhit)) 
      maxhit = divide_down_coordinate(multiply_up_coordinate(maxhit) + get_fixed_pixel_size(40));
  }
  return maxhit;
}

block fix_bitmap_size(block todubl) {
  int oldw=BMP_W(todubl), oldh=BMP_H(todubl);
  int newWidth = multiply_up_coordinate(thisroom.width);
  int newHeight = multiply_up_coordinate(thisroom.height);

  if ((oldw == newWidth) && (oldh == newHeight))
    return todubl;

//  block tempb=alw_create_bitmap(scrnwid,scrnhit);
  block tempb=alw_create_bitmap_ex(alw_bitmap_color_depth(todubl), newWidth, newHeight);
  alw_set_clip_rect(tempb,0,0,BMP_W(tempb)-1,BMP_H(tempb)-1);
  alw_set_clip_state(tempb, TRUE);
  alw_set_clip_rect(todubl,0,0,oldw-1,oldh-1);
  alw_set_clip_state(todubl, TRUE);
  alw_clear_bitmap(tempb);
  alw_stretch_blit(todubl,tempb,0,0,oldw,oldh,0,0,BMP_W(tempb),BMP_H(tempb));
  alw_destroy_bitmap(todubl); todubl=tempb;
  return todubl;
}


//#define _get_script_data_stack_size() (256*sizeof(int)+((int*)&scrpt[10*4])[0]+((int*)&scrpt[12*4])[0])
//#define _get_script_data_stack_size(instac) ((int*)instac->code)[10]
void current_fade_out_effect () {
  if (platform->RunPluginHooks(AGSE_TRANSITIONOUT, 0))
    return;

  // get the screen transition type
  int theTransition = play.fade_effect;
  // was a temporary transition selected? if so, use it
  if (play.next_screen_transition >= 0)
    theTransition = play.next_screen_transition;

  if ((theTransition == FADE_INSTANT) || (play.screen_tint >= 0)) {
    if (!play.keep_screen_during_instant_transition)
      wsetpalette(0,255,alw_black_palette);
  }
  else if (theTransition == FADE_NORMAL)
  {
    my_fade_out(5);
  }
  else if (theTransition == FADE_BOXOUT) 
  {
    gfxDriver->BoxOutEffect(true, get_fixed_pixel_size(16), 1000 / GetGameSpeed());
    play.screen_is_faded_out = 1;
  }
  else 
  {
    alw_get_palette(old_palette);
    temp_virtual = alw_create_bitmap_ex(alw_bitmap_color_depth(abuf),BMP_W(virtual_screen),BMP_H(virtual_screen));
    //alw_blit(abuf,temp_virtual,0,0,0,0,BMP_W(abuf),BMP_H(abuf));
    gfxDriver->GetCopyOfScreenIntoBitmap(temp_virtual);
  }
}



// ============================================================================
// ROOMS
// ============================================================================

void save_room_data_segment () {
  if (croom->tsdatasize > 0)
    free(croom->tsdata);
  croom->tsdata = NULL;
  croom->tsdatasize = roominst->globaldatasize;
  if (croom->tsdatasize > 0) {
    croom->tsdata=(char*)malloc(croom->tsdatasize+10);
    ccFlattenGlobalData (roominst);
    memcpy(croom->tsdata,&roominst->globaldata[0],croom->tsdatasize);
    ccUnFlattenGlobalData (roominst);
  }

}

void unload_old_room() {
  int ff;

  // if switching games on restore, don't do this
  if (displayed_room < 0)
    return;

  platform->WriteDebugString("Unloading room %d", displayed_room);

  current_fade_out_effect();

  alw_clear_bitmap(abuf);
  for (ff=0;ff<croom->numobj;ff++)
    objs[ff].moving = 0;

  if (!play.ambient_sounds_persist) {
    for (ff = 1; ff < MAX_SOUND_CHANNELS; ff++)
      StopAmbientSound(ff);
  }

  cancel_all_scripts();
  numevents = 0;  // cancel any pending room events

  if (roomBackgroundBmp != NULL)
  {
    gfxDriver->DestroyDDB(roomBackgroundBmp);
    roomBackgroundBmp = NULL;
  }

  if (croom==NULL) ;
  else if (roominst!=NULL) {
    save_room_data_segment();
    ccFreeInstance(roominstFork);
    ccFreeInstance(roominst);
    roominstFork = NULL;
    roominst=NULL;
  }
  else croom->tsdatasize=0;
  memset(&play.walkable_areas_on[0],1,MAX_WALK_AREAS+1);
  play.bg_frame=0;
  play.bg_frame_locked=0;
  play.offsets_locked=0;
  remove_screen_overlay(-1);
  if (raw_saved_screen != NULL) {
    wfreeblock(raw_saved_screen);
    raw_saved_screen = NULL;
  }
  for (ff = 0; ff < MAX_BSCENE; ff++)
    play.raw_modified[ff] = 0;
  for (ff = 0; ff < thisroom.numLocalVars; ff++)
    croom->interactionVariableValues[ff] = thisroom.localvars[ff].value;

  // wipe the character cache when we change rooms
  for (ff = 0; ff < game.numcharacters; ff++) {
    if (charcache[ff].inUse) {
      alw_destroy_bitmap (charcache[ff].image);
      charcache[ff].image = NULL;
      charcache[ff].inUse = 0;
    }
    // ensure that any half-moves (eg. with scaled movement) are stopped
    charextra[ff].xwas = INVALID_X;
  }

  play.swap_portrait_lastchar = -1;

  for (ff = 0; ff < croom->numobj; ff++) {
    // un-export the object's script object
    if (objectScriptObjNames[ff][0] == 0)
      continue;
    
    ccRemoveExternalSymbol(objectScriptObjNames[ff]);
  }

  for (ff = 0; ff < MAX_HOTSPOTS; ff++) {
    if (thisroom.hotspotScriptNames[ff][0] == 0)
      continue;

    ccRemoveExternalSymbol(thisroom.hotspotScriptNames[ff]);
  }

  // clear the object cache
  for (ff = 0; ff < MAX_INIT_SPR; ff++) {
    if (objcache[ff].image != NULL) {
      alw_destroy_bitmap (objcache[ff].image);
      objcache[ff].image = NULL;
    }
  }
  // clear the actsps buffers to save memory, since the
  // objects/characters involved probably aren't on the
  // new screen. this also ensures all cached data is flushed
  for (ff = 0; ff < MAX_INIT_SPR + game.numcharacters; ff++) {
    if (actsps[ff] != NULL)
      alw_destroy_bitmap(actsps[ff]);
    actsps[ff] = NULL;

    if (actspsbmp[ff] != NULL)
      gfxDriver->DestroyDDB(actspsbmp[ff]);
    actspsbmp[ff] = NULL;

    if (actspswb[ff] != NULL)
      alw_destroy_bitmap(actspswb[ff]);
    actspswb[ff] = NULL;

    if (actspswbbmp[ff] != NULL)
      gfxDriver->DestroyDDB(actspswbbmp[ff]);
    actspswbbmp[ff] = NULL;

    actspswbcache[ff].valid = 0;
  }

  // if Hide Player Character was ticked, restore it to visible
  if (play.temporarily_turned_off_character >= 0) {
    game.chars[play.temporarily_turned_off_character].on = 1;
    play.temporarily_turned_off_character = -1;
  }

}



// ============================================================================
// WALKABLES
// ============================================================================

void redo_walkable_areas() {

  // since this is an 8-bit memory bitmap, we can just use direct 
  // memory access
  if ((!alw_is_linear_bitmap(thisroom.walls)) || (alw_bitmap_color_depth(thisroom.walls) != 8))
    quit("Walkable areas bitmap not linear");

  alw_blit(walkareabackup, thisroom.walls, 0, 0, 0, 0, BMP_W(thisroom.walls), BMP_H(thisroom.walls));

  int hh,ww;
  for (hh=0;hh<BMP_H(walkareabackup);hh++) {
    for (ww=0;ww<BMP_W(walkareabackup);ww++) {
//      if (play.walkable_areas_on[alw__getpixel(thisroom.walls,ww,hh)]==0)
      if (play.walkable_areas_on[BMP_LINE(thisroom.walls)[hh][ww]]==0)
        alw__putpixel(thisroom.walls,ww,hh,0);
    }
  }

}



// ============================================================================
// SCRIPTS / EVENTS
// ============================================================================

// runs the global script on_event fnuction
void run_on_event (int evtype, int wparam) {
  if (inside_script) {
    curscript->run_another("#on_event", evtype, wparam);
  }
  else
    run_text_script_2iparam(gameinst,"on_event", evtype, wparam);
}



// ============================================================================
// WALK BEHINDS
// ============================================================================

void update_walk_behind_images()
{
  int ee, rr;
  int bpp = (alw_bitmap_color_depth(thisroom.ebscene[play.bg_frame]) + 7) / 8;
  ALW_BITMAP *wbbmp;
  for (ee = 1; ee < MAX_OBJ; ee++)
  {
    update_polled_stuff();
    
    if (walkBehindRight[ee] > 0)
    {
      wbbmp = alw_create_bitmap_ex(alw_bitmap_color_depth(thisroom.ebscene[play.bg_frame]), 
                               (walkBehindRight[ee] - walkBehindLeft[ee]) + 1,
                               (walkBehindBottom[ee] - walkBehindTop[ee]) + 1);
      alw_clear_to_color(wbbmp, alw_bitmap_mask_color(wbbmp));
      int yy, startX = walkBehindLeft[ee], startY = walkBehindTop[ee];
      for (rr = startX; rr <= walkBehindRight[ee]; rr++)
      {
        for (yy = startY; yy <= walkBehindBottom[ee]; yy++)
        {
          if (BMP_LINE(thisroom.object)[yy][rr] == ee)
          {
            for (int ii = 0; ii < bpp; ii++)
              BMP_LINE(wbbmp)[yy - startY][(rr - startX) * bpp + ii] = BMP_LINE(thisroom.ebscene[play.bg_frame])[yy][rr * bpp + ii];
          }
        }
      }

      update_polled_stuff();

      if (walkBehindBitmap[ee] != NULL)
      {
        gfxDriver->DestroyDDB(walkBehindBitmap[ee]);
      }
      walkBehindBitmap[ee] = gfxDriver->CreateDDBFromBitmap(wbbmp, false);
      alw_destroy_bitmap(wbbmp);
    }
  }

  walkBehindsCachedForBgNum = play.bg_frame;
}

void recache_walk_behinds () {
  if (walkBehindExists) {
    free (walkBehindExists);
    free (walkBehindStartY);
    free (walkBehindEndY);
  }

  walkBehindExists = (char*)malloc (BMP_W(thisroom.object));
  walkBehindStartY = (int*)malloc (BMP_W(thisroom.object) * sizeof(int));
  walkBehindEndY = (int*)malloc (BMP_W(thisroom.object) * sizeof(int));
  noWalkBehindsAtAll = 1;

  int ee,rr,tmm;
  const int NO_WALK_BEHIND = 100000;
  for (ee = 0; ee < MAX_OBJ; ee++)
  {
    walkBehindLeft[ee] = NO_WALK_BEHIND;
    walkBehindTop[ee] = NO_WALK_BEHIND;
    walkBehindRight[ee] = 0;
    walkBehindBottom[ee] = 0;

    if (walkBehindBitmap[ee] != NULL)
    {
      gfxDriver->DestroyDDB(walkBehindBitmap[ee]);
      walkBehindBitmap[ee] = NULL;
    }
  }

  update_polled_stuff();

  // since this is an 8-bit memory bitmap, we can just use direct 
  // memory access
  if ((!alw_is_linear_bitmap(thisroom.object)) || (alw_bitmap_color_depth(thisroom.object) != 8))
    quit("Walk behinds bitmap not linear");

  for (ee=0;ee<BMP_W(thisroom.object);ee++) {
    walkBehindExists[ee] = 0;
    for (rr=0;rr<BMP_H(thisroom.object);rr++) {
      tmm = BMP_LINE(thisroom.object)[rr][ee];
      //tmm = alw__getpixel(thisroom.object,ee,rr);
      if ((tmm >= 1) && (tmm < MAX_OBJ)) {
        if (!walkBehindExists[ee]) {
          walkBehindStartY[ee] = rr;
          walkBehindExists[ee] = tmm;
          noWalkBehindsAtAll = 0;
        }
        walkBehindEndY[ee] = rr + 1;  // +1 to allow bottom line of screen to work

        if (ee < walkBehindLeft[tmm]) walkBehindLeft[tmm] = ee;
        if (rr < walkBehindTop[tmm]) walkBehindTop[tmm] = rr;
        if (ee > walkBehindRight[tmm]) walkBehindRight[tmm] = ee;
        if (rr > walkBehindBottom[tmm]) walkBehindBottom[tmm] = rr;
      }
    }
  }

  if (walkBehindMethod == DrawAsSeparateSprite)
  {
    update_walk_behind_images();
  }
}

void check_viewport_coords() 
{
  if (offsetx<0) offsetx=0;
  if (offsety<0) offsety=0;

  int roomWidth = multiply_up_coordinate(thisroom.width);
  int roomHeight = multiply_up_coordinate(thisroom.height);
  if (offsetx + scrnwid > roomWidth)
    offsetx = roomWidth - scrnwid;
  if (offsety + scrnhit > roomHeight)
    offsety = roomHeight - scrnhit;
}

void update_viewport()
{
  if ((thisroom.width > BASEWIDTH) || (thisroom.height > BASEHEIGHT)) {
    if (play.offsets_locked == 0) {
      offsetx = multiply_up_coordinate(playerchar->x) - scrnwid/2;
      offsety = multiply_up_coordinate(playerchar->y) - scrnhit/2;
    }
    check_viewport_coords();
  }
  else {
    offsetx=0;
    offsety=0;
  }
}

int get_walkable_area_pixel(int x, int y)
{
  return alw_getpixel(thisroom.walls, convert_to_low_res(x), convert_to_low_res(y));
}



// ============================================================================
// ROOMS
// ============================================================================

void convert_room_coordinates_to_low_res(roomstruct *rstruc)
{
    int f;
	  for (f = 0; f < rstruc->numsprs; f++)
	  {
		  rstruc->sprs[f].x /= 2;
		  rstruc->sprs[f].y /= 2;
      if (rstruc->objbaseline[f] > 0)
		  {
			  rstruc->objbaseline[f] /= 2;
		  }
	  }

	  for (f = 0; f < rstruc->numhotspots; f++)
	  {
		  rstruc->hswalkto[f].x /= 2;
		  rstruc->hswalkto[f].y /= 2;
	  }

	  for (f = 0; f < rstruc->numobj; f++)
	  {
		  rstruc->objyval[f] /= 2;
	  }

	  rstruc->left /= 2;
	  rstruc->top /= 2;
	  rstruc->bottom /= 2;
	  rstruc->right /= 2;
	  rstruc->width /= 2;
	  rstruc->height /= 2;
}

void load_new_room_screen() 
{
  if (usetup.want_letterbox) {
    int abscreen=0;
    if (abuf==alw_screen) abscreen=1;
    else if (abuf==virtual_screen) abscreen=2;
    // if this is a 640x480 room and we're in letterbox mode, full-screen it
    int newScreenHeight = final_scrn_hit;
    if (multiply_up_coordinate(thisroom.height) < final_scrn_hit) {
      clear_letterbox_borders();
      newScreenHeight = get_fixed_pixel_size(200);
    }

    if (newScreenHeight == BMP_H(_sub_screen))
    {
      alw_screen = _sub_screen;
    }
    else if (BMP_W(_sub_screen) != final_scrn_wid)
    {
      int subBitmapWidth = BMP_W(_sub_screen);
      alw_destroy_bitmap(_sub_screen);
      _sub_screen = alw_create_sub_bitmap(_old_screen, BMP_W(_old_screen) / 2 - subBitmapWidth / 2, BMP_H(_old_screen) / 2 - newScreenHeight / 2, subBitmapWidth, newScreenHeight);
      alw_screen = _sub_screen;
    }
    else
    {
      alw_screen = _old_screen;
    }

    scrnhit = BMP_H(alw_screen);
    vesa_yres = scrnhit;

#if defined(WINDOWS_VERSION) || defined(LINUX_VERSION) || defined(MAC_VERSION)
    filter->SetMouseArea(0,0, scrnwid-1, vesa_yres-1);
#endif

    if (BMP_H(virtual_screen) != scrnhit) {
      int cdepth=alw_bitmap_color_depth(virtual_screen);
      wfreeblock(virtual_screen);
      virtual_screen=alw_create_bitmap_ex(cdepth,scrnwid,scrnhit);
      alw_clear_bitmap(virtual_screen);
      gfxDriver->SetMemoryBackBuffer(virtual_screen);
      //      ignore_mouseoff_bitmap = virtual_screen;
    }

    gfxDriver->SetRenderOffset(get_screen_x_adjustment(virtual_screen), get_screen_y_adjustment(virtual_screen));

    if (abscreen==1) abuf=alw_screen;
    else if (abscreen==2) abuf=virtual_screen;

    update_polled_stuff();
  }
  // update the script viewport height
  scsystem.viewport_height = divide_down_coordinate(scrnhit);
}

void load_new_room_objects( CharacterInfo* forchar, int newnum ) 
{
  // setup objects
  if (forchar != NULL) {
    // if not restoring a game, always reset this room
    troom.beenhere=0;  
    troom.tsdatasize=0;
    memset(&troom.hotspot_enabled[0],1,MAX_HOTSPOTS);
    memset(&troom.region_enabled[0], 1, MAX_REGIONS);
  }
  if ((newnum>=0) & (newnum<MAX_ROOMS))
    croom=&roomstats[newnum];
  else croom=&troom;

  if (croom->beenhere > 0) {
    // if we've been here before, save the Times Run information
    // since we will overwrite the actual NewInteraction structs
    // (cos they have pointers and this might have been loaded from
    // a save game)
    if (thisroom.roomScripts == NULL)
    {
      thisroom.intrRoom->copy_timesrun_from (&croom->intrRoom);
      for (int cc=0;cc < MAX_HOTSPOTS;cc++)
        thisroom.intrHotspot[cc]->copy_timesrun_from (&croom->intrHotspot[cc]);
      for (int cc=0;cc < MAX_INIT_SPR;cc++)
        thisroom.intrObject[cc]->copy_timesrun_from (&croom->intrObject[cc]);
      for (int cc=0;cc < MAX_REGIONS;cc++)
        thisroom.intrRegion[cc]->copy_timesrun_from (&croom->intrRegion[cc]);
    }
  }
  if (croom->beenhere==0) {
    croom->numobj=thisroom.numsprs;
    croom->tsdatasize=0;
    for (int cc=0;cc<croom->numobj;cc++) {
      croom->obj[cc].x=thisroom.sprs[cc].x;
      croom->obj[cc].y=thisroom.sprs[cc].y;

      if (thisroom.wasversion <= 26)
        croom->obj[cc].y += divide_down_coordinate(spriteheight[thisroom.sprs[cc].sprnum]);

      croom->obj[cc].num=thisroom.sprs[cc].sprnum;
      croom->obj[cc].on=thisroom.sprs[cc].on;
      croom->obj[cc].view=-1;
      croom->obj[cc].loop=0;
      croom->obj[cc].frame=0;
      croom->obj[cc].wait=0;
      croom->obj[cc].transparent=0;
      croom->obj[cc].moving=-1;
      croom->obj[cc].flags = thisroom.objectFlags[cc];
      croom->obj[cc].baseline=-1;
      croom->obj[cc].last_zoom = 100;
      croom->obj[cc].last_width = 0;
      croom->obj[cc].last_height = 0;
      croom->obj[cc].blocking_width = 0;
      croom->obj[cc].blocking_height = 0;
      if (thisroom.objbaseline[cc]>=0)
        //        croom->obj[cc].baseoffs=thisroom.objbaseline[cc]-thisroom.sprs[cc].y;
        croom->obj[cc].baseline=thisroom.objbaseline[cc];
    }
    memcpy(&croom->walkbehind_base[0],&thisroom.objyval[0],sizeof(short)*MAX_OBJ);
    for (int cc=0;cc<MAX_FLAGS;cc++) croom->flagstates[cc]=0;

    /*    // we copy these structs for the Score column to work
    croom->misccond=thisroom.misccond;
    for (cc=0;cc<MAX_HOTSPOTS;cc++)
    croom->hscond[cc]=thisroom.hscond[cc];
    for (cc=0;cc<MAX_INIT_SPR;cc++)
    croom->objcond[cc]=thisroom.objcond[cc];*/

    for (int cc=0;cc < MAX_HOTSPOTS;cc++) {
      croom->hotspot_enabled[cc] = 1;
    }
    for (int cc = 0; cc < MAX_REGIONS; cc++) {
      croom->region_enabled[cc] = 1;
    }
    croom->beenhere=1;
    in_new_room=2;
  }
  else {
    // We have been here before
    for (ff = 0; ff < thisroom.numLocalVars; ff++)
      thisroom.localvars[ff].value = croom->interactionVariableValues[ff];
  }

  update_polled_stuff();

  if (thisroom.roomScripts == NULL)
  {
    // copy interactions from room file into our temporary struct
    croom->intrRoom = thisroom.intrRoom[0];
    for (int cc=0;cc<MAX_HOTSPOTS;cc++)
      croom->intrHotspot[cc] = thisroom.intrHotspot[cc][0];
    for (int cc=0;cc<MAX_INIT_SPR;cc++)
      croom->intrObject[cc] = thisroom.intrObject[cc][0];
    for (int cc=0;cc<MAX_REGIONS;cc++)
      croom->intrRegion[cc] = thisroom.intrRegion[cc][0];
  }

  objs=&croom->obj[0];

  for (int cc = 0; cc < MAX_INIT_SPR; cc++) {
    scrObj[cc].obj = &croom->obj[cc];
    objectScriptObjNames[cc][0] = 0;
  }

  for (int cc = 0; cc < croom->numobj; cc++) {
    // export the object's script object
    if (thisroom.objectscriptnames[cc][0] == 0)
      continue;

    if (thisroom.wasversion >= 26) 
    {
      strcpy(objectScriptObjNames[cc], thisroom.objectscriptnames[cc]);
    }
    else
    {
      sprintf(objectScriptObjNames[cc], "o%s", thisroom.objectscriptnames[cc]);
      ac_strlwr(objectScriptObjNames[cc]);
      if (objectScriptObjNames[cc][1] != 0)
        objectScriptObjNames[cc][1] = toupper(objectScriptObjNames[cc][1]);
    }

    ccAddExternalSymbol(objectScriptObjNames[cc], &scrObj[cc]);
  }

  for (int cc = 0; cc < MAX_HOTSPOTS; cc++) {
    if (thisroom.hotspotScriptNames[cc][0] == 0)
      continue;

    ccAddExternalSymbol(thisroom.hotspotScriptNames[cc], &scrHotspot[cc]);
  }
}

void load_new_room_player_pos( CharacterInfo* forchar ) 
{
  play.entered_edge = -1;

  if ((new_room_x != SCR_NO_VALUE) && (forchar != NULL))
  {
    forchar->x = new_room_x;
    forchar->y = new_room_y;
  }
  new_room_x = SCR_NO_VALUE;

  if ((new_room_pos>0) & (forchar!=NULL)) {
    if (new_room_pos>=4000) {
      play.entered_edge = 3;
      forchar->y = thisroom.top + get_fixed_pixel_size(1);
      forchar->x=new_room_pos%1000;
      if (forchar->x==0) forchar->x=thisroom.width/2;
      if (forchar->x <= thisroom.left)
        forchar->x = thisroom.left + 3;
      if (forchar->x >= thisroom.right)
        forchar->x = thisroom.right - 3;
      forchar->loop=0;
    }
    else if (new_room_pos>=3000) {
      play.entered_edge = 2;
      forchar->y = thisroom.bottom - get_fixed_pixel_size(1);
      forchar->x=new_room_pos%1000;
      if (forchar->x==0) forchar->x=thisroom.width/2;
      if (forchar->x <= thisroom.left)
        forchar->x = thisroom.left + 3;
      if (forchar->x >= thisroom.right)
        forchar->x = thisroom.right - 3;
      forchar->loop=3;
    }
    else if (new_room_pos>=2000) {
      play.entered_edge = 1;
      forchar->x = thisroom.right - get_fixed_pixel_size(1);
      forchar->y=new_room_pos%1000;
      if (forchar->y==0) forchar->y=thisroom.height/2;
      if (forchar->y <= thisroom.top)
        forchar->y = thisroom.top + 3;
      if (forchar->y >= thisroom.bottom)
        forchar->y = thisroom.bottom - 3;
      forchar->loop=1;
    }
    else if (new_room_pos>=1000) {
      play.entered_edge = 0;
      forchar->x = thisroom.left + get_fixed_pixel_size(1);
      forchar->y=new_room_pos%1000;
      if (forchar->y==0) forchar->y=thisroom.height/2;
      if (forchar->y <= thisroom.top)
        forchar->y = thisroom.top + 3;
      if (forchar->y >= thisroom.bottom)
        forchar->y = thisroom.bottom - 3;
      forchar->loop=2;
    }
    // if starts on un-walkable area
    if (get_walkable_area_pixel(forchar->x, forchar->y) == 0) {
      if (new_room_pos>=3000) { // bottom or top of screen
        int tryleft=forchar->x - 1,tryright=forchar->x + 1;
        while (1) {
          if (get_walkable_area_pixel(tryleft, forchar->y) > 0) {
            forchar->x=tryleft; break; }
          if (get_walkable_area_pixel(tryright, forchar->y) > 0) {
            forchar->x=tryright; break; }
          int nowhere=0;
          if (tryleft>thisroom.left) { tryleft--; nowhere++; }
          if (tryright<thisroom.right) { tryright++; nowhere++; }
          if (nowhere==0) break;  // no place to go, so leave him
        }
      }
      else if (new_room_pos>=1000) { // left or right
        int tryleft=forchar->y - 1,tryright=forchar->y + 1;
        while (1) {
          if (get_walkable_area_pixel(forchar->x, tryleft) > 0) {
            forchar->y=tryleft; break; }
          if (get_walkable_area_pixel(forchar->x, tryright) > 0) {
            forchar->y=tryright; break; }
          int nowhere=0;
          if (tryleft>thisroom.top) { tryleft--; nowhere++; }
          if (tryright<thisroom.bottom) { tryright++; nowhere++; }
          if (nowhere==0) break;  // no place to go, so leave him
        }
      }
    }
    new_room_pos=0;
  }
  if (forchar!=NULL) {
    play.entered_at_x=forchar->x;
    play.entered_at_y=forchar->y;
    if (forchar->x >= thisroom.right)
      play.entered_edge = 1;
    else if (forchar->x <= thisroom.left)
      play.entered_edge = 0;
    else if (forchar->y >= thisroom.bottom)
      play.entered_edge = 2;
    else if (forchar->y <= thisroom.top)
      play.entered_edge = 3;
  }
  /*  if ((playerchar->x > thisroom.width) | (playerchar->y > thisroom.height))
  quit("!NewRoomEx: x/y co-ordinates are invalid");*/
}


// forchar = playerchar on NewRoom, or NULL if restore saved game
void load_new_room(int newnum,CharacterInfo*forchar) {

  platform->WriteDebugString("Loading room %d", newnum);

  done_es_error = 0;
  play.room_changes ++;
  alw_set_color_depth(8);
  displayed_room=newnum;

  // DETERMINE ROOM FILE NAME ***********************************************************************************************

  char rmfile[20];
  sprintf(rmfile,"room%d.crm",newnum);
  if (newnum == 0) {
    // support both room0.crm and intro.crm
    FILE *inpu = clibfopen(rmfile, "rb");
    if (inpu == NULL)
      strcpy(rmfile, "intro.crm");
    else
      fclose(inpu);
  }

  // RESET BITMAPS ***********************************************************************************************

  // reset these back, because they might have been changed.
  if (thisroom.object!=NULL)
    alw_destroy_bitmap(thisroom.object);
  thisroom.object=alw_create_bitmap(320,200);

  if (thisroom.ebscene[0]!=NULL)
    alw_destroy_bitmap(thisroom.ebscene[0]);
  thisroom.ebscene[0] = alw_create_bitmap(320,200);


  // POLL ***********************************************************************************************

  update_polled_stuff();

  // LOAD ROOM DATA ***********************************************************************************************

  // load the room from disk
  set_eip(200);
  thisroom.gameId = NO_GAME_ID_IN_ROOM_FILE;
  load_room(rmfile, &thisroom, (game.default_resolution > 2));

  if ((thisroom.gameId != NO_GAME_ID_IN_ROOM_FILE) &&
      (thisroom.gameId != game.uniqueid)) {
    quitprintf("!Unable to load '%s'. This room file is assigned to a different game.", rmfile);
  }

  // ROOM COORDS ***********************************************************************************************

  if ((game.default_resolution > 2) && (game.options[OPT_NATIVECOORDINATES] == 0))
  {
    convert_room_coordinates_to_low_res(&thisroom);
  }


  // POLL ***********************************************************************************************

  update_polled_stuff();

  // PROPS ***********************************************************************************************

  set_eip(201);
/*  // apparently, doing this stops volume spiking between tracks
  if (thisroom.options[ST_TUNE]>0) {
    stopmusic();
    delay(100);
  }*/

  play.room_width = thisroom.width;
  play.room_height = thisroom.height;
  play.anim_background_speed = thisroom.bscene_anim_speed;
  play.bg_anim_delay = play.anim_background_speed;

  // PALETTE ***********************************************************************************************

  // do the palette
  for (int cc=0;cc<256;cc++) {
    if (game.paluses[cc]==PAL_BACKGROUND)
      palette[cc]=thisroom.pal[cc];
    else {
      // copy the gamewide colours into the room palette
      for (int dd = 0; dd < thisroom.num_bscenes; dd++)
        thisroom.bpalettes[dd][cc] = palette[cc];
    }
  }

  if ((alw_bitmap_color_depth(thisroom.ebscene[0]) == 8) &&
      (final_col_dep > 8))
    alw_select_palette(palette);

  for (int cc=0;cc<thisroom.num_bscenes;cc++) {
    update_polled_stuff();
  #ifdef USE_15BIT_FIX
    // convert down scenes from 16 to 15-bit if necessary
    if ((final_col_dep != game.color_depth*8) &&
        (alw_bitmap_color_depth(thisroom.ebscene[cc]) == game.color_depth * 8)) {
      block oldblock = thisroom.ebscene[cc];
      thisroom.ebscene[cc] = convert_16_to_15(oldblock);
      wfreeblock(oldblock);
    }
    else if ((alw_bitmap_color_depth (thisroom.ebscene[cc]) == 16) && (convert_16bit_bgr == 1))
      thisroom.ebscene[cc] = convert_16_to_16bgr (thisroom.ebscene[cc]);
  #endif

    thisroom.ebscene[cc] = gfxDriver->ConvertBitmapToSupportedColourDepth(thisroom.ebscene[cc]);
  }

  if ((alw_bitmap_color_depth(thisroom.ebscene[0]) == 8) &&
      (final_col_dep > 8))
    alw_unselect_palette();


  // POLL ***********************************************************************************************
  update_polled_stuff();


  // SCREEN ***********************************************************************************************

  set_eip(202);
  load_new_room_screen();


  // MOUSE ***********************************************************************************************

  SetMouseBounds (0,0,0,0);

  // MISC ***********************************************************************************************

  set_eip(203);
  in_new_room=1;

  // WALKABLE AREAS ***********************************************************************************************

  // walkable_areas_temp is used by the pathfinder to generate a
  // copy of the walkable areas - allocate it here to save time later
  if (walkable_areas_temp != NULL)
    wfreeblock (walkable_areas_temp);
  walkable_areas_temp = alw_create_bitmap_ex (8, BMP_W(thisroom.walls), BMP_H(thisroom.walls));

  // Make a backup copy of the walkable areas prior to
  // any RemoveWalkableArea commands
  if (walkareabackup!=NULL) wfreeblock(walkareabackup);
  walkareabackup=alw_create_bitmap(BMP_W(thisroom.walls),BMP_H(thisroom.walls));

  // WALL SCREEN ***********************************************************************************************

  set_eip(204);
  // copy the walls screen
  alw_blit(thisroom.walls,walkareabackup,0,0,0,0,BMP_W(thisroom.walls),BMP_H(thisroom.walls));
  update_polled_stuff();
  redo_walkable_areas();
  // fix walk-behinds to current screen resolution
  thisroom.object = fix_bitmap_size(thisroom.object);
  update_polled_stuff();

  alw_set_color_depth(final_col_dep);
  // convert backgrounds to current res
  if (thisroom.resolution != get_fixed_pixel_size(1)) {
    for (int cc=0;cc<thisroom.num_bscenes;cc++)
      thisroom.ebscene[cc] = fix_bitmap_size(thisroom.ebscene[cc]);
  }

  if ((BMP_W(thisroom.ebscene[0]) < scrnwid) ||
      (BMP_H(thisroom.ebscene[0]) < scrnhit))
  {
    quitprintf("!The background scene for this room is smaller than the game resolution. If you have recently changed " 
               "the game resolution, you will need to re-import the background for this room. (Room: %d, BG Size: %d x %d)",
               newnum, BMP_W(thisroom.ebscene[0]), BMP_H(thisroom.ebscene[0]));
  }

  recache_walk_behinds();

  // OBJECTS ***********************************************************************************************

  set_eip(205);
  load_new_room_objects(forchar, newnum);

  // SHARED PALETTE ***********************************************************************************************

  set_eip(206);
/*  THIS IS DONE IN THE EDITOR NOW
  thisroom.ebpalShared[0] = 1;
  for (dd = 1; dd < thisroom.num_bscenes; dd++) {
    if (memcmp (&thisroom.bpalettes[dd][0], &palette[0], sizeof(color) * 256) == 0)
      thisroom.ebpalShared[dd] = 1;
    else
      thisroom.ebpalShared[dd] = 0;
  }
  // only make the first frame shared if the last is
  if (thisroom.ebpalShared[thisroom.num_bscenes - 1] == 0)
    thisroom.ebpalShared[0] = 0;*/

  update_polled_stuff();


  // FIXPALETTE ***********************************************************************************************

  set_eip(210);
  if (IS_ANTIALIAS_SPRITES) {
    // sometimes the palette has corrupt entries, which crash
    // the create_rgb_table call
    // so, fix them
    for (ff = 0; ff < 256; ff++) {
      if (palette[ff].r > 63)
        palette[ff].r = 63;
      if (palette[ff].g > 63)
        palette[ff].g = 63;
      if (palette[ff].b > 63)
        palette[ff].b = 63;
    }
    alw_create_rgb_table (&rgb_table, palette, NULL);
    alw_set_rgb_map(&rgb_table);
  }

  // CHARACTER ***********************************************************************************************

  set_eip(211);
  if (forchar!=NULL) {
    // if it's not a Restore Game

    // if a following character is still waiting to come into the
    // previous room, force it out so that the timer resets
    for (ff = 0; ff < game.numcharacters; ff++) {
      if ((game.chars[ff].following >= 0) && (game.chars[ff].room < 0)) {
        if ((game.chars[ff].following == game.playercharacter) &&
            (forchar->prevroom == newnum))
          // the player went back to the previous room, so make sure
          // the following character is still there
          game.chars[ff].room = newnum;
        else
          game.chars[ff].room = game.chars[game.chars[ff].following].room;
      }
    }

    offsetx=0;
    offsety=0;
    forchar->prevroom=forchar->room;
    forchar->room=newnum;
    // only stop moving if it's a new room, not a restore game
    for (int cc=0;cc<game.numcharacters;cc++)
      StopMoving(cc);

  }

  // POLL ***********************************************************************************************


  update_polled_stuff();

  // COMPILED SCRIPTS ***********************************************************************************************

  roominst=NULL;
  if (debug_flags & DBG_NOSCRIPT) ;
  else if (thisroom.compiled_script!=NULL) {
    compile_room_script();
    if (croom->tsdatasize>0) {
      if (croom->tsdatasize != roominst->globaldatasize)
        quit("room script data segment size has changed");
      memcpy(&roominst->globaldata[0],croom->tsdata,croom->tsdatasize);
      ccUnFlattenGlobalData (roominst);
      }
    }

  // ENTERED POS ***********************************************************************************************

  set_eip(207);
  load_new_room_player_pos(forchar);


  // MUSIC ***********************************************************************************************

  if (thisroom.options[ST_TUNE]>0)
    PlayMusicResetQueue(thisroom.options[ST_TUNE]);

  // CURSOR ***********************************************************************************************

  set_eip(208);
  if (forchar!=NULL) {
    if (thisroom.options[ST_MANDISABLED]==0) { forchar->on=1;
      enable_cursor_mode(0); }
    else {
      forchar->on=0;
      disable_cursor_mode(0);
      // remember which character we turned off, in case they
      // use SetPlyaerChracter within this room (so we re-enable
      // the correct character when leaving the room)
      play.temporarily_turned_off_character = game.playercharacter;
    }
    if (forchar->flags & CHF_FIXVIEW) ;
    else if (thisroom.options[ST_MANVIEW]==0) forchar->view=forchar->defview;
    else forchar->view=thisroom.options[ST_MANVIEW]-1;
    forchar->frame=0;   // make him standing
    }
  alw_set_color_map(NULL);

  // UPDATE STUFF ***********************************************************************************************

  set_eip(209);
  update_polled_stuff();
  generate_light_table();
  update_music_volume();
  update_viewport();

  // SCREEN ***********************************************************************************************

  set_eip(212);
  invalidate_screen();
  for (int cc=0;cc<croom->numobj;cc++) {
    if (objs[cc].on == 2)
      MergeObject(cc);
    }

  // FLAGS ***********************************************************************************************

  new_room_flags=0;
  play.gscript_timer=-1;  // avoid screw-ups with changing screens
  play.player_on_region = 0;

  // CLEAR INPUT ***********************************************************************************************

  // trash any input which they might have done while it was loading
  while (ac_kbhit()) { if (ac_getch()==0) ac_getch(); }
  while (ac_mgetbutton()!=NONE) ;

  // RESET PALETTE ***********************************************************************************************

  // no fade in, so set the palette immediately in case of 256-col sprites
  if (game.color_depth > 1)
    setpal();

  // DONE ***********************************************************************************************

  set_eip(220);
  update_polled_stuff();
  DEBUG_CONSOLE("Now in room %d", displayed_room);
  guis_need_update = 1;

  // PLUGINS ***********************************************************************************************

  platform->RunPluginHooks(AGSE_ENTERROOM, displayed_room);


//  MoveToWalkableArea(game.playercharacter);
//  MSS_CHECK_ALL_BLOCKS;
  }




void run_room_event(int id) {
  evblockbasename="room";
  
  if (thisroom.roomScripts != NULL)
  {
    run_interaction_script(thisroom.roomScripts, id);
  }
  else
  {
    run_interaction_event (&croom->intrRoom, id);
  }
}

// new_room: changes the current room number, and loads the new room from disk
void new_room(int newnum,CharacterInfo*forchar) {
  EndSkippingUntilCharStops();
  
  platform->WriteDebugString("Room change requested to room %d", newnum);

  update_polled_stuff();

  // we are currently running Leaves Screen scripts
  in_leaves_screen = newnum;

  // player leaves screen event
  run_room_event(8);
  // Run the global OnRoomLeave event
  run_on_event (GE_LEAVE_ROOM, displayed_room);

  platform->RunPluginHooks(AGSE_LEAVEROOM, displayed_room);

  // update the new room number if it has been altered by OnLeave scripts
  newnum = in_leaves_screen;
  in_leaves_screen = -1;

  if ((playerchar->following >= 0) &&
      (game.chars[playerchar->following].room != newnum)) {
    // the player character is following another character,
    // who is not in the new room. therefore, abort the follow
    playerchar->following = -1;
  }
  update_polled_stuff();

  // change rooms
  unload_old_room();

  update_polled_stuff();

  load_new_room(newnum,forchar);
}



// ============================================================================
// EVENTS?
// ============================================================================

// animation player start

void main_loop_until(int untilwhat,int udata,int mousestuff) {
  play.disabled_user_interface++;
  guis_need_update = 1;
  // Only change the mouse cursor if it hasn't been specifically changed first
  // (or if it's speech, always change it)
  if (((cur_cursor == cur_mode) || (untilwhat == UNTIL_NOOVERLAY)) &&
      (cur_mode != CURS_WAIT))
    set_mouse_cursor(CURS_WAIT);

  restrict_until=untilwhat;
  user_disabled_data=udata;
  return;
}

// event list functions
void setevent(int evtyp,int ev1,int ev2,int ev3) {
  event[numevents].type=evtyp;
  event[numevents].data1=ev1;
  event[numevents].data2=ev2;
  event[numevents].data3=ev3;
  event[numevents].player=game.playercharacter;
  numevents++;
  if (numevents>=MAXEVENTS) quit("too many events posted");
}

void draw_screen_callback()
{
  construct_virtual_screen(false);

  render_black_borders(get_screen_x_adjustment(virtual_screen), get_screen_y_adjustment(virtual_screen));
}

IDriverDependantBitmap* prepare_screen_for_transition_in()
{
  if (temp_virtual == NULL)
    quit("Crossfade: buffer is null attempting transition");

  temp_virtual = gfxDriver->ConvertBitmapToSupportedColourDepth(temp_virtual);
  if (BMP_H(temp_virtual) < scrnhit)
  {
    block enlargedBuffer = alw_create_bitmap_ex(alw_bitmap_color_depth(temp_virtual), BMP_W(temp_virtual), scrnhit);
    alw_blit(temp_virtual, enlargedBuffer, 0, 0, 0, (scrnhit - BMP_H(temp_virtual)) / 2, BMP_W(temp_virtual), BMP_H(temp_virtual));
    alw_destroy_bitmap(temp_virtual);
    temp_virtual = enlargedBuffer;
  }
  else if (BMP_H(temp_virtual) > scrnhit)
  {
    block clippedBuffer = alw_create_bitmap_ex(alw_bitmap_color_depth(temp_virtual), BMP_W(temp_virtual), scrnhit);
    alw_blit(temp_virtual, clippedBuffer, 0, (BMP_H(temp_virtual) - scrnhit) / 2, 0, 0, BMP_W(temp_virtual), BMP_H(temp_virtual));
    alw_destroy_bitmap(temp_virtual);
    temp_virtual = clippedBuffer;
  }
  alw_acquire_bitmap(temp_virtual);
  IDriverDependantBitmap *ddb = gfxDriver->CreateDDBFromBitmap(temp_virtual, false);
  return ddb;
}

void process_event(EventHappened*evp) {
  if (evp->type==EV_TEXTSCRIPT) {
    int resl=0; ccError=0;
    if (evp->data2 > -1000) {
      if (inside_script) {
        char nameToExec[50];
        sprintf (nameToExec, "!%s", tsnames[evp->data1]);
        curscript->run_another(nameToExec, evp->data2, 0);
      }
      else
        resl=run_text_script_iparam(gameinst,tsnames[evp->data1],evp->data2);
    }
    else {
      if (inside_script)
        curscript->run_another (tsnames[evp->data1], 0, 0);
      else
        resl=run_text_script(gameinst,tsnames[evp->data1]);
    }
//    Display("relt: %d err:%d",resl,scErrorNo);
  }
  else if (evp->type==EV_NEWROOM) {
    NewRoom(evp->data1);
  }
  else if (evp->type==EV_RUNEVBLOCK) {
    NewInteraction*evpt=NULL;
    InteractionScripts *scriptPtr = NULL;
    const char *oldbasename = evblockbasename;
    int   oldblocknum = evblocknum;

    if (evp->data1==EVB_HOTSPOT) {

      if (thisroom.hotspotScripts != NULL)
        scriptPtr = thisroom.hotspotScripts[evp->data2];
      else
        evpt=&croom->intrHotspot[evp->data2];

      evblockbasename="hotspot%d";
      evblocknum=evp->data2;
      //platform->WriteDebugString("Running hotspot interaction for hotspot %d, event %d", evp->data2, evp->data3);
    }
    else if (evp->data1==EVB_ROOM) {

      if (thisroom.roomScripts != NULL)
        scriptPtr = thisroom.roomScripts;
      else
        evpt=&croom->intrRoom;
      
      evblockbasename="room";
      if (evp->data3 == 5) {
        in_enters_screen ++;
        run_on_event (GE_ENTER_ROOM, displayed_room);
        
      }
      //platform->WriteDebugString("Running room interaction, event %d", evp->data3);
    }

    if (scriptPtr != NULL)
    {
      run_interaction_script(scriptPtr, evp->data3);
    }
    else if (evpt != NULL)
    {
      run_interaction_event(evpt,evp->data3);
    }
    else
      quit("process_event: RunEvBlock: unknown evb type");

    evblockbasename = oldbasename;
    evblocknum = oldblocknum;

    if ((evp->data3 == 5) && (evp->data1 == EVB_ROOM))
      in_enters_screen --;
    }
  else if (evp->type==EV_FADEIN) {
    // if they change the transition type before the fadein, make
    // sure the screen doesn't freeze up
    play.screen_is_faded_out = 0;

    // determine the transition style
    int theTransition = play.fade_effect;

    if (play.next_screen_transition >= 0) {
      // a one-off transition was selected, so use it
      theTransition = play.next_screen_transition;
      play.next_screen_transition = -1;
    }

    if (platform->RunPluginHooks(AGSE_TRANSITIONIN, 0))
      return;

    if (play.fast_forward)
      return;
    
    if (((theTransition == FADE_CROSSFADE) || (theTransition == FADE_DISSOLVE)) &&
      (temp_virtual == NULL)) 
    {
      // transition type was not crossfade/dissolve when the screen faded out,
      // but it is now when the screen fades in (Eg. a save game was restored
      // with a different setting). Therefore just fade normally.
      my_fade_out(5);
      theTransition = FADE_NORMAL;
    }

    if ((theTransition == FADE_INSTANT) || (play.screen_tint >= 0))
      wsetpalette(0,255,palette);
    else if (theTransition == FADE_NORMAL)
    {
      if (gfxDriver->UsesMemoryBackBuffer())
        gfxDriver->RenderToBackBuffer();

      my_fade_in(palette,5);
    }
    else if (theTransition == FADE_BOXOUT) 
    {
      if (!gfxDriver->UsesMemoryBackBuffer())
      {
        gfxDriver->BoxOutEffect(false, get_fixed_pixel_size(16), 1000 / GetGameSpeed());
      }
      else
      {
        wsetpalette(0,255,palette);
        gfxDriver->RenderToBackBuffer();
        gfxDriver->SetMemoryBackBuffer(alw_screen);
        alw_clear_bitmap(alw_screen);
        render_to_screen(alw_screen, 0, 0);

        int boxwid = get_fixed_pixel_size(16);
        int boxhit = multiply_up_coordinate(GetMaxScreenHeight() / 20);
        while (boxwid < BMP_W(alw_screen)) {
          timerloop = 0;
          boxwid += get_fixed_pixel_size(16);
          boxhit += multiply_up_coordinate(GetMaxScreenHeight() / 20);
          int lxp = scrnwid / 2 - boxwid / 2, lyp = scrnhit / 2 - boxhit / 2;
          gfxDriver->Vsync();
          alw_blit(virtual_screen, alw_screen, lxp, lyp, lxp, lyp,
            boxwid, boxhit);
          render_to_screen(alw_screen, 0, 0);
          acaudio_update_mp3();
          while (timerloop == 0) ;
        }
        gfxDriver->SetMemoryBackBuffer(virtual_screen);
      }
      play.screen_is_faded_out = 0;
    }
    else if (theTransition == FADE_CROSSFADE) 
    {
      if (game.color_depth == 1)
        quit("!Cannot use crossfade screen transition in 256-colour games");

      IDriverDependantBitmap *ddb = prepare_screen_for_transition_in();
      
      int transparency = 254;

      while (transparency > 0) {
        timerloop=0;
        // do the crossfade
        ddb->SetTransparency(transparency);
        invalidate_screen();
        draw_screen_callback();

        if (transparency > 16)
        {
          // on last frame of fade (where transparency < 16), don't
          // draw the old screen on top
          gfxDriver->DrawSprite(0, -(BMP_H(temp_virtual) - BMP_H(virtual_screen)), ddb);
        }
        render_to_screen(alw_screen, 0, 0);
        update_polled_stuff();
        while (timerloop == 0) ;
        transparency -= 16;
      }
      alw_release_bitmap(temp_virtual);
      
      wfreeblock(temp_virtual);
      temp_virtual = NULL;
      wsetpalette(0,255,palette);
      gfxDriver->DestroyDDB(ddb);
    }
    else if (theTransition == FADE_DISSOLVE) {
      int pattern[16]={0,4,14,9,5,11,2,8,10,3,12,7,15,6,13,1};
      int aa,bb,cc,thcol=0;
      color interpal[256];

      IDriverDependantBitmap *ddb = prepare_screen_for_transition_in();
      
      for (aa=0;aa<16;aa++) {
        timerloop=0;
        // merge the palette while dithering
        if (game.color_depth == 1) 
        {
          alw_fade_interpolate(old_palette,palette,interpal,aa*4,0,255);
          wsetpalette(0,255,interpal);
        }
        // do the dissolving
        int maskCol = alw_bitmap_mask_color(temp_virtual);
        for (bb=0;bb<scrnwid;bb+=4) {
          for (cc=0;cc<scrnhit;cc+=4) {
            alw_putpixel(temp_virtual, bb+pattern[aa]/4, cc+pattern[aa]%4, maskCol);
          }
        }
        gfxDriver->UpdateDDBFromBitmap(ddb, temp_virtual, false);
        invalidate_screen();
        draw_screen_callback();
        gfxDriver->DrawSprite(0, -(BMP_H(temp_virtual) - BMP_H(virtual_screen)), ddb);
        render_to_screen(alw_screen, 0, 0);
        update_polled_stuff();
        while (timerloop == 0) ;
      }
      alw_release_bitmap(temp_virtual);
      
      wfreeblock(temp_virtual);
      temp_virtual = NULL;
      wsetpalette(0,255,palette);
      gfxDriver->DestroyDDB(ddb);
    }
    
  }
  else if (evp->type==EV_IFACECLICK)
    process_interface_click(evp->data1, evp->data2, evp->data3);
  else quit("process_event: unknown event to process");
}

void runevent_now (int evtyp, int ev1, int ev2, int ev3) {
   EventHappened evh;
   evh.type = evtyp;
   evh.data1 = ev1;
   evh.data2 = ev2;
   evh.data3 = ev3;
   evh.player = game.playercharacter;
   process_event(&evh);
}

int inside_processevent=0;
void processallevents(int numev,EventHappened*evlist) {
  int dd;

  if (inside_processevent)
    return;

  // make a copy of the events - if processing an event includes
  // a blocking function it will continue to the next game loop
  // and wipe out the event pointer we were passed
  EventHappened copyOfList[MAXEVENTS];
  memcpy(&copyOfList[0], &evlist[0], sizeof(EventHappened) * numev);

  int room_was = play.room_changes;

  inside_processevent++;

  for (dd=0;dd<numev;dd++) {

    process_event(&copyOfList[dd]);

    if (room_was != play.room_changes)
      break;  // changed room, so discard other events
  }

  inside_processevent--;
}

void update_events() {
  processallevents(numevents,&event[0]);
  numevents=0;
  }
// end event list functions



// ============================================================================
// GUI - INVENTORY
// ============================================================================

void update_inv_cursor(int invnum) {

  if ((game.options[OPT_FIXEDINVCURSOR]==0) && (invnum > 0)) {
    int cursorSprite = game.invinfo[invnum].cursorPic;
    game.mcurs[MODE_USE].pic = cursorSprite;
    // all cursor images must be pre-cached
    spriteset.precache(cursorSprite);

    if ((game.invinfo[invnum].hotx > 0) || (game.invinfo[invnum].hoty > 0)) {
      // if the hotspot was set (unfortunately 0,0 isn't a valid co-ord)
      game.mcurs[MODE_USE].hotx=game.invinfo[invnum].hotx;
      game.mcurs[MODE_USE].hoty=game.invinfo[invnum].hoty;
      }
    else {
      game.mcurs[MODE_USE].hotx = spritewidth[cursorSprite] / 2;
      game.mcurs[MODE_USE].hoty = spriteheight[cursorSprite] / 2;
      }
    }
  }



// ============================================================================
// GRAPHICS - SPRITE DRAW
// ============================================================================

void draw_sprite_support_alpha(int xpos, int ypos, block image, int slot) {

  if ((game.spriteflags[slot] & SPF_ALPHACHANNEL) && (trans_mode == 0)) 
  {
    alw_set_alpha_blender();
    alw_draw_trans_sprite(abuf, image, xpos, ypos);
  }
  else {
    put_sprite_256(xpos, ypos, image);
  }

}



// ============================================================================
// GRAPHICS - VIEW - PRECACHE
// ============================================================================

void precache_view(int view) 
{
  if (view < 0) 
    return;

  for (int i = 0; i < views[view].numLoops; i++) {
    for (int j = 0; j < views[view].loops[i].numFrames; j++)
      spriteset.precache (views[view].loops[i].frames[j].pic);
  }
}



// ============================================================================
// GUI - INTERFACE
// ============================================================================

void remove_popup_interface(int ifacenum) {
  if (ifacepopped != ifacenum) return;
  ifacepopped=-1; UnPauseGame();
  guis[ifacenum].on=0;
  if (mousey<=guis[ifacenum].popupyp)
    filter->SetMousePosition(mousex, guis[ifacenum].popupyp+2);
  if ((!IsInterfaceEnabled()) && (cur_cursor == cur_mode))
    // Only change the mouse cursor if it hasn't been specifically changed first
    set_mouse_cursor(CURS_WAIT);
  else if (IsInterfaceEnabled())
    set_default_cursor();

  if (ifacenum==mouse_on_iface) mouse_on_iface=-1;
  guis_need_update = 1;
  }

void process_interface_click(int ifce, int btn, int mbut) {
  if (btn < 0) {
    // click on GUI background
    run_text_script_2iparam(gameinst, guis[ifce].clickEventHandler, (int)&scrGui[ifce], mbut);
    return;
  }

  int btype=(guis[ifce].objrefptr[btn] >> 16) & 0x000ffff;
  int rtype=0,rdata;
  if (btype==GOBJ_BUTTON) {
    GUIButton*gbuto=(GUIButton*)guis[ifce].objs[btn];
    rtype=gbuto->leftclick;
    rdata=gbuto->lclickdata;
    }
  else if ((btype==GOBJ_SLIDER) || (btype == GOBJ_TEXTBOX) || (btype == GOBJ_LISTBOX))
    rtype = IBACT_SCRIPT;
  else quit("unknown GUI object triggered process_interface");

  if (rtype==0) ;
  else if (rtype==IBACT_SETMODE)
    set_cursor_mode(rdata);
  else if (rtype==IBACT_SCRIPT) {
    GUIObject *theObj = guis[ifce].objs[btn];
    // if the object has a special handler script then run it;
    // otherwise, run interface_click
    if ((theObj->GetNumEvents() > 0) &&
        (theObj->eventHandlers[0][0] != 0) &&
        (ccGetSymbolAddr(gameinst, theObj->eventHandlers[0]) != NULL)) {
      // control-specific event handler
      if (strchr(theObj->GetEventArgs(0), ',') != NULL)
        run_text_script_2iparam(gameinst, theObj->eventHandlers[0], (int)theObj, mbut);
      else
        run_text_script_iparam(gameinst, theObj->eventHandlers[0], (int)theObj);
    }
    else
      run_text_script_2iparam(gameinst,"interface_click",ifce,btn);
  }
}



// ============================================================================
// CONTROLS
// ============================================================================

void start_skipping_cutscene () {
  play.fast_forward = 1;
  // if a drop-down icon bar is up, remove it as it will pause the game
  if (ifacepopped>=0)
    remove_popup_interface(ifacepopped);

  // if a text message is currently displayed, remove it
  if (is_text_overlay > 0)
    remove_screen_overlay(OVER_TEXTMSG);

}

void check_skip_cutscene_keypress (int kgn) {

  if ((play.in_cutscene > 0) && (play.in_cutscene != 3)) {
    if ((kgn != 27) && ((play.in_cutscene == 1) || (play.in_cutscene == 5)))
      ;
    else
      start_skipping_cutscene();
  }

}

// check_controls: checks mouse & keyboard interface
// called from main loop.
void check_controls() {
  int numevents_was = numevents;
  set_eip(1007);
  NEXT_ITERATION();

  int aa,mongu=-1;
  // If all GUIs are off, skip the loop
  if ((game.options[OPT_DISABLEOFF]==3) && (all_buttons_disabled > 0)) ;
  else {
    // Scan for mouse-y-pos GUIs, and pop one up if appropriate
    // Also work out the mouse-over GUI while we're at it
    int ll;
    for (ll = 0; ll < game.numgui;ll++) {
      aa = play.gui_draw_order[ll];
      if (guis[aa].is_mouse_on_gui()) mongu=aa;

      if (guis[aa].popup!=POPUP_MOUSEY) continue;
      if (is_complete_overlay>0) break;  // interfaces disabled
  //    if (play.disabled_user_interface>0) break;
      if (ifacepopped==aa) continue;
      if (guis[aa].on==-1) continue;
      // Don't allow it to be popped up while skipping cutscene
      if (play.fast_forward) continue;
      
      if (mousey < guis[aa].popupyp) {
        set_mouse_cursor(CURS_ARROW);
        guis[aa].on=1; guis_need_update = 1;
        ifacepopped=aa; PauseGame();
        break;
      }
    }
  }

  mouse_on_iface=mongu;
  if ((ifacepopped>=0) && (mousey>=guis[ifacepopped].y+guis[ifacepopped].hit))
    remove_popup_interface(ifacepopped);

  // check mouse clicks on GUIs
  static int wasbutdown=0,wasongui=0;

  if ((wasbutdown>0) && (ac_misbuttondown(wasbutdown-1))) {
    for (aa=0;aa<guis[wasongui].numobjs;aa++) {
      if (guis[wasongui].objs[aa]->activated<1) continue;
      if (guis[wasongui].get_control_type(aa)!=GOBJ_SLIDER) continue;
      // GUI Slider repeatedly activates while being dragged
      guis[wasongui].objs[aa]->activated=0;
      setevent(EV_IFACECLICK, wasongui, aa, wasbutdown);
      break;
      }
    }
  else if ((wasbutdown>0) && (!ac_misbuttondown(wasbutdown-1))) {
    guis[wasongui].mouse_but_up();
    int whichbut=wasbutdown;
    wasbutdown=0;

    for (aa=0;aa<guis[wasongui].numobjs;aa++) {
      if (guis[wasongui].objs[aa]->activated<1) continue;
      guis[wasongui].objs[aa]->activated=0;
      if (!IsInterfaceEnabled()) break;

      int cttype=guis[wasongui].get_control_type(aa);
      if ((cttype == GOBJ_BUTTON) || (cttype == GOBJ_SLIDER) || (cttype == GOBJ_LISTBOX)) {
        setevent(EV_IFACECLICK, wasongui, aa, whichbut);
      }
      else if (cttype == GOBJ_INVENTORY) {
        mouse_ifacebut_xoffs=mousex-(guis[wasongui].objs[aa]->x)-guis[wasongui].x;
        mouse_ifacebut_yoffs=mousey-(guis[wasongui].objs[aa]->y)-guis[wasongui].y;
        int iit=offset_over_inv((GUIInv*)guis[wasongui].objs[aa]);
        if (iit>=0) {
          evblocknum=iit;
          play.used_inv_on = iit;
          if (game.options[OPT_HANDLEINVCLICKS]) {
            // Let the script handle the click
            // LEFTINV is 5, RIGHTINV is 6
            setevent(EV_TEXTSCRIPT,TS_MCLICK, whichbut + 4);
          }
          else if (whichbut==2)  // right-click is always Look
            run_event_block_inv(iit, 0);
          else if (cur_mode == MODE_HAND)
            SetActiveInventory(iit);
          else
            RunInventoryInteraction (iit, cur_mode);
          evblocknum=-1;
        }
      }
      else quit("clicked on unknown control type");
      if (guis[wasongui].popup==POPUP_MOUSEY)
        remove_popup_interface(wasongui);
      break;
    }

    run_on_event(GE_GUI_MOUSEUP, wasongui);
  }

  aa=ac_mgetbutton();
  if (aa>NONE) {
    if ((play.in_cutscene == 3) || (play.in_cutscene == 4))
      start_skipping_cutscene();
    if ((play.in_cutscene == 5) && (aa == RIGHT))
      start_skipping_cutscene();

    if (play.fast_forward) { }
    else if ((play.wait_counter > 0) && (play.key_skip_wait > 1))
      play.wait_counter = -1;
    else if (is_text_overlay > 0) {
      if (play.cant_skip_speech & SKIP_MOUSECLICK)
        remove_screen_overlay(OVER_TEXTMSG);
    }
    else if (!IsInterfaceEnabled()) ;  // blocking cutscene, ignore mouse
    else if (platform->RunPluginHooks(AGSE_MOUSECLICK, aa+1)) {
      // plugin took the click
      DEBUG_CONSOLE("Plugin handled mouse button %d", aa+1);
    }
    else if (mongu>=0) {
      if (wasbutdown==0) {
        DEBUG_CONSOLE("Mouse click over GUI %d", mongu);
        guis[mongu].mouse_but_down();
        // run GUI click handler if not on any control
        if ((guis[mongu].mousedownon < 0) && (guis[mongu].clickEventHandler[0] != 0))
          setevent(EV_IFACECLICK, mongu, -1, aa + 1);

        run_on_event(GE_GUI_MOUSEDOWN, mongu);
      }
      wasongui=mongu;
      wasbutdown=aa+1;
    }
    else setevent(EV_TEXTSCRIPT,TS_MCLICK,aa+1);
//    else run_text_script_iparam(gameinst,"on_mouse_click",aa+1);
  }
  aa = check_mouse_wheel();
  if (aa < 0)
    setevent (EV_TEXTSCRIPT, TS_MCLICK, 9);
  else if (aa > 0)
    setevent (EV_TEXTSCRIPT, TS_MCLICK, 8);

  // check keypresses
  if (ac_kbhit()) {
    // in case they press the finish-recording button, make sure we know
    int was_playing = play.playback;

    int kgn = ac_getch();
    if (kgn==0) kgn=ac_getch()+300;
//    if (kgn==367) restart_game();
//    if (kgn==2) Display("numover: %d character movesped: %d, animspd: %d",numscreenover,playerchar->walkspeed,playerchar->animspeed);
//    if (kgn==2) CreateTextOverlay(50,60,170,FONT_SPEECH,14,"This is a test screen overlay which shouldn't disappear");
//    if (kgn==2) { Display("Crashing"); strcpy(NULL, NULL); }
//    if (kgn == 2) FaceLocation (game.playercharacter, playerchar->x + 1, playerchar->y);
    //if (kgn == 2) SetCharacterIdle (game.playercharacter, 5, 0);
    //if (kgn == 2) Display("Some forËign text");
    //if (kgn == 2) do_conversation(5);

    if (kgn == play.replay_hotkey) {
      // start/stop recording
      if (play.recording)
        stop_recording();
      else if ((play.playback) || (was_playing))
        ;  // do nothing (we got the replay of the stop key)
      else
        replay_start_this_time = 1;
    }

    check_skip_cutscene_keypress (kgn);

    if (play.fast_forward) { }
    else if (platform->RunPluginHooks(AGSE_KEYPRESS, kgn)) {
      // plugin took the keypress
      DEBUG_CONSOLE("Keypress code %d taken by plugin", kgn);
    }
    else if ((kgn == '`') && (play.debug_mode > 0)) {
      // debug console
      display_console = !display_console;
    }
    else if ((is_text_overlay > 0) &&
             (play.cant_skip_speech & SKIP_KEYPRESS) &&
             (kgn != 434)) {
      // 434 = F12, allow through for screenshot of text
      // (though atm with one script at a time that won't work)
      // only allow a key to remove the overlay if the icon bar isn't up
      if (IsGamePaused() == 0) {
        // check if it requires a specific keypress
        if ((play.skip_speech_specific_key > 0) &&
          (kgn != play.skip_speech_specific_key)) { }
        else
          remove_screen_overlay(OVER_TEXTMSG);
      }
    }
    else if ((play.wait_counter > 0) && (play.key_skip_wait > 0)) {
      play.wait_counter = -1;
      DEBUG_CONSOLE("Keypress code %d ignored - in Wait", kgn);
    }
    else if ((kgn == 5) && (display_fps == 2)) {
      // if --fps paramter is used, Ctrl+E will max out frame rate
      SetGameSpeed(1000);
      display_fps = 2;
    }
    else if ((kgn == 4) && (play.debug_mode > 0)) {
      // ctrl+D - show info
      char infobuf[900];
      int ff;
	  // MACPORT FIX 9/6/5: added last %s
      sprintf(infobuf,"In room %d %s[Player at %d, %d (view %d, loop %d, frame %d)%s%s%s",
        displayed_room, (noWalkBehindsAtAll ? "(has no walk-behinds)" : ""), playerchar->x,playerchar->y,
        playerchar->view + 1, playerchar->loop,playerchar->frame,
        (IsGamePaused() == 0) ? "" : "[Game paused.",
        (play.ground_level_areas_disabled == 0) ? "" : "[Ground areas disabled.",
        (IsInterfaceEnabled() == 0) ? "[Game in Wait state" : "");
      for (ff=0;ff<croom->numobj;ff++) {
        if (ff >= 8) break; // buffer not big enough for more than 7
        sprintf(&infobuf[strlen(infobuf)],
          "[Object %d: (%d,%d) size (%d x %d) on:%d moving:%s animating:%d slot:%d trnsp:%d clkble:%d",
          ff, objs[ff].x, objs[ff].y,
          (spriteset[objs[ff].num] != NULL) ? spritewidth[objs[ff].num] : 0,
          (spriteset[objs[ff].num] != NULL) ? spriteheight[objs[ff].num] : 0,
          objs[ff].on,
          (objs[ff].moving > 0) ? "yes" : "no", objs[ff].cycling,
          objs[ff].num, objs[ff].transparent,
          ((objs[ff].flags & OBJF_NOINTERACT) != 0) ? 0 : 1 );
      }
      Display(infobuf);
      int chd = game.playercharacter;
      char bigbuffer[STD_BUFFER_SIZE] = "CHARACTERS IN THIS ROOM:[";
      for (ff = 0; ff < game.numcharacters; ff++) {
        if (game.chars[ff].room != displayed_room) continue;
        if (strlen(bigbuffer) > 430) {
          strcat(bigbuffer, "and more...");
          Display(bigbuffer);
          strcpy(bigbuffer, "CHARACTERS IN THIS ROOM (cont'd):[");
        }
        chd = ff;
        sprintf(&bigbuffer[strlen(bigbuffer)], 
          "%s (view/loop/frm:%d,%d,%d  x/y/z:%d,%d,%d  idleview:%d,time:%d,left:%d walk:%d anim:%d follow:%d flags:%X wait:%d zoom:%d)[",
          game.chars[chd].scrname, game.chars[chd].view+1, game.chars[chd].loop, game.chars[chd].frame,
          game.chars[chd].x, game.chars[chd].y, game.chars[chd].z,
          game.chars[chd].idleview, game.chars[chd].idletime, game.chars[chd].idleleft,
          game.chars[chd].walking, game.chars[chd].animating, game.chars[chd].following,
          game.chars[chd].flags, game.chars[chd].wait, charextra[chd].zoom);
      }
      Display(bigbuffer);

    }
/*    else if (kgn == 21) {
      play.debug_mode++;
      script_debug(5,0);
      play.debug_mode--;
    }*/
    else if ((kgn == 22) && (play.wait_counter < 1) && (is_text_overlay == 0) && (restrict_until == 0)) {
      // make sure we can't interrupt a Wait()
      // and desync the music to cutscene
      play.debug_mode++;
      script_debug (1,0);
      play.debug_mode--;
    }
    else if (inside_script) {
      // Don't queue up another keypress if it can't be run instantly
      DEBUG_CONSOLE("Keypress %d ignored (game blocked)", kgn);
    }
    else {
      int keywasprocessed = 0;
      // determine if a GUI Text Box should steal the click
      // it should do if a displayable character (32-255) is
      // pressed, but exclude control characters (<32) and
      // extended keys (eg. up/down arrow; 256+)
      if ( ((kgn >= 32) && (kgn != '[') && (kgn < 256)) || (kgn == 13) || (kgn == 8) ) {
        int uu,ww;
        for (uu=0;uu<game.numgui;uu++) {
          if (guis[uu].on < 1) continue;
          for (ww=0;ww<guis[uu].numobjs;ww++) {
            // not a text box, ignore it
            if ((guis[uu].objrefptr[ww] >> 16)!=GOBJ_TEXTBOX)
              continue;
            GUITextBox*guitex=(GUITextBox*)guis[uu].objs[ww];
            // if the text box is disabled, it cannot except keypresses
            if ((guitex->IsDisabled()) || (!guitex->IsVisible()))
              continue;
            guitex->KeyPress(kgn);
            if (guitex->activated) {
              guitex->activated = 0;
              setevent(EV_IFACECLICK, uu, ww, 1);
            }
            keywasprocessed = 1;
          }
        }
      }
      if (!keywasprocessed) {
        if ((kgn>='a') & (kgn<='z')) kgn-=32;
        DEBUG_CONSOLE("Running on_key_press keycode %d", kgn);
        setevent(EV_TEXTSCRIPT,TS_KEYPRESS,kgn);
      }
    }
//    run_text_script_iparam(gameinst,"on_key_press",kgn);
  }

  if ((IsInterfaceEnabled()) && (IsGamePaused() == 0) &&
      (in_new_room == 0) && (new_room_was == 0)) {
    // Only allow walking off edges if not in wait mode, and
    // if not in Player Enters Screen (allow walking in from off-screen)
    int edgesActivated[4] = {0, 0, 0, 0};
    // Only do it if nothing else has happened (eg. mouseclick)
    if ((numevents == numevents_was) &&
        ((play.ground_level_areas_disabled & GLED_INTERACTION) == 0)) {

      if (playerchar->x <= thisroom.left)
        edgesActivated[0] = 1;
      else if (playerchar->x >= thisroom.right)
        edgesActivated[1] = 1;
      if (playerchar->y >= thisroom.bottom)
        edgesActivated[2] = 1;
      else if (playerchar->y <= thisroom.top)
        edgesActivated[3] = 1;

      if ((play.entered_edge >= 0) && (play.entered_edge <= 3)) {
        // once the player is no longer outside the edge, forget the stored edge
        if (edgesActivated[play.entered_edge] == 0)
          play.entered_edge = -10;
        // if we are walking in from off-screen, don't activate edges
        else
          edgesActivated[play.entered_edge] = 0;
      }

      for (int ii = 0; ii < 4; ii++) {
        if (edgesActivated[ii])
          setevent(EV_RUNEVBLOCK, EVB_ROOM, 0, ii);
      }
    }
  }
  set_eip(1008);

}  // end check_controls



// ============================================================================
// WALKABLE AREAS
// ============================================================================

// return the walkable area at the character's feet, taking into account
// that he might just be off the edge of one
int get_walkable_area_at_location(int xx, int yy) {

  int onarea = get_walkable_area_pixel(xx, yy);

  if (onarea < 0) {
    // the character has walked off the edge of the screen, so stop them
    // jumping up to full size when leaving
    if (xx >= thisroom.width)
      onarea = get_walkable_area_pixel(thisroom.width-1, yy);
    else if (xx < 0)
      onarea = get_walkable_area_pixel(0, yy);
    else if (yy >= thisroom.height)
      onarea = get_walkable_area_pixel(xx, thisroom.height - 1);
    else if (yy < 0)
      onarea = get_walkable_area_pixel(xx, 1);
  }
  if (onarea==0) {
    // the path finder sometimes slightly goes into non-walkable areas;
    // so check for scaling in adjacent pixels
    const int TRYGAP=2;
    onarea = get_walkable_area_pixel(xx + TRYGAP, yy);
    if (onarea<=0)
      onarea = get_walkable_area_pixel(xx - TRYGAP, yy);
    if (onarea<=0)
      onarea = get_walkable_area_pixel(xx, yy + TRYGAP);
    if (onarea<=0)
      onarea = get_walkable_area_pixel(xx, yy - TRYGAP);
    if (onarea < 0)
      onarea = 0;
  }

  return onarea;
}

int get_walkable_area_at_character (int charnum) {
  CharacterInfo *chin = &game.chars[charnum];
  return get_walkable_area_at_location(chin->x, chin->y);
}



// ============================================================================
// DIALOG - LIP SYNC
// ============================================================================

// Calculate which frame of the loop to use for this character of
// speech
int GetLipSyncFrame (char *curtex, int *stroffs) {
  /*char *frameletters[MAXLIPSYNCFRAMES] =
    {"./,/ ", "A", "O", "F/V", "D/N/G/L/R", "B/P/M",
     "Y/H/K/Q/C", "I/T/E/X/th", "U/W", "S/Z/J/ch", NULL,
     NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};*/

  int bestfit_len = 0, bestfit = game.default_lipsync_frame;
  for (int aa = 0; aa < MAXLIPSYNCFRAMES; aa++) {
    char *tptr = game.lipSyncFrameLetters[aa];
    while (tptr[0] != 0) {
      int lenthisbit = strlen(tptr);
      if (strchr(tptr, '/'))
        lenthisbit = strchr(tptr, '/') - tptr;
      
      if ((ac_strnicmp (curtex, tptr, lenthisbit) == 0) && (lenthisbit > bestfit_len)) {
        bestfit = aa;
        bestfit_len = lenthisbit;
      }
      tptr += lenthisbit;
      while (tptr[0] == '/')
        tptr++;
    }
  }
  // If it's an unknown character, use the default frame
  if (bestfit_len == 0)
    bestfit_len = 1;
  *stroffs += bestfit_len;
  return bestfit;
}


int update_lip_sync(int talkview, int talkloop, int *talkframeptr) {
  int talkframe = talkframeptr[0];
  int talkwait = 0;

  // lip-sync speech
  char *nowsaying = &text_lips_text[text_lips_offset];
  // if it's an apostraphe, skip it (we'll, I'll, etc)
  if (nowsaying[0] == '\'') {
    text_lips_offset++;
    nowsaying++;
  }

  if (text_lips_offset >= (int)strlen(text_lips_text))
    talkframe = 0;
  else {
    talkframe = GetLipSyncFrame (nowsaying, &text_lips_offset);
    if (talkframe >= views[talkview].loops[talkloop].numFrames)
      talkframe = 0;
  }

  talkwait = loops_per_character + views[talkview].loops[talkloop].frames[talkframe].speed;

  talkframeptr[0] = talkframe;
  return talkwait;
}



// ============================================================================
// OBJECT
// ============================================================================

void get_object_blocking_rect(int objid, int *x1, int *y1, int *width, int *y2) {
  RoomObject *tehobj = &objs[objid];
  int cwidth, fromx;

  if (tehobj->blocking_width < 1)
    cwidth = divide_down_coordinate(tehobj->last_width) - 4;
  else
    cwidth = tehobj->blocking_width;

  fromx = tehobj->x + (divide_down_coordinate(tehobj->last_width) / 2) - cwidth / 2;
  if (fromx < 0) {
    cwidth += fromx;
    fromx = 0;
  }
  if (fromx + cwidth >= convert_back_to_high_res(BMP_W(walkable_areas_temp)))
    cwidth = convert_back_to_high_res(BMP_W(walkable_areas_temp)) - fromx;

  if (x1)
    *x1 = fromx;
  if (width)
    *width = cwidth;
  if (y1) {
    if (tehobj->blocking_height > 0)
      *y1 = tehobj->y - tehobj->blocking_height / 2;
    else
      *y1 = tehobj->y - 2;
  }
  if (y2) {
    if (tehobj->blocking_height > 0)
      *y2 = tehobj->y + tehobj->blocking_height / 2;
    else
      *y2 = tehobj->y + 3;
  }
}



// ============================================================================
// CHARACTER
// ============================================================================

int wantMoveNow (int chnum, CharacterInfo *chi) {
  // check most likely case first
  if ((charextra[chnum].zoom == 100) || ((chi->flags & CHF_SCALEMOVESPEED) == 0))
    return 1;

  // the % checks don't work when the counter is negative, so once
  // it wraps round, correct it
  while (chi->walkwaitcounter < 0) {
    chi->walkwaitcounter += 12000;
  }

  // scaling 170-200%, move 175% speed
  if (charextra[chnum].zoom >= 170) {
    if ((chi->walkwaitcounter % 4) >= 1)
      return 2;
    else
      return 1;
  }
  // scaling 140-170%, move 150% speed
  else if (charextra[chnum].zoom >= 140) {
    if ((chi->walkwaitcounter % 2) == 1)
      return 2;
    else
      return 1;
  }
  // scaling 115-140%, move 125% speed
  else if (charextra[chnum].zoom >= 115) {
    if ((chi->walkwaitcounter % 4) >= 3)
      return 2;
    else
      return 1;
  }
  // scaling 80-120%, normal speed
  else if (charextra[chnum].zoom >= 80)
    return 1;
  // scaling 60-80%, move 75% speed
  if (charextra[chnum].zoom >= 60) {
    if ((chi->walkwaitcounter % 4) >= 1)
      return 1;
  }
  // scaling 30-60%, move 50% speed
  else if (charextra[chnum].zoom >= 30) {
    if ((chi->walkwaitcounter % 2) == 1)
      return -1;
    else if (charextra[chnum].xwas != INVALID_X) {
      // move the second half of the movement to make it smoother
      chi->x = charextra[chnum].xwas;
      chi->y = charextra[chnum].ywas;
      charextra[chnum].xwas = INVALID_X;
    }
  }
  // scaling 0-30%, move 25% speed
  else {
    if ((chi->walkwaitcounter % 4) >= 3)
      return -1;
    if (((chi->walkwaitcounter % 4) == 1) && (charextra[chnum].xwas != INVALID_X)) {
      // move the second half of the movement to make it smoother
      chi->x = charextra[chnum].xwas;
      chi->y = charextra[chnum].ywas;
      charextra[chnum].xwas = INVALID_X;
    }

  }

  return 0;
}



// ============================================================================
// GRAPHICS - VIEW FRAME
// ============================================================================

// draws a view frame, flipped if appropriate
void DrawViewFrame(block target, ViewFrame *vframe, int x, int y) {
  if (vframe->flags & VFLG_FLIPSPRITE)
    alw_draw_sprite_h_flip(target, spriteset[vframe->pic], x, y);
  else
    alw_draw_sprite(target, spriteset[vframe->pic], x, y);
}



// ============================================================================
// UPDATE STUFF (CHARCS, OBJS, SCRIPTS, etc)
// ============================================================================

void update_stuff_timers() 
{
  if (play.gscript_timer > 0) play.gscript_timer--;
  for (int aa=0;aa<MAX_TIMERS;aa++) {
    if (play.script_timers[aa] > 1) play.script_timers[aa]--;
  }
}

void update_stuff_view_cycle_graphics() 
{
  // update graphics for object if cycling view
  for (int aa=0;aa<croom->numobj;aa++) {
    if (objs[aa].on != 1) continue;
    if (objs[aa].moving>0) {
      do_movelist_move(&objs[aa].moving,&objs[aa].x,&objs[aa].y);
    }
    if (objs[aa].cycling==0) continue;
    if (objs[aa].view<0) continue;
    if (objs[aa].wait>0) { objs[aa].wait--; continue; }

    if (objs[aa].cycling >= ANIM_BACKWARDS) {
      // animate backwards
      objs[aa].frame--;
      if (objs[aa].frame < 0) {
        if ((objs[aa].loop > 0) && 
          (views[objs[aa].view].loops[objs[aa].loop - 1].RunNextLoop())) 
        {
          // If it's a Go-to-next-loop on the previous one, then go back
          objs[aa].loop --;
          objs[aa].frame = views[objs[aa].view].loops[objs[aa].loop].numFrames - 1;
        }
        else if (objs[aa].cycling % ANIM_BACKWARDS == ANIM_ONCE) {
          // leave it on the first frame
          objs[aa].cycling = 0;
          objs[aa].frame = 0;
        }
        else { // repeating animation
          objs[aa].frame = views[objs[aa].view].loops[objs[aa].loop].numFrames - 1;
        }
      }
    }
    else {  // Animate forwards
      objs[aa].frame++;
      if (objs[aa].frame >= views[objs[aa].view].loops[objs[aa].loop].numFrames) {
        // go to next loop thing
        if (views[objs[aa].view].loops[objs[aa].loop].RunNextLoop()) {
          if (objs[aa].loop+1 >= views[objs[aa].view].numLoops)
            quit("!Last loop in a view requested to move to next loop");
          objs[aa].loop++;
          objs[aa].frame=0;
        }
        else if (objs[aa].cycling % ANIM_BACKWARDS == ANIM_ONCE) {
          // leave it on the last frame
          objs[aa].cycling=0;
          objs[aa].frame--;
        }
        else {
          if (play.no_multiloop_repeat == 0) {
            // multi-loop anims, go back to start of it
            while ((objs[aa].loop > 0) && 
              (views[objs[aa].view].loops[objs[aa].loop - 1].RunNextLoop()))
              objs[aa].loop --;
          }
          if (objs[aa].cycling % ANIM_BACKWARDS == ANIM_ONCERESET)
            objs[aa].cycling=0;
          objs[aa].frame=0;
        }
      }
    }  // end if forwards

    ViewFrame*vfptr=&views[objs[aa].view].loops[objs[aa].loop].frames[objs[aa].frame];
    objs[aa].num = vfptr->pic;

    if (objs[aa].cycling == 0)
      continue;

    objs[aa].wait=vfptr->speed+objs[aa].overall_speed;
    CheckViewFrame (objs[aa].view, objs[aa].loop, objs[aa].frame);
  }
}

void update_stuff_shadow_areas() 
{
  // shadow areas
  int onwalkarea = get_walkable_area_at_character (game.playercharacter);
  if (onwalkarea<0) ;
  else if (playerchar->flags & CHF_FIXVIEW) ;
  else { onwalkarea=thisroom.shadinginfo[onwalkarea];
  if (onwalkarea>0) playerchar->view=onwalkarea-1;
  else if (thisroom.options[ST_MANVIEW]==0) playerchar->view=playerchar->defview;
  else playerchar->view=thisroom.options[ST_MANVIEW]-1;
  }
}

void update_stuff_move_anim_chars( int * followingAsSheep, int &numSheep, int MAX_SHEEP ) 
{
  // move & animate characters
  for (int aa=0;aa<game.numcharacters;aa++) {
    if (game.chars[aa].on != 1) continue;
    CharacterInfo*chi=&game.chars[aa];

    // walking
    if (chi->walking >= TURNING_AROUND) {
      // Currently rotating to correct direction
      if (chi->walkwait > 0) chi->walkwait--;
      else {
        // Work out which direction is next
        int wantloop = find_looporder_index(chi->loop) + 1;
        // going anti-clockwise, take one before instead
        if (chi->walking >= TURNING_BACKWARDS)
          wantloop -= 2;
        while (1) {
          if (wantloop >= 8)
            wantloop = 0;
          if (wantloop < 0)
            wantloop = 7;
          if ((turnlooporder[wantloop] >= views[chi->view].numLoops) ||
            (views[chi->view].loops[turnlooporder[wantloop]].numFrames < 1) ||
            ((turnlooporder[wantloop] >= 4) && ((chi->flags & CHF_NODIAGONAL)!=0))) {
              if (chi->walking >= TURNING_BACKWARDS)
                wantloop--;
              else
                wantloop++;
          }
          else break;
        }
        chi->loop = turnlooporder[wantloop];
        chi->walking -= TURNING_AROUND;
        // if still turning, wait for next frame
        if (chi->walking % TURNING_BACKWARDS >= TURNING_AROUND)
          chi->walkwait = chi->animspeed;
        else
          chi->walking = chi->walking % TURNING_BACKWARDS;
        charextra[aa].animwait = 0;
      }
      continue;
    }
    // Make sure it doesn't flash up a blue cup
    if (chi->view < 0) ;
    else if (chi->loop >= views[chi->view].numLoops)
      chi->loop = 0;

    int doing_nothing = 1;

    if ((chi->walking > 0) && (chi->room == displayed_room))
    {
      if (chi->walkwait > 0) chi->walkwait--;
      else 
      {
        chi->flags &= ~CHF_AWAITINGMOVE;

        // Move the character
        int numSteps = wantMoveNow(aa, chi);

        if ((numSteps) && (charextra[aa].xwas != INVALID_X)) {
          // if the zoom level changed mid-move, the walkcounter
          // might not have come round properly - so sort it out
          chi->x = charextra[aa].xwas;
          chi->y = charextra[aa].ywas;
          charextra[aa].xwas = INVALID_X;
        }

        int oldxp = chi->x, oldyp = chi->y;

        for (int ff = 0; ff < abs(numSteps); ff++) {
          if (doNextCharMoveStep (aa, chi))
            break;
          if ((chi->walking == 0) || (chi->walking >= TURNING_AROUND))
            break;
        }

        if (numSteps < 0) {
          // very small scaling, intersperse the movement
          // to stop it being jumpy
          charextra[aa].xwas = chi->x;
          charextra[aa].ywas = chi->y;
          chi->x = ((chi->x) - oldxp) / 2 + oldxp;
          chi->y = ((chi->y) - oldyp) / 2 + oldyp;
        }
        else if (numSteps > 0)
          charextra[aa].xwas = INVALID_X;

        if ((chi->flags & CHF_ANTIGLIDE) == 0)
          chi->walkwaitcounter++;
      }

      if (chi->loop >= views[chi->view].numLoops)
        quitprintf("Unable to render character %d (%s) because loop %d does not exist in view %d", chi->index_id, chi->name, chi->loop, chi->view + 1);

      // check don't overflow loop
      int framesInLoop = views[chi->view].loops[chi->loop].numFrames;
      if (chi->frame > framesInLoop)
      {
        chi->frame = 1;

        if (framesInLoop < 2)
          chi->frame = 0;

        if (framesInLoop < 1)
          quitprintf("Unable to render character %d (%s) because there are no frames in loop %d", chi->index_id, chi->name, chi->loop);
      }

      if (chi->walking<1) {
        charextra[aa].process_idle_this_time = 1;
        doing_nothing=1;
        chi->walkwait=0;
        charextra[aa].animwait = 0;
        // use standing pic
        StopMoving(aa);
        chi->frame = 0;
        CheckViewFrameForCharacter(chi);
      }
      else if (charextra[aa].animwait > 0) charextra[aa].animwait--;
      else {
        if (chi->flags & CHF_ANTIGLIDE)
          chi->walkwaitcounter++;

        if ((chi->flags & CHF_MOVENOTWALK) == 0)
        {
          chi->frame++;
          if (chi->frame >= views[chi->view].loops[chi->loop].numFrames)
          {
            // end of loop, so loop back round skipping the standing frame
            chi->frame = 1;

            if (views[chi->view].loops[chi->loop].numFrames < 2)
              chi->frame = 0;
          }

          charextra[aa].animwait = views[chi->view].loops[chi->loop].frames[chi->frame].speed + chi->animspeed;

          if (chi->flags & CHF_ANTIGLIDE)
            chi->walkwait = charextra[aa].animwait;
          else
            chi->walkwait = 0;

          CheckViewFrameForCharacter(chi);
        }
      }
      doing_nothing = 0;
    }
    // not moving, but animating
    // idleleft is <0 while idle view is playing (.animating is 0)
    if (((chi->animating != 0) || (chi->idleleft < 0)) &&
      ((chi->walking == 0) || ((chi->flags & CHF_MOVENOTWALK) != 0)) &&
      (chi->room == displayed_room)) 
    {
      doing_nothing = 0;
      // idle anim doesn't count as doing something
      if (chi->idleleft < 0)
        doing_nothing = 1;

      if (chi->wait>0) chi->wait--;
      else if ((char_speaking == aa) && (game.options[OPT_LIPSYNCTEXT] != 0)) {
        // currently talking with lip-sync speech
        int fraa = chi->frame;
        chi->wait = update_lip_sync (chi->view, chi->loop, &fraa) - 1;
        // closed mouth at end of sentence
        if ((play.messagetime >= 0) && (play.messagetime < play.close_mouth_speech_time))
          chi->frame = 0;

        if (chi->frame != fraa) {
          chi->frame = fraa;
          CheckViewFrameForCharacter(chi);
        }

        continue;
      }
      else {
        int oldframe = chi->frame;
        if (chi->animating & CHANIM_BACKWARDS) {
          chi->frame--;
          if (chi->frame < 0) {
            // if the previous loop is a Run Next Loop one, go back to it
            if ((chi->loop > 0) && 
              (views[chi->view].loops[chi->loop - 1].RunNextLoop())) {

                chi->loop --;
                chi->frame = views[chi->view].loops[chi->loop].numFrames - 1;
            }
            else if (chi->animating & CHANIM_REPEAT) {

              chi->frame = views[chi->view].loops[chi->loop].numFrames - 1;

              while (views[chi->view].loops[chi->loop].RunNextLoop()) {
                chi->loop++;
                chi->frame = views[chi->view].loops[chi->loop].numFrames - 1;
              }
            }
            else {
              chi->frame++;
              chi->animating = 0;
            }
          }
        }
        else
          chi->frame++;

        if ((aa == char_speaking) &&
          (channels[SCHAN_SPEECH] == NULL) &&
          (play.close_mouth_speech_time > 0) &&
          (play.messagetime < play.close_mouth_speech_time)) {
            // finished talking - stop animation
            chi->animating = 0;
            chi->frame = 0;
        }

        if (chi->frame >= views[chi->view].loops[chi->loop].numFrames) {

          if (views[chi->view].loops[chi->loop].RunNextLoop()) 
          {
            if (chi->loop+1 >= views[chi->view].numLoops)
              quit("!Animating character tried to overrun last loop in view");
            chi->loop++;
            chi->frame=0;
          }
          else if ((chi->animating & CHANIM_REPEAT)==0) {
            chi->animating=0;
            chi->frame--;
            // end of idle anim
            if (chi->idleleft < 0) {
              // constant anim, reset (need this cos animating==0)
              if (chi->idletime == 0)
                chi->frame = 0;
              // one-off anim, stop
              else {
                ReleaseCharacterView(aa);
                chi->idleleft=chi->idletime;
              }
            }
          }
          else {
            chi->frame=0;
            // if it's a multi-loop animation, go back to start
            if (play.no_multiloop_repeat == 0) {
              while ((chi->loop > 0) && 
                (views[chi->view].loops[chi->loop - 1].RunNextLoop()))
                chi->loop--;
            }
          }
        }
        chi->wait = views[chi->view].loops[chi->loop].frames[chi->frame].speed;
        // idle anim doesn't have speed stored cos animating==0
        if (chi->idleleft < 0)
          chi->wait += chi->animspeed+5;
        else 
          chi->wait += (chi->animating >> 8) & 0x00ff;

        if (chi->frame != oldframe)
          CheckViewFrameForCharacter(chi);
      }
    }

    if ((chi->following >= 0) && (chi->followinfo == FOLLOW_ALWAYSONTOP)) {
      // an always-on-top follow
      if (numSheep >= MAX_SHEEP)
        quit("too many sheep");
      followingAsSheep[numSheep] = aa;
      numSheep++;
    }
    // not moving, but should be following another character
    else if ((chi->following >= 0) && (doing_nothing == 1)) {
      short distaway=(chi->followinfo >> 8) & 0x00ff;
      // no character in this room
      if ((game.chars[chi->following].on == 0) || (chi->on == 0)) ;
      else if (chi->room < 0) {
        chi->room ++;
        if (chi->room == 0) {
          // appear in the new room
          chi->room = game.chars[chi->following].room;
          chi->x = play.entered_at_x;
          chi->y = play.entered_at_y;
        }
      }
      // wait a bit, so we're not constantly walking
      else if (Random(100) < (chi->followinfo & 0x00ff)) ;
      // the followed character has changed room
      else if ((chi->room != game.chars[chi->following].room)
        && (game.chars[chi->following].on == 0))
        ;  // do nothing if the player isn't visible
      else if (chi->room != game.chars[chi->following].room) {
        chi->prevroom = chi->room;
        chi->room = game.chars[chi->following].room;

        if (chi->room == displayed_room) {
          // only move to the room-entered position if coming into
          // the current room
          if (play.entered_at_x > (thisroom.width - 8)) {
            chi->x = thisroom.width+8;
            chi->y = play.entered_at_y;
          }
          else if (play.entered_at_x < 8) {
            chi->x = -8;
            chi->y = play.entered_at_y;
          }
          else if (play.entered_at_y > (thisroom.height - 8)) {
            chi->y = thisroom.height+8;
            chi->x = play.entered_at_x;
          }
          else if (play.entered_at_y < thisroom.top+8) {
            chi->y = thisroom.top+1;
            chi->x = play.entered_at_x;
          }
          else {
            // not at one of the edges
            // delay for a few seconds to let the player move
            chi->room = -play.follow_change_room_timer;
          }
          if (chi->room >= 0) {
            walk_character(aa,play.entered_at_x,play.entered_at_y,1, true);
            doing_nothing = 0;
          }
        }
      }
      else if (chi->room != displayed_room) {
        // if the characetr is following another character and
        // neither is in the current room, don't try to move
      }
      else if ((abs(game.chars[chi->following].x - chi->x) > distaway+30) |
        (abs(game.chars[chi->following].y - chi->y) > distaway+30) |
        ((chi->followinfo & 0x00ff) == 0)) {
          // in same room
          int goxoffs=(Random(50)-25);
          // make sure he's not standing on top of the other man
          if (goxoffs < 0) goxoffs-=distaway;
          else goxoffs+=distaway;
          walk_character(aa,game.chars[chi->following].x + goxoffs,
            game.chars[chi->following].y + (Random(50)-25),0, true);
          doing_nothing = 0;
      }
    }

    // no idle animation, so skip this bit
    if (chi->idleview < 1) ;
    // currently playing idle anim
    else if (chi->idleleft < 0) ;
    // not in the current room
    else if (chi->room != displayed_room) ;
    // they are moving or animating (or the view is locked), so 
    // reset idle timeout
    else if ((doing_nothing == 0) || ((chi->flags & CHF_FIXVIEW) != 0))
      chi->idleleft = chi->idletime;
    // count idle time
    else if ((loopcounter%40==0) || (charextra[aa].process_idle_this_time == 1)) {
      chi->idleleft--;
      if (chi->idleleft == -1) {
        int useloop=chi->loop;
        DEBUG_CONSOLE("%s: Now idle (view %d)", chi->scrname, chi->idleview+1);
        SetCharacterView(aa,chi->idleview+1);
        // SetCharView resets it to 0
        chi->idleleft = -2;
        int maxLoops = views[chi->idleview].numLoops;
        // if the char is set to "no diagonal loops", don't try
        // to use diagonal idle loops either
        if ((maxLoops > 4) && (useDiagonal(chi)))
          maxLoops = 4;
        // If it's not a "swimming"-type idleanim, choose a random loop
        // if there arent enough loops to do the current one.
        if ((chi->idletime > 0) && (useloop >= maxLoops)) {
          do {
            useloop = rand() % maxLoops;
            // don't select a loop which is a continuation of a previous one
          } while ((useloop > 0) && (views[chi->idleview].loops[useloop-1].RunNextLoop()));
        }
        // Normal idle anim - just reset to loop 0 if not enough to
        // use the current one
        else if (useloop >= maxLoops)
          useloop = 0;

        animate_character(chi,useloop,
          chi->animspeed+5,(chi->idletime == 0) ? 1 : 0, 1);

        // don't set Animating while the idle anim plays
        chi->animating = 0;
      }
    }  // end do idle animation

    charextra[aa].process_idle_this_time = 0;
  }

}

void update_stuff_following_exactly( int numSheep, int * followingAsSheep ) 
{
  // update location of all following_exactly characters
  for (int aa = 0; aa < numSheep; aa++) {
    CharacterInfo *chi = &game.chars[followingAsSheep[aa]];

    chi->x = game.chars[chi->following].x;
    chi->y = game.chars[chi->following].y;
    chi->z = game.chars[chi->following].z;
    chi->room = game.chars[chi->following].room;
    chi->prevroom = game.chars[chi->following].prevroom;

    int usebase = game.chars[chi->following].get_baseline();

    if (chi->flags & CHF_BEHINDSHEPHERD)
      chi->baseline = usebase - 1;
    else
      chi->baseline = usebase + 1;
  }
}

void update_stuff_overlay_timers() 
{
  // update overlay timers
  for (int aa=0;aa<numscreenover;aa++) {
    if (screenover[aa].timeout > 0) {
      screenover[aa].timeout--;
      if (screenover[aa].timeout == 0)
        remove_screen_overlay(screenover[aa].type);
    }
  }
}

void update_stuff_speech_text() 
{
  // determine if speech text should be removed
  if (play.messagetime>=0) {
    play.messagetime--;
    // extend life of text if the voice hasn't finished yet
    if (channels[SCHAN_SPEECH] != NULL) {
      if ((!rec_isSpeechFinished()) && (play.fast_forward == 0)) {
        //if ((!channels[SCHAN_SPEECH]->done) && (play.fast_forward == 0)) {
        if (play.messagetime <= 1)
          play.messagetime = 1;
      }
      else  // if the voice has finished, remove the speech
        play.messagetime = 0;
    }

    if (play.messagetime < 1) 
    {
      if (play.fast_forward > 0)
      {
        remove_screen_overlay(OVER_TEXTMSG);
      }
      else if (play.cant_skip_speech & SKIP_AUTOTIMER)
      {
        remove_screen_overlay(OVER_TEXTMSG);
        play.ignore_user_input_until_time = globalTimerCounter + (play.ignore_user_input_after_text_timeout_ms / time_between_timers);
      }
    }
  }
}

void update_stuff_sierra_speech() 
{
  // update sierra-style speech
  if ((face_talking >= 0) && (play.fast_forward == 0)) 
  {
    int updatedFrame = 0;

    if ((facetalkchar->blinkview > 0) && (facetalkAllowBlink)) {
      if (facetalkchar->blinktimer > 0) {
        // countdown to playing blink anim
        facetalkchar->blinktimer--;
        if (facetalkchar->blinktimer == 0) {
          facetalkchar->blinkframe = 0;
          facetalkchar->blinktimer = -1;
          updatedFrame = 2;
        }
      }
      else if (facetalkchar->blinktimer < 0) {
        // currently playing blink anim
        if (facetalkchar->blinktimer < ( (0 - 6) - views[facetalkchar->blinkview].loops[facetalkBlinkLoop].frames[facetalkchar->blinkframe].speed)) {
          // time to advance to next frame
          facetalkchar->blinktimer = -1;
          facetalkchar->blinkframe++;
          updatedFrame = 2;
          if (facetalkchar->blinkframe >= views[facetalkchar->blinkview].loops[facetalkBlinkLoop].numFrames) 
          {
            facetalkchar->blinkframe = 0;
            facetalkchar->blinktimer = facetalkchar->blinkinterval;
          }
        }
        else
          facetalkchar->blinktimer--;
      }

    }

    if (curLipLine >= 0) {
      // check voice lip sync
      int spchOffs = channels[SCHAN_SPEECH]->get_pos_ms ();
      if (curLipLinePhenome >= splipsync[curLipLine].numPhenomes) {
        // the lip-sync has finished, so just stay idle
      }
      else 
      {
        while ((curLipLinePhenome < splipsync[curLipLine].numPhenomes) &&
          ((curLipLinePhenome < 0) || (spchOffs >= splipsync[curLipLine].endtimeoffs[curLipLinePhenome])))
        {
          curLipLinePhenome ++;
          if (curLipLinePhenome >= splipsync[curLipLine].numPhenomes)
            facetalkframe = game.default_lipsync_frame;
          else
            facetalkframe = splipsync[curLipLine].frame[curLipLinePhenome];

          if (facetalkframe >= views[facetalkview].loops[facetalkloop].numFrames)
            facetalkframe = 0;

          updatedFrame |= 1;
        }
      }
    }
    else if (facetalkwait>0) facetalkwait--;
    // don't animate if the speech has finished
    else if ((play.messagetime < 1) && (facetalkframe == 0) && (play.close_mouth_speech_time > 0))
      ;
    else {
      // Close mouth at end of sentence
      if ((play.messagetime < play.close_mouth_speech_time) &&
        (play.close_mouth_speech_time > 0)) {
          facetalkframe = 0;
          facetalkwait = play.messagetime;
      }
      else if ((game.options[OPT_LIPSYNCTEXT]) && (facetalkrepeat > 0)) {
        // lip-sync speech (and not a thought)
        facetalkwait = update_lip_sync (facetalkview, facetalkloop, &facetalkframe);
        // It is actually displayed for facetalkwait+1 loops
        // (because when it's 1, it gets --'d then wait for next time)
        facetalkwait --;
      }
      else {
        // normal non-lip-sync
        facetalkframe++;
        if ((facetalkframe >= views[facetalkview].loops[facetalkloop].numFrames) ||
          ((play.messagetime < 1) && (play.close_mouth_speech_time > 0))) {

            if ((facetalkframe >= views[facetalkview].loops[facetalkloop].numFrames) &&
              (views[facetalkview].loops[facetalkloop].RunNextLoop())) 
            {
              facetalkloop++;
            }
            else 
            {
              facetalkloop = 0;
            }
            facetalkframe = 0;
            if (!facetalkrepeat)
              facetalkwait = 999999;
        }
        if ((facetalkframe != 0) || (facetalkrepeat == 1))
          facetalkwait = views[facetalkview].loops[facetalkloop].frames[facetalkframe].speed + GetCharacterSpeechAnimationDelay(facetalkchar);
      }
      updatedFrame |= 1;
    }

    // is_text_overlay might be 0 if it was only just destroyed this loop
    if ((updatedFrame) && (is_text_overlay > 0)) {

      if (updatedFrame & 1)
        CheckViewFrame (facetalkview, facetalkloop, facetalkframe);
      if (updatedFrame & 2)
        CheckViewFrame (facetalkchar->blinkview, facetalkBlinkLoop, facetalkchar->blinkframe);

      int yPos = 0;
      int thisPic = views[facetalkview].loops[facetalkloop].frames[facetalkframe].pic;

      if (game.options[OPT_SPEECHTYPE] == 3) {
        // QFG4-style fullscreen dialog
        yPos = (BMP_H(screenover[face_talking].pic) / 2) - (spriteheight[thisPic] / 2);
        alw_clear_to_color(screenover[face_talking].pic, 0);
      }
      else {
        alw_clear_to_color(screenover[face_talking].pic, alw_bitmap_mask_color(screenover[face_talking].pic));
      }

      DrawViewFrame(screenover[face_talking].pic, &views[facetalkview].loops[facetalkloop].frames[facetalkframe], 0, yPos);
      //      alw_draw_sprite(screenover[face_talking].pic, spriteset[thisPic], 0, yPos);

      if ((facetalkchar->blinkview > 0) && (facetalkchar->blinktimer < 0)) {
        // draw the blinking sprite on top
        DrawViewFrame(screenover[face_talking].pic,
          &views[facetalkchar->blinkview].loops[facetalkBlinkLoop].frames[facetalkchar->blinkframe],
          0, yPos);

        /*        alw_draw_sprite(screenover[face_talking].pic,
        spriteset[views[facetalkchar->blinkview].frames[facetalkloop][facetalkchar->blinkframe].pic],
        0, yPos);*/
      }

      gfxDriver->UpdateDDBFromBitmap(screenover[face_talking].bmp, screenover[face_talking].pic, false);
    }  // end if updatedFrame
  }
}



// update_stuff: moves and animates objects, executes repeat scripts, and
// the like.
void update_stuff() {

  set_eip(20);
  update_stuff_timers();
  update_stuff_view_cycle_graphics();

  set_eip(21);
  update_stuff_shadow_areas();

  set_eip(22);
  #define MAX_SHEEP 30
  int numSheep = 0;
  int followingAsSheep[MAX_SHEEP];
  update_stuff_move_anim_chars(followingAsSheep, numSheep, MAX_SHEEP);
  update_stuff_following_exactly(numSheep, followingAsSheep);

  set_eip(23);
  update_stuff_overlay_timers();
  update_stuff_speech_text();

  set_eip(24);
  update_stuff_sierra_speech();

  set_eip(25);
}



// ============================================================================
// TEXT - MACROS (messages)
// ============================================================================

void replace_macro_tokens(char*statusbarformat,char*cur_stb_text) {
  char*curptr=&statusbarformat[0];
  char tmpm[3];
  char*endat = curptr + strlen(statusbarformat);
  cur_stb_text[0]=0;
  char tempo[STD_BUFFER_SIZE];

  while (1) {
    if (curptr[0]==0) break;
    if (curptr>=endat) break;
    if (curptr[0]=='@') {
      char *curptrWasAt = curptr;
      char macroname[21]; int idd=0; curptr++;
      for (idd=0;idd<20;idd++) {
        if (curptr[0]=='@') {
          macroname[idd]=0;
          curptr++;
          break;
        }
        // unterminated macro (eg. "@SCORETEXT"), so abort
        if (curptr[0] == 0)
          break;
        macroname[idd]=curptr[0];
        curptr++;
      }
      macroname[idd]=0; 
      tempo[0]=0;
      if (ac_stricmp(macroname,"score")==0)
        sprintf(tempo,"%d",play.score);
      else if (ac_stricmp(macroname,"totalscore")==0)
        sprintf(tempo,"%d",MAXSCORE);
      else if (ac_stricmp(macroname,"scoretext")==0)
        sprintf(tempo,"%d of %d",play.score,MAXSCORE);
      else if (ac_stricmp(macroname,"gamename")==0)
        strcpy(tempo, play.game_name);
      else if (ac_stricmp(macroname,"overhotspot")==0) {
        // While game is in Wait mode, no overhotspot text
        if (!IsInterfaceEnabled())
          tempo[0] = 0;
        else
          GetLocationName(divide_down_coordinate(mousex), divide_down_coordinate(mousey), tempo);
      }
      else { // not a macro, there's just a @ in the message
        curptr = curptrWasAt + 1;
        strcpy(tempo, "@");
      }
        
      strcat(cur_stb_text,tempo);
    }
    else {
      tmpm[0]=curptr[0]; tmpm[1]=0;
      strcat(cur_stb_text,tmpm);
      curptr++;
    }
  }
}



// ============================================================================
// GUI INV
// ============================================================================

int GUIInv::CharToDisplay() {
  if (this->charId < 0)
    return game.playercharacter;

  return this->charId;
}

void GUIInv::Draw() {
  if ((IsDisabled()) && (gui_disabled_style == GUIDIS_BLACKOUT))
    return;

  // backwards compatibility
  play.inv_numinline = this->itemsPerLine;
  play.inv_numdisp = this->numLines * this->itemsPerLine;
  play.obsolete_inv_numorder = charextra[game.playercharacter].invorder_count;
  // if the user changes top_inv_item, switch into backwards
  // compatibiltiy mode
  if (play.inv_top) {
    play.inv_backwards_compatibility = 1;
  }

  if (play.inv_backwards_compatibility) {
    this->topIndex = play.inv_top;
  }

  // draw the items
  int xxx = x;
  int uu, cxp = x, cyp = y;
  int lastItem = this->topIndex + (this->itemsPerLine * this->numLines);
  if (lastItem > charextra[this->CharToDisplay()].invorder_count)
    lastItem = charextra[this->CharToDisplay()].invorder_count;

  for (uu = this->topIndex; uu < lastItem; uu++) {
    // draw inv graphic
    wputblock(cxp, cyp, spriteset[game.invinfo[charextra[this->CharToDisplay()].invorder[uu]].pic], 1);
    cxp += multiply_up_coordinate(this->itemWidth);

    // go to next row when appropriate
    if ((uu - this->topIndex) % this->itemsPerLine == (this->itemsPerLine - 1)) {
      cxp = xxx;
      cyp += multiply_up_coordinate(this->itemHeight);
    }
  }

  if ((IsDisabled()) &&
      (gui_disabled_style == GUIDIS_GREYOUT) && 
      (play.inventory_greys_out == 1)) {
    int col8 = get_col8_lookup(8);
    int jj, kk;   // darken the inventory when disabled
    for (jj = 0; jj < wid; jj++) {
      for (kk = jj % 2; kk < hit; kk += 2)
        alw_putpixel(abuf, x + jj, y + kk, col8);
    }
  }

}



// ============================================================================
// GRAPHICS - BITMAPS
// ============================================================================

// Avoid freeing and reallocating the memory if possible
IDriverDependantBitmap* recycle_ddb_bitmap(IDriverDependantBitmap *bimp, ALW_BITMAP *source, bool hasAlpha) {
  if (bimp != NULL) {
    // same colour depth, width and height -> reuse
    if (((bimp->GetColorDepth() + 1) / 8 == bmp_bpp(source)) && 
        (bimp->GetWidth() == BMP_W(source)) && (bimp->GetHeight() == BMP_H(source)))
    {
      gfxDriver->UpdateDDBFromBitmap(bimp, source, hasAlpha);
      return bimp;
    }

    gfxDriver->DestroyDDB(bimp);
  }
  bimp = gfxDriver->CreateDDBFromBitmap(source, hasAlpha, false);
  return bimp;
}



// ============================================================================
// WALK BEHINDS
// ============================================================================

// sort_out_walk_behinds: modifies the supplied sprite by overwriting parts
// of it with transparent pixels where there are walk-behind areas
// Returns whether any pixels were updated
int sort_out_walk_behinds(block sprit,int xx,int yy,int basel, block copyPixelsFrom, block checkPixelsFrom, int zoom) {
  if (noWalkBehindsAtAll)
    return 0;

  if ((!alw_is_memory_bitmap(thisroom.object)) ||
      (!alw_is_memory_bitmap(sprit)))
    quit("!sort_out_walk_behinds: wb bitmap not linear");

  int rr,tmm, toheight;//,tcol;
  // precalculate this to try and shave some time off
  int maskcol = alw_bitmap_mask_color(sprit);
  int spcoldep = alw_bitmap_color_depth(sprit);
  int screenhit = BMP_H(thisroom.object);
  short *shptr, *shptr2;
  long *loptr, *loptr2;
  int pixelsChanged = 0;
  int ee = 0;
  if (xx < 0)
    ee = 0 - xx;

  if ((checkPixelsFrom != NULL) && (alw_bitmap_color_depth(checkPixelsFrom) != spcoldep))
    quit("sprite colour depth does not match background colour depth");

  for ( ; ee < BMP_W(sprit); ee++) {
    if (ee + xx >= BMP_W(thisroom.object))
      break;

    if ((!walkBehindExists[ee+xx]) ||
        (walkBehindEndY[ee+xx] <= yy) ||
        (walkBehindStartY[ee+xx] > yy+BMP_H(sprit)))
      continue;

    toheight = BMP_H(sprit);

    if (walkBehindStartY[ee+xx] < yy)
      rr = 0;
    else
      rr = (walkBehindStartY[ee+xx] - yy);

    // Since we will use _getpixel, ensure we only check within the screen
    if (rr + yy < 0)
      rr = 0 - yy;
    if (toheight + yy > screenhit)
      toheight = screenhit - yy;
    if (toheight + yy > walkBehindEndY[ee+xx])
      toheight = walkBehindEndY[ee+xx] - yy;
    if (rr < 0)
      rr = 0;

    for ( ; rr < toheight;rr++) {
      
      // we're ok with _getpixel because we've checked the screen edges
      //tmm = alw__getpixel(thisroom.object,ee+xx,rr+yy);
      // actually, _getpixel is well inefficient, do it ourselves
      // since we know it's 8-bit bitmap
      tmm = BMP_LINE(thisroom.object)[rr+yy][ee+xx];
      if (tmm<1) continue;
      if (croom->walkbehind_base[tmm] <= basel) continue;

      if (copyPixelsFrom != NULL)
      {
        if (spcoldep <= 8)
        {
          if (BMP_LINE(checkPixelsFrom)[(rr * 100) / zoom][(ee * 100) / zoom] != maskcol) {
            BMP_LINE(sprit)[rr][ee] = BMP_LINE(copyPixelsFrom)[rr + yy][ee + xx];
            pixelsChanged = 1;
          }
        }
        else if (spcoldep <= 16) {
          shptr = (short*)&BMP_LINE(sprit)[rr][0];
          shptr2 = (short*)&BMP_LINE(checkPixelsFrom)[(rr * 100) / zoom][0];
          if (shptr2[(ee * 100) / zoom] != maskcol) {
            shptr[ee] = ((short*)(&BMP_LINE(copyPixelsFrom)[rr + yy][0]))[ee + xx];
            pixelsChanged = 1;
          }
        }
        else if (spcoldep == 24) {
          char *chptr = (char*)&BMP_LINE(sprit)[rr][0];
          char *chptr2 = (char*)&BMP_LINE(checkPixelsFrom)[(rr * 100) / zoom][0];
          if (memcmp(&chptr2[((ee * 100) / zoom) * 3], &maskcol, 3) != 0) {
            memcpy(&chptr[ee * 3], &BMP_LINE(copyPixelsFrom)[rr + yy][(ee + xx) * 3], 3);
            pixelsChanged = 1;
          }
        }
        else if (spcoldep <= 32) {
          loptr = (long*)&BMP_LINE(sprit)[rr][0];
          loptr2 = (long*)&BMP_LINE(checkPixelsFrom)[(rr * 100) / zoom][0];
          if (loptr2[(ee * 100) / zoom] != maskcol) {
            loptr[ee] = ((long*)(&BMP_LINE(copyPixelsFrom)[rr + yy][0]))[ee + xx];
            pixelsChanged = 1;
          }
        }
      }
      else
      {
        pixelsChanged = 1;
        if (spcoldep <= 8)
          BMP_LINE(sprit)[rr][ee] = maskcol;
        else if (spcoldep <= 16) {
          shptr = (short*)&BMP_LINE(sprit)[rr][0];
          shptr[ee] = maskcol;
        }
        else if (spcoldep == 24) {
          char *chptr = (char*)&BMP_LINE(sprit)[rr][0];
          memcpy(&chptr[ee * 3], &maskcol, 3);
        }
        else if (spcoldep <= 32) {
          loptr = (long*)&BMP_LINE(sprit)[rr][0];
          loptr[ee] = maskcol;
        }
        else
          quit("!Sprite colour depth >32 ??");
      }
    }
  }
  return pixelsChanged;
}

void invalidate_cached_walkbehinds() 
{
  memset(&actspswbcache[0], 0, sizeof(CachedActSpsData) * actSpsCount);
}

void sort_out_char_sprite_walk_behind(int actspsIndex, int xx, int yy, int basel, int zoom, int width, int height)
{
  if (noWalkBehindsAtAll)
    return;

  if ((!actspswbcache[actspsIndex].valid) ||
    (actspswbcache[actspsIndex].xWas != xx) ||
    (actspswbcache[actspsIndex].yWas != yy) ||
    (actspswbcache[actspsIndex].baselineWas != basel))
  {
    actspswb[actspsIndex] = recycle_bitmap(actspswb[actspsIndex], alw_bitmap_color_depth(thisroom.ebscene[play.bg_frame]), width, height);

    block wbSprite = actspswb[actspsIndex];
    alw_clear_to_color(wbSprite, alw_bitmap_mask_color(wbSprite));

    actspswbcache[actspsIndex].isWalkBehindHere = sort_out_walk_behinds(wbSprite, xx, yy, basel, thisroom.ebscene[play.bg_frame], actsps[actspsIndex], zoom);
    actspswbcache[actspsIndex].xWas = xx;
    actspswbcache[actspsIndex].yWas = yy;
    actspswbcache[actspsIndex].baselineWas = basel;
    actspswbcache[actspsIndex].valid = 1;

    if (actspswbcache[actspsIndex].isWalkBehindHere)
    {
      actspswbbmp[actspsIndex] = recycle_ddb_bitmap(actspswbbmp[actspsIndex], actspswb[actspsIndex], false);
    }
  }

  if (actspswbcache[actspsIndex].isWalkBehindHere)
  {
    add_to_sprite_list(actspswbbmp[actspsIndex], xx - offsetx, yy - offsety, basel, 0, -1, true);
  }
}



// ============================================================================
// GRAPHICS - DRAWING
// ============================================================================

void clear_draw_list() {
  thingsToDrawSize = 0;
}
void add_thing_to_draw(IDriverDependantBitmap* bmp, int x, int y, int trans, bool alphaChannel) {
  thingsToDrawList[thingsToDrawSize].pic = NULL;
  thingsToDrawList[thingsToDrawSize].bmp = bmp;
  thingsToDrawList[thingsToDrawSize].x = x;
  thingsToDrawList[thingsToDrawSize].y = y;
  thingsToDrawList[thingsToDrawSize].transparent = trans;
  thingsToDrawList[thingsToDrawSize].hasAlphaChannel = alphaChannel;
  thingsToDrawSize++;
  if (thingsToDrawSize >= MAX_THINGS_TO_DRAW - 1)
    quit("add_thing_to_draw: too many things added");
}

// the sprite list is an intermediate list used to order 
// objects and characters by their baselines before everything
// is added to the Thing To Draw List
void clear_sprite_list() {
  sprlistsize=0;
}
void add_to_sprite_list(IDriverDependantBitmap* spp, int xx, int yy, int baseline, int trans, int sprNum, bool isWalkBehind) {

  // completely invisible, so don't draw it at all
  if (trans == 255)
    return;

  if ((sprNum >= 0) && ((game.spriteflags[sprNum] & SPF_ALPHACHANNEL) != 0))
    sprlist[sprlistsize].hasAlphaChannel = true;
  else
    sprlist[sprlistsize].hasAlphaChannel = false;

  sprlist[sprlistsize].bmp = spp;
  sprlist[sprlistsize].baseline = baseline;
  sprlist[sprlistsize].x=xx;
  sprlist[sprlistsize].y=yy;
  sprlist[sprlistsize].transparent=trans;

  if (walkBehindMethod == DrawAsSeparateSprite)
    sprlist[sprlistsize].takesPriorityIfEqual = !isWalkBehind;
  else
    sprlist[sprlistsize].takesPriorityIfEqual = isWalkBehind;

  sprlistsize++;

  if (sprlistsize >= MAX_SPRITES_ON_SCREEN)
    quit("Too many sprites have been added to the sprite list. There is a limit of 75 objects and characters being visible at the same time. You may want to reconsider your design since you have over 75 objects/characters visible at once.");

  if (spp == NULL)
    quit("add_to_sprite_list: attempted to draw NULL sprite");
}

void put_sprite_256(int xxx,int yyy,block piccy) {

  if (trans_mode >= 255) {
    // fully transparent, don't draw it at all
    trans_mode = 0;
    return;
  }

  int screen_depth = alw_bitmap_color_depth(abuf);

#ifdef USE_15BIT_FIX
  if (alw_bitmap_color_depth(piccy) < screen_depth) {

    if ((alw_bitmap_color_depth(piccy) == 8) && (screen_depth >= 24)) {
      // 256-col sprite -> truecolor background
      // this is automatically supported by allegro, no twiddling needed
      alw_draw_sprite(abuf, piccy, xxx, yyy);
      return;
    }
    // 256-col spirte -> hi-color background, or
    // 16-bit sprite -> 32-bit background
    block hctemp=alw_create_bitmap_ex(screen_depth, BMP_W(piccy), BMP_H(piccy));
    alw_blit(piccy,hctemp,0,0,0,0,BMP_W(hctemp),BMP_H(hctemp));
    int bb,cc,mask_col = alw_bitmap_mask_color(abuf);
    if (alw_bitmap_color_depth(piccy) == 8) {
      // only do this for 256-col, cos the blit call converts
      // transparency for 16->32 bit
      for (bb=0;bb<BMP_W(hctemp);bb++) {
        for (cc=0;cc<BMP_H(hctemp);cc++)
          if (alw__getpixel(piccy,bb,cc)==0) alw_putpixel(hctemp,bb,cc,mask_col);
      }
    }
    wputblock(xxx,yyy,hctemp,1);
    wfreeblock(hctemp);
  }
  else
#endif
  {
    if ((trans_mode!=0) && (game.color_depth > 1) && (bmp_bpp(piccy) > 1) && (bmp_bpp(abuf) > 1)) {
      alw_set_trans_blender(0,0,0,trans_mode);
      alw_draw_trans_sprite(abuf,piccy,xxx,yyy);
      }
/*    else if ((lit_mode < 0) && (game.color_depth == 1) && (bmp_bpp(piccy) == 1)) {
      alw_draw_lit_sprite(abuf,piccy,xxx,yyy,250 - ((-lit_mode) * 5)/2);
      }*/
    else
      wputblock(xxx,yyy,piccy,1);
  }
  trans_mode=0;
}

void repair_alpha_channel(block dest, block bgpic)
{
  // Repair the alpha channel, because sprites may have been drawn
  // over it by the buttons, etc
  int theWid = (BMP_W(dest) < BMP_W(bgpic)) ? BMP_W(dest) : BMP_W(bgpic);
  int theHit = (BMP_H(dest) < BMP_H(bgpic)) ? BMP_H(dest) : BMP_H(bgpic);
  for (int y = 0; y < theHit; y++) 
  {
    unsigned long *destination = ((unsigned long*)BMP_LINE(dest)[y]);
    unsigned long *source = ((unsigned long*)BMP_LINE(bgpic)[y]);
    for (int x = 0; x < theWid; x++) 
    {
      destination[x] |= (source[x] & 0xff000000);
    }
  }
}


// used by GUI renderer to draw images
void draw_sprite_compensate(int picc,int xx,int yy,int useAlpha) 
{
  if ((useAlpha) && 
    (game.options[OPT_NEWGUIALPHA] > 0) &&
    (alw_bitmap_color_depth(abuf) == 32))
  {
    if (game.spriteflags[picc] & SPF_ALPHACHANNEL) {
      set_additive_alpha_blender();
      alw_draw_trans_sprite(abuf, spriteset[picc], xx, yy);
    }
    else {
      set_opaque_alpha_blender();
      alw_draw_trans_sprite(abuf, spriteset[picc], xx, yy);
    }
  }
  else
  {
    put_sprite_256(xx, yy, spriteset[picc]);
  }
}

// function to sort the sprites into baseline order
extern "C" int compare_listentries(const void *elem1, const void *elem2) {
  SpriteListEntry *e1, *e2;
  e1 = (SpriteListEntry*)elem1;
  e2 = (SpriteListEntry*)elem2;

  if (e1->baseline == e2->baseline) 
  { 
    if (e1->takesPriorityIfEqual)
      return 1;
    if (e2->takesPriorityIfEqual)
      return -1;
  }

  // returns >0 if e1 is lower down, <0 if higher, =0 if the same
  return e1->baseline - e2->baseline;
}

void draw_sprite_list() {

  if (walkBehindMethod == DrawAsSeparateSprite)
  {
    for (int ee = 1; ee < MAX_OBJ; ee++)
    {
      if (walkBehindBitmap[ee] != NULL)
      {
        add_to_sprite_list(walkBehindBitmap[ee], walkBehindLeft[ee] - offsetx, walkBehindTop[ee] - offsety, 
                           croom->walkbehind_base[ee], 0, -1, true);
      }
    }
  }

  // 2.60.672 - convert horrid bubble sort to use qsort instead
  qsort(sprlist, sprlistsize, sizeof(SpriteListEntry), compare_listentries);

  clear_draw_list();

  add_thing_to_draw(NULL, AGSE_PRESCREENDRAW, 0, TRANS_RUN_PLUGIN, false);

  // copy the sorted sprites into the Things To Draw list
  thingsToDrawSize += sprlistsize;
  memcpy(&thingsToDrawList[1], sprlist, sizeof(SpriteListEntry) * sprlistsize);
}

// Avoid freeing and reallocating the memory if possible
block recycle_bitmap(block bimp, int coldep, int wid, int hit) {
  if (bimp != NULL) {
    // same colour depth, width and height -> reuse
    if ((alw_bitmap_color_depth(bimp) == coldep) && (BMP_W(bimp) == wid)
       && (BMP_H(bimp) == hit))
      return bimp;

    alw_destroy_bitmap(bimp);
  }
  bimp = alw_create_bitmap_ex(coldep, wid, hit);
  return bimp;
}



// Draws srcimg onto destimg, tinting to the specified level
// Totally overwrites the contents of the destination image
void tint_image (block srcimg, block destimg, int red, int grn, int blu, int light_level, int luminance) {

  if ((alw_bitmap_color_depth(srcimg) != alw_bitmap_color_depth(destimg)) ||
      (alw_bitmap_color_depth(srcimg) <= 8)) {
    debug_log("Image tint failed - images must both be hi-color");
    // the caller expects something to have been copied
    alw_blit(srcimg, destimg, 0, 0, 0, 0, BMP_W(srcimg), BMP_H(srcimg));
    return;
  }

  // For performance reasons, we have a seperate blender for
  // when light is being adjusted and when it is not.
  // If luminance >= 250, then normal brightness, otherwise darken
  if (luminance >= 250)
    alw_set_blender_mode (_myblender_color15, _myblender_color16, _myblender_color32, red, grn, blu, 0);
  else
    alw_set_blender_mode (_myblender_color15_light, _myblender_color16_light, _myblender_color32_light, red, grn, blu, 0);

  if (light_level >= 100) {
    // fully colourised
    alw_clear_to_color(destimg, alw_bitmap_mask_color(destimg));
    alw_draw_lit_sprite(destimg, srcimg, 0, 0, luminance);
  }
  else {
    // light_level is between -100 and 100 normally; 0-100 in
    // this case when it's a RGB tint
    light_level = (light_level * 25) / 10;

    // Copy the image to the new bitmap
    alw_blit(srcimg, destimg, 0, 0, 0, 0, BMP_W(srcimg), BMP_H(srcimg));
    // Render the colourised image to a temporary bitmap,
    // then transparently draw it over the original image
    block finaltarget = alw_create_bitmap_ex(alw_bitmap_color_depth(srcimg), BMP_W(srcimg), BMP_H(srcimg));
    alw_clear_to_color(finaltarget, alw_bitmap_mask_color(finaltarget));
    alw_draw_lit_sprite(finaltarget, srcimg, 0, 0, luminance);

    // customized trans blender to preserve alpha channel
    set_my_trans_blender (0, 0, 0, light_level);
    alw_draw_trans_sprite (destimg, finaltarget, 0, 0);
    alw_destroy_bitmap (finaltarget);
  }
}

void prepare_characters_for_drawing() {
  int zoom_level,newwidth,newheight,onarea,sppic,atxp,atyp,useindx;
  int light_level,coldept,aa;
  int tint_red, tint_green, tint_blue, tint_amount, tint_light = 255;

  set_eip(33);
  // draw characters
  for (aa=0;aa<game.numcharacters;aa++) {
    if (game.chars[aa].on==0) continue;
    if (game.chars[aa].room!=displayed_room) continue;
    eip_guinum = aa;
    useindx = aa + MAX_INIT_SPR;

    CharacterInfo*chin=&game.chars[aa];
    set_eip(330);
    // if it's on but set to view -1, they're being silly
    if (chin->view < 0) {
      quitprintf("!The character '%s' was turned on in the current room (room %d) but has not been assigned a view number.",
        chin->name, displayed_room);
    }

    if (chin->frame >= views[chin->view].loops[chin->loop].numFrames)
      chin->frame = 0;

    if ((chin->loop >= views[chin->view].numLoops) ||
        (views[chin->view].loops[chin->loop].numFrames < 1)) {
      quitprintf("!The character '%s' could not be displayed because there were no frames in loop %d of view %d.",
        chin->name, chin->loop, chin->view + 1);
    }

    sppic=views[chin->view].loops[chin->loop].frames[chin->frame].pic;
    if ((sppic < 0) || (sppic >= MAX_SPRITES))
      sppic = 0;  // in case it's screwed up somehow
    set_eip(331);
    // sort out the stretching if required
    onarea = get_walkable_area_at_character (aa);
    set_eip(332);
    light_level = 0;
    tint_amount = 0;
     
    if (chin->flags & CHF_MANUALSCALING)  // character ignores scaling
      zoom_level = charextra[aa].zoom;
    else if ((onarea <= 0) && (thisroom.walk_area_zoom[0] == 0)) {
      zoom_level = charextra[aa].zoom;
      if (zoom_level == 0)
        zoom_level = 100;
    }
    else
      zoom_level = get_area_scaling (onarea, chin->x, chin->y);

    charextra[aa].zoom = zoom_level;

    if (chin->flags & CHF_HASTINT) {
      // object specific tint, use it
      tint_red = charextra[aa].tint_r;
      tint_green = charextra[aa].tint_g;
      tint_blue = charextra[aa].tint_b;
      tint_amount = charextra[aa].tint_level;
      tint_light = charextra[aa].tint_light;
      light_level = 0;
    }
    else {
      get_local_tint(chin->x, chin->y, chin->flags & CHF_NOLIGHTING,
        &tint_amount, &tint_red, &tint_green, &tint_blue,
        &tint_light, &light_level);
    }

    /*if (actsps[useindx]!=NULL) {
      wfreeblock(actsps[useindx]);
      actsps[useindx] = NULL;
    }*/

    set_eip(3330);
    int isMirrored = 0, specialpic = sppic;
    bool usingCachedImage = false;

    coldept = alw_bitmap_color_depth(spriteset[sppic]);

    // adjust the sppic if mirrored, so it doesn't accidentally
    // cache the mirrored frame as the real one
    if (views[chin->view].loops[chin->loop].frames[chin->frame].flags & VFLG_FLIPSPRITE) {
      isMirrored = 1;
      specialpic = -sppic;
    }

    set_eip(3331);

    // if the character was the same sprite and scaling last time,
    // just use the cached image
    if ((charcache[aa].inUse) &&
        (charcache[aa].sppic == specialpic) &&
        (charcache[aa].scaling == zoom_level) &&
        (charcache[aa].tintredwas == tint_red) &&
        (charcache[aa].tintgrnwas == tint_green) &&
        (charcache[aa].tintbluwas == tint_blue) &&
        (charcache[aa].tintamntwas == tint_amount) &&
        (charcache[aa].tintlightwas == tint_light) &&
        (charcache[aa].lightlevwas == light_level)) 
    {
      if (walkBehindMethod == DrawOverCharSprite)
      {
        actsps[useindx] = recycle_bitmap(actsps[useindx], alw_bitmap_color_depth(charcache[aa].image), BMP_W(charcache[aa].image), BMP_H(charcache[aa].image));
        alw_blit (charcache[aa].image, actsps[useindx], 0, 0, 0, 0, BMP_W(actsps[useindx]), BMP_H(actsps[useindx]));
      }
      else 
      {
        usingCachedImage = true;
      }
    }
    else if ((charcache[aa].inUse) && 
             (charcache[aa].sppic == specialpic) &&
             (gfxDriver->HasAcceleratedStretchAndFlip()))
    {
      usingCachedImage = true;
    }
    else if (charcache[aa].inUse) {
      //alw_destroy_bitmap (charcache[aa].image);
      charcache[aa].inUse = 0;
    }

    set_eip(3332);
    
    if (zoom_level != 100) {
      // it needs to be stretched, so calculate the new dimensions

      scale_sprite_size(sppic, zoom_level, &newwidth, &newheight);
      charextra[aa].width=newwidth;
      charextra[aa].height=newheight;
    }
    else {
      // draw at original size, so just use the sprite width and height
      charextra[aa].width=0;
      charextra[aa].height=0;
      newwidth = spritewidth[sppic];
      newheight = spriteheight[sppic];
    }

    set_eip(3336);

    // Calculate the X & Y co-ordinates of where the sprite will be
    atxp=(multiply_up_coordinate(chin->x) - offsetx) - newwidth/2;
    atyp=(multiply_up_coordinate(chin->y) - newheight) - offsety;

    charcache[aa].scaling = zoom_level;
    charcache[aa].sppic = specialpic;
    charcache[aa].tintredwas = tint_red;
    charcache[aa].tintgrnwas = tint_green;
    charcache[aa].tintbluwas = tint_blue;
    charcache[aa].tintamntwas = tint_amount;
    charcache[aa].tintlightwas = tint_light;
    charcache[aa].lightlevwas = light_level;

    // If cache needs to be re-drawn
    if (!charcache[aa].inUse) {

      // create the base sprite in actsps[useindx], which will
      // be scaled and/or flipped, as appropriate
      int actspsUsed = 0;
      if (!gfxDriver->HasAcceleratedStretchAndFlip())
      {
        actspsUsed = scale_and_flip_sprite(
                            useindx, coldept, zoom_level, sppic,
                            newwidth, newheight, isMirrored);
      }
      else 
      {
        // ensure actsps exists
        actsps[useindx] = recycle_bitmap(actsps[useindx], coldept, spritewidth[sppic], spriteheight[sppic]);
      }

      set_eip(335);

      if (((light_level != 0) || (tint_amount != 0)) &&
          (!gfxDriver->HasAcceleratedStretchAndFlip())) {
        // apply the lightening or tinting
        block comeFrom = NULL;
        // if possible, direct read from the source image
        if (!actspsUsed)
          comeFrom = spriteset[sppic];

        apply_tint_or_light(useindx, light_level, tint_amount, tint_red,
                            tint_green, tint_blue, tint_light, coldept,
                            comeFrom);
      }
      else if (!actspsUsed)
        // no scaling, flipping or tinting was done, so just blit it normally
        alw_blit (spriteset[sppic], actsps[useindx], 0, 0, 0, 0, BMP_W(actsps[useindx]), BMP_H(actsps[useindx]));

      // update the character cache with the new image
      charcache[aa].inUse = 1;
      //charcache[aa].image = alw_create_bitmap_ex (coldept, BMP_W(actsps[useindx]), BMP_H(actsps[useindx]));
      charcache[aa].image = recycle_bitmap(charcache[aa].image, coldept, BMP_W(actsps[useindx]), BMP_H(actsps[useindx]));
      alw_blit (actsps[useindx], charcache[aa].image, 0, 0, 0, 0, BMP_W(actsps[useindx]), BMP_H(actsps[useindx]));

    } // end if !cache.inUse

    int usebasel = chin->get_baseline();

    // adjust the Y positioning for the character's Z co-ord
    atyp -= multiply_up_coordinate(chin->z);

    set_eip(336);

    int bgX = atxp + offsetx + chin->pic_xoffs;
    int bgY = atyp + offsety + chin->pic_yoffs;

    if (chin->flags & CHF_NOWALKBEHINDS) {
      // ignore walk-behinds, do nothing
      if (walkBehindMethod == DrawAsSeparateSprite)
      {
        usebasel += thisroom.height;
      }
    }
    else if (walkBehindMethod == DrawAsSeparateCharSprite) 
    {
      sort_out_char_sprite_walk_behind(useindx, bgX, bgY, usebasel, charextra[aa].zoom, newwidth, newheight);
    }
    else if (walkBehindMethod == DrawOverCharSprite)
    {
      sort_out_walk_behinds(actsps[useindx], bgX, bgY, usebasel);
    }

    if ((!usingCachedImage) || (actspsbmp[useindx] == NULL))
    {
      bool hasAlpha = (game.spriteflags[sppic] & SPF_ALPHACHANNEL) != 0;

      actspsbmp[useindx] = recycle_ddb_bitmap(actspsbmp[useindx], actsps[useindx], hasAlpha);
    }

    if (gfxDriver->HasAcceleratedStretchAndFlip()) 
    {
      actspsbmp[useindx]->SetStretch(newwidth, newheight);
      actspsbmp[useindx]->SetFlippedLeftRight(isMirrored != 0);
      actspsbmp[useindx]->SetTint(tint_red, tint_green, tint_blue, (tint_amount * 256) / 100);

      if (tint_amount != 0)
      {
        if (tint_light == 0) // tint with 0 luminance, pass as 1 instead
          actspsbmp[useindx]->SetLightLevel(1);
        else if (tint_light < 250)
          actspsbmp[useindx]->SetLightLevel(tint_light);
        else
          actspsbmp[useindx]->SetLightLevel(0);
      }
      else if (light_level != 0)
        actspsbmp[useindx]->SetLightLevel((light_level * 25) / 10 + 256);
      else
        actspsbmp[useindx]->SetLightLevel(0);

    }

    set_eip(337);
    // disable alpha blending with tinted sprites (because the
    // alpha channel was lost in the tinting process)
    //if (((tint_level) && (tint_amount < 100)) || (light_level))
      //sppic = -1;
    add_to_sprite_list(actspsbmp[useindx], atxp + chin->pic_xoffs, atyp + chin->pic_yoffs, usebasel, chin->transparency, sppic);

    chin->actx=atxp+offsetx;
    chin->acty=atyp+offsety;
  }
}

// draw_screen_background: draws the background scene, all the interfaces
// and objects; basically, the entire screen
void draw_screen_background() {

  static int offsetxWas = -100, offsetyWas = -100;

  screen_reset = 1;

  if (is_complete_overlay) {
    // this is normally called as part of drawing sprites - clear it
    // here instead
    clear_draw_list();
    return;
  }

  // don't draw it before the room fades in
/*  if ((in_new_room > 0) & (game.color_depth > 1)) {
    clear(abuf);
    return;
    }*/
  set_eip(30);
  update_viewport();
  
  set_eip(31);

  if ((offsetx != offsetxWas) || (offsety != offsetyWas)) {
    invalidate_screen();

    offsetxWas = offsetx;
    offsetyWas = offsety;
  }

  if (play.screen_tint >= 0)
    invalidate_screen();

  if (gfxDriver->RequiresFullRedrawEachFrame())
  {
    if (roomBackgroundBmp == NULL) 
    {
      update_polled_stuff();
      roomBackgroundBmp = gfxDriver->CreateDDBFromBitmap(thisroom.ebscene[play.bg_frame], false, true);

      if ((walkBehindMethod == DrawAsSeparateSprite) && (walkBehindsCachedForBgNum != play.bg_frame))
      {
        update_walk_behind_images();
      }
    }
    else if (current_background_is_dirty)
    {
      update_polled_stuff();
      gfxDriver->UpdateDDBFromBitmap(roomBackgroundBmp, thisroom.ebscene[play.bg_frame], false);
      current_background_is_dirty = false;
      if (walkBehindMethod == DrawAsSeparateSprite)
      {
        update_walk_behind_images();
      }
    }
    gfxDriver->DrawSprite(-offsetx, -offsety, roomBackgroundBmp);
  }
  else
  {
    // the following line takes up to 50% of the game CPU time at
    // high resolutions and colour depths - if we can optimise it
    // somehow, significant performance gains to be had
    update_invalid_region_and_reset(-offsetx, -offsety, thisroom.ebscene[play.bg_frame], abuf);
  }

  clear_sprite_list();

  if ((debug_flags & DBG_NOOBJECTS)==0) {

    prepare_objects_for_drawing();

    prepare_characters_for_drawing ();

    if ((debug_flags & DBG_NODRAWSPRITES)==0) {
      set_eip(34);
      draw_sprite_list();
    }
  }
  set_eip(36);
}



// ============================================================================
// SCRIPTS
// ============================================================================

void get_script_name(ccInstance *rinst, char *curScrName) {
  if (rinst == NULL)
    strcpy (curScrName, "Not in a script");
  else if (rinst->instanceof == gamescript)
    strcpy (curScrName, "Global script");
  else if (rinst->instanceof == thisroom.compiled_script)
    sprintf (curScrName, "Room %d script", displayed_room);
  else
    strcpy (curScrName, "Unknown script");
}



// ============================================================================
// GRAPHICS - OVERLAYS
// ============================================================================

void get_overlay_position(int overlayidx, int *x, int *y) {
  int tdxp, tdyp;

  if (screenover[overlayidx].x == OVR_AUTOPLACE) {
    // auto place on character
    int charid = screenover[overlayidx].y;
    int charpic = views[game.chars[charid].view].loops[game.chars[charid].loop].frames[0].pic;
    
    tdyp = multiply_up_coordinate(game.chars[charid].get_effective_y()) - offsety - 5;
    if (charextra[charid].height<1)
      tdyp -= spriteheight[charpic];
    else
      tdyp -= charextra[charid].height;

    tdyp -= BMP_H(screenover[overlayidx].pic);
    if (tdyp < 5) tdyp=5;
    tdxp = (multiply_up_coordinate(game.chars[charid].x) - BMP_W(screenover[overlayidx].pic)/2) - offsetx;
    if (tdxp < 0) tdxp=0;

    if ((tdxp + BMP_W(screenover[overlayidx].pic)) >= scrnwid)
      tdxp = (scrnwid - BMP_W(screenover[overlayidx].pic)) - 1;
    if (game.chars[charid].room != displayed_room) {
      tdxp = scrnwid/2 - BMP_W(screenover[overlayidx].pic)/2;
      tdyp = scrnhit/2 - BMP_H(screenover[overlayidx].pic)/2;
    }
  }
  else {
    tdxp = screenover[overlayidx].x;
    tdyp = screenover[overlayidx].y;

    if (!screenover[overlayidx].positionRelativeToScreen)
    {
      tdxp -= offsetx;
      tdyp -= offsety;
    }
  }
  *x = tdxp;
  *y = tdyp;
}

void draw_fps()
{
  static IDriverDependantBitmap* ddb = NULL;
  static block fpsDisplay = NULL;

  if (fpsDisplay == NULL)
  {
    fpsDisplay = alw_create_bitmap_ex(final_col_dep, get_fixed_pixel_size(100), (wgetfontheight(FONT_SPEECH) + get_fixed_pixel_size(5)));
    fpsDisplay = gfxDriver->ConvertBitmapToSupportedColourDepth(fpsDisplay);
  }
  alw_clear_to_color(fpsDisplay, alw_bitmap_mask_color(fpsDisplay));
  block oldAbuf = abuf;
  abuf = fpsDisplay;
  char tbuffer[60];
  sprintf(tbuffer,"FPS: %d",fps);
  wtextcolor(14);
  wouttext_outline(1, 1, FONT_SPEECH, tbuffer);
  abuf = oldAbuf;

  if (ddb == NULL)
    ddb = gfxDriver->CreateDDBFromBitmap(fpsDisplay, false);
  else
    gfxDriver->UpdateDDBFromBitmap(ddb, fpsDisplay, false);

  int yp = scrnhit - BMP_H(fpsDisplay);

  gfxDriver->DrawSprite(1, yp, ddb);
  invalidate_sprite(1, yp, ddb);

  sprintf(tbuffer,"Loop %ld", loopcounter);
  draw_and_invalidate_text(get_fixed_pixel_size(250), yp, FONT_SPEECH,tbuffer);
}

// draw_screen_overlay: draws any stuff currently on top of the background,
// like a message box or popup interface
void draw_screen_overlay() {
  int gg;

  add_thing_to_draw(NULL, AGSE_PREGUIDRAW, 0, TRANS_RUN_PLUGIN, false);

  // draw overlays, except text boxes
  for (gg=0;gg<numscreenover;gg++) {
    // complete overlay draw in non-transparent mode
    if (screenover[gg].type == OVER_COMPLETE)
      add_thing_to_draw(screenover[gg].bmp, screenover[gg].x, screenover[gg].y, TRANS_OPAQUE, false);
    else if (screenover[gg].type != OVER_TEXTMSG) {
      int tdxp, tdyp;
      get_overlay_position(gg, &tdxp, &tdyp);
      add_thing_to_draw(screenover[gg].bmp, tdxp, tdyp, 0, screenover[gg].hasAlphaChannel);
    }
  }

  // Draw GUIs - they should always be on top of overlays like
  // speech background text
  set_eip(35);
  mouse_on_iface_button=-1;
  if (((debug_flags & DBG_NOIFACE)==0) && (displayed_room >= 0)) {
    int aa;

    if (playerchar->activeinv >= MAX_INV) {
      quit("!The player.activeinv variable has been corrupted, probably as a result\n"
       "of an incorrect assignment in the game script.");
    }
    if (playerchar->activeinv < 1) gui_inv_pic=-1;
    else gui_inv_pic=game.invinfo[playerchar->activeinv].pic;
/*    for (aa=0;aa<game.numgui;aa++) {
      if (guis[aa].on<1) continue;
      guis[aa].draw();
      guis[aa].poll();
      }*/
    set_eip(37);
    if (guis_need_update) {
      block abufwas = abuf;
      guis_need_update = 0;
      for (aa=0;aa<game.numgui;aa++) {
        if (guis[aa].on<1) continue;
        eip_guinum = aa;
        set_eip(370);
        alw_clear_to_color (guibg[aa], alw_bitmap_mask_color(guibg[aa]));
        abuf = guibg[aa];
        set_eip(372);
        guis[aa].draw_at(0,0);
        set_eip(373);

        bool isAlpha = false;
        if (guis[aa].is_alpha()) 
        {
          isAlpha = true;

          if ((game.options[OPT_NEWGUIALPHA] == 0) && (guis[aa].bgpic > 0))
          {
            // old-style (pre-3.0.2) GUI alpha rendering
            repair_alpha_channel(guibg[aa], spriteset[guis[aa].bgpic]);
          }
        }

        if (guibgbmp[aa] != NULL) 
        {
          gfxDriver->UpdateDDBFromBitmap(guibgbmp[aa], guibg[aa], isAlpha);
        }
        else
        {
          guibgbmp[aa] = gfxDriver->CreateDDBFromBitmap(guibg[aa], isAlpha);
        }
        set_eip(374);
      }
      abuf = abufwas;
    }
    set_eip(38);
    // Draw the GUIs
    for (gg = 0; gg < game.numgui; gg++) {
      aa = play.gui_draw_order[gg];
      if (guis[aa].on < 1) continue;

      // Don't draw GUI if "GUIs Turn Off When Disabled"
      if ((game.options[OPT_DISABLEOFF] == 3) &&
          (all_buttons_disabled > 0) &&
          (guis[aa].popup != POPUP_NOAUTOREM))
        continue;

      add_thing_to_draw(guibgbmp[aa], guis[aa].x, guis[aa].y, guis[aa].transparency, guis[aa].is_alpha());
      
      // only poll if the interface is enabled (mouseovers should not
      // work while in Wait state)
      if (IsInterfaceEnabled())
        guis[aa].poll();
    }
  }

  // draw text boxes (so that they appear over GUIs)
  for (gg=0;gg<numscreenover;gg++) 
  {
    if (screenover[gg].type == OVER_TEXTMSG) 
    {
      int tdxp, tdyp;
      get_overlay_position(gg, &tdxp, &tdyp);
      add_thing_to_draw(screenover[gg].bmp, tdxp, tdyp, 0, false);
    }
  }

  set_eip(1099);

  // *** Draw the Things To Draw List ***

  SpriteListEntry *thisThing;

  for (gg = 0; gg < thingsToDrawSize; gg++) {
    thisThing = &thingsToDrawList[gg];

    if (thisThing->bmp != NULL) {
      // mark the image's region as dirty
      invalidate_sprite(thisThing->x, thisThing->y, thisThing->bmp);
    }
    else if ((thisThing->transparent != TRANS_RUN_PLUGIN) &&
      (thisThing->bmp == NULL)) 
    {
      quit("Null pointer added to draw list");
    }
    
    if (thisThing->bmp != NULL)
    {
      if (thisThing->transparent <= 255)
      {
        thisThing->bmp->SetTransparency(thisThing->transparent);
      }

      gfxDriver->DrawSprite(thisThing->x, thisThing->y, thisThing->bmp);
    }
    else if (thisThing->transparent == TRANS_RUN_PLUGIN) 
    {
      // meta entry to run the plugin hook
      gfxDriver->DrawSprite(thisThing->x, thisThing->y, NULL);
    }
    else
      quit("Unknown entry in draw list");
  }

  clear_draw_list();

  set_eip(1100);


  if (display_fps) 
  {
    draw_fps();
  }
/*
  if (channels[SCHAN_SPEECH] != NULL) {
    
    char tbuffer[60];
    sprintf(tbuffer,"mpos: %d", channels[SCHAN_SPEECH]->get_pos_ms());
    write_log(tbuffer);
    int yp = scrnhit - (wgetfontheight(FONT_SPEECH) + 25 * symult);
    wtextcolor(14);
    draw_and_invalidate_text(1, yp, FONT_SPEECH,tbuffer);
  }*/

  if (play.recording) {
    // Flash "REC" while recording
    wtextcolor (12);
    //if ((loopcounter % (frames_per_second * 2)) > frames_per_second/2) {
      char tformat[30];
      sprintf (tformat, "REC %02d:%02d:%02d", replay_time / 3600, (replay_time % 3600) / 60, replay_time % 60);
      draw_and_invalidate_text(get_fixed_pixel_size(5), get_fixed_pixel_size(10), FONT_SPEECH, tformat);
    //}
  }
  else if (play.playback) {
    wtextcolor (10);
    char tformat[30];
    sprintf (tformat, "PLAY %02d:%02d:%02d", replay_time / 3600, (replay_time % 3600) / 60, replay_time % 60);

    draw_and_invalidate_text(get_fixed_pixel_size(5), get_fixed_pixel_size(10), FONT_SPEECH, tformat);
  }

  set_eip(1101);
}



// ============================================================================
// GRAPHICS DRIVER
// ============================================================================

bool GfxDriverNullSpriteCallback(int x, int y)
{
  if (displayed_room < 0)
  {
    // if no room loaded, various stuff won't be initialized yet
    return 1;
  }
  return (platform->RunPluginHooks(x, y) != 0);
}

void GfxDriverOnInitCallback(void *data)
{
  platform->RunPluginInitGfxHooks(gfxDriver->GetDriverID(), data);
}



// ============================================================================
// SCREEN STACK
// ============================================================================


void push_screen () {
  if (numOnStack >= 10)
    quit("!Too many push screen calls");

  screenstack[numOnStack] = abuf;
  numOnStack++;
}
void pop_screen() {
  if (numOnStack <= 0)
    quit("!Too many pop screen calls");
  numOnStack--;
  wsetscreen(screenstack[numOnStack]);
}



// ============================================================================
// SCREEN
// ============================================================================

// update_screen: copies the contents of the virtual screen to the actual
// screen, and draws the mouse cursor on.
void update_screen() {
  // cos hi-color doesn't fade in, don't draw it the first time
  if ((in_new_room > 0) & (game.color_depth > 1))
    return;
  gfxDriver->DrawSprite(AGSE_POSTSCREENDRAW, 0, NULL);

  update_animating_cursor();

  // draw the debug console, if appropriate
  if ((play.debug_mode > 0) && (display_console != 0)) 
  {
    int otextc = textcol, ypp = 1;
    int txtheight = wgetfontheight(0);
    int barheight = (DEBUG_CONSOLE_NUMLINES - 1) * txtheight + 4;

    if (debugConsoleBuffer == NULL)
      debugConsoleBuffer = alw_create_bitmap_ex(final_col_dep, scrnwid, barheight);

    push_screen();
    abuf = debugConsoleBuffer;
    wsetcolor(15);
    wbar (0, 0, scrnwid - 1, barheight);
    wtextcolor(16);
    for (int jj = first_debug_line; jj != last_debug_line; jj = (jj + 1) % DEBUG_CONSOLE_NUMLINES) {
      wouttextxy(1, ypp, 0, debug_line[jj].text);
      wouttextxy(scrnwid - get_fixed_pixel_size(40), ypp, 0, debug_line[jj].script);
      ypp += txtheight;
    }
    textcol = otextc;
    pop_screen();

    if (debugConsole == NULL)
      debugConsole = gfxDriver->CreateDDBFromBitmap(debugConsoleBuffer, false, true);
    else
      gfxDriver->UpdateDDBFromBitmap(debugConsole, debugConsoleBuffer, false);

    gfxDriver->DrawSprite(0, 0, debugConsole);
    invalidate_sprite(0, 0, debugConsole);
  }

  update_and_draw_mouse_on_screen();

  write_screen();

  wsetscreen(virtual_screen);

  if (!play.screen_is_faded_out) {
    // always update the palette, regardless of whether the plugin
    // vetos the screen update
    if (bg_just_changed) {
      setpal ();
      bg_just_changed = 0;
    }
  }

  screen_is_dirty = 0;
}



// ============================================================================
// VERSION
// ============================================================================

const char *get_engine_version() {
  return ACI_VERSION_TEXT;
}



// ============================================================================
// QUIT
// ============================================================================

void atexit_handler() {
  if (proper_exit==0) {
    sprintf(pexbuf,"\nError: the program has exited without requesting it.\n"
      "Program pointer: %+03d  (write this number down), ACI version " ACI_VERSION_TEXT "\n"
      "If you see a list of numbers above, please write them down and contact\n"
      "Chris Jones. Otherwise, note down any other information displayed.\n",
      our_eip);
    platform->DisplayAlert(pexbuf);
  }

  if (!(music_file == NULL))
    free(music_file);

  if (!(speech_file == NULL))
    free(speech_file);

  // Deliberately commented out, because chances are game_file_name
  // was not allocated on the heap, it points to argv[0] or
  // the gamefilenamebuf memory
  // It will get freed by the system anyway, leaving it in can
  // cause a crash on exit
  /*if (!(game_file_name == NULL))
    free(game_file_name);*/
}


bool send_exception_to_editor(char *qmsg)
{
#ifdef WINDOWS_VERSION
  want_exit = 0;
  // allow the editor to break with the error message
  const char *errorMsgToSend = qmsg;
  if (errorMsgToSend[0] == '?')
    errorMsgToSend++;

  if (editor_window_handle != NULL)
    SetForegroundWindow(editor_window_handle);

  if (!send_message_to_editor("ERROR", errorMsgToSend))
    return false;

  while ((check_for_messages_from_editor() == 0) && (want_exit == 0))
  {
    acaudio_update_mp3();
    platform->Delay(10);
  }
#endif
  return true;
}


// quit - exits the engine, shutting down everything gracefully
// The parameter is the message to print. If this message begins with
// an '!' character, then it is printed as a "contact game author" error.
// If it begins with a '|' then it is treated as a "thanks for playing" type
// message. If it begins with anything else, it is treated as an internal
// error.
// "!|" is a special code used to mean that the player has aborted (Alt+X)
void quit(const char*quitmsg) {
  int i;
  // Need to copy it in case it's from a plugin (since we're
  // about to free plugins)
  char qmsgbufr[STD_BUFFER_SIZE];
  strncpy(qmsgbufr, quitmsg, STD_BUFFER_SIZE);
  qmsgbufr[STD_BUFFER_SIZE - 1] = 0;
  char *qmsg = &qmsgbufr[0];

  bool handledErrorInEditor = false;

  if (editor_debugging_initialized)
  {
    if ((qmsg[0] == '!') && (qmsg[1] != '|'))
    {
      handledErrorInEditor = send_exception_to_editor(&qmsg[1]);
    }
    send_message_to_editor("EXIT");
    editor_debugger->Shutdown();
  }

  set_eip(9900);
  stop_recording();

  if (need_to_stop_cd)
    cd_manager(3,0);

  set_eip(9020);
  ccUnregisterAllObjects();

  set_eip(9019);
  platform->AboutToQuitGame();

  set_eip(9016);
  platform->ShutdownPlugins();

  if ((qmsg[0] == '|') && (check_dynamic_sprites_at_exit) && 
      (game.options[OPT_DEBUGMODE] != 0)) {
    // game exiting normally -- make sure the dynamic sprites
    // have been deleted
    for (i = 1; i < spriteset.elements; i++) {
      if (game.spriteflags[i] & SPF_DYNAMICALLOC)
        debug_log("Dynamic sprite %d was never deleted", i);
    }
  }

  // allegro_exit assumes screen is correct
  if (_old_screen)
    alw_screen = _old_screen;

  platform->FinishedUsingGraphicsMode();

  if (use_cdplayer)
    platform->ShutdownCDPlayer();

  set_eip(9917);
  game.options[OPT_CROSSFADEMUSIC] = 0;
  stopmusic();
  if (opts.mod_player)
    remove_mod_player();
  alw_remove_sound();
  set_eip(9901);

  char alertis[1500]="\0";
  if (qmsg[0]=='|') ; //qmsg++;
  else if (qmsg[0]=='!') { 
    qmsg++;

    if (qmsg[0] == '|')
      strcpy (alertis, "Abort key pressed.\n\n");
    else if (qmsg[0] == '?') {
      strcpy(alertis, "A fatal error has been generated by the script using the AbortGame function. Please contact the game author for support.\n\n");
      qmsg++;
    }
    else
      strcpy(alertis,"An error has occurred. Please contact the game author for support, as this "
        "is likely to be a scripting error and not a bug in AGS.\n"
        "(ACI version " ACI_VERSION_TEXT ")\n\n");

    strcat (alertis, get_cur_script(5) );

    if (qmsg[0] != '|')
      strcat(alertis,"\nError: ");
    else
      qmsg = "";
    }
  else if (qmsg[0] == '%') {
    qmsg++;

    sprintf(alertis, "A warning has been generated. This is not normally fatal, but you have selected "
      "to treat warnings as errors.\n"
      "(ACI version " ACI_VERSION_TEXT ")\n\n%s\n", get_cur_script(5));
  }
  else strcpy(alertis,"An internal error has occurred. Please note down the following information.\n"
   "If the problem persists, post the details on the AGS Technical Forum.\n"
   "(ACI version " ACI_VERSION_TEXT ")\n"
   "\nError: ");

  shutdown_font_renderer();
  set_eip(9902);

  // close graphics mode (Win) or return to text mode (DOS)
  if (_sub_screen) {
    alw_destroy_bitmap(_sub_screen);
    _sub_screen = NULL;
  }
  
  set_eip(9907);

  close_translation();

  set_eip(9908);

  // Release the display mode (and anything dependant on the window)
  if (gfxDriver != NULL)
    gfxDriver->UnInit();

  // Tell Allegro that we are no longer in graphics mode
  alw_set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);

  // successful exit displays no messages (because Windoze closes the dos-box
  // if it is empty).
  if (qmsg[0]=='|') ;
  else if (!handledErrorInEditor)
  {
    // Display the message (at this point the window still exists)
    sprintf(pexbuf,"%s\n",qmsg);
    strcat(alertis,pexbuf);
    platform->DisplayAlert(alertis);
  }

  // remove the game window
  alw_allegro_exit();

  if (gfxDriver != NULL)
  {
    delete gfxDriver;
    gfxDriver = NULL;
  }
  
  platform->PostAllegroExit();

  set_eip(9903);

  // wipe all the interaction structs so they don't de-alloc the children twice
  memset (&roomstats[0], 0, sizeof(RoomStatus) * MAX_ROOMS);
  memset (&troom, 0, sizeof(RoomStatus));

/*  _CrtMemState memstart;
  _CrtMemCheckpoint(&memstart);
  _CrtMemDumpStatistics( &memstart );*/

/*  // print the FPS if there wasn't an error
  if ((play.debug_mode!=0) && (qmsg[0]=='|'))
    printf("Last cycle fps: %d\n",fps);*/
  alw_al_ffblk	dfb;
  int	dun = alw_al_findfirst("~ac*.tmp",&dfb,FA_SEARCH);
  while (!dun) {
    unlink(dfb.name);
    dun = alw_al_findnext(&dfb);
  }
  alw_al_findclose (&dfb);

  proper_exit=1;

  write_log_debug("***** ENGINE HAS SHUTDOWN");

  set_eip(9904);
  exit(EXIT_NORMAL);
}

extern "C" {
	void quit_c(char*msg) {
		quit(msg);
		  }
}



// ============================================================================
// TEXT
// ============================================================================

void set_default_glmsg (int msgnum, const char* val) {
  if (game.messages[msgnum-500] == NULL) {
    game.messages[msgnum-500] = (char*)malloc (strlen(val)+5);
    strcpy (game.messages[msgnum-500], val);
  }
}

void split_lines_rightleft (char *todis, int wii, int fonnt) {
  // start on the last character
  char *thisline = todis + strlen(todis) - 1;
  char prevlwas, *prevline = NULL;
  // work backwards
  while (thisline >= todis) {

    int needBreak = 0;
    if (thisline <= todis) 
      needBreak = 1;
    // ignore \[ sequence
    else if ((thisline > todis) && (thisline[-1] == '\\')) { }
    else if (thisline[0] == '[') {
      needBreak = 1;
      thisline++;
    }
    else if (wgettextwidth_compensate(thisline, fonnt) >= wii) {
      // go 'back' to the nearest word
      while ((thisline[0] != ' ') && (thisline[0] != 0))
        thisline++;

      if (thisline[0] == 0)
        quit("!Single word too wide for window");

      thisline++;
      needBreak = 1;
    }

    if (needBreak) {
      strcpy(lines[numlines], thisline);
      removeBackslashBracket(lines[numlines]);
      numlines++;
      if (prevline) {
        prevline[0] = prevlwas;
      }
      thisline--;
      prevline = thisline;
      prevlwas = prevline[0];
      prevline[0] = 0;
    }

    thisline--;
  }
  if (prevline)
    prevline[0] = prevlwas;
}



char *reverse_text(char *text) {
  int stlen = strlen(text), rr;
  char *backwards = (char*)malloc(stlen + 1);
  for (rr = 0; rr < stlen; rr++)
    backwards[rr] = text[(stlen - rr) - 1];
  backwards[stlen] = 0;
  return backwards;
}

void wouttext_reverseifnecessary(int x, int y, int font, char *text) {
  char *backwards = NULL;
  char *otext = text;
  if (game.options[OPT_RIGHTLEFTWRITE]) {
    backwards = reverse_text(text);
    otext = backwards;
  }

  wouttext_outline(x, y, font, otext);

  if (backwards)
    free(backwards);
}

void break_up_text_into_lines(int wii,int fonnt,char*todis) {
  if (fonnt == -1)
    fonnt = play.normal_font;

//  char sofar[100];
  if (todis[0]=='&') {
    while ((todis[0]!=' ') & (todis[0]!=0)) todis++;
    if (todis[0]==' ') todis++;
    }
  numlines=0;
  longestline=0;

  // Don't attempt to display anything if the width is tiny
  if (wii < 3)
    return;

  int rr;

  if (game.options[OPT_RIGHTLEFTWRITE] == 0)
  {
    split_lines_leftright(todis, wii, fonnt);
  }
  else {
    // Right-to-left just means reverse the text then
    // write it as normal
    char *backwards = reverse_text(todis);
    split_lines_rightleft (backwards, wii, fonnt);
    free(backwards);
  }

  for (rr=0;rr<numlines;rr++) {
    if (wgettextwidth_compensate(lines[rr],fonnt) > longestline)
      longestline = wgettextwidth_compensate(lines[rr],fonnt);
  }
}



// ============================================================================
// SOUND
// ============================================================================

void stop_all_sound_and_music() 
{
  int a;
  stopmusic();
  // make sure it doesn't start crossfading when it comes back
  crossFading = 0;
  // any ambient sound will be aborted
  for (a = 0; a <= MAX_SOUND_CHANNELS; a++)
    stop_and_destroy_channel(a);
}

void shutdown_sound() 
{
  stop_all_sound_and_music();

  if (opts.mod_player)
    remove_mod_player();
  alw_remove_sound();
}



// ============================================================================
// PLAYER
// ============================================================================

void setup_player_character(int charid) {
  game.playercharacter = charid;
  playerchar = &game.chars[charid];
  _sc_PlayerCharPtr = ccGetObjectHandleFromAddress((char*)playerchar);
}














// ============================================================================
// DYN OBJ GLOBALS
// ============================================================================

CCGUIObject ccDynamicGUIObject;
CCCharacter ccDynamicCharacter;
CCHotspot   ccDynamicHotspot;
CCRegion    ccDynamicRegion;
CCInventory ccDynamicInv;
CCGUI       ccDynamicGUI;
CCObject    ccDynamicObject;
CCDialog    ccDynamicDialog;
ScriptString myScriptStringImpl;
ScriptDialogOptionsRendering ccDialogOptionsRendering;
ScriptDrawingSurface* dialogOptionsRenderingSurface;



// ============================================================================
// SCRIPT DESERIALISER
// ============================================================================

struct AGSDeSerializer : ICCObjectReader {

  virtual void Unserialize(int index, const char *objectType, const char *serializedData, int dataSize) {
    if (strcmp(objectType, "GUIObject") == 0) {
      ccDynamicGUIObject.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "Character") == 0) {
      ccDynamicCharacter.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "Hotspot") == 0) {
      ccDynamicHotspot.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "Region") == 0) {
      ccDynamicRegion.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "Inventory") == 0) {
      ccDynamicInv.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "Dialog") == 0) {
      ccDynamicDialog.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "GUI") == 0) {
      ccDynamicGUI.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "Object") == 0) {
      ccDynamicObject.Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "String") == 0) {
      ScriptString *scf = new ScriptString();
      scf->Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "File") == 0) {
      // files cannot be restored properly -- so just recreate
      // the object; attempting any operations on it will fail
      sc_File *scf = new sc_File();
      ccRegisterUnserializedObject(index, scf, scf);
    }
    else if (strcmp(objectType, "Overlay") == 0) {
      ScriptOverlay *scf = new ScriptOverlay();
      scf->Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "DateTime") == 0) {
      ScriptDateTime *scf = new ScriptDateTime();
      scf->Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "ViewFrame") == 0) {
      ScriptViewFrame *scf = new ScriptViewFrame();
      scf->Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "DynamicSprite") == 0) {
      ScriptDynamicSprite *scf = new ScriptDynamicSprite();
      scf->Unserialize(index, serializedData, dataSize);
    }
    else if (strcmp(objectType, "DrawingSurface") == 0) {
      ScriptDrawingSurface *sds = new ScriptDrawingSurface();
      sds->Unserialize(index, serializedData, dataSize);

      if (sds->isLinkedBitmapOnly)
      {
        dialogOptionsRenderingSurface = sds;
      }
    }
    else if (strcmp(objectType, "DialogOptionsRendering") == 0)
    {
      ccDialogOptionsRendering.Unserialize(index, serializedData, dataSize);
    }
    else if (!unserialize_audio_script_object(index, objectType, serializedData, dataSize)) 
    {
      // check if the type is read by a plugin
      for (int ii = 0; ii < numPluginReaders; ii++) {
        if (strcmp(objectType, pluginReaders[ii].type) == 0) {
          pluginReaders[ii].reader->Unserialize(index, serializedData, dataSize);
          return;
        }
      }
      quitprintf("Unserialise: unknown object type: '%s'", objectType);
    }
  }

};

AGSDeSerializer ccUnserializer;



// ============================================================================
// GUI
// ============================================================================

void export_gui_controls(int ee) {

  for (int ff = 0; ff < guis[ee].numobjs; ff++) {
    if (guis[ee].objs[ff]->scriptName[0] != 0)
      ccAddExternalSymbol(guis[ee].objs[ff]->scriptName, guis[ee].objs[ff]);

    ccRegisterManagedObject(guis[ee].objs[ff], &ccDynamicGUIObject);
  }
}

void unexport_gui_controls(int ee) {

  for (int ff = 0; ff < guis[ee].numobjs; ff++) {
    if (guis[ee].objs[ff]->scriptName[0] != 0)
      ccRemoveExternalSymbol(guis[ee].objs[ff]->scriptName);

    if (!ccUnRegisterManagedObject(guis[ee].objs[ff]))
      quit("unable to unregister guicontrol object");
  }
}



// ============================================================================
// SCRIPT
// ============================================================================

int create_global_script() {
  ccSetOption(SCOPT_AUTOIMPORT, 1);
  for (int kk = 0; kk < numScriptModules; kk++) {
    moduleInst[kk] = ccCreateInstance(scriptModules[kk]);
    if (moduleInst[kk] == NULL)
      return -3;
    // create a forked instance for rep_exec_always
    moduleInstFork[kk] = ccForkInstance(moduleInst[kk]);
    if (moduleInstFork[kk] == NULL)
      return -3;

    moduleRepExecAddr[kk] = ccGetSymbolAddr(moduleInst[kk], REP_EXEC_NAME);
  }
  gameinst = ccCreateInstance(gamescript);
  if (gameinst == NULL)
    return -3;
  // create a forked instance for rep_exec_always
  gameinstFork = ccForkInstance(gameinst);
  if (gameinstFork == NULL)
    return -3;

  if (dialogScriptsScript != NULL)
  {
    dialogScriptsInst = ccCreateInstance(dialogScriptsScript);
    if (dialogScriptsInst == NULL)
      return -3;
  }

  ccSetOption(SCOPT_AUTOIMPORT, 0);
  return 0;
}



// ============================================================================
// GRAPHICS - VIEWS/PIXELS
// ============================================================================

void allocate_memory_for_views(int viewCount)
{
  views = (ViewStruct*)calloc(sizeof(ViewStruct) * viewCount, 1);
  game.viewNames = (char**)malloc(sizeof(char*) * viewCount);
  game.viewNames[0] = (char*)malloc(MAXVIEWNAMELENGTH * viewCount);

  for (int i = 1; i < viewCount; i++)
  {
    game.viewNames[i] = game.viewNames[0] + (MAXVIEWNAMELENGTH * i);
  }
}

int adjust_pixel_size_for_loaded_data(int size, int filever)
{
  if (filever < 37)
  {
    return multiply_up_coordinate(size);
  }
  return size;
}

void adjust_pixel_sizes_for_loaded_data(int *x, int *y, int filever)
{
  x[0] = adjust_pixel_size_for_loaded_data(x[0], filever);
  y[0] = adjust_pixel_size_for_loaded_data(y[0], filever);
}

void adjust_sizes_for_resolution(int filever)
{
  int ee;
  for (ee = 0; ee < game.numcursors; ee++) 
  {
    game.mcurs[ee].hotx = adjust_pixel_size_for_loaded_data(game.mcurs[ee].hotx, filever);
    game.mcurs[ee].hoty = adjust_pixel_size_for_loaded_data(game.mcurs[ee].hoty, filever);
  }

  for (ee = 0; ee < game.numinvitems; ee++) 
  {
    adjust_pixel_sizes_for_loaded_data(&game.invinfo[ee].hotx, &game.invinfo[ee].hoty, filever);
  }

  for (ee = 0; ee < game.numgui; ee++) 
  {
    GUIMain*cgp=&guis[ee];
    adjust_pixel_sizes_for_loaded_data(&cgp->x, &cgp->y, filever);
    if (cgp->wid < 1)
      cgp->wid = 1;
    if (cgp->hit < 1)
      cgp->hit = 1;
    // Temp fix for older games
    if (cgp->wid == BASEWIDTH - 1)
      cgp->wid = BASEWIDTH;

    adjust_pixel_sizes_for_loaded_data(&cgp->wid, &cgp->hit, filever);
    
    cgp->popupyp = adjust_pixel_size_for_loaded_data(cgp->popupyp, filever);

    for (ff = 0; ff < cgp->numobjs; ff++) 
    {
      adjust_pixel_sizes_for_loaded_data(&cgp->objs[ff]->x, &cgp->objs[ff]->y, filever);
      adjust_pixel_sizes_for_loaded_data(&cgp->objs[ff]->wid, &cgp->objs[ff]->hit, filever);
      cgp->objs[ff]->activated=0;
    }
  }

  if ((filever >= 37) && (game.options[OPT_NATIVECOORDINATES] == 0) &&
      (game.default_resolution > 2))
  {
    // New 3.1 format game file, but with Use Native Coordinates off

    for (ee = 0; ee < game.numcharacters; ee++) 
    {
      game.chars[ee].x /= 2;
      game.chars[ee].y /= 2;
    }

    for (ee = 0; ee < numguiinv; ee++)
    {
      guiinv[ee].itemWidth /= 2;
      guiinv[ee].itemHeight /= 2;
    }
  }

}



// ============================================================================
// MAIN - LOAD GAME
// ============================================================================

int load_game_file() {
  int ee, bb;
  char teststr[31];

  game_paused = 0;  // reset the game paused flag
  ifacepopped = -1;

  FILE*iii = clibfopen("game28.dta","rb");
  if (iii==NULL) return -1;

  set_eip(-18);
  setup_script_exports();

  set_eip(-16);

  teststr[30]=0;
  fread(&teststr[0],30,1,iii);
  int filever=getw(iii);
  if (filever < 42) {
    fclose(iii);
    return -2; 
  }
  int engineverlen = getw(iii);
  char engineneeds[20];
  // MACPORT FIX 13/6/5: switch 'size' and 'nmemb' so it doesn't treat the string as an int
  fread(&engineneeds[0], sizeof(char), engineverlen, iii);
  engineneeds[engineverlen] = 0;

  if (filever > GAME_FILE_VERSION) {
    platform->DisplayAlert("This game requires a newer version of AGS (%s). It cannot be run.", engineneeds);
    fclose(iii);
    return -2;
  }

  if (strcmp (engineneeds, ACI_VERSION_TEXT) > 0)
    platform->DisplayAlert("This game requires a newer version of AGS (%s). It may not run correctly.", engineneeds);
  
  {
    int major, minor;
    sscanf(engineneeds, "%d.%d", &major, &minor);
    engineNeedsAsInt = 100*major + minor;
  }

  loaded_game_file_version = filever;

  game.charScripts = NULL;
  game.invScripts = NULL;
  memset(&game.spriteflags[0], 0, MAX_SPRITES);

#ifndef ALLEGRO_BIG_ENDIAN
  fread(&game, sizeof (GameSetupStructBase), 1, iii);
#else
  GameSetupStructBase *gameBase = (GameSetupStructBase *) &game;
  gameBase->ReadFromFile(iii);
#endif
    
  if (game.numfonts > MAX_FONTS)
    quit("!This game requires a newer version of AGS. Too many fonts for this version to handle.");

  fread(&game.guid[0], 1, MAX_GUID_LENGTH, iii);
  fread(&game.saveGameFileExtension[0], 1, MAX_SG_EXT_LENGTH, iii);
  fread(&game.saveGameFolderName[0], 1, MAX_SG_FOLDER_LEN, iii);

  if (game.saveGameFileExtension[0] != 0)
    sprintf(saveGameSuffix, ".%s", game.saveGameFileExtension);
  else
    saveGameSuffix[0] = 0;

  fread(&game.fontflags[0], 1, game.numfonts, iii);
  fread(&game.fontoutline[0], 1, game.numfonts, iii);

  int numToRead = getw(iii);
  if (numToRead > MAX_SPRITES) {
    quit("Too many sprites; need newer AGS version");
  }
  fread(&game.spriteflags[0], 1, numToRead, iii);
#ifndef ALLEGRO_BIG_ENDIAN
  fread(&game.invinfo[0], sizeof(InventoryItemInfo), game.numinvitems, iii);
#else
  for (int iteratorCount = 0; iteratorCount < game.numinvitems; ++iteratorCount)
  {
    game.invinfo[iteratorCount].ReadFromFile(iii);
  }
#endif

  if (game.numcursors > MAX_CURSOR)
    quit("Too many cursors: need newer AGS version");
#ifndef ALLEGRO_BIG_ENDIAN
  fread(&game.mcurs[0], sizeof(MouseCursor), game.numcursors, iii);
#else
  for (int iteratorCount = 0; iteratorCount < game.numcursors; ++iteratorCount)
  {
    game.mcurs[iteratorCount].ReadFromFile(iii);
  }
#endif

  numGlobalVars = 0;
  game.charScripts = new InteractionScripts*[game.numcharacters];
  game.invScripts = new InteractionScripts*[game.numinvitems];
  for (bb = 0; bb < game.numcharacters; bb++) {
    game.charScripts[bb] = new InteractionScripts();
    deserialize_interaction_scripts(iii, game.charScripts[bb]);
  }
  for (bb = 1; bb < game.numinvitems; bb++) {
    game.invScripts[bb] = new InteractionScripts();
    deserialize_interaction_scripts(iii, game.invScripts[bb]);
  }

  if (game.dict != NULL) {
    game.dict = (WordsDictionary*)malloc(sizeof(WordsDictionary));
    read_dictionary (game.dict, iii);
  }

  if (game.compiled_script == NULL)
    quit("No global script in game; data load error");

  gamescript = fread_script(iii);
  if (gamescript == NULL)
    quit("Global script load failed; need newer version?");

  dialogScriptsScript = fread_script(iii);
  if (dialogScriptsScript == NULL)
    quit("Dialog scripts load failed; need newer version?");

  numScriptModules = getw(iii);
  if (numScriptModules > MAX_SCRIPT_MODULES)
    quit("too many script modules; need newer version?");

  for (bb = 0; bb < numScriptModules; bb++) {
    scriptModules[bb] = fread_script(iii);
    if (scriptModules[bb] == NULL)
      quit("Script module load failure; need newer version?");
    moduleInst[bb] = NULL;
    moduleInstFork[bb] = NULL;
    moduleRepExecAddr[bb] = NULL;
  }
  
  set_eip(-15);

  charextra = (CharacterExtras*)calloc(game.numcharacters, sizeof(CharacterExtras));
  mls = (MoveList*)calloc(game.numcharacters + MAX_INIT_SPR + 1, sizeof(MoveList));
  actSpsCount = game.numcharacters + MAX_INIT_SPR + 2;
  actsps = (block*)calloc(actSpsCount, sizeof(block));
  actspsbmp = (IDriverDependantBitmap**)calloc(actSpsCount, sizeof(IDriverDependantBitmap*));
  actspswb = (block*)calloc(actSpsCount, sizeof(block));
  actspswbbmp = (IDriverDependantBitmap**)calloc(actSpsCount, sizeof(IDriverDependantBitmap*));
  actspswbcache = (CachedActSpsData*)calloc(actSpsCount, sizeof(CachedActSpsData));
  game.charProps = (CustomProperties*)calloc(game.numcharacters, sizeof(CustomProperties));

  allocate_memory_for_views(game.numviews);

  for (int iteratorCount = 0; iteratorCount < game.numviews; ++iteratorCount)
  {
    views[iteratorCount].ReadFromFile(iii);
  }

  set_eip(-14);

  game.chars=(CharacterInfo*)calloc(1,sizeof(CharacterInfo)*game.numcharacters+5);
#ifndef ALLEGRO_BIG_ENDIAN
  fread(&game.chars[0],sizeof(CharacterInfo),game.numcharacters,iii);
#else
  for (int iteratorCount = 0; iteratorCount < game.numcharacters; ++iteratorCount)
  {
    game.chars[iteratorCount].ReadFromFile(iii);
  }
#endif
  charcache = (CharacterCache*)calloc(1,sizeof(CharacterCache)*game.numcharacters+5);

  fread(&game.lipSyncFrameLetters[0][0], MAXLIPSYNCFRAMES, 50, iii);

  for (ee=0;ee<MAXGLOBALMES;ee++) {
    if (game.messages[ee]==NULL) continue;
    game.messages[ee]=(char*)malloc(500);
    read_string_decrypt(iii, game.messages[ee]);
  }
  set_default_glmsg (983, "Sorry, not now.");
  set_default_glmsg (984, "Restore");
  set_default_glmsg (985, "Cancel");
  set_default_glmsg (986, "Select a game to restore:");
  set_default_glmsg (987, "Save");
  set_default_glmsg (988, "Type a name to save as:");
  set_default_glmsg (989, "Replace");
  set_default_glmsg (990, "The save directory is full. You must replace an existing game:");
  set_default_glmsg (991, "Replace:");
  set_default_glmsg (992, "With:");
  set_default_glmsg (993, "Quit");
  set_default_glmsg (994, "Play");
  set_default_glmsg (995, "Are you sure you want to quit?");
  set_eip(-13);

  dialog=(DialogTopic*)malloc(sizeof(DialogTopic)*game.numdialog+5);

#ifndef ALLEGRO_BIG_ENDIAN
  fread(&dialog[0],sizeof(DialogTopic),game.numdialog,iii);
#else
  for (int iteratorCount = 0; iteratorCount < game.numdialog; ++iteratorCount)
  {
    dialog[iteratorCount].ReadFromFile(iii);
  }
#endif

  read_gui(iii,guis,&game, &guis);

  for (bb = 0; bb < numguilabels; bb++) {
    // labels are not clickable by default
    guilabels[bb].SetClickable(false);
  }

  play.gui_draw_order = (int*)calloc(game.numgui * sizeof(int), 1);

  platform->ReadPluginsFromDisk(iii);

  if (game.propSchema.UnSerialize(iii))
    quit("load room: unable to deserialize prop schema");

  int errors = 0;

  for (bb = 0; bb < game.numcharacters; bb++)
    errors += game.charProps[bb].UnSerialize (iii);
  for (bb = 0; bb < game.numinvitems; bb++)
    errors += game.invProps[bb].UnSerialize (iii);

  if (errors > 0)
    quit("LoadGame: errors encountered reading custom props");

  for (bb = 0; bb < game.numviews; bb++)
    fgetstring_limit(game.viewNames[bb], iii, MAXVIEWNAMELENGTH);

  for (bb = 0; bb < game.numinvitems; bb++)
    fgetstring_limit(game.invScriptNames[bb], iii, MAX_SCRIPT_NAME_LEN);

  for (bb = 0; bb < game.numdialog; bb++)
    fgetstring_limit(game.dialogScriptNames[bb], iii, MAX_SCRIPT_NAME_LEN);

  if (filever >= 41)
  {
    game.audioClipTypeCount = getw(iii);

    if (game.audioClipTypeCount > MAX_AUDIO_TYPES)
      quit("LoadGame: too many audio types");

    game.audioClipTypes = (AudioClipType*)malloc(game.audioClipTypeCount * sizeof(AudioClipType));
    fread(&game.audioClipTypes[0], sizeof(AudioClipType), game.audioClipTypeCount, iii);
    game.audioClipCount = getw(iii);
    game.audioClips = (ScriptAudioClip*)malloc(game.audioClipCount * sizeof(ScriptAudioClip));
    fread(&game.audioClips[0], sizeof(ScriptAudioClip), game.audioClipCount, iii);
    play.score_sound = getw(iii);
  }
  else
  {
    game.audioClipCount = 0;
    play.score_sound = -1;
  }

  if ((filever >= 36) && (game.options[OPT_DEBUGMODE] != 0))
  {
    game.roomCount = getw(iii);
    game.roomNumbers = (int*)malloc(game.roomCount * sizeof(int));
    game.roomNames = (char**)malloc(game.roomCount * sizeof(char*));
    for (bb = 0; bb < game.roomCount; bb++)
    {
      game.roomNumbers[bb] = getw(iii);
      fgetstring_limit(pexbuf, iii, sizeof(pexbuf));
      game.roomNames[bb] = (char*)malloc(strlen(pexbuf) + 1);
      strcpy(game.roomNames[bb], pexbuf);
    }
  }
  else
  {
    game.roomCount = 0;
  }

  fclose(iii);

  update_gui_zorder();

  if (game.numfonts == 0)
    return -2;  // old v2.00 version

  set_eip(-11);
  characterScriptObjNames = (char**)malloc(sizeof(char*) * game.numcharacters);

  for (ee=0;ee<game.numcharacters;ee++) {
    game.chars[ee].walking = 0;
    game.chars[ee].animating = 0;
    game.chars[ee].pic_xoffs = 0;
    game.chars[ee].pic_yoffs = 0;
    game.chars[ee].blinkinterval = 140;
    game.chars[ee].blinktimer = game.chars[ee].blinkinterval;
    game.chars[ee].index_id = ee;
    game.chars[ee].blocking_width = 0;
    game.chars[ee].blocking_height = 0;
    game.chars[ee].prevroom = -1;
    game.chars[ee].loop = 0;
    game.chars[ee].frame = 0;
    game.chars[ee].walkwait = -1;
    ccRegisterManagedObject(&game.chars[ee], &ccDynamicCharacter);

    // export the character's script object
    characterScriptObjNames[ee] = (char*)malloc(strlen(game.chars[ee].scrname) + 5);
    strcpy(characterScriptObjNames[ee], game.chars[ee].scrname);

    ccAddExternalSymbol(characterScriptObjNames[ee], &game.chars[ee]);
  }

  for (ee = 0; ee < MAX_HOTSPOTS; ee++) {
    scrHotspot[ee].id = ee;
    scrHotspot[ee].reserved = 0;

    ccRegisterManagedObject(&scrHotspot[ee], &ccDynamicHotspot);
  }

  for (ee = 0; ee < MAX_REGIONS; ee++) {
    scrRegion[ee].id = ee;
    scrRegion[ee].reserved = 0;

    ccRegisterManagedObject(&scrRegion[ee], &ccDynamicRegion);
  }

  for (ee = 0; ee < MAX_INV; ee++) {
    scrInv[ee].id = ee;
    scrInv[ee].reserved = 0;

    ccRegisterManagedObject(&scrInv[ee], &ccDynamicInv);

    if (game.invScriptNames[ee][0] != 0)
      ccAddExternalSymbol(game.invScriptNames[ee], &scrInv[ee]);
  }

  for (ee = 0; ee < game.numdialog; ee++) {
    scrDialog[ee].id = ee;
    scrDialog[ee].reserved = 0;

    ccRegisterManagedObject(&scrDialog[ee], &ccDynamicDialog);

    if (game.dialogScriptNames[ee][0] != 0)
      ccAddExternalSymbol(game.dialogScriptNames[ee], &scrDialog[ee]);
  }

  scrGui = (ScriptGUI*)malloc(sizeof(ScriptGUI) * game.numgui);
  for (ee = 0; ee < game.numgui; ee++) {
    scrGui[ee].gui = NULL;
    scrGui[ee].id = -1;
  }

  guiScriptObjNames = (char**)malloc(sizeof(char*) * game.numgui);

  for (ee=0;ee<game.numgui;ee++) {
    guis[ee].rebuild_array();
    if ((guis[ee].popup == POPUP_NONE) || (guis[ee].popup == POPUP_NOAUTOREM))
      guis[ee].on = 1;
    else
      guis[ee].on = 0;

    // export all the GUI's controls
    export_gui_controls(ee);

    // copy the script name to its own memory location
    // because ccAddExtSymbol only keeps a reference
    guiScriptObjNames[ee] = (char*)malloc(21);
    strcpy(guiScriptObjNames[ee], guis[ee].name);

    scrGui[ee].gui = &guis[ee];
    scrGui[ee].id = ee;

    ccAddExternalSymbol(guiScriptObjNames[ee], &scrGui[ee]);
    ccRegisterManagedObject(&scrGui[ee], &ccDynamicGUI);
  }

  //ccRegisterManagedObject(&dummygui, NULL);
  //ccRegisterManagedObject(&dummyguicontrol, NULL);

  set_eip(-22);
  for (ee=0;ee<game.numfonts;ee++) 
  {
    int fontsize = game.fontflags[ee] & FFLG_SIZEMASK;
    if (fontsize == 0)
      fontsize = 8;

    if ((game.options[OPT_NOSCALEFNT] == 0) && (game.default_resolution > 2))
      fontsize *= 2;

    if (!wloadfont_size(ee, fontsize))
      quitprintf("Unable to load font %d, no renderer could load a matching file", ee);
  }

  wtexttransparent(TEXTFG);
  play.fade_effect=game.options[OPT_FADETYPE];

  set_eip(-21);

  for (ee = 0; ee < MAX_INIT_SPR; ee++) {
    ccRegisterManagedObject(&scrObj[ee], &ccDynamicObject);
  }

  register_audio_script_objects();

  ccRegisterManagedObject(&ccDialogOptionsRendering, &ccDialogOptionsRendering);
  
  dialogOptionsRenderingSurface = new ScriptDrawingSurface();
  dialogOptionsRenderingSurface->isLinkedBitmapOnly = true;
  long dorsHandle = ccRegisterManagedObject(dialogOptionsRenderingSurface, dialogOptionsRenderingSurface);
  ccAddObjectReference(dorsHandle);

  ccAddExternalSymbol("character",&game.chars[0]);
  setup_player_character(game.playercharacter);
  ccAddExternalSymbol("player", &_sc_PlayerCharPtr);
  ccAddExternalSymbol("object",&scrObj[0]);
  ccAddExternalSymbol("gui",&scrGui[0]);
  ccAddExternalSymbol("hotspot",&scrHotspot[0]);
  ccAddExternalSymbol("region",&scrRegion[0]);
  ccAddExternalSymbol("inventory",&scrInv[0]);
  ccAddExternalSymbol("dialog", &scrDialog[0]);

  set_eip(-23);
  platform->StartPlugins();

  set_eip(-24);
  ccSetScriptAliveTimer(150000);
  ccSetStringClassImpl(&myScriptStringImpl);
  if (create_global_script())
    return -3;

  return 0;
}



// ============================================================================
// MAIN - UNLOAD GAME
// ============================================================================

void free_do_once_tokens() 
{
  for (int i = 0; i < play.num_do_once_tokens; i++)
  {
    free(play.do_once_tokens[i]);
  }
  if (play.do_once_tokens != NULL) 
  {
    free(play.do_once_tokens);
    play.do_once_tokens = NULL;
  }
  play.num_do_once_tokens = 0;
}


// Free all the memory associated with the game
void unload_game_file() {
  int bb, ee;

  for (bb = 0; bb < game.numcharacters; bb++) {
    if (game.charScripts != NULL)
      delete game.charScripts[bb];

    if (game.intrChar != NULL)
    {
      if (game.intrChar[bb] != NULL)
        delete game.intrChar[bb];
      game.intrChar[bb] = NULL;
    }
    free(characterScriptObjNames[bb]);
    game.charProps[bb].reset();
  }
  if (game.intrChar != NULL)
  {
    free(game.intrChar);
    game.intrChar = NULL;
  }
  free(characterScriptObjNames);
  free(charextra);
  free(mls);
  free(actsps);
  free(actspsbmp);
  free(actspswb);
  free(actspswbbmp);
  free(actspswbcache);
  free(game.charProps);

  for (bb = 1; bb < game.numinvitems; bb++) {
    if (game.invScripts != NULL)
      delete game.invScripts[bb];
    if (game.intrInv[bb] != NULL)
      delete game.intrInv[bb];
    game.intrInv[bb] = NULL;
  }

  if (game.charScripts != NULL)
  {
    delete game.charScripts;
    delete game.invScripts;
    game.charScripts = NULL;
    game.invScripts = NULL;
  }

  if (game.dict != NULL) {
    game.dict->free_memory();
    free (game.dict);
    game.dict = NULL;
  }

  if ((gameinst != NULL) && (gameinst->pc != 0))
    quit("Error: unload_game called while script still running");
    //ccAbortAndDestroyInstance (gameinst);
  else {
    ccFreeInstance(gameinstFork);
    ccFreeInstance(gameinst);
    gameinstFork = NULL;
    gameinst = NULL;
  }

  ccFreeScript (gamescript);
  gamescript = NULL;

  if ((dialogScriptsInst != NULL) && (dialogScriptsInst->pc != 0))
    quit("Error: unload_game called while dialog script still running");
  else if (dialogScriptsInst != NULL)
  {
    ccFreeInstance(dialogScriptsInst);
    dialogScriptsInst = NULL;
  }

  if (dialogScriptsScript != NULL)
  {
    ccFreeScript(dialogScriptsScript);
    dialogScriptsScript = NULL;
  }

  for (ee = 0; ee < numScriptModules; ee++) {
    ccFreeInstance(moduleInstFork[ee]);
    ccFreeInstance(moduleInst[ee]);
    ccFreeScript(scriptModules[ee]);
  }
  numScriptModules = 0;

  if (game.audioClipCount > 0)
  {
    free(game.audioClips);
    game.audioClipCount = 0;
    free(game.audioClipTypes);
    game.audioClipTypeCount = 0;
  }

  free(game.viewNames[0]);
  free(game.viewNames);
  free (views);
  views = NULL;

  free (game.chars);
  game.chars = NULL;

  free (charcache);
  charcache = NULL;

  if (splipsync != NULL)
  {
    for (ee = 0; ee < numLipLines; ee++)
    {
      free(splipsync[ee].endtimeoffs);
      free(splipsync[ee].frame);
    }
    free(splipsync);
    splipsync = NULL;
    numLipLines = 0;
    curLipLine = -1;
  }

  for (ee=0;ee < MAXGLOBALMES;ee++) {
    if (game.messages[ee]==NULL) continue;
    free (game.messages[ee]);
    game.messages[ee] = NULL;
  }

  for (ee = 0; ee < game.roomCount; ee++)
  {
    free(game.roomNames[ee]);
  }
  if (game.roomCount > 0)
  {
    free(game.roomNames);
    free(game.roomNumbers);
    game.roomCount = 0;
  }

  for (ee=0;ee<game.numdialog;ee++) {
    if (dialog[ee].optionscripts!=NULL)
      free (dialog[ee].optionscripts);
    dialog[ee].optionscripts = NULL;
  }
  free (dialog);
  dialog = NULL;

  for (ee = 0; ee < game.numgui; ee++) {
    free (guibg[ee]);
    guibg[ee] = NULL;
    free(guiScriptObjNames[ee]);
  }

  free(guiScriptObjNames);
  free(guibg);
  free (guis);
  guis = NULL;
  free(scrGui);

  platform->ShutdownPlugins();
  ccRemoveAllSymbols();
  ccUnregisterAllObjects();

  for (ee=0;ee<game.numfonts;ee++)
    wfreefont(ee);

  free_do_once_tokens();
  free(play.gui_draw_order);

  free (roomstats);
  roomstats=(RoomStatus*)calloc(sizeof(RoomStatus),MAX_ROOMS);
  for (ee=0;ee<MAX_ROOMS;ee++) {
    roomstats[ee].beenhere=0;
    roomstats[ee].numobj=0;
    roomstats[ee].tsdatasize=0;
    roomstats[ee].tsdata=NULL;
  }
  
}

// **** text script exported functions



// ============================================================================
// GUI - BUTTON
// ============================================================================

// button corners
void do_corner(int sprn,int xx1,int yy1,int typx,int typy) {
  if (sprn<0) return;
  block thisone = spriteset[sprn];
  if (thisone == NULL)
    thisone = spriteset[0];

  put_sprite_256(xx1+typx*spritewidth[sprn],yy1+typy*spriteheight[sprn],thisone);
//  wputblock(xx1+typx*spritewidth[sprn],yy1+typy*spriteheight[sprn],thisone,1);
}

int get_but_pic(GUIMain*guo,int indx) {
  return guibuts[guo->objrefptr[indx] & 0x000ffff].pic;
  }

void draw_button_background(int xx1,int yy1,int xx2,int yy2,GUIMain*iep) {
  if (iep==NULL) {  // standard window
    alw_rectfill(abuf,xx1,yy1,xx2,yy2,get_col8_lookup(15));
    alw_rect(abuf,xx1,yy1,xx2,yy2,get_col8_lookup(16));
/*    wsetcolor(opts.tws.backcol); wbar(xx1,yy1,xx2,yy2);
    wsetcolor(opts.tws.textcol); wrectangle(xx1+1,yy1+1,xx2-1,yy2-1);*/
    }
  else {
    if (iep->bgcol >= 0) wsetcolor(iep->bgcol);
    else wsetcolor(0); // black backrgnd behind picture

    if (iep->bgcol > 0)
      wbar(xx1,yy1,xx2,yy2);

    int leftRightWidth = spritewidth[get_but_pic(iep,4)];
    int topBottomHeight = spriteheight[get_but_pic(iep,6)];
    if (iep->bgpic>0) {
      // offset the background image and clip it so that it is drawn
      // such that the border graphics can have a transparent outside
      // edge
      int bgoffsx = xx1 - leftRightWidth / 2;
      int bgoffsy = yy1 - topBottomHeight / 2;
      alw_set_clip_rect(abuf, bgoffsx, bgoffsy, xx2 + leftRightWidth / 2, yy2 + topBottomHeight / 2);
      alw_set_clip_state(abuf, TRUE);
      int bgfinishx = xx2;
      int bgfinishy = yy2;
      int bgoffsyStart = bgoffsy;
      while (bgoffsx <= bgfinishx)
      {
        bgoffsy = bgoffsyStart;
        while (bgoffsy <= bgfinishy)
        {
          wputblock(bgoffsx, bgoffsy, spriteset[iep->bgpic],0);
          bgoffsy += spriteheight[iep->bgpic];
        }
        bgoffsx += spritewidth[iep->bgpic];
      }
      // return to normal clipping rectangle
      alw_set_clip_rect(abuf, 0, 0, BMP_W(abuf) - 1, BMP_H(abuf) - 1);
      alw_set_clip_state(abuf, TRUE);
    }
    int uu;
    for (uu=yy1;uu <= yy2;uu+=spriteheight[get_but_pic(iep,4)]) {
      do_corner(get_but_pic(iep,4),xx1,uu,-1,0);   // left side
      do_corner(get_but_pic(iep,5),xx2+1,uu,0,0);  // right side
      }
    for (uu=xx1;uu <= xx2;uu+=spritewidth[get_but_pic(iep,6)]) {
      do_corner(get_but_pic(iep,6),uu,yy1,0,-1);  // top side
      do_corner(get_but_pic(iep,7),uu,yy2+1,0,0); // bottom side
      }
    do_corner(get_but_pic(iep,0),xx1,yy1,-1,-1);  // top left
    do_corner(get_but_pic(iep,1),xx1,yy2+1,-1,0);  // bottom left
    do_corner(get_but_pic(iep,2),xx2+1,yy1,0,-1);  //  top right
    do_corner(get_but_pic(iep,3),xx2+1,yy2+1,0,0);  // bottom right
    }
  }



// ============================================================================
// TEXT WINDOW
// ============================================================================

// Calculate the width that the left and right border of the textwindow
// GUI take up
int get_textwindow_border_width (int twgui) {
  if (twgui < 0)
    return 0;

  if (!guis[twgui].is_textwindow())
    quit("!GUI set as text window but is not actually a text window GUI");

  int borwid = spritewidth[get_but_pic(&guis[twgui], 4)] + 
               spritewidth[get_but_pic(&guis[twgui], 5)];

  return borwid;
}

// get the hegiht of the text window's top border
int get_textwindow_top_border_height (int twgui) {
  if (twgui < 0)
    return 0;

  if (!guis[twgui].is_textwindow())
    quit("!GUI set as text window but is not actually a text window GUI");

  return spriteheight[get_but_pic(&guis[twgui], 6)];
}


// draw_text_window: draws the normal or custom text window
// create a new bitmap the size of the window before calling, and
//   point abuf to it
// returns text start x & y pos in parameters
void draw_text_window(int*xins,int*yins,int*xx,int*yy,int*wii,int ovrheight=0, int ifnum=-1) {
  if (ifnum < 0)
    ifnum = game.options[OPT_TWCUSTOM];

  if (ifnum <= 0) {
    if (ovrheight)
      quit("!Cannot use QFG4 style options without custom text window");
    draw_button_background(0,0,BMP_W(abuf) - 1,BMP_H(abuf) - 1,NULL);
    wtextcolor(16);
    xins[0]=3;
    yins[0]=3;
    }
  else {
    if (ifnum >= game.numgui)
      quitprintf("!Invalid GUI %d specified as text window (total GUIs: %d)", ifnum, game.numgui);
    if (!guis[ifnum].is_textwindow())
      quit("!GUI set as text window but is not actually a text window GUI");

    int tbnum = get_but_pic(&guis[ifnum], 0);

    wii[0] += get_textwindow_border_width (ifnum);
    xx[0]-=spritewidth[tbnum];
    yy[0]-=spriteheight[tbnum];
    if (ovrheight == 0)
      ovrheight = numlines*texthit;

    if ((wantFreeScreenop > 0) && (screenop != NULL))
      alw_destroy_bitmap(screenop);
    screenop = alw_create_bitmap_ex(final_col_dep,wii[0],ovrheight+6+spriteheight[tbnum]*2);
    alw_clear_to_color(screenop, alw_bitmap_mask_color(screenop));
    wsetscreen(screenop);
    int xoffs=spritewidth[tbnum],yoffs=spriteheight[tbnum];
    draw_button_background(xoffs,yoffs,(BMP_W(abuf) - xoffs) - 1,(BMP_H(abuf) - yoffs) - 1,&guis[ifnum]);
    wtextcolor(guis[ifnum].fgcol);
    xins[0]=xoffs+3;
    yins[0]=yoffs+3;
  }

}

void draw_text_window_and_bar(int*xins,int*yins,int*xx,int*yy,int*wii,int ovrheight=0, int ifnum=-1) {

  draw_text_window(xins, yins, xx, yy, wii, ovrheight, ifnum);

  if ((topBar.wantIt) && (screenop != NULL)) {
    // top bar on the dialog window with character's name
    // create an enlarged window, then free the old one
    block newScreenop = alw_create_bitmap_ex(final_col_dep, BMP_W(screenop), BMP_H(screenop) + topBar.height);
    alw_blit(screenop, newScreenop, 0, 0, 0, topBar.height, BMP_W(screenop), BMP_H(screenop));
    wfreeblock(screenop);
    screenop = newScreenop;
    wsetscreen(screenop);

    // draw the top bar
    alw_rectfill(screenop, 0, 0, BMP_W(screenop) - 1, topBar.height - 1, get_col8_lookup(play.top_bar_backcolor));
    if (play.top_bar_backcolor != play.top_bar_bordercolor) {
      // draw the border
      for (int j = 0; j < multiply_up_coordinate(play.top_bar_borderwidth); j++)
        alw_rect(screenop, j, j, BMP_W(screenop) - (j + 1), topBar.height - (j + 1), get_col8_lookup(play.top_bar_bordercolor));
    }
    
    int textcolwas = textcol;
    // draw the text
    int textx = (BMP_W(screenop) / 2) - wgettextwidth_compensate(topBar.text, topBar.font) / 2;
    wtextcolor(play.top_bar_textcolor);
    wouttext_outline(textx, play.top_bar_borderwidth + get_fixed_pixel_size(1), topBar.font, topBar.text);
    // restore the current text colour
    textcol = textcolwas;

    // don't draw it next time
    topBar.wantIt = 0;
    // adjust the text Y position
    yins[0] += topBar.height;
  }
  else if (topBar.wantIt)
    topBar.wantIt = 0;
}


void wouttext_outline(int xxp, int yyp, int usingfont, char*texx) {
  int otextc=textcol;

  if (game.fontoutline[usingfont] >= 0) {
    wtextcolor(play.speech_text_shadow);
    // MACPORT FIX 9/6/5: cast
    wouttextxy(xxp, yyp, (int)game.fontoutline[usingfont], texx);
  }
  else if (game.fontoutline[usingfont] == FONT_OUTLINE_AUTO) {
    wtextcolor(play.speech_text_shadow);

    int outlineDist = 1;

    if ((game.options[OPT_NOSCALEFNT] == 0) && (!fontRenderers[usingfont]->SupportsExtendedCharacters(usingfont))) {
      // if it's a scaled up SCI font, move the outline out more
      outlineDist = get_fixed_pixel_size(1);
    }

    // move the text over so that it's still within the bounding rect
    xxp += outlineDist;
    yyp += outlineDist;

    wouttextxy(xxp - outlineDist, yyp, usingfont, texx);
    wouttextxy(xxp + outlineDist, yyp, usingfont, texx);
    wouttextxy(xxp, yyp + outlineDist, usingfont, texx);
    wouttextxy(xxp, yyp - outlineDist, usingfont, texx);
    wouttextxy(xxp - outlineDist, yyp - outlineDist, usingfont, texx);
    wouttextxy(xxp - outlineDist, yyp + outlineDist, usingfont, texx);
    wouttextxy(xxp + outlineDist, yyp + outlineDist, usingfont, texx);
    wouttextxy(xxp + outlineDist, yyp - outlineDist, usingfont, texx);
  }

  textcol = otextc;
  wouttextxy(xxp, yyp, usingfont, texx);
}

int GetTextDisplayTime (char *text, int canberel) {
  int uselen = strlen(text);

  int fpstimer = frames_per_second;

  // if it's background speech, make it stay relative to game speed
  if ((canberel == 1) && (play.bgspeech_game_speed == 1))
    fpstimer = 40;

  if (source_text_length >= 0) {
    // sync to length of original text, to make sure any animations
    // and music sync up correctly
    uselen = source_text_length;
    source_text_length = -1;
  }
  else {
    if ((text[0] == '&') && (play.unfactor_speech_from_textlength != 0)) {
      // if there's an "&12 text" type line, remove "&12 " from the source
      // length
      int j = 0;
      while ((text[j] != ' ') && (text[j] != 0))
        j++;
      j++;
      uselen -= j;
    }

  }

  if (uselen <= 0)
    return 0;

  if (play.text_speed + play.text_speed_modifier <= 0)
    quit("!Text speed is zero; unable to display text. Check your game.text_speed settings.");
  
  // Store how many game loops per character of text
  // This is calculated using a hard-coded 15 for the text speed,
  // so that it's always the same no matter how fast the user
  // can read.
  loops_per_character = (((uselen/play.lipsync_speed)+1) * fpstimer) / uselen;

  int textDisplayTimeInMS = ((uselen / (play.text_speed + play.text_speed_modifier)) + 1) * 1000;
  if (textDisplayTimeInMS < play.text_min_display_time_ms)
    textDisplayTimeInMS = play.text_min_display_time_ms;

  return (textDisplayTimeInMS * fpstimer) / 1000;
}

int convert_gui_disabled_style(int oldStyle) {
  int toret = GUIDIS_GREYOUT;

  // if GUIs Turn Off is selected, don't grey out buttons for
  // any Persistent GUIs which remain
  // set to 0x80 so that it is still non-zero, but has no effect
  if (oldStyle == 3)
    toret = GUIDIS_GUIOFF;
  // GUIs Go Black
  else if (oldStyle == 1)
    toret = GUIDIS_BLACKOUT;
  // GUIs unchanged
  else if (oldStyle == 2)
    toret = GUIDIS_UNCHANGED;

  return toret;
}

void update_gui_disabled_status() {
  // update GUI display status (perhaps we've gone into
  // an interface disabled state)
  int all_buttons_was = all_buttons_disabled;
  all_buttons_disabled = 0;

  if (!IsInterfaceEnabled()) {
    all_buttons_disabled = gui_disabled_style;
  }

  if (all_buttons_was != all_buttons_disabled) {
    // GUIs might have been removed/added
    for (int aa = 0; aa < game.numgui; aa++) {
      guis[aa].control_positions_changed();
    }
    guis_need_update = 1;
    invalidate_screen();
  }
}


int adjust_x_for_guis (int xx, int yy) {
  if ((game.options[OPT_DISABLEOFF]==3) && (all_buttons_disabled > 0))
    return xx;
  // If it's covered by a GUI, move it right a bit
  for (int aa=0;aa < game.numgui; aa++) {
    if (guis[aa].on < 1)
      continue;
    if ((guis[aa].x > xx) || (guis[aa].y > yy) || (guis[aa].y + guis[aa].hit < yy))
      continue;
    // totally transparent GUI, ignore
    if ((guis[aa].bgcol == 0) && (guis[aa].bgpic < 1))
      continue;

    // try to deal with full-width GUIs across the top
    if (guis[aa].x + guis[aa].wid >= get_fixed_pixel_size(280))
      continue;

    if (xx < guis[aa].x + guis[aa].wid) 
      xx = guis[aa].x + guis[aa].wid + 2;        
  }
  return xx;
}

int adjust_y_for_guis ( int yy) {
  if ((game.options[OPT_DISABLEOFF]==3) && (all_buttons_disabled > 0))
    return yy;
  // If it's covered by a GUI, move it down a bit
  for (int aa=0;aa < game.numgui; aa++) {
    if (guis[aa].on < 1)
      continue;
    if (guis[aa].y > yy)
      continue;
    // totally transparent GUI, ignore
    if ((guis[aa].bgcol == 0) && (guis[aa].bgpic < 1))
      continue;

    // try to deal with full-height GUIs down the left or right
    if (guis[aa].hit > get_fixed_pixel_size(50))
      continue;

    if (yy < guis[aa].y + guis[aa].hit) 
      yy = guis[aa].y + guis[aa].hit + 2;        
  }
  return yy;
}

void wouttext_aligned (int usexp, int yy, int oriwid, int usingfont, const char *text, int align) {

  if (align == SCALIGN_CENTRE)
    usexp = usexp + (oriwid / 2) - (wgettextwidth_compensate(text, usingfont) / 2);
  else if (align == SCALIGN_RIGHT)
    usexp = usexp + (oriwid - wgettextwidth_compensate(text, usingfont));

  wouttext_outline(usexp, yy, usingfont, (char *)text);
}


bool ShouldAntiAliasText() {
  return (game.options[OPT_ANTIALIASFONTS] != 0);
}

// Pass yy = -1 to find Y co-ord automatically
// allowShrink = 0 for none, 1 for leftwards, 2 for rightwards
// pass blocking=2 to create permanent overlay
int _display_main(int xx,int yy,int wii,char*todis,int blocking,int usingfont,int asspch, int isThought, int allowShrink, bool overlayPositionFixed) 
{
  bool alphaChannel = false;
  ensure_text_valid_for_font(todis, usingfont);
  break_up_text_into_lines(wii-6,usingfont,todis);
  texthit = wgetfontheight(usingfont);

  // if it's a normal message box and the game was being skipped,
  // ensure that the screen is up to date before the message box
  // is drawn on top of it
  if ((play.skip_until_char_stops >= 0) && (blocking == 1))
    render_graphics();

  EndSkippingUntilCharStops();

  if (topBar.wantIt) {
    // ensure that the window is wide enough to display
    // any top bar text
    int topBarWid = wgettextwidth_compensate(topBar.text, topBar.font);
    topBarWid += multiply_up_coordinate(play.top_bar_borderwidth + 2) * 2;
    if (longestline < topBarWid)
      longestline = topBarWid;
    // the top bar should behave like DisplaySpeech wrt blocking
    blocking = 0;
  }

  if (asspch > 0) {
    // update the all_buttons_disabled variable in advance
    // of the adjust_x/y_for_guis calls
    play.disabled_user_interface++;
    update_gui_disabled_status();
    play.disabled_user_interface--;
  }

  if (xx == OVR_AUTOPLACE) ;
  // centre text in middle of screen
  else if (yy<0) yy=(scrnhit/2-(numlines*texthit)/2)-3;
  // speech, so it wants to be above the character's head
  else if (asspch > 0) {
    yy-=numlines*texthit;
    if (yy < 5) yy=5;
    yy = adjust_y_for_guis (yy);
  }

  if (longestline < wii - get_fixed_pixel_size(6)) {
    // shrink the width of the dialog box to fit the text
    int oldWid = wii;
    //if ((asspch >= 0) || (allowShrink > 0))
    // If it's not speech, or a shrink is allowed, then shrink it
    if ((asspch == 0) || (allowShrink > 0))
      wii = longestline + get_fixed_pixel_size(6);
    
    // shift the dialog box right to align it, if necessary
    if ((allowShrink == 2) && (xx >= 0))
      xx += (oldWid - wii);
  }

  if (xx<-1) { 
    xx=(-xx)-wii/2;
    if (xx < 0)
      xx = 0;

    xx = adjust_x_for_guis (xx, yy);

    if (xx + wii >= scrnwid)
      xx = (scrnwid - wii) - 5;
  }
  else if (xx<0) xx=scrnwid/2-wii/2;

  int ee, extraHeight = get_fixed_pixel_size(6);
  wtextcolor(15);
  if (blocking < 2)
    remove_screen_overlay(OVER_TEXTMSG);

  screenop = alw_create_bitmap_ex(final_col_dep, (wii > 0) ? wii : 2, numlines*texthit + extraHeight);
  wsetscreen(screenop);
  alw_clear_to_color(screenop,alw_bitmap_mask_color(screenop));

  // inform draw_text_window to free the old bitmap
  wantFreeScreenop = 1;
  
  if ((strlen (todis) < 1) || (strcmp (todis, "  ") == 0) || (wii == 0)) ;
    // if it's an empty speech line, don't draw anything
  else if (asspch) { //wtextcolor(12);
    int ttxleft = 0, ttxtop = get_fixed_pixel_size(3), oriwid = wii - 6;
    int usingGui = -1, drawBackground = 0;
 
    if ((asspch < 0) && (game.options[OPT_SPEECHTYPE] >= 2)) {
      usingGui = play.speech_textwindow_gui;
      drawBackground = 1;
    }
    else if ((isThought) && (game.options[OPT_THOUGHTGUI] > 0)) {
      usingGui = game.options[OPT_THOUGHTGUI];
      // make it treat it as drawing inside a window now
      if (asspch > 0)
        asspch = -asspch;
      drawBackground = 1;
    }

    if (drawBackground)
      draw_text_window_and_bar(&ttxleft, &ttxtop, &xx, &yy, &wii, 0, usingGui);
    else if ((ShouldAntiAliasText()) && (final_col_dep >= 24))
      alphaChannel = true;

    for (ee=0;ee<numlines;ee++) {
      //int ttxp=wii/2 - wgettextwidth_compensate(lines[ee], usingfont)/2;
      int ttyp=ttxtop+ee*texthit;
      // asspch < 0 means that it's inside a text box so don't
      // centre the text
      if (asspch < 0) {
        if ((usingGui >= 0) && 
            ((game.options[OPT_SPEECHTYPE] >= 2) || (isThought)))
          wtextcolor(guis[usingGui].fgcol);
        else
          wtextcolor(-asspch);
        
        wouttext_aligned(ttxleft, ttyp, oriwid, usingfont, lines[ee], play.text_align);
      }
      else {
        wtextcolor(asspch);
        //wouttext_outline(ttxp,ttyp,usingfont,lines[ee]);
        wouttext_aligned(ttxleft, ttyp, wii, usingfont, lines[ee], play.speech_text_align);
      }
    }
  }
  else {
    int xoffs,yoffs, oriwid = wii - 6;
    draw_text_window_and_bar(&xoffs,&yoffs,&xx,&yy,&wii);

    if (game.options[OPT_TWCUSTOM] > 0)
    {
      alphaChannel = guis[game.options[OPT_TWCUSTOM]].is_alpha();
    }

    adjust_y_coordinate_for_text(&yoffs, usingfont);

    for (ee=0;ee<numlines;ee++)
      wouttext_aligned (xoffs, yoffs + ee * texthit, oriwid, usingfont, lines[ee], play.text_align);
  }

  wantFreeScreenop = 0;

  int ovrtype = OVER_TEXTMSG;
  if (blocking == 2) ovrtype=OVER_CUSTOM;
  else if (blocking >= OVER_CUSTOM) ovrtype=blocking;

  int nse = add_screen_overlay(xx, yy, ovrtype, screenop, alphaChannel);

  wsetscreen(virtual_screen);
  if (blocking>=2) {
    return screenover[nse].type;
  }

  if (blocking) {
    if (play.fast_forward) {
      remove_screen_overlay(OVER_TEXTMSG);
      play.messagetime=-1;
      return 0;
    }

/*    wputblock(xx,yy,screenop,1);
    remove_screen_overlay(OVER_TEXTMSG);*/

    if (!play.mouse_cursor_hidden)
      ac_domouse(1);
    // play.skip_display has same values as SetSkipSpeech:
    // 0 = click mouse or key to skip
    // 1 = key only
    // 2 = can't skip at all
    // 3 = only on keypress, no auto timer
    // 4 = mouse only
    int countdown = GetTextDisplayTime (todis);
    int skip_setting = user_to_internal_skip_speech(play.skip_display);
    while (1) {
      timerloop = 0;
      NEXT_ITERATION();
/*      if (!play.mouse_cursor_hidden)
        ac_domouse(0);
      write_screen();*/

      render_graphics();

      update_polled_stuff_and_crossfade();
      if (ac_mgetbutton()>NONE) {
        // If we're allowed, skip with mouse
        if (skip_setting & SKIP_MOUSECLICK)
          break;
      }
      if (ac_kbhit()) {
        // discard keypress, and don't leave extended keys over
        int kp = ac_getch();
        if (kp == 0) ac_getch();

        // let them press ESC to skip the cutscene
        check_skip_cutscene_keypress (kp);
        if (play.fast_forward)
          break;

        if (skip_setting & SKIP_KEYPRESS)
          break;
      }
      while ((timerloop == 0) && (play.fast_forward == 0)) {
        update_polled_stuff();
        platform->YieldCPU();
      }
      countdown--;

      if (channels[SCHAN_SPEECH] != NULL) {
        // extend life of text if the voice hasn't finished yet
        if ((!rec_isSpeechFinished()) && (play.fast_forward == 0)) {
          if (countdown <= 1)
            countdown = 1;
        }
        else  // if the voice has finished, remove the speech
          countdown = 0;
      }

      if ((countdown < 1) && (skip_setting & SKIP_AUTOTIMER))
      {
        play.ignore_user_input_until_time = globalTimerCounter + (play.ignore_user_input_after_text_timeout_ms / time_between_timers);
        break;
      }
      // if skipping cutscene, don't get stuck on No Auto Remove
      // text boxes
      if ((countdown < 1) && (play.fast_forward))
        break;
    }
    if (!play.mouse_cursor_hidden)
      ac_domouse(2);
    remove_screen_overlay(OVER_TEXTMSG);

    construct_virtual_screen(true);
  }
  else {
    // if the speech does not time out, but we are skipping a cutscene,
    // allow it to time out
    if ((play.messagetime < 0) && (play.fast_forward))
      play.messagetime = 2;

    if (!overlayPositionFixed)
    {
      screenover[nse].positionRelativeToScreen = false;
      screenover[nse].x += offsetx;
      screenover[nse].y += offsety;
    }

    do_main_cycle(UNTIL_NOOVERLAY,0);
  }

  play.messagetime=-1;
  return 0;
}

void _display_at(int xx,int yy,int wii,char*todis,int blocking,int asspch, int isThought, int allowShrink, bool overlayPositionFixed) {
  int usingfont=FONT_NORMAL;
  if (asspch) usingfont=FONT_SPEECH;
  int needStopSpeech = 0;

  EndSkippingUntilCharStops();

  if (todis[0]=='&') {
    // auto-speech
    int igr=atoi(&todis[1]);
    while ((todis[0]!=' ') & (todis[0]!=0)) todis++;
    if (todis[0]==' ') todis++;
    if (igr <= 0)
      quit("Display: auto-voice symbol '&' not followed by valid integer");
    if (play_speech(play.narrator_speech,igr)) {
      // if Voice Only, then blank out the text
      if (play.want_speech == 2)
        todis = "  ";
    }
    needStopSpeech = 1;
  }
  _display_main(xx,yy,wii,todis,blocking,usingfont,asspch, isThought, allowShrink, overlayPositionFixed);

  if (needStopSpeech)
    stop_speech();
}



// ============================================================================
// SPEECH
// ============================================================================

int play_speech(int charid,int sndid) {
  stop_and_destroy_channel (SCHAN_SPEECH);

  // don't play speech if we're skipping a cutscene
  if (play.fast_forward)
    return 0;
  if ((play.want_speech < 1) || (speech_file == NULL))
    return 0;

  SOUNDCLIP *speechmp3;
/*  char finame[40]="~SPEECH.VOX~NARR";
  if (charid >= 0)
    strncpy(&finame[12],game.chars[charid].scrname,4);*/

  char finame[40] = "~";
  strcat(finame, alw_get_filename(speech_file));
  strcat(finame, "~");

  if (charid >= 0) {
    // append the first 4 characters of the script name to the filename
    char theScriptName[5];
    if (game.chars[charid].scrname[0] == 'c')
      strncpy(theScriptName, &game.chars[charid].scrname[1], 4);
    else
      strncpy(theScriptName, game.chars[charid].scrname, 4);
    theScriptName[4] = 0;
    strcat(finame, theScriptName);
  }
  else
    strcat(finame, "NARR");

  // append the speech number
  sprintf(&finame[strlen(finame)],"%d",sndid);

  int ii;  // Compare the base file name to the .pam file name
  char *basefnptr = strchr (&finame[4], '~') + 1;
  curLipLine = -1;  // See if we have voice lip sync for this line
  curLipLinePhenome = -1;
  for (ii = 0; ii < numLipLines; ii++) {
    if (ac_stricmp(splipsync[ii].filename, basefnptr) == 0) {
      curLipLine = ii;
      break;
    }
  }
  // if the lip-sync is being used for voice sync, disable
  // the text-related lipsync
  if (numLipLines > 0)
    game.options[OPT_LIPSYNCTEXT] = 0;

  strcat (finame, ".WAV");
  speechmp3 = my_load_wave (finame, play.speech_volume, 0);

  if (speechmp3 == NULL) {
    strcpy (&finame[strlen(finame)-3], "ogg");
    speechmp3 = my_load_ogg (finame, play.speech_volume);
  }

  if (speechmp3 == NULL) {
    strcpy (&finame[strlen(finame)-3], "mp3");
    speechmp3 = my_load_mp3 (finame, play.speech_volume);
  }

  if (speechmp3 != NULL) {
    if (speechmp3->play() == 0)
      speechmp3 = NULL;
  }

  if (speechmp3 == NULL) {
    debug_log ("Speech load failure: '%s'",finame);
    curLipLine = -1;
    return 0;
  }

  channels[SCHAN_SPEECH] = speechmp3;
  play.music_vol_was = play.music_master_volume;

  // Negative value means set exactly; positive means drop that amount
  if (play.speech_music_drop < 0)
    play.music_master_volume = -play.speech_music_drop;
  else
    play.music_master_volume -= play.speech_music_drop;

  apply_volume_drop_modifier(true);
  update_music_volume();
  update_music_at = 0;
  mvolcounter = 0;

  update_ambient_sound_vol();

  // change Sierra w/bgrnd  to Sierra without background when voice
  // is available (for Tierra)
  if ((game.options[OPT_SPEECHTYPE] == 2) && (play.no_textbg_when_voice > 0)) {
    game.options[OPT_SPEECHTYPE] = 1;
    play.no_textbg_when_voice = 2;
  }

  return 1;
}

void stop_speech() {
  if (channels[SCHAN_SPEECH] != NULL) {
    play.music_master_volume = play.music_vol_was;
    // update the music in a bit (fixes two speeches follow each other
    // and music going up-then-down)
    update_music_at = 20;
    mvolcounter = 1;
    stop_and_destroy_channel (SCHAN_SPEECH);
    curLipLine = -1;

    if (play.no_textbg_when_voice == 2) {
      // set back to Sierra w/bgrnd
      play.no_textbg_when_voice = 1;
      game.options[OPT_SPEECHTYPE] = 2;
    }
  }
}



// ============================================================================
// GRAPHICS
// ============================================================================

int my_getpixel(ALW_BITMAP *blk, int x, int y) {
  if ((x < 0) || (y < 0) || (x >= BMP_W(blk)) || (y >= BMP_H(blk)))
    return -1;

  // strip the alpha channel
  return alw_getpixel(blk, x, y) & 0x00ffffff;
}



block GetObjectImage(int obj, int *isFlipped) 
{
  if (!gfxDriver->HasAcceleratedStretchAndFlip())
  {
    if (actsps[obj] != NULL) {
      // the actsps image is pre-flipped, so no longer register the image as such
      if (isFlipped)
        *isFlipped = 0;

      return actsps[obj];
    }
  }
  return spriteset[objs[obj].num];
}



// ============================================================================
// GRAPHICS - SPRITE POS/RECT
// ============================================================================

int isposinbox(int mmx,int mmy,int lf,int tp,int rt,int bt) {
  if ((mmx>=lf) & (mmx<=rt) & (mmy>=tp) & (mmy<=bt)) return TRUE;
  else return FALSE;
  }

// xx,yy is the position in room co-ordinates that we are checking
// arx,ary is the sprite x/y co-ordinates
int is_pos_in_sprite(int xx,int yy,int arx,int ary, block sprit, int spww,int sphh, int flipped) {
  if (spww==0) spww = divide_down_coordinate(BMP_W(sprit)) - 1;
  if (sphh==0) sphh = divide_down_coordinate(BMP_H(sprit)) - 1;

  if (isposinbox(xx,yy,arx,ary,arx+spww,ary+sphh)==FALSE)
    return FALSE;

  if (game.options[OPT_PIXPERFECT]) 
  {
    // if it's transparent, or off the edge of the sprite, ignore
    int xpos = multiply_up_coordinate(xx - arx);
    int ypos = multiply_up_coordinate(yy - ary);

    if (gfxDriver->HasAcceleratedStretchAndFlip())
    {
      // hardware acceleration, so the sprite in memory will not have
      // been stretched, it will be original size. Thus, adjust our
      // calculations to compensate
      multiply_up_coordinates(&spww, &sphh);

      if (spww != BMP_W(sprit))
        xpos = (xpos * BMP_W(sprit)) / spww;
      if (sphh != BMP_H(sprit))
        ypos = (ypos * BMP_H(sprit)) / sphh;
    }

    if (flipped)
      xpos = (BMP_W(sprit) - 1) - xpos;

    int gpcol = my_getpixel(sprit, xpos, ypos);

    if ((gpcol == alw_bitmap_mask_color(sprit)) || (gpcol == -1))
      return FALSE;
  }
  return TRUE;
}



int is_pos_on_character(int xx,int yy) {
  int cc,sppic,lowestyp=0,lowestwas=-1;
  for (cc=0;cc<game.numcharacters;cc++) {
    if (game.chars[cc].room!=displayed_room) continue;
    if (game.chars[cc].on==0) continue;
    if (game.chars[cc].flags & CHF_NOINTERACT) continue;
    if (game.chars[cc].view < 0) continue;
    CharacterInfo*chin=&game.chars[cc];

    if ((chin->view < 0) || 
        (chin->loop >= views[chin->view].numLoops) ||
        (chin->frame >= views[chin->view].loops[chin->loop].numFrames))
    {
      continue;
    }

    sppic=views[chin->view].loops[chin->loop].frames[chin->frame].pic;
    int usewid = charextra[cc].width;
    int usehit = charextra[cc].height;
    if (usewid==0) usewid=spritewidth[sppic];
    if (usehit==0) usehit=spriteheight[sppic];
    int xxx = chin->x - divide_down_coordinate(usewid) / 2;
    int yyy = chin->get_effective_y() - divide_down_coordinate(usehit);

    int mirrored = views[chin->view].loops[chin->loop].frames[chin->frame].flags & VFLG_FLIPSPRITE;
    block theImage = GetCharacterImage(cc, &mirrored);

    if (is_pos_in_sprite(xx,yy,xxx,yyy, theImage,
                         divide_down_coordinate(usewid),
                         divide_down_coordinate(usehit), mirrored) == FALSE)
      continue;

    int use_base = chin->get_baseline();
    if (use_base < lowestyp) continue;
    lowestyp=use_base;
    lowestwas=cc;
  }
  char_lowest_yp = lowestyp;
  return lowestwas;
}



// ============================================================================
// WALKABLE
// ============================================================================

void remove_walkable_areas_from_temp(int fromx, int cwidth, int starty, int endy) {

  fromx = convert_to_low_res(fromx);
  cwidth = convert_to_low_res(cwidth);
  starty = convert_to_low_res(starty);
  endy = convert_to_low_res(endy);

  int yyy;
  if (endy >= BMP_H(walkable_areas_temp))
    endy = BMP_H(walkable_areas_temp) - 1;
  if (starty < 0)
    starty = 0;
  
  for (; cwidth > 0; cwidth --) {
    for (yyy = starty; yyy <= endy; yyy++)
      alw__putpixel (walkable_areas_temp, fromx, yyy, 0);
    fromx ++;
  }

}

int is_point_in_rect(int x, int y, int left, int top, int right, int bottom) {
  if ((x >= left) && (x < right) && (y >= top ) && (y <= bottom))
    return 1;
  return 0;
}

block prepare_walkable_areas (int sourceChar) {
  // copy the walkable areas to the temp bitmap
  alw_blit (thisroom.walls, walkable_areas_temp, 0,0,0,0,BMP_W(thisroom.walls),BMP_H(thisroom.walls));
  // if the character who's moving doesn't block, don't bother checking
  if (sourceChar < 0) ;
  else if (game.chars[sourceChar].flags & CHF_NOBLOCKING)
    return walkable_areas_temp;

  int ww;
  // for each character in the current room, make the area under
  // them unwalkable
  for (ww = 0; ww < game.numcharacters; ww++) {
    if (game.chars[ww].on != 1) continue;
    if (game.chars[ww].room != displayed_room) continue;
    if (ww == sourceChar) continue;
    if (game.chars[ww].flags & CHF_NOBLOCKING) continue;
    if (convert_to_low_res(game.chars[ww].y) >= BMP_H(walkable_areas_temp)) continue;
    if (convert_to_low_res(game.chars[ww].x) >= BMP_W(walkable_areas_temp)) continue;
    if ((game.chars[ww].y < 0) || (game.chars[ww].x < 0)) continue;

    CharacterInfo *char1 = &game.chars[ww];
    int cwidth, fromx;

    if (is_char_on_another(sourceChar, ww, &fromx, &cwidth))
      continue;
    if ((sourceChar >= 0) && (is_char_on_another(ww, sourceChar, NULL, NULL)))
      continue;

    remove_walkable_areas_from_temp(fromx, cwidth, char1->get_blocking_top(), char1->get_blocking_bottom());
  }

  // check for any blocking objects in the room, and deal with them
  // as well
  for (ww = 0; ww < croom->numobj; ww++) {
    if (objs[ww].on != 1) continue;
    if ((objs[ww].flags & OBJF_SOLID) == 0)
      continue;
    if (convert_to_low_res(objs[ww].y) >= BMP_H(walkable_areas_temp)) continue;
    if (convert_to_low_res(objs[ww].x) >= BMP_W(walkable_areas_temp)) continue;
    if ((objs[ww].y < 0) || (objs[ww].x < 0)) continue;

    int x1, y1, width, y2;
    get_object_blocking_rect(ww, &x1, &y1, &width, &y2);

    // if the character is currently standing on the object, ignore
    // it so as to allow him to escape
    if ((sourceChar >= 0) &&
        (is_point_in_rect(game.chars[sourceChar].x, game.chars[sourceChar].y, 
                          x1, y1, x1 + width, y2)))
      continue;

    remove_walkable_areas_from_temp(x1, width, y1, y2);
  }

  return walkable_areas_temp;
}



// ============================================================================
// PROPERTIES
// ============================================================================

// begin custom property functions

// Get an integer property
int get_int_property (CustomProperties *cprop, const char *property) {
  int idx = game.propSchema.findProperty(property);

  if (idx < 0)
    quit("!GetProperty: no such property found in schema. Make sure you are using the property's name, and not its description, when calling this command.");

  if (game.propSchema.propType[idx] == PROP_TYPE_STRING)
    quit("!GetProperty: need to use GetPropertyString for a text property");

  const char *valtemp = cprop->getPropertyValue(property);
  if (valtemp == NULL) {
    valtemp = game.propSchema.defaultValue[idx];
  }
  return atoi(valtemp);
}

// Get a string property
void get_text_property (CustomProperties *cprop, const char *property, char *bufer) {
  int idx = game.propSchema.findProperty(property);

  if (idx < 0)
    quit("!GetPropertyText: no such property found in schema. Make sure you are using the property's name, and not its description, when calling this command.");

  if (game.propSchema.propType[idx] != PROP_TYPE_STRING)
    quit("!GetPropertyText: need to use GetProperty for a non-text property");

  const char *valtemp = cprop->getPropertyValue(property);
  if (valtemp == NULL) {
    valtemp = game.propSchema.defaultValue[idx];
  }
  strcpy (bufer, valtemp);
}

const char* get_text_property_dynamic_string(CustomProperties *cprop, const char *property) {
  int idx = game.propSchema.findProperty(property);

  if (idx < 0)
    quit("!GetTextProperty: no such property found in schema. Make sure you are using the property's name, and not its description, when calling this command.");

  if (game.propSchema.propType[idx] != PROP_TYPE_STRING)
    quit("!GetTextProperty: need to use GetProperty for a non-text property");

  const char *valtemp = cprop->getPropertyValue(property);
  if (valtemp == NULL) {
    valtemp = game.propSchema.defaultValue[idx];
  }

  return CreateNewScriptString(valtemp);
}



// end custom property functions



// ============================================================================
// MOVELIST
// ============================================================================

int do_movelist_move(short*mlnum,int*xx,int*yy) {
  int need_to_fix_sprite=0;
  if (mlnum[0]<1) quit("movelist_move: attempted to move on a non-exist movelist");
  MoveList*cmls; cmls=&mls[mlnum[0]];
  alw_fixed xpermove=cmls->xpermove[cmls->onstage],ypermove=cmls->ypermove[cmls->onstage];

  short targetx=short((cmls->pos[cmls->onstage+1] >> 16) & 0x00ffff);
  short targety=short(cmls->pos[cmls->onstage+1] & 0x00ffff);
  int xps=xx[0],yps=yy[0];
  if (cmls->doneflag & 1) {
    // if the X-movement has finished, and the Y-per-move is < 1, finish
    // This can cause jump at the end, but without it the character will
    // walk on the spot for a while if the Y-per-move is for example 0.2
//    if ((ypermove & 0xfffff000) == 0) cmls->doneflag|=2;
//    int ypmm=(ypermove >> 16) & 0x0000ffff;

    // NEW 2.15 SR-1 plan: if X-movement has finished, and Y-per-move is < 1,
    // allow it to finish more easily by moving target zone

    int adjAmnt = 3;
    // 2.70: if the X permove is also <=1, don't do the skipping
    if (((xpermove & 0xffff0000) == 0xffff0000) ||
        ((xpermove & 0xffff0000) == 0x00000000))
      adjAmnt = 2;

    // 2.61 RC1: correct this to work with > -1 as well as < 1
    if (ypermove == 0) { }
    // Y per move is < 1, so finish the move
    else if ((ypermove & 0xffff0000) == 0)
      targety -= adjAmnt;
    // Y per move is -1 exactly, don't snap to finish
    else if (ypermove == 0xffff0000) { }
    // Y per move is > -1, so finish the move
    else if ((ypermove & 0xffff0000) == 0xffff0000)
      targety += adjAmnt;
  }
  else xps=cmls->fromx+(int)(alw_fixtof(xpermove)*(float)cmls->onpart);

  if (cmls->doneflag & 2) {
    // Y-movement has finished

    int adjAmnt = 3;

    // if the Y permove is also <=1, don't skip as far
    if (((ypermove & 0xffff0000) == 0xffff0000) ||
        ((ypermove & 0xffff0000) == 0x00000000))
      adjAmnt = 2;

    if (xpermove == 0) { }
    // Y per move is < 1, so finish the move
    else if ((xpermove & 0xffff0000) == 0)
      targetx -= adjAmnt;
    // X per move is -1 exactly, don't snap to finish
    else if (xpermove == 0xffff0000) { }
    // X per move is > -1, so finish the move
    else if ((xpermove & 0xffff0000) == 0xffff0000)
      targetx += adjAmnt;

/*    int xpmm=(xpermove >> 16) & 0x0000ffff;
//    if ((xpmm==0) | (xpmm==0xffff)) cmls->doneflag|=1;
    if (xpmm==0) cmls->doneflag|=1;*/
    }
  else yps=cmls->fromy+(int)(alw_fixtof(ypermove)*(float)cmls->onpart);
  // check if finished horizontal movement
  if (((xpermove > 0) && (xps >= targetx)) ||
      ((xpermove < 0) && (xps <= targetx))) {
    cmls->doneflag|=1;
    xps = targetx;
    // if the Y is almost there too, finish it
    // this is new in v2.40
    // removed in 2.70
    /*if (abs(yps - targety) <= 2)
      yps = targety;*/
  }
  else if (xpermove == 0)
    cmls->doneflag|=1;
  // check if finished vertical movement
  if ((ypermove > 0) & (yps>=targety)) {
    cmls->doneflag|=2;
    yps = targety;
  }
  else if ((ypermove < 0) & (yps<=targety)) {
    cmls->doneflag|=2;
    yps = targety;
  }
  else if (ypermove == 0)
    cmls->doneflag|=2;

  if ((cmls->doneflag & 0x03)==3) {
    // this stage is done, go on to the next stage
    // signed shorts to ensure that numbers like -20 do not become 65515
    cmls->fromx=(signed short)((cmls->pos[cmls->onstage+1] >> 16) & 0x000ffff);
    cmls->fromy=(signed short)(cmls->pos[cmls->onstage+1] & 0x000ffff);
    if ((cmls->fromx > 65000) || (cmls->fromy > 65000))
      quit("do_movelist: int to short rounding error");

    cmls->onstage++; cmls->onpart=-1; cmls->doneflag&=0xf0;
    cmls->lastx=-1;
    if (cmls->onstage < cmls->numstage) {
      xps=cmls->fromx; yps=cmls->fromy; }
    if (cmls->onstage>=cmls->numstage-1) {  // last stage is just dest pos
      cmls->numstage=0;
      mlnum[0]=0;
      need_to_fix_sprite=1;
      }
    else need_to_fix_sprite=2;
    }
  cmls->onpart++;
  xx[0]=xps; yy[0]=yps;
  return need_to_fix_sprite;
  }



// ============================================================================
// LANGUAGE (commented out)
// ============================================================================

/*void GetLanguageString(int indxx,char*buffr) {
  VALIDATE_STRING(buffr);
  char*bptr=get_language_text(indxx);
  if (bptr==NULL) strcpy(buffr,"[language string error]");
  else strncpy(buffr,bptr,199);
  buffr[199]=0;
  }*/



// ============================================================================
// ROOM STATS
// ============================================================================

int HasBeenToRoom (int roomnum) {
  if ((roomnum < 0) || (roomnum >= MAX_ROOMS))
    quit("!HasBeenToRoom: invalid room number specified");

  if (roomstats[roomnum].beenhere)
    return 1;
  return 0;
}

// **** end of trext script exported functions



// ============================================================================
// INTERACTION
// ============================================================================

InteractionVariable *get_interaction_variable (int varindx) {
  
  if ((varindx >= LOCAL_VARIABLE_OFFSET) && (varindx < LOCAL_VARIABLE_OFFSET + thisroom.numLocalVars))
    return &thisroom.localvars[varindx - LOCAL_VARIABLE_OFFSET];

  if ((varindx < 0) || (varindx >= numGlobalVars))
    quit("!invalid interaction variable specified");

  return &globalvars[varindx];
}

InteractionVariable *FindGraphicalVariable(const char *varName) {
  int ii;
  for (ii = 0; ii < numGlobalVars; ii++) {
    if (ac_stricmp (globalvars[ii].name, varName) == 0)
      return &globalvars[ii];
  }
  for (ii = 0; ii < thisroom.numLocalVars; ii++) {
    if (ac_stricmp (thisroom.localvars[ii].name, varName) == 0)
      return &thisroom.localvars[ii];
  }
  return NULL;
}


int get_nivalue (NewInteractionCommandList *nic, int idx, int parm) {
  if (nic->command[idx].data[parm].valType == VALTYPE_VARIABLE) {
    // return the value of the variable
    return get_interaction_variable(nic->command[idx].data[parm].val)->value;
  }
  return nic->command[idx].data[parm].val;
}


char bname[40],bne[40];
char* make_ts_func_name(const char*base,int iii,int subd) {
  sprintf(bname,base,iii);
  sprintf(bne,"%s_%c",bname,subd+'a');
  return &bne[0];
}

#define IPARAM1 get_nivalue(nicl, i, 0)
#define IPARAM2 get_nivalue(nicl, i, 1)
#define IPARAM3 get_nivalue(nicl, i, 2)
#define IPARAM4 get_nivalue(nicl, i, 3)
#define IPARAM5 get_nivalue(nicl, i, 4)

// the 'cmdsrun' parameter counts how many commands are run.
// if a 'Inv Item Was Used' check does not pass, it doesn't count
// so cmdsrun remains 0 if no inventory items matched
int run_interaction_commandlist (NewInteractionCommandList *nicl, int *timesrun, int*cmdsrun) {
  int i;

  if (nicl == NULL)
    return -1;

  for (i = 0; i < nicl->numCommands; i++) {
    cmdsrun[0] ++;
    int room_was = play.room_changes;
    
    switch (nicl->command[i].type) {
      case 0:  // Do nothing
        break;
      case 1:  // Run script
        { 
        TempEip tempip(4001);
        acaudio_update_mp3();
        if ((strstr(evblockbasename,"character")!=0) || (strstr(evblockbasename,"inventory")!=0)) {
          // Character or Inventory (global script)
          char *torun = make_ts_func_name(evblockbasename,evblocknum,nicl->command[i].data[0].val);
          // we are already inside the mouseclick event of the script, can't nest calls
          if (inside_script) 
            curscript->run_another (torun, 0, 0);
          else run_text_script(gameinst,torun);
          }
        else {
          // Other (room script)
          if (inside_script) {
            char funcName[60];
            strcpy(funcName,"|");
            strcat(funcName,make_ts_func_name(evblockbasename,evblocknum,nicl->command[i].data[0].val));
            curscript->run_another (funcName, 0, 0);
            }
          else
            run_text_script(roominst,make_ts_func_name(evblockbasename,evblocknum,nicl->command[i].data[0].val));
          }
        acaudio_update_mp3();
        break;
      }
      case 2:  // Add score (first time)
        if (timesrun[0] > 0)
          break;
        timesrun[0] ++;
      case 3:  // Add score
        GiveScore (IPARAM1);
        break;
      case 4:  // Display Message
/*        if (comprdata<0)
          display_message_aschar=evb->data[ss];*/
        DisplayMessage(IPARAM1);
        break;
      case 5:  // Play Music
        PlayMusicResetQueue(IPARAM1);
        break;
      case 6:  // Stop Music
        stopmusic ();
        break;
      case 7:  // Play Sound
        play_sound (IPARAM1);
        break;
      case 8:  // Play Flic
        play_flc_file(IPARAM1, IPARAM2);
        break;
      case 9:  // Run Dialog
		{ int room_was = play.room_changes;
        RunDialog(IPARAM1);
		// if they changed room within the dialog script,
		// the interaction command list is no longer valid
        if (room_was != play.room_changes)
          return -1;
		}
        break;
      case 10: // Enable Dialog Option
        SetDialogOption (IPARAM1, IPARAM2, 1);
        break;
      case 11: // Disable Dialog Option
        SetDialogOption (IPARAM1, IPARAM2, 0);
        break;
      case 12: // Go To Screen
        Character_ChangeRoomAutoPosition(playerchar, IPARAM1, IPARAM2);
        return -1;
      case 13: // Add Inventory
        add_inventory (IPARAM1);
        break;
      case 14: // Move Object
        MoveObject (IPARAM1, IPARAM2, IPARAM3, IPARAM4);
        // if they want to wait until finished, do so
        if (IPARAM5)
          do_main_cycle(UNTIL_MOVEEND,(int)&objs[IPARAM1].moving);
        break;
      case 15: // Object Off
        ObjectOff (IPARAM1);
        break;
      case 16: // Object On
        ObjectOn (IPARAM1);
        break;
      case 17: // Set Object View
        SetObjectView (IPARAM1, IPARAM2);
        break;
      case 18: // Animate Object
        AnimateObject (IPARAM1, IPARAM2, IPARAM3, IPARAM4);
        break;
      case 19: // Move Character
        if (IPARAM4)
          MoveCharacterBlocking (IPARAM1, IPARAM2, IPARAM3, 0);
        else
          MoveCharacter (IPARAM1, IPARAM2, IPARAM3);
        break;
      case 20: // If Inventory Item was used
        if (play.usedinv == IPARAM1) {
          if (game.options[OPT_NOLOSEINV] == 0)
            lose_inventory (play.usedinv);
          if (run_interaction_commandlist (nicl->command[i].get_child_list(), timesrun, cmdsrun))
            return -1;
        }
        else
          cmdsrun[0] --;
        break;
      case 21: // if player has inventory item
        if (playerchar->inv[IPARAM1] > 0)
          if (run_interaction_commandlist (nicl->command[i].get_child_list(), timesrun, cmdsrun))
            return -1;
        break;
      case 22: // if a character is moving
        if (game.chars[IPARAM1].walking)
          if (run_interaction_commandlist (nicl->command[i].get_child_list(), timesrun, cmdsrun))
            return -1;
        break;
      case 23: // if two variables are equal
        if (IPARAM1 == IPARAM2)
          if (run_interaction_commandlist (nicl->command[i].get_child_list(), timesrun, cmdsrun))
            return -1;
        break;
      case 24: // Stop character walking
        StopMoving (IPARAM1);
        break;
      case 25: // Go to screen at specific co-ordinates
        NewRoomEx (IPARAM1, IPARAM2, IPARAM3);
        return -1;
      case 26: // Move NPC to different room
        if (!is_valid_character(IPARAM1))
          quit("!Move NPC to different room: invalid character specified");
        game.chars[IPARAM1].room = IPARAM2;
        break;
      case 27: // Set character view
        SetCharacterView (IPARAM1, IPARAM2);
        break;
      case 28: // Release character view
        ReleaseCharacterView (IPARAM1);
        break;
      case 29: // Follow character
        FollowCharacter (IPARAM1, IPARAM2);
        break;
      case 30: // Stop following
        FollowCharacter (IPARAM1, -1);
        break;
      case 31: // Disable hotspot
        DisableHotspot (IPARAM1);
        break;
      case 32: // Enable hotspot
        EnableHotspot (IPARAM1);
        break;
      case 33: // Set variable value
        get_interaction_variable(nicl->command[i].data[0].val)->value = IPARAM2;
        break;
      case 34: // Run animation
        scAnimateCharacter(IPARAM1, IPARAM2, IPARAM3, 0);
        do_main_cycle(UNTIL_SHORTIS0,(int)&game.chars[IPARAM1].animating);
        break;
      case 35: // Quick animation
        SetCharacterView (IPARAM1, IPARAM2);
        scAnimateCharacter(IPARAM1, IPARAM3, IPARAM4, 0);
        do_main_cycle(UNTIL_SHORTIS0,(int)&game.chars[IPARAM1].animating);
        ReleaseCharacterView (IPARAM1);
        break;
      case 36: // Set idle animation
        SetCharacterIdle (IPARAM1, IPARAM2, IPARAM3);
        break;
      case 37: // Disable idle animation
        SetCharacterIdle (IPARAM1, -1, -1);
        break;
      case 38: // Lose inventory item
        lose_inventory (IPARAM1);
        break;
      case 39: // Show GUI
        InterfaceOn (IPARAM1);
        break;
      case 40: // Hide GUI
        InterfaceOff (IPARAM1);
        break;
      case 41: // Stop running more commands
        return -1;
      case 42: // Face location
        FaceLocation (IPARAM1, IPARAM2, IPARAM3);
        break;
      case 43: // Pause command processor
        scrWait (IPARAM1);
        break;
      case 44: // Change character view
        ChangeCharacterView (IPARAM1, IPARAM2);
        break;
      case 45: // If player character is
        if (GetPlayerCharacter() == IPARAM1)
          if (run_interaction_commandlist (nicl->command[i].get_child_list(), timesrun, cmdsrun))
            return -1;
        break;
      case 46: // if cursor mode is
        if (GetCursorMode() == IPARAM1)
          if (run_interaction_commandlist (nicl->command[i].get_child_list(), timesrun, cmdsrun))
            return -1;
        break;
      case 47: // if player has been to room
        if (HasBeenToRoom(IPARAM1))
          if (run_interaction_commandlist (nicl->command[i].get_child_list(), timesrun, cmdsrun))
            return -1;
        break;
      default:
        quit("unknown new interaction command");
        break;
    }

    // if the room changed within the action, nicl is no longer valid
    if (room_was != play.room_changes)
      return -1;
  }
  return 0;

}

void run_unhandled_event (int evnt) {

  if (play.check_interaction_only)
    return;

  int evtype=0;
  if (ac_strnicmp(evblockbasename,"hotspot",7)==0) evtype=1;
  else if (ac_strnicmp(evblockbasename,"object",6)==0) evtype=2;
  else if (ac_strnicmp(evblockbasename,"character",9)==0) evtype=3;
  else if (ac_strnicmp(evblockbasename,"inventory",9)==0) evtype=5;
  else if (ac_strnicmp(evblockbasename,"region",6)==0)
    return;  // no unhandled_events for regions

  // clicked Hotspot 0, so change the type code
  if ((evtype == 1) & (evblocknum == 0) & (evnt != 0) & (evnt != 5) & (evnt != 6))
    evtype = 4;
  if ((evtype==1) & ((evnt==0) | (evnt==5) | (evnt==6)))
    ;  // character stands on hotspot, mouse moves over hotspot, any click
  else if ((evtype==2) & (evnt==4)) ;  // any click on object
  else if ((evtype==3) & (evnt==4)) ;  // any click on character
  else if (evtype > 0) {
    can_run_delayed_command();

    if (inside_script)
      curscript->run_another ("#unhandled_event", evtype, evnt);
    else
      run_text_script_2iparam(gameinst,"unhandled_event",evtype,evnt);
  }

}

// Returns 0 normally, or -1 to indicate that the NewInteraction has
// become invalid and don't run another interaction on it
// (eg. a room change occured)
int run_interaction_event (NewInteraction *nint, int evnt, int chkAny, int isInv) {

  if ((nint->response[evnt] == NULL) || (nint->response[evnt]->numCommands == 0)) {
    // no response defined for this event
    // If there is a response for "Any Click", then abort now so as to
    // run that instead
    if (chkAny < 0) ;
    else if ((nint->response[chkAny] != NULL) && (nint->response[chkAny]->numCommands > 0))
      return 0;

    // Otherwise, run unhandled_event
    run_unhandled_event(evnt);
    
    return 0;
  }

  if (play.check_interaction_only) {
    play.check_interaction_only = 2;
    return -1;
  }

  int cmdsrun = 0, retval = 0;
  // Right, so there were some commands defined in response to the event.
  retval = run_interaction_commandlist (nint->response[evnt], &nint->timesRun[evnt], &cmdsrun);

  // An inventory interaction, but the wrong item was used
  if ((isInv) && (cmdsrun == 0))
    run_unhandled_event (evnt);

  return retval;
}

// Returns 0 normally, or -1 to indicate that the NewInteraction has
// become invalid and don't run another interaction on it
// (eg. a room change occured)
int run_interaction_script(InteractionScripts *nint, int evnt, int chkAny, int isInv) {

  if ((nint->scriptFuncNames[evnt] == NULL) || (nint->scriptFuncNames[evnt][0] == 0)) {
    // no response defined for this event
    // If there is a response for "Any Click", then abort now so as to
    // run that instead
    if (chkAny < 0) ;
    else if ((nint->scriptFuncNames[chkAny] != NULL) && (nint->scriptFuncNames[chkAny][0] != 0))
      return 0;

    // Otherwise, run unhandled_event
    run_unhandled_event(evnt);
    
    return 0;
  }

  if (play.check_interaction_only) {
    play.check_interaction_only = 2;
    return -1;
  }

  int room_was = play.room_changes;

  acaudio_update_mp3();
  if ((strstr(evblockbasename,"character")!=0) || (strstr(evblockbasename,"inventory")!=0)) {
    // Character or Inventory (global script)
    if (inside_script) 
      curscript->run_another (nint->scriptFuncNames[evnt], 0, 0);
    else run_text_script(gameinst, nint->scriptFuncNames[evnt]);
    }
  else {
    // Other (room script)
    if (inside_script) {
      char funcName[60];
      sprintf(funcName, "|%s", nint->scriptFuncNames[evnt]);
      curscript->run_another (funcName, 0, 0);
    }
    else
      run_text_script(roominst, nint->scriptFuncNames[evnt]);
  }
  acaudio_update_mp3();

  int retval = 0;
  // if the room changed within the action
  if (room_was != play.room_changes)
    retval = -1;

  return retval;
}



// ============================================================================
// DIALOG
// ============================================================================

int run_dialog_request (int parmtr) {
  play.stop_dialog_at_end = DIALOG_RUNNING;
  run_text_script_iparam(gameinst, "dialog_request", parmtr);

  if (play.stop_dialog_at_end == DIALOG_STOP) {
    play.stop_dialog_at_end = DIALOG_NONE;
    return -2;
  }
  if (play.stop_dialog_at_end >= DIALOG_NEWTOPIC) {
    int tval = play.stop_dialog_at_end - DIALOG_NEWTOPIC;
    play.stop_dialog_at_end = DIALOG_NONE;
    return tval;
  }
  if (play.stop_dialog_at_end >= DIALOG_NEWROOM) {
    int roomnum = play.stop_dialog_at_end - DIALOG_NEWROOM;
    play.stop_dialog_at_end = DIALOG_NONE;
    NewRoom(roomnum);
    return -2;
  }
  play.stop_dialog_at_end = DIALOG_NONE;
  return -1;
}

#define RUN_DIALOG_STOP_DIALOG   -2
#define RUN_DIALOG_GOTO_PREVIOUS -4
// dialog manager stuff

int run_dialog_script(DialogTopic*dtpp, int dialogID, int offse, int optionIndex) {
  said_speech_line = 0;
  int result;

  char funcName[100];
  sprintf(funcName, "_run_dialog%d", dialogID);
  run_text_script_iparam(dialogScriptsInst, funcName, optionIndex);
  result = dialogScriptsInst->returnValue;

  if (in_new_room > 0)
    return RUN_DIALOG_STOP_DIALOG;

  if (said_speech_line > 0) {
    // the line below fixes the problem with the close-up face remaining on the
    // screen after they finish talking; however, it makes the dialog options
    // area flicker when going between topics.
    DisableInterface();
    mainloop(); // redraw the screen to make sure it looks right
    EnableInterface();
    // if we're not about to abort the dialog, switch back to arrow
    if (result != RUN_DIALOG_STOP_DIALOG)
      set_mouse_cursor(CURS_ARROW);
  }

  return result;
}

int write_dialog_options(int dlgxp, int curyp, int numdisp, int mouseison, int areawid,
    int bullet_wid, int usingfont, DialogTopic*dtop, char*disporder, short*dispyp,
    int txthit, int utextcol) {
  int ww;

  for (ww=0;ww<numdisp;ww++) {

    if ((dtop->optionflags[disporder[ww]] & DFLG_HASBEENCHOSEN) &&
        (play.read_dialog_option_colour >= 0)) {
      // 'read' colour
      wtextcolor(play.read_dialog_option_colour);
    }
    else {
      // 'unread' colour
      wtextcolor(playerchar->talkcolor);
    }

    if (mouseison==ww) {
      if (textcol==get_col8_lookup(utextcol))
        wtextcolor(13); // the normal colour is the same as highlight col
      else wtextcolor(utextcol);
    }

    break_up_text_into_lines(areawid-(8+bullet_wid),usingfont,get_translation(dtop->optionnames[disporder[ww]]));
    dispyp[ww]=curyp;
    if (game.dialog_bullet > 0)
      wputblock(dlgxp,curyp,spriteset[game.dialog_bullet],1);
    int cc;
    if (game.options[OPT_DIALOGNUMBERED]) {
      char tempbfr[20];
      int actualpicwid = 0;
      if (game.dialog_bullet > 0)
        actualpicwid = spritewidth[game.dialog_bullet]+3;

      sprintf (tempbfr, "%d.", ww + 1);
      wouttext_outline (dlgxp + actualpicwid, curyp, usingfont, tempbfr);
    }
    for (cc=0;cc<numlines;cc++) {
      wouttext_outline(dlgxp+((cc==0) ? 0 : 9)+bullet_wid, curyp, usingfont, lines[cc]);
      curyp+=txthit;
    }
    if (ww < numdisp-1)
      curyp += multiply_up_coordinate(game.options[OPT_DIALOGGAP]);
  }
  return curyp;
}

#define GET_OPTIONS_HEIGHT {\
  needheight = 0;\
  for (ww=0;ww<numdisp;ww++) {\
    break_up_text_into_lines(areawid-(8+bullet_wid),usingfont,get_translation(dtop->optionnames[disporder[ww]]));\
    needheight += (numlines * txthit) + multiply_up_coordinate(game.options[OPT_DIALOGGAP]);\
  }\
  if (parserInput) needheight += parserInput->hit + multiply_up_coordinate(game.options[OPT_DIALOGGAP]);\
 }


void draw_gui_for_dialog_options(GUIMain *guib, int dlgxp, int dlgyp) {
  if (guib->bgcol != 0) {
    wsetcolor(guib->bgcol);
    wbar(dlgxp, dlgyp, dlgxp + guib->wid, dlgyp + guib->hit);
  }
  if (guib->bgpic > 0)
    put_sprite_256 (dlgxp, dlgyp, spriteset[guib->bgpic]);

  wsetcolor(0);
}

bool get_custom_dialog_options_dimensions(int dlgnum)
{
  ccDialogOptionsRendering.Reset();
  ccDialogOptionsRendering.dialogID = dlgnum;

  getDialogOptionsDimensionsFunc.param1 = &ccDialogOptionsRendering;
  run_function_on_non_blocking_thread(&getDialogOptionsDimensionsFunc);

  if ((ccDialogOptionsRendering.width > 0) &&
      (ccDialogOptionsRendering.height > 0))
  {
    return true;
  }
  return false;
}

#define MAX_TOPIC_HISTORY 50
#define DLG_OPTION_PARSER 99

int show_dialog_options(int dlgnum, int sayChosenOption, bool runGameLoopsInBackground) 
{
  int dlgxp,dlgyp = get_fixed_pixel_size(160);
  int usingfont=FONT_NORMAL;
  int txthit = wgetfontheight(usingfont);
  int curswas=cur_cursor;
  int bullet_wid = 0, needheight;
  IDriverDependantBitmap *ddb = NULL;
  ALW_BITMAP *subBitmap = NULL;
  GUITextBox *parserInput = NULL;
  DialogTopic*dtop = NULL;

  if ((dlgnum < 0) || (dlgnum >= game.numdialog))
    quit("!RunDialog: invalid dialog number specified");

  can_run_delayed_command();

  play.in_conversation ++;

  update_polled_stuff();

  if (game.dialog_bullet > 0)
    bullet_wid = spritewidth[game.dialog_bullet]+3;

  // numbered options, leave space for the numbers
  if (game.options[OPT_DIALOGNUMBERED])
    bullet_wid += wgettextwidth_compensate("9. ", usingfont);

  said_text = 0;

  update_polled_stuff();

  block tempScrn = alw_create_bitmap_ex(final_col_dep, BMP_W(alw_screen), BMP_H(alw_screen));

  set_mouse_cursor(CURS_ARROW);

  dtop=&dialog[dlgnum];

  int ww,chose=-1,numdisp=0;

  //get_real_screen();
  wsetscreen(virtual_screen);

  char disporder[MAXTOPICOPTIONS];
  short dispyp[MAXTOPICOPTIONS];
  int parserActivated = 0;
  if ((dtop->topicFlags & DTFLG_SHOWPARSER) && (play.disable_dialog_parser == 0)) {
    parserInput = new GUITextBox();
    parserInput->hit = txthit + get_fixed_pixel_size(4);
    parserInput->exflags = 0;
    parserInput->font = usingfont;
  }

  wtexttransparent(TEXTFG);
  numdisp=0;
  for (ww=0;ww<dtop->numoptions;ww++) {
    if ((dtop->optionflags[ww] & DFLG_ON)==0) continue;
    ensure_text_valid_for_font(dtop->optionnames[ww], usingfont);
    disporder[numdisp]=ww;
    numdisp++;
  }
  if (numdisp<1) quit("!DoDialog: all options have been turned off");
  // Don't display the options if there is only one and the parser
  // is not enabled.
  if ((numdisp > 1) || (parserInput != NULL) || (play.show_single_dialog_option)) {
    wsetcolor(0); //wbar(0,dlgyp-1,scrnwid-1,dlgyp+numdisp*txthit+1);
    int areawid, is_textwindow = 0;
    int forecol = 14, savedwid;

    int mouseison=-1,curyp;
    int mousewason=-10;
    int dirtyx = 0, dirtyy = 0;
    int dirtywidth = BMP_W(virtual_screen), dirtyheight = BMP_H(virtual_screen);
    bool usingCustomRendering = false;

    dlgxp = 1;
    if (get_custom_dialog_options_dimensions(dlgnum))
    {
      usingCustomRendering = true;
      dirtyx = multiply_up_coordinate(ccDialogOptionsRendering.x);
      dirtyy = multiply_up_coordinate(ccDialogOptionsRendering.y);
      dirtywidth = multiply_up_coordinate(ccDialogOptionsRendering.width);
      dirtyheight = multiply_up_coordinate(ccDialogOptionsRendering.height);
    }
    else if (game.options[OPT_DIALOGIFACE] > 0)
    {
      GUIMain*guib=&guis[game.options[OPT_DIALOGIFACE]];
      if (guib->is_textwindow()) {
        // text-window, so do the QFG4-style speech options
        is_textwindow = 1;
        forecol = guib->fgcol;
      }
      else {
        dlgxp = guib->x;
        dlgyp = guib->y;
        draw_gui_for_dialog_options(guib, dlgxp, dlgyp);

        dirtyx = dlgxp;
        dirtyy = dlgyp;
        dirtywidth = guib->wid;
        dirtyheight = guib->hit;

        areawid=guib->wid - 5;

        GET_OPTIONS_HEIGHT

        if (game.options[OPT_DIALOGUPWARDS]) {
          // They want the options upwards from the bottom
          dlgyp = (guib->y + guib->hit) - needheight;
        }
        
      }
    }
    else {
      //dlgyp=(scrnhit-numdisp*txthit)-1;
      areawid=scrnwid-5;
      GET_OPTIONS_HEIGHT
      dlgyp = scrnhit - needheight;
      wbar(0,dlgyp-1,scrnwid-1,scrnhit-1);

      dirtyx = 0;
      dirtyy = dlgyp - 1;
      dirtywidth = scrnwid;
      dirtyheight = scrnhit - dirtyy;
    }
    if (!is_textwindow)
      areawid -= multiply_up_coordinate(play.dialog_options_x) * 2;

    int orixp = dlgxp, oriyp = dlgyp;
    int wantRefresh = 0;
    mouseison=-10;
    
    update_polled_stuff();
    //alw_blit(virtual_screen, tempScrn, 0, 0, 0, 0, BMP_W(alw_screen), BMP_H(alw_screen));
    if (!play.mouse_cursor_hidden)
      ac_domouse(1);
    update_polled_stuff();

 redraw_options:

    wantRefresh = 1;

    if (usingCustomRendering)
    {
      tempScrn = recycle_bitmap(tempScrn, final_col_dep, 
        multiply_up_coordinate(ccDialogOptionsRendering.width), 
        multiply_up_coordinate(ccDialogOptionsRendering.height));
    }

    alw_clear_to_color(tempScrn, alw_bitmap_mask_color(tempScrn));
    wsetscreen(tempScrn);

    dlgxp = orixp;
    dlgyp = oriyp;
    // lengthy drawing to screen, so lock it for speed
    //alw_acquire_screen();

    if (usingCustomRendering)
    {
      ccDialogOptionsRendering.surfaceToRenderTo = dialogOptionsRenderingSurface;
      ccDialogOptionsRendering.surfaceAccessed = false;
      dialogOptionsRenderingSurface->linkedBitmapOnly = tempScrn;
      dialogOptionsRenderingSurface->hasAlphaChannel = false;

      renderDialogOptionsFunc.param1 = &ccDialogOptionsRendering;
      run_function_on_non_blocking_thread(&renderDialogOptionsFunc);

      if (!ccDialogOptionsRendering.surfaceAccessed)
        quit("!dialog_options_get_dimensions was implemented, but no dialog_options_render function drew anything to the surface");

      if (parserInput)
      {
        parserInput->x = multiply_up_coordinate(ccDialogOptionsRendering.parserTextboxX);
        curyp = multiply_up_coordinate(ccDialogOptionsRendering.parserTextboxY);
        areawid = multiply_up_coordinate(ccDialogOptionsRendering.parserTextboxWidth);
        if (areawid == 0)
          areawid = BMP_W(tempScrn);
      }
    }
    else if (is_textwindow) {
      // text window behind the options
      areawid = multiply_up_coordinate(play.max_dialogoption_width);
      int biggest = 0;
      for (ww=0;ww<numdisp;ww++) {
        break_up_text_into_lines(areawid-(8+bullet_wid),usingfont,get_translation(dtop->optionnames[disporder[ww]]));
        if (longestline > biggest)
          biggest = longestline;
      }
      if (biggest < areawid - (12+bullet_wid))
        areawid = biggest + (12+bullet_wid);

      if (areawid < multiply_up_coordinate(play.min_dialogoption_width)) {
        areawid = multiply_up_coordinate(play.min_dialogoption_width);
        if (play.min_dialogoption_width > play.max_dialogoption_width)
          quit("!game.min_dialogoption_width is larger than game.max_dialogoption_width");
      }

      GET_OPTIONS_HEIGHT

      savedwid = areawid;
      int txoffs=0,tyoffs=0,yspos = scrnhit/2-needheight/2;
      int xspos = scrnwid/2 - areawid/2;
      // shift window to the right if QG4-style full-screen pic
      if ((game.options[OPT_SPEECHTYPE] == 3) && (said_text > 0))
        xspos = (scrnwid - areawid) - get_fixed_pixel_size(10);

      // needs to draw the right text window, not the default
      push_screen();
      draw_text_window(&txoffs,&tyoffs,&xspos,&yspos,&areawid,needheight, game.options[OPT_DIALOGIFACE]);
      pop_screen();
      // snice draw_text_window incrases the width, restore it
      areawid = savedwid;
      //wnormscreen();

      dirtyx = xspos;
      dirtyy = yspos;
      dirtywidth = BMP_W(screenop);
      dirtyheight = BMP_H(screenop);

      wputblock(xspos,yspos,screenop,1);
      wfreeblock(screenop); screenop = NULL;

      // Ignore the dialog_options_x/y offsets when using a text window
      txoffs += xspos;
      tyoffs += yspos;
      dlgyp = tyoffs;
      curyp = write_dialog_options(txoffs,tyoffs,numdisp,mouseison,areawid,bullet_wid,usingfont,dtop,disporder,dispyp,txthit,forecol);
      if (parserInput)
        parserInput->x = txoffs;
    }
    else {

      if (wantRefresh) {
        // redraw the black background so that anti-alias
        // fonts don't re-alias themselves
        if (game.options[OPT_DIALOGIFACE] == 0) {
          wsetcolor(16);
          wbar(0,dlgyp-1,scrnwid-1,scrnhit-1);
        }
        else {
          GUIMain* guib = &guis[game.options[OPT_DIALOGIFACE]];
          if (!guib->is_textwindow())
            draw_gui_for_dialog_options(guib, dlgxp, dlgyp);
        }
      }

      dirtyx = 0;
      dirtywidth = scrnwid;

      if (game.options[OPT_DIALOGIFACE] > 0) 
      {
        // the whole GUI area should be marked dirty in order
        // to ensure it gets drawn
        GUIMain* guib = &guis[game.options[OPT_DIALOGIFACE]];
        dirtyheight = guib->hit;
        dirtyy = dlgyp;
      }
      else
      {
        dirtyy = dlgyp - 1;
        dirtyheight = needheight + 1;
      }

      dlgxp += multiply_up_coordinate(play.dialog_options_x);
      dlgyp += multiply_up_coordinate(play.dialog_options_y);

      // if they use a negative dialog_options_y, make sure the
      // area gets marked as dirty
      if (dlgyp < dirtyy)
        dirtyy = dlgyp;

      //curyp = dlgyp + 1;
      curyp = dlgyp;
      curyp = write_dialog_options(dlgxp,curyp,numdisp,mouseison,areawid,bullet_wid,usingfont,dtop,disporder,dispyp,txthit,forecol);

      /*if (curyp > scrnhit) {
        dlgyp = scrnhit - (curyp - dlgyp);
        wbar(0,dlgyp-1,scrnwid-1,scrnhit-1);
        goto redraw_options;
      }*/
      if (parserInput)
        parserInput->x = dlgxp;
    }

    if (parserInput) {
      // Set up the text box, if present
      parserInput->y = curyp + multiply_up_coordinate(game.options[OPT_DIALOGGAP]);
      parserInput->wid = areawid - get_fixed_pixel_size(10);
      parserInput->textcol = playerchar->talkcolor;
      if (mouseison == DLG_OPTION_PARSER)
        parserInput->textcol = forecol;

      if (game.dialog_bullet)  // the parser X will get moved in a second
        wputblock(parserInput->x, parserInput->y, spriteset[game.dialog_bullet], 1);

      parserInput->wid -= bullet_wid;
      parserInput->x += bullet_wid;

      parserInput->Draw();
      parserInput->activated = 0;
    }

    wantRefresh = 0;
    wsetscreen(virtual_screen);

    update_polled_stuff();

    subBitmap = recycle_bitmap(subBitmap, alw_bitmap_color_depth(tempScrn), dirtywidth, dirtyheight);
    subBitmap = gfxDriver->ConvertBitmapToSupportedColourDepth(subBitmap);

    update_polled_stuff();

    if (usingCustomRendering)
    {
      alw_blit(tempScrn, subBitmap, 0, 0, 0, 0, BMP_W(tempScrn), BMP_H(tempScrn));
      invalidate_rect(dirtyx, dirtyy, dirtyx + BMP_W(subBitmap), dirtyy + BMP_H(subBitmap));
    }
    else
    {
      alw_blit(tempScrn, subBitmap, dirtyx, dirtyy, 0, 0, dirtywidth, dirtyheight);
    }

    if ((ddb != NULL) && 
      ((ddb->GetWidth() != dirtywidth) ||
       (ddb->GetHeight() != dirtyheight)))
    {
      gfxDriver->DestroyDDB(ddb);
      ddb = NULL;
    }
    if (ddb == NULL)
      ddb = gfxDriver->CreateDDBFromBitmap(subBitmap, false, false);
    else
      gfxDriver->UpdateDDBFromBitmap(ddb, subBitmap, false);

    render_graphics(ddb, dirtyx, dirtyy);

    while (1) {

      if (runGameLoopsInBackground)
      {
        play.disabled_user_interface++;
        mainloop(false, ddb, dirtyx, dirtyy);
        play.disabled_user_interface--;
      }
      else
      {
        timerloop = 0;
        NEXT_ITERATION();

        render_graphics(ddb, dirtyx, dirtyy);
      
        update_polled_stuff_and_crossfade();
      }

      if (ac_kbhit()) {
        int gkey = ac_getch();
        if (parserInput) {
          wantRefresh = 1;
          // type into the parser 
          if ((gkey == 361) || ((gkey == ' ') && (strlen(parserInput->text) == 0))) {
            // write previous contents into textbox (F3 or Space when box is empty)
            for (unsigned int i = strlen(parserInput->text); i < strlen(play.lastParserEntry); i++) {
              parserInput->KeyPress(play.lastParserEntry[i]);
            }
            //ac_domouse(2);
            goto redraw_options;
          }
          else if ((gkey >= 32) || (gkey == 13) || (gkey == 8)) {
            parserInput->KeyPress(gkey);
            if (!parserInput->activated) {
              //ac_domouse(2);
              goto redraw_options;
            }
          }
        }
        // Allow selection of options by keyboard shortcuts
        else if ((gkey >= '1') && (gkey <= '9')) {
          gkey -= '1';
          if (gkey < numdisp) {
            chose = disporder[gkey];
            break;
          }
        }
      }
      mousewason=mouseison;
      mouseison=-1;
      if (usingCustomRendering)
      {
        if ((mousex >= dirtyx) && (mousey >= dirtyy) &&
            (mousex < dirtyx + BMP_W(tempScrn)) &&
            (mousey < dirtyy + BMP_H(tempScrn)))
        {
          getDialogOptionUnderCursorFunc.param1 = &ccDialogOptionsRendering;
          run_function_on_non_blocking_thread(&getDialogOptionUnderCursorFunc);

          if (!getDialogOptionUnderCursorFunc.atLeastOneImplementationExists)
            quit("!The script function dialog_options_get_active is not implemented. It must be present to use a custom dialogue system.");

          mouseison = ccDialogOptionsRendering.activeOptionID;
        }
        else
        {
          ccDialogOptionsRendering.activeOptionID = -1;
        }
      }
      else if ((mousey <= dlgyp) || (mousey > curyp)) ;
      else {
        mouseison=numdisp-1;
        for (ww=0;ww<numdisp;ww++) {
          if (mousey < dispyp[ww]) { mouseison=ww-1; break; }
        }
        if ((mouseison<0) | (mouseison>=numdisp)) mouseison=-1;
      }

      if (parserInput != NULL) {
        int relativeMousey = mousey;
        if (usingCustomRendering)
          relativeMousey -= dirtyy;

        if ((relativeMousey > parserInput->y) && 
            (relativeMousey < parserInput->y + parserInput->hit))
          mouseison = DLG_OPTION_PARSER;

        if (parserInput->activated)
          parserActivated = 1;
      }

      int mouseButtonPressed = ac_mgetbutton();

      if (mouseButtonPressed != NONE) {
        if (mouseison < 0) 
        {
          if (usingCustomRendering)
          {
            runDialogOptionMouseClickHandlerFunc.param1 = &ccDialogOptionsRendering;
            runDialogOptionMouseClickHandlerFunc.param2 = (void*)(mouseButtonPressed + 1);
            run_function_on_non_blocking_thread(&runDialogOptionMouseClickHandlerFunc);

            if (runDialogOptionMouseClickHandlerFunc.atLeastOneImplementationExists)
              goto redraw_options;
          }
          continue;
        }
        if (mouseison == DLG_OPTION_PARSER) {
          // they clicked the text box
          parserActivated = 1;
        }
        else if (usingCustomRendering)
        {
          chose = mouseison;
          break;
        }
        else {
          chose=disporder[mouseison];
          break;
        }
      }

      if (usingCustomRendering)
      {
        int mouseWheelTurn = check_mouse_wheel();
        if (mouseWheelTurn != 0)
        {
            runDialogOptionMouseClickHandlerFunc.param1 = &ccDialogOptionsRendering;
            runDialogOptionMouseClickHandlerFunc.param2 = (void*)((mouseWheelTurn < 0) ? 9 : 8);
            run_function_on_non_blocking_thread(&runDialogOptionMouseClickHandlerFunc);

            if (runDialogOptionMouseClickHandlerFunc.atLeastOneImplementationExists)
              goto redraw_options;

            continue;
        }
      }

      if (parserActivated) {
        // They have selected a custom parser-based option
        if (parserInput->text[0] != 0) {
          chose = DLG_OPTION_PARSER;
          break;
        }
        else {
          parserActivated = 0;
          parserInput->activated = 0;
        }
      }
      if (mousewason != mouseison) {
        //ac_domouse(2);
        goto redraw_options;
      }
      while ((timerloop == 0) && (play.fast_forward == 0)) {
        update_polled_stuff();
        platform->YieldCPU();
      }

    }
    if (!play.mouse_cursor_hidden)
      ac_domouse(2);
  }
  else 
    chose = disporder[0];  // only one choice, so select it

  while (ac_kbhit()) ac_getch(); // empty keyboard buffer
  //leave_real_screen();
  construct_virtual_screen(true);

  if (parserActivated) 
  {
    strcpy (play.lastParserEntry, parserInput->text);
    ParseText (parserInput->text);
    chose = CHOSE_TEXTPARSER;
  }

  if (parserInput) {
    delete parserInput;
    parserInput = NULL;
  }

  if (ddb != NULL)
    gfxDriver->DestroyDDB(ddb);
  if (subBitmap != NULL)
    alw_destroy_bitmap(subBitmap);

  set_mouse_cursor(curswas);
  // In case it's the QFG4 style dialog, remove the black screen
  play.in_conversation--;
  remove_screen_overlay(OVER_COMPLETE);

  wfreeblock(tempScrn);

  if (chose != CHOSE_TEXTPARSER)
  {
    dtop->optionflags[chose] |= DFLG_HASBEENCHOSEN;

    bool sayTheOption = false;
    if (sayChosenOption == SAYCHOSEN_YES)
    {
      sayTheOption = true;
    }
    else if (sayChosenOption == SAYCHOSEN_USEFLAG)
    {
      sayTheOption = ((dtop->optionflags[chose] & DFLG_NOREPEAT) == 0);
    }

    if (sayTheOption)
      DisplaySpeech(get_translation(dtop->optionnames[chose]), game.playercharacter);
  }

  return chose;
}

void do_conversation(int dlgnum) 
{
  EndSkippingUntilCharStops();

  int dlgnum_was = dlgnum;
  int previousTopics[MAX_TOPIC_HISTORY];
  int numPrevTopics = 0;
  DialogTopic *dtop = &dialog[dlgnum];

  // run the startup script
  int tocar = run_dialog_script(dtop, dlgnum, dtop->startupentrypoint, 0);
  if ((tocar == RUN_DIALOG_STOP_DIALOG) ||
      (tocar == RUN_DIALOG_GOTO_PREVIOUS)) 
  {
    // 'stop' or 'goto-previous' from first startup script
    remove_screen_overlay(OVER_COMPLETE);
    play.in_conversation--;
    return;
  }
  else if (tocar >= 0)
    dlgnum = tocar;

  while (dlgnum >= 0)
  {
    if (dlgnum >= game.numdialog)
      quit("!RunDialog: invalid dialog number specified");

    dtop = &dialog[dlgnum];

    if (dlgnum != dlgnum_was) 
    {
      // dialog topic changed, so play the startup
      // script for the new topic
      tocar = run_dialog_script(dtop, dlgnum, dtop->startupentrypoint, 0);
      dlgnum_was = dlgnum;
      if (tocar == RUN_DIALOG_GOTO_PREVIOUS) {
        if (numPrevTopics < 1) {
          // goto-previous on first topic -- end dialog
          tocar = RUN_DIALOG_STOP_DIALOG;
        }
        else {
          tocar = previousTopics[numPrevTopics - 1];
          numPrevTopics--;
        }
      }
      if (tocar == RUN_DIALOG_STOP_DIALOG)
        break;
      else if (tocar >= 0) {
        // save the old topic number in the history
        if (numPrevTopics < MAX_TOPIC_HISTORY) {
          previousTopics[numPrevTopics] = dlgnum;
          numPrevTopics++;
        }
        dlgnum = tocar;
        continue;
      }
    }

    int chose = show_dialog_options(dlgnum, SAYCHOSEN_USEFLAG, (game.options[OPT_RUNGAMEDLGOPTS] != 0));

    if (chose == CHOSE_TEXTPARSER)
    {
      said_speech_line = 0;
  
      tocar = run_dialog_request(dlgnum);

      if (said_speech_line > 0) {
        // fix the problem with the close-up face remaining on screen
        DisableInterface();
        mainloop(); // redraw the screen to make sure it looks right
        EnableInterface();
        set_mouse_cursor(CURS_ARROW);
      }
    }
    else 
    {
      tocar = run_dialog_script(dtop, dlgnum, dtop->entrypoints[chose], chose + 1);
    }

    if (tocar == RUN_DIALOG_GOTO_PREVIOUS) {
      if (numPrevTopics < 1) {
        tocar = RUN_DIALOG_STOP_DIALOG;
      }
      else {
        tocar = previousTopics[numPrevTopics - 1];
        numPrevTopics--;
      }
    }
    if (tocar == RUN_DIALOG_STOP_DIALOG) break;
    else if (tocar >= 0) {
      // save the old topic number in the history
      if (numPrevTopics < MAX_TOPIC_HISTORY) {
        previousTopics[numPrevTopics] = dlgnum;
        numPrevTopics++;
      }
      dlgnum = tocar;
    }

  }

}

// end dialog manager



// ============================================================================
// SAVE GAMES
// ============================================================================

// save game functions
int find_highest_room_entered() {
  int qq,fndas=-1;
  for (qq=0;qq<MAX_ROOMS;qq++) {
    if (roomstats[qq].beenhere!=0) fndas=qq;
  }
  // This is actually legal - they might start in room 400 and save
  //if (fndas<0) quit("find_highest_room: been in no rooms?");
  return fndas;
}

void serialize_bitmap(block thispic, FILE*ooo) {
  if (thispic != NULL) {
    putw(BMP_W(thispic),ooo);
    putw(BMP_H(thispic),ooo);
    putw(alw_bitmap_color_depth(thispic),ooo);
    for (int cc=0;cc<BMP_H(thispic);cc++)
      fwrite(&BMP_LINE(thispic)[cc][0],BMP_W(thispic),alw_bitmap_color_depth(thispic)/8,ooo);
    }
  }

uint64_t write_screen_shot_for_vista(FILE *ooo, block screenshot) 
{
  uint64_t fileSize = 0;
  char tempFileName[MAX_PATH];
  sprintf(tempFileName, "%s""_tmpscht.bmp", saveGameDirectory);
  
  alw_save_bitmap(tempFileName, screenshot, palette);

  update_polled_stuff();
  
  if (alw_exists(tempFileName))
  {
    fileSize = alw_file_size_ex(tempFileName);
    char *buffer = (char*)malloc(fileSize);

    FILE *input = fopen(tempFileName, "rb");
    fread(buffer, fileSize, 1, input);
    fclose(input);
    unlink(tempFileName);

    fwrite(buffer, fileSize, 1, ooo);
    free(buffer);
  }
  return fileSize;
}

#define MAGICNUMBER 0xbeefcafe
// Write the save game position to the file
void save_game_data (FILE *ooo, block screenshot) {
  int bb, cc, dd;

  platform->RunPluginHooks(AGSE_PRESAVEGAME, 0);

  putw(SGVERSION,ooo);
  // store the screenshot at the start to make it easily accesible
  putw((screenshot == NULL) ? 0 : 1, ooo);

  if (screenshot)
    serialize_bitmap(screenshot, ooo);

  fputstring(ACI_VERSION_TEXT, ooo);
  fputstring(usetup.main_data_filename, ooo);
  putw(scrnhit,ooo);
  putw(final_col_dep, ooo);
  putw(frames_per_second,ooo);
  putw(cur_mode,ooo);
  putw(cur_cursor,ooo);
  putw(offsetx,ooo); putw(offsety,ooo);
  putw(loopcounter,ooo);

  putw(spriteset.elements, ooo);
  for (bb = 1; bb < spriteset.elements; bb++) {
    if (game.spriteflags[bb] & SPF_DYNAMICALLOC) {
      putw(bb, ooo);
      fputc(game.spriteflags[bb], ooo);
      serialize_bitmap(spriteset[bb], ooo);
    }
  }
  // end of dynamic sprite list
  putw(0, ooo);

  // write the data segment of the global script
  int gdatasize=gameinst->globaldatasize;
  putw(gdatasize,ooo);
  ccFlattenGlobalData (gameinst);
  // MACPORT FIX: just in case gdatasize is 2 or 4, don't want to swap endian
  fwrite(&gameinst->globaldata[0], 1, gdatasize, ooo);
  ccUnFlattenGlobalData (gameinst);
  // write the script modules data segments
  putw(numScriptModules, ooo);
  for (bb = 0; bb < numScriptModules; bb++) {
    int glsize = moduleInst[bb]->globaldatasize;
    putw(glsize, ooo);
    if (glsize > 0) {
      ccFlattenGlobalData(moduleInst[bb]);
      fwrite(&moduleInst[bb]->globaldata[0], 1, glsize, ooo);
      ccUnFlattenGlobalData(moduleInst[bb]);
    }
  }

  putw(displayed_room, ooo);

  if (displayed_room >= 0) {
    // update the current room script's data segment copy
    if (roominst!=NULL)
      save_room_data_segment();

    // Update the saved interaction variable values
    for (ff = 0; ff < thisroom.numLocalVars; ff++)
      croom->interactionVariableValues[ff] = thisroom.localvars[ff].value;

  }

  // write the room state for all the rooms the player has been in
  for (bb = 0; bb < MAX_ROOMS; bb++) {
    if (roomstats[bb].beenhere) {
      fputc (1, ooo);
      fwrite(&roomstats[bb],sizeof(RoomStatus),1,ooo);
      if (roomstats[bb].tsdatasize>0)
        fwrite(&roomstats[bb].tsdata[0], 1, roomstats[bb].tsdatasize, ooo);
    }
    else
      fputc (0, ooo);
  }

  update_polled_stuff();

  if (play.cur_music_number >= 0) {
    if (IsMusicPlaying() == 0)
      play.cur_music_number = -1;
    }

  fwrite(&play,sizeof(GameState),1,ooo);

  for (bb = 0; bb < play.num_do_once_tokens; bb++)
  {
    fputstring(play.do_once_tokens[bb], ooo);
  }
  fwrite(&play.gui_draw_order[0], sizeof(int), game.numgui, ooo);

  fwrite(&mls[0],sizeof(MoveList), game.numcharacters + MAX_INIT_SPR + 1, ooo);

  fwrite(&game,sizeof(GameSetupStructBase),1,ooo);
  fwrite(&game.invinfo[0], sizeof(InventoryItemInfo), game.numinvitems, ooo);
  fwrite(&game.mcurs[0], sizeof(MouseCursor), game.numcursors, ooo);

  if (game.invScripts == NULL)
  {
    for (bb = 0; bb < game.numinvitems; bb++)
      fwrite (&game.intrInv[bb]->timesRun[0], sizeof (int), MAX_NEWINTERACTION_EVENTS, ooo);
    for (bb = 0; bb < game.numcharacters; bb++)
      fwrite (&game.intrChar[bb]->timesRun[0], sizeof (int), MAX_NEWINTERACTION_EVENTS, ooo); 
  }

  fwrite (&game.options[0], sizeof(int), OPT_HIGHESTOPTION+1, ooo);
  fputc (game.options[OPT_LIPSYNCTEXT], ooo);

  fwrite(&game.chars[0],sizeof(CharacterInfo),game.numcharacters,ooo);
  fwrite(&charextra[0],sizeof(CharacterExtras),game.numcharacters,ooo);
  fwrite(&palette[0],sizeof(color),256,ooo);
  for (bb=0;bb<game.numdialog;bb++)
    fwrite(&dialog[bb].optionflags[0],sizeof(int),MAXTOPICOPTIONS,ooo);
  putw(mouse_on_iface,ooo);
  putw(mouse_on_iface_button,ooo);
  putw(mouse_pushed_iface,ooo);
  putw (ifacepopped, ooo);
  putw(game_paused,ooo);
  //putw(mi.trk,ooo);
  write_gui(ooo,guis,&game);
  putw(numAnimButs, ooo);
  fwrite(&animbuts[0], sizeof(AnimatingGUIButton), numAnimButs, ooo);

  putw(game.audioClipTypeCount, ooo);
  fwrite(&game.audioClipTypes[0], sizeof(AudioClipType), game.audioClipTypeCount, ooo);

  fwrite(&thisroom.regionLightLevel[0],sizeof(short), MAX_REGIONS,ooo);
  fwrite(&thisroom.regionTintLevel[0],sizeof(int), MAX_REGIONS,ooo);
  fwrite(&thisroom.walk_area_zoom[0],sizeof(short), MAX_WALK_AREAS + 1, ooo);
  fwrite(&thisroom.walk_area_zoom2[0],sizeof(short), MAX_WALK_AREAS + 1, ooo);

  fwrite (&ambient[0], sizeof(AmbientSound), MAX_SOUND_CHANNELS, ooo);
  putw(numscreenover,ooo);
  fwrite(&screenover[0],sizeof(ScreenOverlay),numscreenover,ooo);
  for (bb=0;bb<numscreenover;bb++) {
    serialize_bitmap (screenover[bb].pic, ooo);
  }

  update_polled_stuff();

  for (bb = 0; bb < MAX_DYNAMIC_SURFACES; bb++)
  {
    if (dynamicallyCreatedSurfaces[bb] == NULL)
    {
      fputc(0, ooo);
    }
    else
    {
      fputc(1, ooo);
      serialize_bitmap(dynamicallyCreatedSurfaces[bb], ooo);
    }
  }

  update_polled_stuff();

  if (displayed_room >= 0) {

    for (bb = 0; bb < MAX_BSCENE; bb++) {
      if (play.raw_modified[bb])
        serialize_bitmap (thisroom.ebscene[bb], ooo);
    }

    putw ((raw_saved_screen == NULL) ? 0 : 1, ooo);
    if (raw_saved_screen)
      serialize_bitmap (raw_saved_screen, ooo);

    // save the current troom, in case they save in room 600 or whatever
    fwrite(&troom,sizeof(RoomStatus),1,ooo);
    if (troom.tsdatasize>0)
      fwrite(&troom.tsdata[0],troom.tsdatasize,1,ooo);

  }

  putw (numGlobalVars, ooo);
  fwrite (&globalvars[0], sizeof(InteractionVariable), numGlobalVars, ooo);

  putw (game.numviews, ooo);
  for (bb = 0; bb < game.numviews; bb++) {
    for (cc = 0; cc < views[bb].numLoops; cc++) {
      for (dd = 0; dd < views[bb].loops[cc].numFrames; dd++)
      {
        putw(views[bb].loops[cc].frames[dd].sound, ooo);
        putw(views[bb].loops[cc].frames[dd].pic, ooo);
      }
    }
  }
  putw (MAGICNUMBER+1, ooo);

  putw(game.audioClipCount, ooo);
  for (bb = 0; bb <= MAX_SOUND_CHANNELS; bb++)
  {
    if ((channels[bb] != NULL) && (channels[bb]->done == 0) && (channels[bb]->sourceClip != NULL))
    {
      putw(((ScriptAudioClip*)channels[bb]->sourceClip)->id, ooo);
      putw(channels[bb]->get_pos(), ooo);
      putw(channels[bb]->priority, ooo);
      putw(channels[bb]->repeat ? 1 : 0, ooo);
      putw(channels[bb]->vol, ooo);
      putw(channels[bb]->panning, ooo);
      putw(channels[bb]->volAsPercentage, ooo);
      putw(channels[bb]->panningAsPercentage, ooo);
    }
    else
    {
      putw(-1, ooo);
    }
  }
  putw(crossFading, ooo);
  putw(crossFadeVolumePerStep, ooo);
  putw(crossFadeStep, ooo);
  putw(crossFadeVolumeAtStart, ooo);

  platform->RunPluginHooks(AGSE_SAVEGAME, (int)ooo);
  putw (MAGICNUMBER, ooo);  // to verify the plugins

  // save the room music volume
  putw(thisroom.options[ST_VOLUME], ooo);

  ccSerializeAllObjects(ooo);

  putw(current_music_type, ooo);

  update_polled_stuff();
}

// On Windows we could just use IIDFromString but this is platform-independant
void convert_guid_from_text_to_binary(const char *guidText, unsigned char *buffer) 
{
  guidText++; // skip {
  for (int bytesDone = 0; bytesDone < 16; bytesDone++)
  {
    if (*guidText == '-')
      guidText++;

    char tempString[3];
    tempString[0] = guidText[0];
    tempString[1] = guidText[1];
    tempString[2] = 0;
    int thisByte = 0;
    sscanf(tempString, "%X", &thisByte);

    buffer[bytesDone] = thisByte;
    guidText += 2;
  }

  // Swap bytes to give correct GUID order
  unsigned char temp;
  temp = buffer[0]; buffer[0] = buffer[3]; buffer[3] = temp;
  temp = buffer[1]; buffer[1] = buffer[2]; buffer[2] = temp;
  temp = buffer[4]; buffer[4] = buffer[5]; buffer[5] = temp;
  temp = buffer[6]; buffer[6] = buffer[7]; buffer[7] = temp;
}



// ============================================================================
// SAVE GAMES - RESTORE GAMES
// ============================================================================

// Some people have been having crashes with the save game list,
// so make sure the game name is valid
void safeguard_string (unsigned char *descript) {
  int it;
  for (it = 0; it < 50; it++) {
    if ((descript[it] < 1) || (descript[it] > 127))
      break;
  }
  if (descript[it] != 0)
    descript[it] = 0;
}


block read_serialized_bitmap(FILE* ooo) {
  block thispic;
  int picwid = getw(ooo);
  int pichit = getw(ooo);
  int piccoldep = getw(ooo);
  thispic = alw_create_bitmap_ex(piccoldep,picwid,pichit);
  if (thispic == NULL)
    return NULL;
  for (int vv=0; vv < pichit; vv++)
    fread(&BMP_LINE(thispic)[vv][0], picwid, piccoldep/8, ooo);
  return thispic;
}



void first_room_initialization() {
  starting_room = displayed_room;
  t1 = time(NULL);
  lastcounter=0;
  loopcounter=0;
  mouse_z_was = alw_mouse_z;
}

int restore_game_data (FILE *ooo, const char *nametouse) {
  int vv, bb;

  if (getw(ooo)!=SGVERSION) {
    fclose(ooo);
    return -3;
  }
  int isScreen = getw(ooo);
  if (isScreen) {
    // skip the screenshot
    wfreeblock(read_serialized_bitmap(ooo));
  }

  fgetstring_limit(rbuffer, ooo, 200);
  int vercmp = strcmp(rbuffer, ACI_VERSION_TEXT);
  if ((vercmp > 0) || (strcmp(rbuffer, LOWEST_SGVER_COMPAT) < 0) ||
      (strlen(rbuffer) > strlen(LOWEST_SGVER_COMPAT))) {
    fclose(ooo);
    return -4;
  }
  fgetstring_limit (rbuffer, ooo, 180);
  rbuffer[180] = 0;
  if (ac_stricmp (rbuffer, usetup.main_data_filename)) {
    fclose(ooo);
    return -5;
  }
  int gamescrnhit = getw(ooo);
  // a 320x240 game, they saved in a 320x200 room but try to restore
  // from within a 320x240 room, make it work
  if (final_scrn_hit == (gamescrnhit * 12) / 10)
    gamescrnhit = scrnhit;
  // they saved in a 320x240 room but try to restore from a 320x200
  // room, fix it
  else if (gamescrnhit == final_scrn_hit)
    gamescrnhit = scrnhit;

  if (gamescrnhit != scrnhit) {
    Display("This game was saved with the interpreter running at a different "
      "resolution. It cannot be restored.");
    fclose(ooo);
    return -6;
  }

  if (getw(ooo) != final_col_dep) {
    Display("This game was saved with the engine running at a different colour depth. It cannot be restored.");
    fclose(ooo);
    return -7;
  }

  unload_old_room();

  remove_screen_overlay(-1);
  is_complete_overlay=0; is_text_overlay=0;
  set_game_speed(getw(ooo));
  int sg_cur_mode=getw(ooo);
  int sg_cur_cursor=getw(ooo);
  offsetx = getw(ooo);
  offsety = getw(ooo);
  loopcounter = getw(ooo);

  for (bb = 1; bb < spriteset.elements; bb++) {
    if (game.spriteflags[bb] & SPF_DYNAMICALLOC) {
      // do this early, so that it changing guibuts doesn't
      // affect the restored data
      free_dynamic_sprite(bb);
    }
  }
  // ensure the sprite set is at least as large as it was
  // when the game was saved
  spriteset.enlargeTo(getw(ooo));
  // get serialized dynamic sprites
  int sprnum = getw(ooo);
  while (sprnum) {
    unsigned char spriteflag = fgetc(ooo);
    add_dynamic_sprite(sprnum, read_serialized_bitmap(ooo));
    game.spriteflags[sprnum] = spriteflag;
    sprnum = getw(ooo);
  }

  clear_music_cache();

  for (vv = 0; vv < game.numgui; vv++) {
    if (guibg[vv])
      wfreeblock (guibg[vv]);
    guibg[vv] = NULL;

    if (guibgbmp[vv])
      gfxDriver->DestroyDDB(guibgbmp[vv]);
    guibgbmp[vv] = NULL;
  }
  
  update_polled_stuff();

  ccFreeInstance(gameinstFork);
  ccFreeInstance(gameinst);
  gameinstFork = NULL;
  gameinst = NULL;
  for (vv = 0; vv < numScriptModules; vv++) {
    ccFreeInstance(moduleInstFork[vv]);
    ccFreeInstance(moduleInst[vv]);
    moduleInst[vv] = NULL;
  }

  if (dialogScriptsInst != NULL)
  {
    ccFreeInstance(dialogScriptsInst);
    dialogScriptsInst = NULL;
  }

  update_polled_stuff();

  // read the global script data segment
  int gdatasize = getw(ooo);
  char *newglobaldatabuffer = (char*)malloc(gdatasize);
  fread(newglobaldatabuffer, sizeof(char), gdatasize, ooo);
  //fread(&gameinst->globaldata[0],gdatasize,1,ooo);
  //ccUnFlattenGlobalData (gameinst);

  char *scriptModuleDataBuffers[MAX_SCRIPT_MODULES];
  int scriptModuleDataSize[MAX_SCRIPT_MODULES];

  if (getw(ooo) != numScriptModules)
    quit("wrong script module count; cannot restore game");
  for (vv = 0; vv < numScriptModules; vv++) {
    scriptModuleDataSize[vv] = getw(ooo);
    scriptModuleDataBuffers[vv] = (char*)malloc(scriptModuleDataSize[vv]);
    fread(&scriptModuleDataBuffers[vv][0], sizeof(char), scriptModuleDataSize[vv], ooo);
  }

  displayed_room = getw(ooo);

  // now the rooms
  for (vv=0;vv<MAX_ROOMS;vv++) {
    if (roomstats[vv].tsdata==NULL) ;
    else if (roomstats[vv].tsdatasize>0) {
      free(roomstats[vv].tsdata);
      roomstats[vv].tsdatasize=0; roomstats[vv].tsdata=NULL;
      }
    roomstats[vv].beenhere=0;
    }
  long gobackto=ftell(ooo);
  fclose(ooo);
  ooo=fopen(nametouse,"rb");
  fseek(ooo,gobackto,SEEK_SET);

  // read the room state for all the rooms the player has been in
  for (vv=0;vv<MAX_ROOMS;vv++) {
    if ((roomstats[vv].tsdatasize>0) & (roomstats[vv].tsdata!=NULL))
      free(roomstats[vv].tsdata);
    roomstats[vv].tsdatasize=0;
    roomstats[vv].tsdata=NULL;
    roomstats[vv].beenhere = fgetc (ooo);

    if (roomstats[vv].beenhere) {
      fread(&roomstats[vv],sizeof(RoomStatus),1,ooo);
      if (roomstats[vv].tsdatasize>0) {
        roomstats[vv].tsdata=(char*)malloc(roomstats[vv].tsdatasize+8);
        fread(&roomstats[vv].tsdata[0],roomstats[vv].tsdatasize,1,ooo);
      }
    }
  }

/*  for (vv=0;vv<MAX_ROOMS;vv++) {
    if ((roomstats[vv].tsdatasize>0) & (roomstats[vv].tsdata!=NULL))
      free(roomstats[vv].tsdata);
    roomstats[vv].tsdatasize=0;
    roomstats[vv].tsdata=NULL;
  }
  int numtoread=getw(ooo);
  if ((numtoread < 0) | (numtoread>MAX_ROOMS)) {
    sprintf(rbuffer,"Save game has invalid value for rooms_entered: %d",numtoread);
    quit(rbuffer);
    }
  fread(&roomstats[0],sizeof(RoomStatus),numtoread,ooo);
  for (vv=0;vv<numtoread;vv++) {
    if (roomstats[vv].tsdatasize>0) {
      roomstats[vv].tsdata=(char*)malloc(roomstats[vv].tsdatasize+5);
      fread(&roomstats[vv].tsdata[0],roomstats[vv].tsdatasize,1,ooo);
      }
    else roomstats[vv].tsdata=NULL;
    }*/

  int speech_was = play.want_speech, musicvox = play.seperate_music_lib;
  // preserve the replay settings
  int playback_was = play.playback, recording_was = play.recording;
  int gamestep_was = play.gamestep;
  int screenfadedout_was = play.screen_is_faded_out;
  int roomchanges_was = play.room_changes;
  // make sure the pointer is preserved
  int *gui_draw_order_was = play.gui_draw_order;

  free_do_once_tokens();

  //fread (&play, 76, 4, ooo);
  //fread (((char*)&play) + 78*4, sizeof(GameState) - 78*4, 1, ooo);
  fread(&play,sizeof(GameState),1,ooo);
  // Preserve whether the music vox is available
  play.seperate_music_lib = musicvox;
  // If they had the vox when they saved it, but they don't now
  if ((speech_was < 0) && (play.want_speech >= 0))
    play.want_speech = (-play.want_speech) - 1;
  // If they didn't have the vox before, but now they do
  else if ((speech_was >= 0) && (play.want_speech < 0))
    play.want_speech = (-play.want_speech) - 1;

  play.screen_is_faded_out = screenfadedout_was;
  play.playback = playback_was;
  play.recording = recording_was;
  play.gamestep = gamestep_was;
  play.room_changes = roomchanges_was;
  play.gui_draw_order = gui_draw_order_was;

  if (play.num_do_once_tokens > 0)
  {
    play.do_once_tokens = (char**)malloc(sizeof(char*) * play.num_do_once_tokens);
    for (bb = 0; bb < play.num_do_once_tokens; bb++)
    {
      fgetstring_limit(rbuffer, ooo, 200);
      play.do_once_tokens[bb] = (char*)malloc(strlen(rbuffer) + 1);
      strcpy(play.do_once_tokens[bb], rbuffer);
    }
  }

  fread(&play.gui_draw_order[0], sizeof(int), game.numgui, ooo);
  fread(&mls[0],sizeof(MoveList), game.numcharacters + MAX_INIT_SPR + 1, ooo);

  // save pointer members before reading
  char* gswas=game.globalscript;
  ccScript* compsc=game.compiled_script;
  CharacterInfo* chwas=game.chars;
  WordsDictionary *olddict = game.dict;
  char* mesbk[MAXGLOBALMES];
  int numchwas = game.numcharacters;
  for (vv=0;vv<MAXGLOBALMES;vv++) mesbk[vv]=game.messages[vv];
  int numdiwas = game.numdialog, numinvwas = game.numinvitems;
  int numviewswas = game.numviews;
  int numGuisWas = game.numgui;

  fread(&game,sizeof(GameSetupStructBase),1,ooo);

  if (game.numdialog!=numdiwas)
    quit("!Restore_Game: Game has changed (dlg), unable to restore");
  if ((numchwas != game.numcharacters) || (numinvwas != game.numinvitems))
    quit("!Restore_Game: Game has changed (inv), unable to restore position");
  if (game.numviews != numviewswas)
    quit("!Restore_Game: Game has changed (views), unable to restore position");

  fread(&game.invinfo[0], sizeof(InventoryItemInfo), game.numinvitems, ooo);
  fread(&game.mcurs[0], sizeof(MouseCursor), game.numcursors, ooo);

  if (game.invScripts == NULL)
  {
    for (bb = 0; bb < game.numinvitems; bb++)
      fread (&game.intrInv[bb]->timesRun[0], sizeof (int), MAX_NEWINTERACTION_EVENTS, ooo);
    for (bb = 0; bb < game.numcharacters; bb++)
      fread (&game.intrChar[bb]->timesRun[0], sizeof (int), MAX_NEWINTERACTION_EVENTS, ooo);
  }

  // restore pointer members
  game.globalscript=gswas;
  game.compiled_script=compsc;
  game.chars=chwas;
  game.dict = olddict;
  for (vv=0;vv<MAXGLOBALMES;vv++) game.messages[vv]=mesbk[vv];

  fread(&game.options[0], sizeof(int), OPT_HIGHESTOPTION+1, ooo);
  game.options[OPT_LIPSYNCTEXT] = fgetc(ooo);

  fread(&game.chars[0],sizeof(CharacterInfo),game.numcharacters,ooo);
  fread(&charextra[0],sizeof(CharacterExtras),game.numcharacters,ooo);
  if (roominst!=NULL) {  // so it doesn't overwrite the tsdata
    ccFreeInstance(roominstFork);
    ccFreeInstance(roominst); 
    roominstFork = NULL;
    roominst=NULL;
  }
  fread(&palette[0],sizeof(color),256,ooo);
  for (vv=0;vv<game.numdialog;vv++)
    fread(&dialog[vv].optionflags[0],sizeof(int),MAXTOPICOPTIONS,ooo);
  mouse_on_iface=getw(ooo);
  mouse_on_iface_button=getw(ooo);
  mouse_pushed_iface=getw(ooo);
  ifacepopped = getw(ooo);
  game_paused=getw(ooo);

  for (vv = 0; vv < game.numgui; vv++)
    unexport_gui_controls(vv);

  read_gui(ooo,guis,&game);

  if (numGuisWas != game.numgui)
    quit("!Restore_Game: Game has changed (GUIs), unable to restore position");

  for (vv = 0; vv < game.numgui; vv++)
    export_gui_controls(vv);

  numAnimButs = getw(ooo);
  fread(&animbuts[0], sizeof(AnimatingGUIButton), numAnimButs, ooo);

  if (getw(ooo) != game.audioClipTypeCount)
    quit("!Restore_Game: game has changed (audio types), unable to restore");

  fread(&game.audioClipTypes[0], sizeof(AudioClipType), game.audioClipTypeCount, ooo);

  short saved_light_levels[MAX_REGIONS];
  int   saved_tint_levels[MAX_REGIONS];
  fread(&saved_light_levels[0], sizeof(short), MAX_REGIONS, ooo);
  fread(&saved_tint_levels[0], sizeof(int), MAX_REGIONS, ooo);

  short saved_zoom_levels1[MAX_WALK_AREAS + 1];
  short saved_zoom_levels2[MAX_WALK_AREAS + 1];
  fread(&saved_zoom_levels1[0],sizeof(short), MAX_WALK_AREAS + 1, ooo);
  fread(&saved_zoom_levels2[0],sizeof(short), MAX_WALK_AREAS + 1, ooo);

  int doAmbient[MAX_SOUND_CHANNELS], cc, dd;
  int crossfadeInChannelWas = play.crossfading_in_channel;
  int crossfadeOutChannelWas = play.crossfading_out_channel;

  for (bb = 0; bb <= MAX_SOUND_CHANNELS; bb++)
  {
    stop_and_destroy_channel_ex(bb, false);
  }

  play.crossfading_in_channel = crossfadeInChannelWas;
  play.crossfading_out_channel = crossfadeOutChannelWas;

  fread(&ambient[0], sizeof(AmbientSound), MAX_SOUND_CHANNELS, ooo);

  for (bb = 1; bb < MAX_SOUND_CHANNELS; bb++) {
    if (ambient[bb].channel == 0)
      doAmbient[bb] = 0;
    else {
      doAmbient[bb] = ambient[bb].num;
      ambient[bb].channel = 0;
    }
  }

  numscreenover = getw(ooo);
  fread(&screenover[0],sizeof(ScreenOverlay),numscreenover,ooo);
  for (bb=0;bb<numscreenover;bb++) {
    if (screenover[bb].pic != NULL)
    {
      screenover[bb].pic = read_serialized_bitmap(ooo);
      screenover[bb].bmp = gfxDriver->CreateDDBFromBitmap(screenover[bb].pic, false);
    }
  }

  update_polled_stuff();

  // load into a temp array since ccUnserialiseObjects will destroy
  // it otherwise
  block dynamicallyCreatedSurfacesFromSaveGame[MAX_DYNAMIC_SURFACES];
  for (bb = 0; bb < MAX_DYNAMIC_SURFACES; bb++)
  {
    if (fgetc(ooo) == 0)
    {
      dynamicallyCreatedSurfacesFromSaveGame[bb] = NULL;
    }
    else
    {
      dynamicallyCreatedSurfacesFromSaveGame[bb] = read_serialized_bitmap(ooo);
    }
  }

  update_polled_stuff();

  block newbscene[MAX_BSCENE];
  for (bb = 0; bb < MAX_BSCENE; bb++)
    newbscene[bb] = NULL;

  if (displayed_room >= 0) {

    for (bb = 0; bb < MAX_BSCENE; bb++) {
      newbscene[bb] = NULL;
      if (play.raw_modified[bb]) {
        newbscene[bb] = read_serialized_bitmap (ooo);
      }
    }
    bb = getw(ooo);
    if (raw_saved_screen != NULL) {
      wfreeblock(raw_saved_screen);
      raw_saved_screen = NULL;
    }
    if (bb)
      raw_saved_screen = read_serialized_bitmap(ooo);

    if (troom.tsdata != NULL)
      free (troom.tsdata);
    // get the current troom, in case they save in room 600 or whatever
    fread(&troom,sizeof(RoomStatus),1,ooo);
    if (troom.tsdatasize > 0) {
      troom.tsdata=(char*)malloc(troom.tsdatasize+5);
      fread(&troom.tsdata[0],troom.tsdatasize,1,ooo);
    }
    else
      troom.tsdata = NULL;

  }

  if (getw (ooo) != numGlobalVars) 
    quit("!Game has been modified since save; unable to restore game (GM01)");

  fread (&globalvars[0], sizeof(InteractionVariable), numGlobalVars, ooo);

  if (getw(ooo) != game.numviews)
    quit("!Game has been modified since save; unable to restore (GV02)");

  for (bb = 0; bb < game.numviews; bb++) {
    for (cc = 0; cc < views[bb].numLoops; cc++) {
      for (dd = 0; dd < views[bb].loops[cc].numFrames; dd++)
      {
        views[bb].loops[cc].frames[dd].sound = getw(ooo);
        views[bb].loops[cc].frames[dd].pic = getw(ooo);
      }
    }
  }

  if (getw(ooo) != MAGICNUMBER+1)
    quit("!Game has been modified since save; unable to restore (GV03)");

  if (getw(ooo) != game.audioClipCount)
    quit("Game has changed: different audio clip count");

  play.crossfading_in_channel = 0;
  play.crossfading_out_channel = 0;
  int channelPositions[MAX_SOUND_CHANNELS + 1];
  for (bb = 0; bb <= MAX_SOUND_CHANNELS; bb++)
  {
    channelPositions[bb] = 0;
    int audioClipIndex = getw(ooo);
    if (audioClipIndex >= 0)
    {
      if (audioClipIndex >= game.audioClipCount)
        quit("save game error: invalid audio clip index");

      channelPositions[bb] = getw(ooo);
      if (channelPositions[bb] < 0) channelPositions[bb] = 0;
      int priority = getw(ooo);
      int repeat = getw(ooo);
      int vol = getw(ooo);
      int pan = getw(ooo);
      int volAsPercent = getw(ooo);
      int panAsPercent = getw(ooo);
      play_audio_clip_on_channel(bb, &game.audioClips[audioClipIndex], priority, repeat, channelPositions[bb]);
      if (channels[bb] != NULL)
      {
        channels[bb]->set_panning(pan);
        channels[bb]->set_volume(vol);
        channels[bb]->panningAsPercentage = panAsPercent;
        channels[bb]->volAsPercentage = volAsPercent;
      }
    }
  }
  if ((crossfadeInChannelWas > 0) && (channels[crossfadeInChannelWas] != NULL))
    play.crossfading_in_channel = crossfadeInChannelWas;
  if ((crossfadeOutChannelWas > 0) && (channels[crossfadeOutChannelWas] != NULL))
    play.crossfading_out_channel = crossfadeOutChannelWas;

  // If there were synced audio tracks, the time taken to load in the
  // different channels will have thrown them out of sync, so re-time it
  for (bb = 0; bb <= MAX_SOUND_CHANNELS; bb++)
  {
    if ((channelPositions[bb] > 0) && (channels[bb] != NULL) && (channels[bb]->done == 0))
    {
      channels[bb]->seek(channelPositions[bb]);
    }
  }
  crossFading = getw(ooo);
  crossFadeVolumePerStep = getw(ooo);
  crossFadeStep = getw(ooo);
  crossFadeVolumeAtStart = getw(ooo);

  recache_queued_clips_after_loading_save_game();

  platform->RunPluginHooks(AGSE_RESTOREGAME, (int)ooo);
  if (getw(ooo) != (unsigned)MAGICNUMBER)
    quit("!One of the game plugins did not restore its game data correctly.");

  // save the new room music vol for later use
  int newRoomVol = getw(ooo);

  if (ccUnserializeAllObjects(ooo, &ccUnserializer))
    quitprintf("LoadGame: Error during deserialization: %s", ccErrorString);

  // preserve legacy music type setting
  current_music_type = getw(ooo);

  fclose(ooo);

  // restore these to the ones retrieved from the save game
  for (bb = 0; bb < MAX_DYNAMIC_SURFACES; bb++)
  {
    dynamicallyCreatedSurfaces[bb] = dynamicallyCreatedSurfacesFromSaveGame[bb];
  }

  if (create_global_script())
    quitprintf("Unable to recreate global script: %s", ccErrorString);

  if (gameinst->globaldatasize != gdatasize)
    quit("!Restore_game: Global script changed, cannot restore game");

  // read the global data into the newly created script
  memcpy(&gameinst->globaldata[0], newglobaldatabuffer, gdatasize);
  free(newglobaldatabuffer);
  ccUnFlattenGlobalData(gameinst);

  // restore the script module data
  for (bb = 0; bb < numScriptModules; bb++) {
    if (scriptModuleDataSize[bb] != moduleInst[bb]->globaldatasize)
      quit("!Restore Game: script module global data changed, unable to restore");
    memcpy(&moduleInst[bb]->globaldata[0], scriptModuleDataBuffers[bb], scriptModuleDataSize[bb]);
    free(scriptModuleDataBuffers[bb]);
    ccUnFlattenGlobalData(moduleInst[bb]);
  }
  

  setup_player_character(game.playercharacter);

  int gstimer=play.gscript_timer;
  int oldx1 = play.mboundx1, oldx2 = play.mboundx2;
  int oldy1 = play.mboundy1, oldy2 = play.mboundy2;
  int musicWasRepeating = play.current_music_repeating;
  int newms = play.cur_music_number;

  // disable the queue momentarily
  int queuedMusicSize = play.music_queue_size;
  play.music_queue_size = 0;

  update_polled_stuff();

  if (displayed_room >= 0)
    load_new_room(displayed_room,NULL);//&game.chars[game.playercharacter]);

  update_polled_stuff();

  play.gscript_timer=gstimer;

  // restore the correct room volume (they might have modified
  // it with SetMusicVolume)
  thisroom.options[ST_VOLUME] = newRoomVol;

  filter->SetMouseLimit(oldx1,oldy1,oldx2,oldy2);
  
  set_cursor_mode(sg_cur_mode);
  set_mouse_cursor(sg_cur_cursor);
  if (sg_cur_mode == MODE_USE)
    SetActiveInventory (playerchar->activeinv);
  // ensure that the current cursor is locked
  spriteset.precache(game.mcurs[sg_cur_cursor].pic);

#if (ALW_ALLEGRO_DATE > 19990103)
  alw_set_window_title(play.game_name);
#endif

  update_polled_stuff();

  if (displayed_room >= 0) {

    for (bb = 0; bb < MAX_BSCENE; bb++) {
      if (newbscene[bb]) {
        wfreeblock(thisroom.ebscene[bb]);
        thisroom.ebscene[bb] = newbscene[bb];
      }
    }

    in_new_room=3;  // don't run "enters screen" events
    // now that room has loaded, copy saved light levels in
    memcpy(&thisroom.regionLightLevel[0],&saved_light_levels[0],sizeof(short)*MAX_REGIONS);
    memcpy(&thisroom.regionTintLevel[0],&saved_tint_levels[0],sizeof(int)*MAX_REGIONS);
    generate_light_table();

    memcpy(&thisroom.walk_area_zoom[0], &saved_zoom_levels1[0], sizeof(short) * (MAX_WALK_AREAS + 1));
    memcpy(&thisroom.walk_area_zoom2[0], &saved_zoom_levels2[0], sizeof(short) * (MAX_WALK_AREAS + 1));

    on_background_frame_change();

  }

  gui_disabled_style = convert_gui_disabled_style(game.options[OPT_DISABLEOFF]);
/*
  play_sound(-1);
  
  stopmusic();
  // use the repeat setting when the current track was started
  int musicRepeatSetting = play.music_repeat;
  SetMusicRepeat(musicWasRepeating);
  if (newms>=0) {
    // restart the background music
    if (newms == 1000)
      PlayMP3File (play.playmp3file_name);
    else {
      play.cur_music_number=2000;  // make sure it gets played
      newmusic(newms);
    }
  }
  SetMusicRepeat(musicRepeatSetting);
  if (play.silent_midi)
    PlaySilentMIDI (play.silent_midi);
  SeekMIDIPosition(midipos);
  //SeekMODPattern (modtrack);
  //SeekMP3PosMillis (mp3mpos);

  if (musicpos > 0) {
    // For some reason, in Prodigal after this Seek line is called
    // it can cause the next update_polled_stuff to crash;
    // must be some sort of bug in AllegroMP3
    if ((crossFading > 0) && (channels[crossFading] != NULL))
      channels[crossFading]->seek(musicpos);
    else if (channels[SCHAN_MUSIC] != NULL)
      channels[SCHAN_MUSIC]->seek(musicpos);
  }*/

  // restore the queue now that the music is playing
  play.music_queue_size = queuedMusicSize;
  
  if (play.digital_master_volume >= 0)
    System_SetVolume(play.digital_master_volume);

  for (vv = 1; vv < MAX_SOUND_CHANNELS; vv++) {
    if (doAmbient[vv])
      PlayAmbientSound(vv, doAmbient[vv], ambient[vv].vol, ambient[vv].x, ambient[vv].y);
  }

  for (vv = 0; vv < game.numgui; vv++) {
    guibg[vv] = alw_create_bitmap_ex (final_col_dep, guis[vv].wid, guis[vv].hit);
    guibg[vv] = gfxDriver->ConvertBitmapToSupportedColourDepth(guibg[vv]);
  }

  if (gfxDriver->SupportsGammaControl())
    gfxDriver->SetGamma(play.gamma_adjustment);

  guis_need_update = 1;

  play.ignore_user_input_until_time = 0;
  update_polled_stuff();

  platform->RunPluginHooks(AGSE_POSTRESTOREGAME, 0);

  if (displayed_room < 0) {
    // the restart point, no room was loaded
    load_new_room(playerchar->room, playerchar);
    playerchar->prevroom = -1;

    first_room_initialization();
  }

  if ((play.music_queue_size > 0) && (cachedQueuedMusic == NULL)) {
    cachedQueuedMusic = load_music_from_disk(play.music_queue[0], 0);
  }

  return 0;
}


int do_game_load(const char *nametouse, int slotNumber, char *descrp, int *wantShot)
{
  gameHasBeenRestored++;

  FILE*ooo=fopen(nametouse,"rb");
  if (ooo==NULL)
    return -1;

  // skip Vista header
  fseek(ooo, sizeof(RICH_GAME_MEDIA_HEADER), SEEK_SET);

  fread(rbuffer,sgsiglen,1,ooo);
  rbuffer[sgsiglen]=0;
  if (strcmp(rbuffer,sgsig)!=0) {
    // not a save game
    fclose(ooo);
    return -2; 
  }
  int oldeip = our_eip;
  set_eip(2050);

  fgetstring_limit(rbuffer,ooo, 180);
  rbuffer[180] = 0;
  safeguard_string ((unsigned char*)rbuffer);

  if (descrp!=NULL) {
    // just want slot description, so return
    strcpy(descrp,rbuffer);
    fclose (ooo);
    our_eip = oldeip;
    return 0;
  }

  if (wantShot != NULL) {
    // just want the screenshot
    if (getw(ooo)!=SGVERSION) {
      fclose(ooo);
      return -3;
    }
    int isScreen = getw(ooo);
    *wantShot = 0;

    if (isScreen) {
      int gotSlot = spriteset.findFreeSlot();
      // load the screenshot
      block redin = read_serialized_bitmap(ooo);
      if (gotSlot > 0) {
        // add it into the sprite set
        add_dynamic_sprite(gotSlot, gfxDriver->ConvertBitmapToSupportedColourDepth(redin));

        *wantShot = gotSlot;
      }
      else
      {
        alw_destroy_bitmap(redin);
      }
    }
    fclose (ooo);
    our_eip = oldeip;
    return 0;
  }

  set_eip(2051);

  // do the actual restore
  int ress = restore_game_data(ooo, nametouse);

  our_eip = oldeip;

  if (ress == -5) {
    // saved in different game
    RunAGSGame (rbuffer, 0, 0);
    load_new_game_restore = slotNumber;
    return 0;
  }

  if (ress)
    return ress;

  run_on_event (GE_RESTORE_GAME, slotNumber);

  // ensure keyboard buffer is clean
  // use the raw versions rather than the rec_ versions so we don't
  // interfere with the replay sync
  while (alw_keypressed()) alw_readkey();

  return 0;
}

int load_game(int slotn, char*descrp, int *wantShot) {
  char nametouse[260];
  get_save_game_path(slotn, nametouse);

  return do_game_load(nametouse, slotn, descrp, wantShot);
}



// ============================================================================
// CUTSCENES
// ============================================================================

// Helper functions used by StartCutscene/EndCutscene, but also
// by SkipUntilCharacterStops
void initialize_skippable_cutscene() {
  play.end_cutscene_music = -1;
}

void stop_fast_forwarding() {
  // when the skipping of a cutscene comes to an end, update things
  play.fast_forward = 0;
  setpal();
  if (play.end_cutscene_music >= 0)
    newmusic(play.end_cutscene_music);

  // Restore actual volume of sounds
  for (int aa = 0; aa < MAX_SOUND_CHANNELS; aa++)
  {
    if ((channels[aa] != NULL) && (!channels[aa]->done) && 
        (channels[aa]->volAsPercentage == 0) &&
        (channels[aa]->originalVolAsPercentage > 0)) 
    {
      channels[aa]->volAsPercentage = channels[aa]->originalVolAsPercentage;
      channels[aa]->set_volume((channels[aa]->volAsPercentage * 255) / 100);
    }
  }

  update_music_volume();
}


void EndSkippingUntilCharStops() {
  // not currently skipping, so ignore
  if (play.skip_until_char_stops < 0)
    return;

  stop_fast_forwarding();
  play.skip_until_char_stops = -1;
}



// ============================================================================
// SCRIPT EXPORTS
// ============================================================================

// quick hack to get Gemini Rue running.
int ShellExecute(char * operation, char * file, char * parameters, int showCommand) {    return 0;  }
void srSetSnowDriftSpeed(int x, int y){ }
void srSetSnowDriftRange(int x, int y){ }
void srSetSnowFallSpeed(int x, int y){ }
void srChangeSnowAmount(int x){ }
void srSetSnowBaseline(int x, int y){ }
void srChangeRainAmount(int x){}
void srSetRainView(int x, int y, int z, int omega){}
void srSetRainTransparency(int x, int y){}
void srSetSnowTransparency(int x, int y){}
void srSetSnowDefaultView(int x, int y){}
void srSetRainDefaultView(int x, int y){}
void srSetRainWindSpeed(int x){}
void srSetSnowWindSpeed(int x){}
void srSetWindSpeed(int x){}
void srSetRainBaseline(int x, int y){}
void srSetBaseline(int x, int y){}
void srSetSnowAmount(int x){}
void srSetRainAmount(int x){}
void srSetRainFallSpeed(int x, int y){}
void srSetSnowView(int x, int y, int z, int omega){}

  
#define scAdd_External_Symbol ccAddExternalSymbol
void setup_script_exports() {
  // the ^5 after the function name is the number of params
  // this is to allow an extra parameter to be added in a later
  // version without screwing up the stack in previous versions
  // (just export both the ^5 and the ^6 as seperate funcs)

  register_audio_script_functions();
  register_character_script_functions();
  register_object_script_functions();
  register_dialog_script_functions();
  register_dialog_options_rendering_info_script_functions();
  register_file_script_functions();
  register_overlay_script_functions();
  register_inventory_item_script_functions();
  register_gui_script_functions();
  register_gui_control_script_functions();
  register_label_script_functions();
  register_button_script_functions();
  register_slider_script_functions();
  register_textbox_script_functions();
  register_inv_window_script_functions();
  register_list_box_script_functions();
  register_mouse_script_functions();
  register_maths_script_functions();
  register_hotspot_script_functions();
  register_region_script_functions();
  register_datetime_script_functions();
  register_drawing_surface_script_functions();
  register_dynamic_sprite_script_functions();
  register_string_script_functions();
  register_game_script_functions();
  register_system_script_functions();
  register_room_script_functions();
  register_parser_script_functions();
  register_view_frame_script_functions();
  register_text_script_functions();
  register_screen_script_functions();
  register_palette_script_functions();
  register_recording_script_functions();
  register_multimedia_script_functions();

  //scAdd_External_Symbol("GetLanguageString",(void *)GetLanguageString);
  
  // gemini rue hack
  scAdd_External_Symbol("ShellExecute",(void *)ShellExecute);
  scAdd_External_Symbol("srSetSnowDriftSpeed",(void *)srSetSnowDriftSpeed);
  scAdd_External_Symbol("srSetSnowDriftRange",(void *)srSetSnowDriftRange);
  scAdd_External_Symbol("srSetSnowFallSpeed",(void *)srSetSnowFallSpeed);
  scAdd_External_Symbol("srChangeSnowAmount",(void *)srChangeSnowAmount);
  scAdd_External_Symbol("srSetSnowBaseline",(void *)srSetSnowBaseline);
  scAdd_External_Symbol("srChangeRainAmount",(void *)srChangeRainAmount);
  scAdd_External_Symbol("srSetRainView",(void *)srSetRainView);
  scAdd_External_Symbol("srSetRainTransparency",(void *)srSetRainTransparency);
  scAdd_External_Symbol("srSetSnowTransparency",(void *)srSetSnowTransparency);
  scAdd_External_Symbol("srSetSnowDefaultView",(void *)srSetSnowDefaultView);
  scAdd_External_Symbol("srSetRainDefaultView",(void *)srSetRainDefaultView);
  scAdd_External_Symbol("srSetRainWindSpeed",(void *)srSetRainWindSpeed);
  scAdd_External_Symbol("srSetSnowWindSpeed",(void *)srSetSnowWindSpeed);
  scAdd_External_Symbol("srSetWindSpeed",(void *)srSetWindSpeed);
  scAdd_External_Symbol("srSetRainBaseline",(void *)srSetRainBaseline);
  scAdd_External_Symbol("srSetBaseline",(void *)srSetBaseline);
  scAdd_External_Symbol("srSetSnowAmount",(void *)srSetSnowAmount);
  scAdd_External_Symbol("srSetRainAmount",(void *)srSetRainAmount);
  scAdd_External_Symbol("srSetRainFallSpeed",(void *)srSetRainFallSpeed);
  scAdd_External_Symbol("srSetSnowView",(void *)srSetSnowView);
  
  
  scAdd_External_Symbol("game",&play);
  scAdd_External_Symbol("gs_globals",&play.globalvars[0]);
  scAdd_External_Symbol("mouse",&global_mouse_state.scmouse);
  scAdd_External_Symbol("palette",&palette[0]);
  scAdd_External_Symbol("system",&scsystem);
  scAdd_External_Symbol("savegameindex",&play.filenumbers[0]);
}






// ============================================================================
// ROOM - NEW ROOM
// ============================================================================

void check_new_room() {
  // if they're in a new room, run Player Enters Screen and on_event(ENTER_ROOM)
  if ((in_new_room>0) & (in_new_room!=3)) {
    EventHappened evh;
    evh.type = EV_RUNEVBLOCK;
    evh.data1 = EVB_ROOM;
    evh.data2 = 0;
    evh.data3 = 5;
    evh.player=game.playercharacter;
    // make sure that any script calls don't re-call enters screen
    int newroom_was = in_new_room;
    in_new_room = 0;
    play.disabled_user_interface ++;
    process_event(&evh);
    play.disabled_user_interface --;
    in_new_room = newroom_was;
//    setevent(EV_RUNEVBLOCK,EVB_ROOM,0,5);
  }
}



// ============================================================================
// GRAPHICS
// ============================================================================

void construct_virtual_screen(bool fullRedraw) 
{
  gfxDriver->ClearDrawList();

  if (play.fast_forward)
    return;

  set_eip(3);

  gfxDriver->UseSmoothScaling(IS_ANTIALIAS_SPRITES);

  platform->RunPluginHooks(AGSE_PRERENDER, 0);

  if (displayed_room >= 0) {
    
    if (fullRedraw)
      invalidate_screen();

    draw_screen_background();
  }
  else if (!gfxDriver->RequiresFullRedrawEachFrame()) 
  {
    // if the driver is not going to redraw the screen,
    // black it out so we don't get cursor trails
    alw_clear_bitmap(abuf);
  }

  // reset the Baselines Changed flag now that we've drawn stuff
  walk_behind_baselines_changed = 0;

  // make sure that the mp3 is always playing smoothly
  acaudio_update_mp3();
  set_eip(4);
  draw_screen_overlay();

  if (fullRedraw)
  {
    // ensure the virtual screen is reconstructed
    // in case we want to take any screenshots before
    // the next game loop
    if (gfxDriver->UsesMemoryBackBuffer())
      gfxDriver->RenderToBackBuffer();
  }
}

// Draw everything 
void render_graphics(IDriverDependantBitmap *extraBitmap, int extraX, int extraY) {

  construct_virtual_screen(false);
  set_eip(5);

  if (extraBitmap != NULL)
    gfxDriver->DrawSprite(extraX, extraY, extraBitmap);

  update_screen();
}



// ============================================================================
// MAIN LOOP
// ============================================================================

void mainloop(bool checkControls, IDriverDependantBitmap *extraBitmap, int extraX, int extraY) {
  acaudio_update_mp3();

  int numEventsAtStartOfFunction = numevents;

  if (want_exit) {
    want_exit = 0;
    proper_exit = 1;
    quit("||exit!");
/*#ifdef WINDOWS_VERSION
    // the Quit thread is running now, so exit this main one.
    ExitThread (1);
#endif*/
  }
  ccNotifyScriptStillAlive ();
  set_eip(1);
  timerloop=0;
  if (want_quit) {
    want_quit = 0;
    QuitGame(1);
    }
  if ((in_enters_screen != 0) & (displayed_room == starting_room))
    quit("!A text script run in the Player Enters Screen event caused the\n"
      "screen to be updated. If you need to use Wait(), do so in After Fadein");
  if ((in_enters_screen != 0) && (done_es_error == 0)) {
    debug_log("Wait() was used in Player Enters Screen - use Enters Screen After Fadein instead");
    done_es_error = 1;
  }
  if (no_blocking_functions)
    quit("!A blocking function was called from within a non-blocking event such as " REP_EXEC_ALWAYS_NAME);

  // if we're not fading in, don't count the fadeouts
  if ((play.no_hicolor_fadein) && (game.options[OPT_FADETYPE] == FADE_NORMAL))
    play.screen_is_faded_out = 0;

  set_eip(1014);
  update_gui_disabled_status();

  set_eip(1004);
  if (in_new_room == 0) {
    // Run the room and game script repeatedly_execute
    run_function_on_non_blocking_thread(&repExecAlways);
    setevent(EV_TEXTSCRIPT,TS_REPEAT);
    setevent(EV_RUNEVBLOCK,EVB_ROOM,0,6);  
  }
  // run this immediately to make sure it gets done before fade-in
  // (player enters screen)
  check_new_room ();
  
  set_eip(1005);

  if ((play.ground_level_areas_disabled & GLED_INTERACTION) == 0) {
    // check if he's standing on a hotspot
    int hotspotThere = get_hotspot_at(playerchar->x, playerchar->y);
    // run Stands on Hotspot event
    setevent(EV_RUNEVBLOCK, EVB_HOTSPOT, hotspotThere, 0);

    // check current region
    int onRegion = GetRegionAt (playerchar->x, playerchar->y);
    int inRoom = displayed_room;

    if (onRegion != play.player_on_region) {
      // we need to save this and set play.player_on_region
      // now, so it's correct going into RunRegionInteraction
      int oldRegion = play.player_on_region;
    
      play.player_on_region = onRegion;
      // Walks Off last region
      if (oldRegion > 0)
        RunRegionInteraction (oldRegion, 2);
      // Walks Onto new region
      if (onRegion > 0)
        RunRegionInteraction (onRegion, 1);
    }
    if (play.player_on_region > 0)   // player stands on region
      RunRegionInteraction (play.player_on_region, 0);

    // one of the region interactions sent us to another room
    if (inRoom != displayed_room) {
      check_new_room();
    }

    // if in a Wait loop which is no longer valid (probably
    // because the Region interaction did a NewRoom), abort
    // the rest of the loop
    if ((restrict_until) && (!wait_loop_still_valid())) {
      // cancel the Rep Exec and Stands on Hotspot events that
      // we just added -- otherwise the event queue gets huge
      numevents = numEventsAtStartOfFunction;
      return;
    }
  } // end if checking ground level interactions

  mouse_on_iface=-1;

  check_debug_keys();

  // don't let the player do anything before the screen fades in
  if ((in_new_room == 0) && (checkControls)) {
    int inRoom = displayed_room;
    check_controls();
    // If an inventory interaction changed the room
    if (inRoom != displayed_room)
      check_new_room();
  }
  set_eip(2);
  if (debug_flags & DBG_NOUPDATE) ;
  else if (game_paused==0) update_stuff();

  // update animating GUI buttons
  // this bit isn't in update_stuff because it always needs to
  // happen, even when the game is paused
  for (int aa = 0; aa < numAnimButs; aa++) {
    if (UpdateAnimatingButton(aa)) {
      StopButtonAnimation(aa);
      aa--;
    } 
  }

  update_polled_stuff_and_crossfade();

  if (!play.fast_forward) {
    int mwasatx=mousex,mwasaty=mousey;

    // Only do this if we are not skipping a cutscene
    render_graphics(extraBitmap, extraX, extraY);

    // Check Mouse Moves Over Hotspot event
    static int offsetxWas = -100, offsetyWas = -100;

    if (((mwasatx!=mousex) || (mwasaty!=mousey) ||
        (offsetxWas != offsetx) || (offsetyWas != offsety)) &&
        (displayed_room >= 0)) 
    {
      // mouse moves over hotspot
      if (__GetLocationType(divide_down_coordinate(mousex), divide_down_coordinate(mousey), 1) == LOCTYPE_HOTSPOT) {
        int onhs = getloctype_index;
        
        setevent(EV_RUNEVBLOCK,EVB_HOTSPOT,onhs,6); 
      }
    }

    offsetxWas = offsetx;
    offsetyWas = offsety;

#ifdef MAC_VERSION
    // take a breather after the heavy work
    // cuts down on CPU usage and reduces the fan noise
    alw_rest(2);
#endif
  }
  set_eip(6);
  new_room_was = in_new_room;
  if (in_new_room>0)
    setevent(EV_FADEIN,0,0,0);
  in_new_room=0;
  update_events();
  if ((new_room_was > 0) && (in_new_room == 0)) {
    // if in a new room, and the room wasn't just changed again in update_events,
    // then queue the Enters Screen scripts
    // run these next time round, when it's faded in
    if (new_room_was==2)  // first time enters screen
      setevent(EV_RUNEVBLOCK,EVB_ROOM,0,4);
    if (new_room_was!=3)   // enters screen after fadein
      setevent(EV_RUNEVBLOCK,EVB_ROOM,0,7);
  }
  set_eip(7);
//    if (ac_mgetbutton()>NONE) break;
  update_polled_stuff();
  if (play.bg_anim_delay > 0) play.bg_anim_delay--;
  else if (play.bg_frame_locked) ;
  else {
    play.bg_anim_delay = play.anim_background_speed;
    play.bg_frame++;
    if (play.bg_frame >= thisroom.num_bscenes)
      play.bg_frame=0;
    if (thisroom.num_bscenes >= 2) {
      // get the new frame's palette
      on_background_frame_change();
    }
  }
  loopcounter++;

  if (play.wait_counter > 0) play.wait_counter--;
  if (play.shakesc_length > 0) play.shakesc_length--;

  if (loopcounter % 5 == 0)
  {
    update_ambient_sound_vol();
    update_directional_sound_vol();
  }

  if (replay_start_this_time) {
    replay_start_this_time = 0;
    start_replay_record();
  }

  if (play.fast_forward)
    return;

  set_eip(72);
  if (time(NULL) != t1) {
    t1 = time(NULL);
    fps = loopcounter - lastcounter;
    lastcounter = loopcounter;
  }

  // make sure we poll, cos a low framerate (eg 5 fps) could stutter
  // mp3 music
  while (timerloop == 0) {
    update_polled_stuff();
    platform->YieldCPU();
  }
}


int check_write_access() {

  if (platform->GetDiskFreeSpaceMB() < 2)
    return 0;

  set_eip(-1895);

  // The Save Game Dir is the only place that we should write to
  char tempPath[MAX_PATH];
  sprintf(tempPath, "%s""tmptest.tmp", saveGameDirectory);
  FILE *yy = fopen(tempPath, "wb");
  if (yy == NULL)
    return 0;

  set_eip(-1896);

  fwrite("just to test the drive free space", 30, 1, yy);
  fclose(yy);

  set_eip(-1897);

  if (unlink(tempPath))
    return 0;

  return 1;
}




int wait_loop_still_valid() {
  if (restrict_until == 0)
    quit("end_wait_loop called but game not in loop_until state");
  int retval = restrict_until;

  if (restrict_until==UNTIL_MOVEEND) {
    short*wkptr=(short*)user_disabled_data;
    if (wkptr[0]<1) retval=0;
  }
  else if (restrict_until==UNTIL_CHARIS0) {
    char*chptr=(char*)user_disabled_data;
    if (chptr[0]==0) retval=0;
  }
  else if (restrict_until==UNTIL_NEGATIVE) {
    short*wkptr=(short*)user_disabled_data;
    if (wkptr[0]<0) retval=0;
  }
  else if (restrict_until==UNTIL_INTISNEG) {
    int*wkptr=(int*)user_disabled_data;
    if (wkptr[0]<0) retval=0;
  }
  else if (restrict_until==UNTIL_NOOVERLAY) {
    if (is_text_overlay < 1) retval=0;
  }
  else if (restrict_until==UNTIL_INTIS0) {
    int*wkptr=(int*)user_disabled_data;
    if (wkptr[0]<0) retval=0;
  }
  else if (restrict_until==UNTIL_SHORTIS0) {
    short*wkptr=(short*)user_disabled_data;
    if (wkptr[0]==0) retval=0;
  }
  else quit("loop_until: unknown until event");

  return retval;
}

int main_game_loop() {
  if (displayed_room < 0)
    quit("!A blocking function was called before the first room has been loaded");

  mainloop(true);
  // Call GetLocationName - it will internally force a GUI refresh
  // if the result it returns has changed from last time
  char tempo[STD_BUFFER_SIZE];
  GetLocationName(divide_down_coordinate(mousex), divide_down_coordinate(mousey), tempo);

  if ((play.get_loc_name_save_cursor >= 0) &&
      (play.get_loc_name_save_cursor != play.get_loc_name_last_time) &&
      (mouse_on_iface < 0) && (ifacepopped < 0)) {
    // we have saved the cursor, but the mouse location has changed
    // and it's time to restore it
    play.get_loc_name_save_cursor = -1;
    set_cursor_mode(play.restore_cursor_mode_to);

    if (cur_mode == play.restore_cursor_mode_to)
    {
      // make sure it changed -- the new mode might have been disabled
      // in which case don't change the image
      set_mouse_cursor(play.restore_cursor_image_to);
    }
    DEBUG_CONSOLE("Restore mouse to mode %d cursor %d", play.restore_cursor_mode_to, play.restore_cursor_image_to);
  }

  set_eip(76);
  if (restrict_until==0) ;
  else {
    restrict_until = wait_loop_still_valid();
    set_eip(77);

    if (restrict_until==0) {
      set_default_cursor();
      guis_need_update = 1;
      play.disabled_user_interface--;
/*      if (user_disabled_for==FOR_ANIMATION)
        run_animation((FullAnimation*)user_disabled_data2,user_disabled_data3);
      else*/ if (user_disabled_for==FOR_EXITLOOP) {
        user_disabled_for=0; return -1; }
      else if (user_disabled_for==FOR_SCRIPT) {
        quit("err: for_script obsolete (v2.1 and earlier only)");
      }
      else
        quit("Unknown user_disabled_for in end restrict_until");

      user_disabled_for=0;
    }
  }
  set_eip(78);
  return 0;
}

void do_main_cycle(int untilwhat,int daaa) {
  // blocking cutscene - end skipping
  EndSkippingUntilCharStops();

  // this function can get called in a nested context, so
  // remember the state of these vars in case a higher level
  // call needs them
  int cached_restrict_until = restrict_until;
  int cached_user_disabled_data = user_disabled_data;
  int cached_user_disabled_for = user_disabled_for;

  main_loop_until(untilwhat,daaa,0);
  user_disabled_for=FOR_EXITLOOP;
  while (main_game_loop()==0) ;

  restrict_until = cached_restrict_until;
  user_disabled_data = cached_user_disabled_data;
  user_disabled_for = cached_user_disabled_for;
}



// ============================================================================
// SCRIPTS
// ============================================================================


void setup_exports(char*expfrom) {
  char namof[30]="\0"; temphdr[0]=0;
  while (expfrom[0]!=0) {
    expfrom=strstr(expfrom,"function ");
    if (expfrom==NULL) break;
    if (expfrom[-1]!=10) { expfrom++; continue; }
    expfrom+=9;
    int iid=0;
    while (expfrom[iid]!='(') { namof[iid]=expfrom[iid]; iid++; }
    namof[iid]=0;
    strcat(temphdr,"export ");
    strcat(temphdr,namof);
    strcat(temphdr,";\r\n");
    }
  int aa;
  for (aa=0;aa<game.numcharacters-1;aa++) {
    if (game.chars[aa].scrname[0]==0) continue;
    strcat(temphdr,"#define ");
    strcat(temphdr,game.chars[aa].scrname);
    strcat(temphdr," ");
    char*ptro=&temphdr[strlen(temphdr)];
    sprintf(ptro,"%d\r\n",aa);
    }
  }


void compile_room_script() {
  ccError = 0;

  roominst = ccCreateInstance(thisroom.compiled_script);

  if ((ccError!=0) || (roominst==NULL)) {
   char thiserror[400];
   sprintf(thiserror, "Unable to create local script: %s", ccErrorString);
   quit(thiserror);
  }

  roominstFork = ccForkInstance(roominst);
  if (roominstFork == NULL)
    quitprintf("Unable to create forked room instance: %s", ccErrorString);

  repExecAlways.roomHasFunction = true;
  getDialogOptionsDimensionsFunc.roomHasFunction = true;
}



// ============================================================================
// GRAPHICS - SPRITE GFX STUFF
// ============================================================================

void get_new_size_for_sprite (int ee, int ww, int hh, int &newwid, int &newhit) {
  newwid = ww * current_screen_resolution_multiplier;
  newhit = hh * current_screen_resolution_multiplier;
  if (game.spriteflags[ee] & SPF_640x400) 
  {
    if (current_screen_resolution_multiplier == 2) {
      newwid = ww;
      newhit = hh;
    }
    else {
      newwid=(ww/2) * current_screen_resolution_multiplier;
      newhit=(hh/2) * current_screen_resolution_multiplier;
      // just make sure - could crash if wid or hit is 0
      if (newwid < 1)
        newwid = 1;
      if (newhit < 1)
        newhit = 1;
    }
  }
}

// set any alpha-transparent pixels in the image to the appropriate
// RGB mask value so that the draw_sprite calls work correctly
void set_rgb_mask_using_alpha_channel(block image)
{
  int x, y;

  for (y=0; y < BMP_H(image); y++) 
  {
    unsigned long*psrc = (unsigned long *)BMP_LINE(image)[y];

    for (x=0; x < BMP_W(image); x++) 
    {
      if ((psrc[x] & 0xff000000) == 0x00000000)
        psrc[x] = ALW_MASK_COLOR_32;
		}
  }
}

// from is a 32-bit RGBA image, to is a 15/16/24-bit destination image
block remove_alpha_channel(block from) {
  int depth = final_col_dep;

  block to = alw_create_bitmap_ex(depth, BMP_W(from), BMP_H(from));
  int maskcol = alw_bitmap_mask_color(to);
  int y,x;
  unsigned long c,b,g,r;

  if (depth == 24) {
    // 32-to-24
    for (y=0; y < BMP_H(from); y++) {
      unsigned long*psrc = (unsigned long *)BMP_LINE(from)[y];
      unsigned char*pdest = (unsigned char*)BMP_LINE(to)[y];

      for (x=0; x < BMP_W(from); x++) {
			  c = psrc[x];
        // less than 50% opaque, remove the pixel
        if (((c >> 24) & 0x00ff) < 128)
          c = maskcol;

        // copy the RGB values across
        memcpy(&pdest[x * 3], &c, 3);
		  }
    }
  }
  else {  // 32 to 15 or 16

    for (y=0; y < BMP_H(from); y++) {
      unsigned long*psrc = (unsigned long *)BMP_LINE(from)[y];
      unsigned short*pdest = (unsigned short *)BMP_LINE(to)[y];

      for (x=0; x < BMP_W(from); x++) {
			  c = psrc[x];
        // less than 50% opaque, remove the pixel
        if (((c >> 24) & 0x00ff) < 128)
          pdest[x] = maskcol;
        else {
          // otherwise, copy it across
			    r = (c >> 16) & 0x00ff;
          g = (c >> 8) & 0x00ff;
          b = c & 0x00ff;
			    pdest[x] = alw_makecol_depth(depth, r, g, b);
        }
		  }
    }
  }

  return to;
}

void pre_save_sprite(int ee) {
  // not used, we don't save
}


void initialize_sprite (int ee) {

  if ((ee < 0) || (ee > spriteset.elements))
    quit("initialize_sprite: invalid sprite number");

  if ((spriteset[ee] == NULL) && (ee > 0)) {
    // replace empty sprites with blue cups, to avoid crashes
    //spriteset[ee] = spriteset[0];
    spriteset.set (ee, spriteset[0]);
    spritewidth[ee] = spritewidth[0];
    spriteheight[ee] = spriteheight[0];
  }
  else if (spriteset[ee]==NULL) {
    spritewidth[ee]=0;
    spriteheight[ee]=0;
  }
  else {
    // stretch sprites to correct resolution
    int oldeip = our_eip;
    set_eip(4300);

    if (game.spriteflags[ee] & SPF_HADALPHACHANNEL) {
      // we stripped the alpha channel out last time, put
      // it back so that we can remove it properly again
      game.spriteflags[ee] |= SPF_ALPHACHANNEL;
    }

    curspr = spriteset[ee];
    get_new_size_for_sprite (ee, BMP_W(curspr), BMP_H(curspr), newwid, newhit);

    eip_guinum = ee;
    eip_guiobj = newwid;

    if ((newwid != BMP_W(curspr)) || (newhit != BMP_H(curspr))) {
      tmpdbl = alw_create_bitmap_ex(alw_bitmap_color_depth(curspr),newwid,newhit);
      if (tmpdbl == NULL)
        quit("Not enough memory to load sprite graphics");
      alw_acquire_bitmap (tmpdbl);
      alw_acquire_bitmap (curspr);
      alw_clear_to_color(tmpdbl,alw_bitmap_mask_color(tmpdbl));
/*#ifdef USE_CUSTOM_EXCEPTION_HANDLER
      __try {
#endif*/
        alw_stretch_sprite(tmpdbl,curspr,0,0,BMP_W(tmpdbl),BMP_H(tmpdbl));
/*#ifdef USE_CUSTOM_EXCEPTION_HANDLER
      } __except (1) {
        // I can't trace this fault, but occasionally stretch_sprite
        // crashes, even with valid source and dest bitmaps. So,
        // for now, just ignore the exception, since the stretch
        // looks successful
      //MessageBox (allegro_wnd, "ERROR", "FATAL ERROR", MB_OK);
      }
#endif*/
      alw_release_bitmap (curspr);
      alw_release_bitmap (tmpdbl);
      wfreeblock(curspr);
      spriteset.set (ee, tmpdbl);
    }

    spritewidth[ee]=wgetblockwidth(spriteset[ee]);
    spriteheight[ee]=wgetblockheight(spriteset[ee]);

    int spcoldep = alw_bitmap_color_depth(spriteset[ee]);

    if (((spcoldep > 16) && (final_col_dep <= 16)) ||
        ((spcoldep == 16) && (final_col_dep > 16))) {
      // 16-bit sprite in 32-bit game or vice versa - convert
      // so that scaling and draw_sprite calls work properly
      block oldSprite = spriteset[ee];
      block newSprite;
      
      if (game.spriteflags[ee] & SPF_ALPHACHANNEL)
        newSprite = remove_alpha_channel(oldSprite);
      else {
        newSprite = alw_create_bitmap_ex(final_col_dep, spritewidth[ee], spriteheight[ee]);
        alw_blit(oldSprite, newSprite, 0, 0, 0, 0, spritewidth[ee], spriteheight[ee]);
      }
      spriteset.set(ee, newSprite);
      alw_destroy_bitmap(oldSprite);
      spcoldep = final_col_dep;
    }
    else if ((spcoldep == 32) && (final_col_dep == 32) &&
      ((game.spriteflags[ee] & SPF_ALPHACHANNEL) != 0))
    {
      set_rgb_mask_using_alpha_channel(spriteset[ee]);
    }
#ifdef USE_15BIT_FIX
    else if ((final_col_dep != game.color_depth*8) && (spcoldep == game.color_depth*8)) {
      // running in 15-bit mode with a 16-bit game, convert sprites
      block oldsprite = spriteset[ee];

      if (game.spriteflags[ee] & SPF_ALPHACHANNEL)
        // 32-to-24 with alpha channel
        spriteset.set (ee, remove_alpha_channel(oldsprite));
      else
        spriteset.set (ee, convert_16_to_15(oldsprite));

      alw_destroy_bitmap(oldsprite);
    }
    if ((convert_16bit_bgr == 1) && (alw_bitmap_color_depth(spriteset[ee]) == 16))
      spriteset.set (ee, convert_16_to_16bgr (spriteset[ee]));
#endif

    if ((spcoldep == 8) && (final_col_dep > 8))
      alw_select_palette(palette);

    spriteset.set(ee, gfxDriver->ConvertBitmapToSupportedColourDepth(spriteset[ee]));

    if ((spcoldep == 8) && (final_col_dep > 8))
      alw_unselect_palette();

    if (final_col_dep < 32) {
      game.spriteflags[ee] &= ~SPF_ALPHACHANNEL;
      // save the fact that it had one for the next time this
      // is re-loaded from disk
      game.spriteflags[ee] |= SPF_HADALPHACHANNEL;
    }

    platform->RunPluginHooks(AGSE_SPRITELOAD, ee);
    update_polled_stuff();

    our_eip = oldeip;
  }
}



// ============================================================================
// GRAPHICS
// ============================================================================

int init_gfx_mode(int wid,int hit,int cdep) {

  // a mode has already been initialized, so abort
  if (working_gfx_mode_status == 0) return 0;

  if (debug_15bit_mode)
    cdep = 15;
  else if (debug_24bit_mode)
    cdep = 24;

  platform->WriteDebugString("Attempt to switch gfx mode to %d x %d (%d-bit)", wid, hit, cdep);

#ifdef ENABLE_THIS_LATER
  if (usetup.refresh >= 50)
    alw_request_refresh_rate(usetup.refresh);
#endif

  final_scrn_wid = wid;
  final_scrn_hit = hit;
  final_col_dep = cdep;

  if (game.color_depth == 1) {
    final_col_dep = 8;
  }
  else {
    alw_set_color_depth(cdep);
  }

  working_gfx_mode_status = (gfxDriver->Init(wid, hit, final_col_dep, usetup.windowed > 0, &timerloop) ? 0 : -1);

  if (working_gfx_mode_status == 0) 
    platform->WriteDebugString("Succeeded. Using gfx mode %d x %d (%d-bit)", wid, hit, final_col_dep);
  else
    platform->WriteDebugString("Failed, resolution not supported");

  if ((working_gfx_mode_status < 0) && (usetup.windowed > 0) && (editor_debugging_enabled == 0)) {
    usetup.windowed ++;
    if (usetup.windowed > 2) usetup.windowed = 0;
    return init_gfx_mode(wid,hit,cdep);
  }
  return working_gfx_mode_status;    
}



// ============================================================================
// MAIN - CLOSE HOOK
// ============================================================================

void winclosehook() {
  want_exit = 1;
  abort_engine = 1;
  check_dynamic_sprites_at_exit = 0;
/*  while (want_exit == 1)
    yield_timeslice();
  / *if (want_quit == 0)
    want_quit = 1;
  else* / quit("|game aborted");
*/
}



// ============================================================================
// CONFIG
// ============================================================================

void init_game_settings() {
  int ee;

  for (ee=0;ee<256;ee++) {
    if (game.paluses[ee]!=PAL_BACKGROUND)
      palette[ee]=game.defpal[ee];
  }

  if (game.options[OPT_NOSCALEFNT]) wtext_multiply=1;

  for (ee = 0; ee < game.numcursors; ee++) 
  {
    // The cursor graphics are assigned to mousecurs[] and so cannot
    // be removed from memory
    if (game.mcurs[ee].pic >= 0)
      spriteset.precache (game.mcurs[ee].pic);

    // just in case they typed an invalid view number in the editor
    if (game.mcurs[ee].view >= game.numviews)
      game.mcurs[ee].view = -1;

    if (game.mcurs[ee].view >= 0)
      precache_view (game.mcurs[ee].view);
  }
  // may as well preload the character gfx
  if (playerchar->view >= 0)
    precache_view (playerchar->view);

  for (ee = 0; ee < MAX_INIT_SPR; ee++)
    objcache[ee].image = NULL;

/*  dummygui.guiId = -1;
  dummyguicontrol.guin = -1;
  dummyguicontrol.objn = -1;*/

  set_eip(-6);
//  game.chars[0].talkview=4;
  //init_language_text(game.langcodes[0]);

  for (ee = 0; ee < MAX_INIT_SPR; ee++) {
    scrObj[ee].id = ee;
    scrObj[ee].obj = NULL;
  }

  for (ee=0;ee<game.numcharacters;ee++) {
    memset(&game.chars[ee].inv[0],0,MAX_INV*sizeof(short));
    game.chars[ee].activeinv=-1;
    game.chars[ee].following=-1;
    game.chars[ee].followinfo=97 | (10 << 8);
    game.chars[ee].idletime=20;  // can be overridden later with SetIdle or summink
    game.chars[ee].idleleft=game.chars[ee].idletime;
    game.chars[ee].transparency = 0;
    game.chars[ee].baseline = -1;
    game.chars[ee].walkwaitcounter = 0;
    game.chars[ee].z = 0;
    charextra[ee].xwas = INVALID_X;
    charextra[ee].zoom = 100;
    if (game.chars[ee].view >= 0) {
      // set initial loop to 0
      game.chars[ee].loop = 0;
      // or to 1 if they don't have up/down frames
      if (views[game.chars[ee].view].loops[0].numFrames < 1)
        game.chars[ee].loop = 1;
    }
    charextra[ee].process_idle_this_time = 0;
    charextra[ee].invorder_count = 0;
    charextra[ee].slow_move_counter = 0;
    charextra[ee].animwait = 0;
  }
  // multiply up gui positions
  guibg = (block*)malloc(sizeof(block) * game.numgui);
  guibgbmp = (IDriverDependantBitmap**)malloc(sizeof(IDriverDependantBitmap*) * game.numgui);
  for (ee=0;ee<game.numgui;ee++) {
    guibgbmp[ee] = NULL;
    GUIMain*cgp=&guis[ee];
    guibg[ee] = alw_create_bitmap_ex (final_col_dep, cgp->wid, cgp->hit);
    guibg[ee] = gfxDriver->ConvertBitmapToSupportedColourDepth(guibg[ee]);
  }

  set_eip(-5);
  for (ee=0;ee<game.numinvitems;ee++) {
    if (game.invinfo[ee].flags & IFLG_STARTWITH) playerchar->inv[ee]=1;
    else playerchar->inv[ee]=0;
  }
  play.score=0;
  play.sierra_inv_color=7;
  play.talkanim_speed = 5;
  play.inv_item_wid = 40;
  play.inv_item_hit = 22;
  play.messagetime=-1;
  play.disabled_user_interface=0;
  play.gscript_timer=-1;
  play.debug_mode=game.options[OPT_DEBUGMODE];
  play.inv_top=0;
  play.inv_numdisp=0;
  play.obsolete_inv_numorder=0;
  play.text_speed=15;
  play.text_min_display_time_ms = 1000;
  play.ignore_user_input_after_text_timeout_ms = 500;
  play.ignore_user_input_until_time = 0;
  play.lipsync_speed = 15;
  play.close_mouth_speech_time = 10;
  play.disable_antialiasing = 0;
  play.rtint_level = 0;
  play.rtint_light = 255;
  play.text_speed_modifier = 0;
  play.text_align = SCALIGN_LEFT;
  // Make the default alignment to the right with right-to-left text
  if (game.options[OPT_RIGHTLEFTWRITE])
    play.text_align = SCALIGN_RIGHT;

  play.speech_bubble_width = get_fixed_pixel_size(100);
  play.bg_frame=0;
  play.bg_frame_locked=0;
  play.bg_anim_delay=0;
  play.anim_background_speed = 0;
  play.silent_midi = 0;
  play.current_music_repeating = 0;
  play.skip_until_char_stops = -1;
  play.get_loc_name_last_time = -1;
  play.get_loc_name_save_cursor = -1;
  play.restore_cursor_mode_to = -1;
  play.restore_cursor_image_to = -1;
  play.ground_level_areas_disabled = 0;
  play.next_screen_transition = -1;
  play.temporarily_turned_off_character = -1;
  play.inv_backwards_compatibility = 0;
  play.gamma_adjustment = 100;
  play.num_do_once_tokens = 0;
  play.do_once_tokens = NULL;
  play.music_queue_size = 0;
  play.shakesc_length = 0;
  play.wait_counter=0;
  play.key_skip_wait = 0;
  play.cur_music_number=-1;
  play.music_repeat=1;
  play.music_master_volume=160;
  play.digital_master_volume = 100;
  play.screen_flipped=0;
  play.offsets_locked=0;
  play.cant_skip_speech = user_to_internal_skip_speech(game.options[OPT_NOSKIPTEXT]);
  play.sound_volume = 255;
  play.speech_volume = 255;
  play.normal_font = 0;
  play.speech_font = 1;
  play.speech_text_shadow = 16;
  play.screen_tint = -1;
  play.bad_parsed_word[0] = 0;
  play.swap_portrait_side = 0;
  play.swap_portrait_lastchar = -1;
  play.in_conversation = 0;
  play.skip_display = 3;
  play.no_multiloop_repeat = 0;
  play.in_cutscene = 0;
  play.fast_forward = 0;
  play.totalscore = game.totalscore;
  play.roomscript_finished = 0;
  play.no_textbg_when_voice = 0;
  play.max_dialogoption_width = get_fixed_pixel_size(180);
  play.no_hicolor_fadein = 0;
  play.bgspeech_game_speed = 0;
  play.bgspeech_stay_on_display = 0;
  play.unfactor_speech_from_textlength = 0;
  play.mp3_loop_before_end = 70;
  play.speech_music_drop = 60;
  play.room_changes = 0;
  play.check_interaction_only = 0;
  play.replay_hotkey = 318;  // Alt+R
  play.dialog_options_x = 0;
  play.dialog_options_y = 0;
  play.min_dialogoption_width = 0;
  play.disable_dialog_parser = 0;
  play.ambient_sounds_persist = 0;
  play.screen_is_faded_out = 0;
  play.player_on_region = 0;
  play.top_bar_backcolor = 8;
  play.top_bar_textcolor = 16;
  play.top_bar_bordercolor = 8;
  play.top_bar_borderwidth = 1;
  play.top_bar_ypos = 25;
  play.top_bar_font = -1;
  play.screenshot_width = 160;
  play.screenshot_height = 100;
  play.speech_text_align = SCALIGN_CENTRE;
  play.auto_use_walkto_points = 1;
  play.inventory_greys_out = 0;
  play.skip_speech_specific_key = 0;
  play.abort_key = 324;  // Alt+X
  play.fade_to_red = 0;
  play.fade_to_green = 0;
  play.fade_to_blue = 0;
  play.show_single_dialog_option = 0;
  play.keep_screen_during_instant_transition = 0;
  play.read_dialog_option_colour = -1;
  play.narrator_speech = game.playercharacter;
  play.crossfading_out_channel = 0;
  play.speech_textwindow_gui = game.options[OPT_TWCUSTOM];
  if (play.speech_textwindow_gui == 0)
    play.speech_textwindow_gui = -1;
  strcpy(play.game_name, game.gamename);
  play.lastParserEntry[0] = 0;
  play.follow_change_room_timer = 150;
  for (ee = 0; ee < MAX_BSCENE; ee++) 
    play.raw_modified[ee] = 0;
  play.game_speed_modifier = 0;
  if (debug_flags & DBG_DEBUGMODE)
    play.debug_mode = 1;
  gui_disabled_style = convert_gui_disabled_style(game.options[OPT_DISABLEOFF]);

  memset(&play.walkable_areas_on[0],1,MAX_WALK_AREAS+1);
  memset(&play.script_timers[0],0,MAX_TIMERS * sizeof(int));
  memset(&play.default_audio_type_volumes[0], -1, MAX_AUDIO_TYPES * sizeof(int));

  // reset graphical script vars (they're still used by some games)
  for (ee = 0; ee < MAXGLOBALVARS; ee++) 
    play.globalvars[ee] = 0;

  for (ee = 0; ee < MAXGLOBALSTRINGS; ee++)
    play.globalstrings[ee][0] = 0;

  for (ee = 0; ee < MAX_SOUND_CHANNELS; ee++)
    last_sound_played[ee] = -1;

  if (usetup.translation)
    init_translation (usetup.translation);

  update_invorder();
  displayed_room = -10;
}



// Replace the filename part of complete path WASGV with INIFIL
void INIgetdirec(char *wasgv, char *inifil) {
  int u = strlen(wasgv) - 1;

  for (u = strlen(wasgv) - 1; u >= 0; u--) {
    if ((wasgv[u] == '\\') || (wasgv[u] == '/')) {
      memcpy(&wasgv[u + 1], inifil, strlen(inifil) + 1);
      break;
    }
  }

  if (u <= 0) {
    // no slashes - either the path is just "f:acwin.exe"
    if (strchr(wasgv, ':') != NULL)
      memcpy(strchr(wasgv, ':') + 1, inifil, strlen(inifil) + 1);
    // or it's just "acwin.exe" (unlikely)
    else
      strcpy(wasgv, inifil);
  }

}

char *INIreaditem(const char *sectn, const char *entry) {
  FILE *fin = fopen(filetouse, "rt");
  if (fin == NULL)
    return NULL;

  char templine[200];
  char wantsect[100];
  sprintf (wantsect, "[%s]", sectn);

  while (!feof(fin)) {
    fgets (templine, 199, fin);
    // find the section
    if (ac_strnicmp (wantsect, templine, strlen(wantsect)) == 0) {
      while (!feof(fin)) {
        // we're in the right section, find the entry
        fgets (templine, 199, fin);
        if (templine[0] == '[')
          break;
        if (feof(fin))
          break;
        // Strip CRLF
        templine[strlen(templine)-1] = 0;
        // Have we found the entry?
        if (ac_strnicmp (templine, entry, strlen(entry)) == 0) {
          char *pptr = &templine[strlen(entry)];
          while ((pptr[0] == ' ') || (pptr[0] == '\t'))
            pptr++;
          if (pptr[0] == '=') {
            pptr++;
            while ((pptr[0] == ' ') || (pptr[0] == '\t'))
              pptr++;
            char *toret = (char*)malloc (strlen(pptr) + 5);
            strcpy (toret, pptr);
            fclose (fin);
            return toret;
          }
        }
      }
    }
  }
  fclose (fin);
  return NULL;
}

int INIreadint (const char *sectn, const char *item, int errornosect = 1) {
  char *tempstr = INIreaditem (sectn, item);
  if (tempstr == NULL)
    return -1;

  int toret = atoi(tempstr);
  free (tempstr);
  return toret;
}


void read_config_file(char *argv0) {

  // Try current directory for config first; else try exe dir
  strcpy (ac_conf_file_defname, "acsetup.cfg");
  ac_config_file = &ac_conf_file_defname[0];
  FILE *ppp = fopen(ac_config_file, "rb");
  if (ppp == NULL) {

    strcpy(conffilebuf,argv0);
    
/*    for (int ee=0;ee<(int)strlen(conffilebuf);ee++) {
      if (conffilebuf[ee]=='/') conffilebuf[ee]='\\';
    }*/
    alw_fix_filename_case(conffilebuf);
    alw_fix_filename_slashes(conffilebuf);
    
    INIgetdirec(conffilebuf,ac_config_file);
//    printf("Using config: '%s'\n",conffilebuf);
    ac_config_file=&conffilebuf[0];
  }
  else {
    fclose(ppp);
    // put the full path, or it gets written back to the Windows folder
    ac_getcwd(ac_config_file, 255);
    strcat (ac_config_file, "\\acsetup.cfg");
    alw_fix_filename_case(ac_config_file);
    alw_fix_filename_slashes(ac_config_file);
  }

  // set default dir if no config file
  usetup.data_files_dir = ".";
  usetup.translation = NULL;
  usetup.main_data_filename = "ac2game.dat";
#ifdef WINDOWS_VERSION
  usetup.digicard = DIGI_DIRECTAMX(0);
#endif

  ppp=fopen(ac_config_file,"rt");
  if (ppp!=NULL) {
    strcpy(filetouse,ac_config_file);
    fclose(ppp);
#ifndef WINDOWS_VERSION
    usetup.digicard=INIreadint("sound","digiid");
    usetup.midicard=INIreadint("sound","midiid");
#else
    int idx = INIreadint("sound","digiwinindx", 0);
    if (idx == 0)
      idx = DIGI_DIRECTAMX(0);
    else if (idx == 1)
      idx = DIGI_WAVOUTID(0);
    else if (idx == 2)
      idx = ALW_DIGI_NONE;
    else if (idx == 3) 
      idx = DIGI_DIRECTX(0);
    else 
      idx = ALW_DIGI_AUTODETECT;
    usetup.digicard = idx;

    idx = INIreadint("sound","midiwinindx", 0);
    if (idx == 1)
      idx = ALW_MIDI_NONE;
    else if (idx == 2)
      idx = MIDI_WIN32MAPPER;
    else
      idx = ALW_MIDI_AUTODETECT;
    usetup.midicard = idx;

    if (usetup.digicard < 0)
      usetup.digicard = ALW_DIGI_AUTODETECT;
    if (usetup.midicard < 0)
      usetup.midicard = ALW_MIDI_AUTODETECT;
#endif

#if defined(WINDOWS_VERSION) || defined(LINUX_VERSION) || defined(MAC_VERSION)
    usetup.windowed = INIreadint("misc","windowed");
    if (usetup.windowed < 0)
      usetup.windowed = 0;
#endif

    usetup.refresh = INIreadint ("misc", "refresh", 0);
    usetup.enable_antialiasing = INIreadint ("misc", "antialias", 0);
    usetup.force_hicolor_mode = INIreadint("misc", "notruecolor", 0);
    usetup.enable_side_borders = INIreadint("misc", "sideborders", 0);
    force_letterbox = INIreadint ("misc", "forceletterbox", 0);

    if (usetup.enable_antialiasing < 0)
      usetup.enable_antialiasing = 0;
    if (usetup.force_hicolor_mode < 0)
      usetup.force_hicolor_mode = 0;
    if (usetup.enable_side_borders < 0)
      usetup.enable_side_borders = 1;

    // This option is backwards (usevox is 0 if no_speech_pack)
    usetup.no_speech_pack = INIreadint ("sound", "usespeech", 0);
    if (usetup.no_speech_pack == 0)
      usetup.no_speech_pack = 1;
    else
      usetup.no_speech_pack = 0;

    usetup.data_files_dir = INIreaditem("misc","datadir");
    if (usetup.data_files_dir == NULL)
      usetup.data_files_dir = ".";
    // strip any trailing slash
#if defined(LINUX_VERSION) || defined(MAC_VERSION)
    if (usetup.data_files_dir[strlen(usetup.data_files_dir)-1] == '/')
      usetup.data_files_dir[strlen(usetup.data_files_dir)-1] = 0;
#else
    if ((strlen(usetup.data_files_dir) < 4) && (usetup.data_files_dir[1] == ':'))
    { }  // if the path is just  d:\  don't strip the slash
    else if (usetup.data_files_dir[strlen(usetup.data_files_dir)-1] == '\\')
      usetup.data_files_dir[strlen(usetup.data_files_dir)-1] = 0;
#endif

    usetup.main_data_filename = INIreaditem ("misc", "datafile");
    if (usetup.main_data_filename == NULL)
      usetup.main_data_filename = "ac2game.dat";

    usetup.gfxFilterID = INIreaditem("misc", "gfxfilter");

    usetup.gfxDriverID = INIreaditem("misc", "gfxdriver");

    usetup.translation = INIreaditem ("language", "translation");
    int tempint = INIreadint ("misc", "cachemax");
    if (tempint > 0)
      spriteset.maxCacheSize = tempint * 1024;

    char *repfile = INIreaditem ("misc", "replay");
    if (repfile != NULL) {
      strcpy (replayfile, repfile);
      free (repfile);
      play.playback = 1;
    }
    else
      play.playback = 0;

  }

  if (usetup.gfxDriverID == NULL)
    usetup.gfxDriverID = "DX5";

}



// ============================================================================
// INIT
// ============================================================================

void start_game() {
  set_cursor_mode(MODE_WALK);
  filter->SetMousePosition(160,100);
  newmusic(0);

  set_eip(-42);

  for (int kk = 0; kk < numScriptModules; kk++)
    run_text_script(moduleInst[kk], "game_start");

  run_text_script(gameinst,"game_start");

  set_eip(-43);

  SetRestartPoint();

  set_eip(-3);

  if (displayed_room < 0) {
    current_fade_out_effect();
    load_new_room(playerchar->room,playerchar);
    // load_new_room updates it, but it should be -1 in the first room
    playerchar->prevroom = -1;
  }

  first_room_initialization();
}

void initialize_start_and_play_game(int override_start_room, const char *loadSaveGameOnStartup)
{
  try { // BEGIN try for ALI3DEXception
  
  set_cursor_mode (MODE_WALK);

  if (convert_16bit_bgr) {
    // Disable text as speech while displaying the warning message
    // This happens if the user's graphics card does BGR order 16-bit colour
    int oldalways = game.options[OPT_ALWAYSSPCH];
    game.options[OPT_ALWAYSSPCH] = 0;
    Display ("WARNING: AGS has detected that you have an incompatible graphics card for this game. You may experience colour problems during the game. Try running the game with \"--15bit\" command line parameter and see if that helps.[[Click the mouse to continue.");
    game.options[OPT_ALWAYSSPCH] = oldalways;
  }

  srand (play.randseed);
  play.gamestep = 0;
  if (override_start_room)
    playerchar->room = override_start_room;

  write_log_debug("Checking replay status");

  if (play.recording) {
    start_recording();
  }
  else if (play.playback) {
    FILE *in = fopen(replayfile, "rb");
    if (in != NULL) {
      char buffer [100];
      fread (buffer, 12, 1, in);
      buffer[12] = 0;
      if (strcmp (buffer, "AGSRecording") != 0) {
        Display("ERROR: Invalid recorded data file");
        play.playback = 0;
      }
      else {
        fgetstring_limit (buffer, in, 12);
        if (buffer[0] != '2') 
          quit("!Replay file is from an old version of AGS");
        if (strcmp (buffer, "2.55.553") < 0)
          quit("!Replay file was recorded with an older incompatible version");

        if (strcmp (buffer, ACI_VERSION_TEXT)) {
          // Disable text as speech while displaying the warning message
          // This happens if the user's graphics card does BGR order 16-bit colour
          int oldalways = game.options[OPT_ALWAYSSPCH];
          game.options[OPT_ALWAYSSPCH] = 0;
          play.playback = 0;
          Display("Warning! replay is from a different version of AGS (%s) - it may not work properly.", buffer);
          play.playback = 1;
          srand (play.randseed);
          play.gamestep = 0;
          game.options[OPT_ALWAYSSPCH] = oldalways;
        }

        int replayver = getw(in);

        if ((replayver < 1) || (replayver > 3))
          quit("!Unsupported Replay file version");

        if (replayver >= 2) {
          fgetstring_limit (buffer, in, 99);
          int uid = getw (in);
          if ((strcmp (buffer, game.gamename) != 0) || (uid != game.uniqueid)) {
            char msg[150];
            sprintf (msg, "!This replay is meant for the game '%s' and will not work correctly with this game.", buffer);
            quit (msg);
          }
          // skip the total time
          getw (in);
          // replay description, maybe we'll use this later
          fgetstring_limit (buffer, in, 99);
        }

        play.randseed = getw(in);
        int flen = ac_fstream_sizebytes(in) - ftell (in);
        if (replayver >= 3) {
          flen = getw(in) * sizeof(short);
        }
        recordbuffer = (short*)malloc (flen);
        fread (recordbuffer, flen, 1, in);
        srand (play.randseed);
        recbuffersize = flen / sizeof(short);
        recsize = 0;
        disable_mgetgraphpos = 1;
        replay_time = 0;
        replay_last_second = loopcounter;
        if (replayver >= 3) {
          int issave = getw(in);
          if (issave) {
            if (restore_game_data (in, replayfile))
              quit("!Error running replay... could be incorrect game version");
            replay_last_second = loopcounter;
          }
        }
        fclose (in);
      }
    }
    else // file not found
      play.playback = 0;
  }

  write_log_debug("Engine initialization complete");
  write_log_debug("Starting game");
  
  if (editor_debugging_enabled)
  {
    SetMultitasking(1);
    if (init_editor_debugging())
    {
      timerloop = 0;
      while (timerloop < 20)
      {
        // pick up any breakpoints in game_start
        check_for_messages_from_editor();
      }

      ccSetDebugHook(scriptDebugHook);
    }
  }

  if (loadSaveGameOnStartup != NULL)
  {
    int saveGameNumber = 1000;
    const char *sgName = strstr(loadSaveGameOnStartup, "agssave.");
    if (sgName != NULL)
    {
      sscanf(sgName, "agssave.%03d", &saveGameNumber);
    }
    current_fade_out_effect();
    int loadGameErrorCode = do_game_load(loadSaveGameOnStartup, saveGameNumber, NULL, NULL);
    if (loadGameErrorCode)
    {
      quitprintf("Unable to resume the save game. Try starting the game over. (Error: %s)", load_game_errors[-loadGameErrorCode]);
    }
  }

  // only start if replay playback hasn't loaded a game
  if (displayed_room < 0)
    start_game();

  while (!abort_engine) {
    main_game_loop();

    if (load_new_game) {
      RunAGSGame (NULL, load_new_game, 0);
      load_new_game = 0;
    }
  }

  } catch (Ali3DException gfxException)
  {
    quit((char*)gfxException._message);
  }

}



// ============================================================================
// GRAPHICS - INIT
// ============================================================================

int initialize_graphics_filter(const char *filterID, int width, int height, int colDepth)
{
  int idx = 0;
  GFXFilter **filterList;

  if (ac_stricmp(usetup.gfxDriverID, "D3D9") == 0)
  {
    filterList = get_d3d_gfx_filter_list(false);
  }
  else
  {
    filterList = get_allegro_gfx_filter_list(false);
  }
  
  // by default, select No Filter
  filter = filterList[0];

  GFXFilter *thisFilter = filterList[idx];
  while (thisFilter != NULL) {

    if ((filterID != NULL) &&
        (strcmp(thisFilter->GetFilterID(), filterID) == 0))
      filter = thisFilter;
    else if (idx > 0)
      delete thisFilter;

    idx++;
    thisFilter = filterList[idx];
  }

  const char *filterError = filter->Initialize(width, height, colDepth);
  if (filterError != NULL) {
    proper_exit = 1;
    platform->DisplayAlert("Unable to initialize the graphics filter. It returned the following error:\n'%s'\n\nTry running Setup and selecting a different graphics filter.", filterError);
    return -1;
  }

  return 0;
}

int try_widescreen_bordered_graphics_mode_if_appropriate(int initasx, int initasy, int firstDepth)
{
  if (working_gfx_mode_status == 0) return 0;
  if (usetup.enable_side_borders == 0)
  {
    platform->WriteDebugString("Widescreen side borders: disabled in Setup");
    return 1;
  }
  if (usetup.windowed > 0)
  {
    platform->WriteDebugString("Widescreen side borders: disabled (windowed mode)");
    return 1;
  }

  int failed = 1;
  int desktopWidth, desktopHeight;
  if (alw_get_desktop_resolution(&desktopWidth, &desktopHeight) == 0)
  {
    int gameHeight = initasy;

    int screenRatio = (desktopWidth * 1000) / desktopHeight;
    int gameRatio = (initasx * 1000) / gameHeight;
    // 1250 = 1280x1024 
    // 1333 = 640x480, 800x600, 1024x768, 1152x864, 1280x960
    // 1600 = 640x400, 960x600, 1280x800, 1680x1050
    // 1666 = 1280x768

    platform->WriteDebugString("Widescreen side borders: game resolution: %d x %d; desktop resolution: %d x %d", initasx, gameHeight, desktopWidth, desktopHeight);

    if ((screenRatio > 1500) && (gameRatio < 1500))
    {
      int tryWidth = (initasx * screenRatio) / gameRatio;
      int supportedRes = gfxDriver->FindSupportedResolutionWidth(tryWidth, gameHeight, firstDepth, 110);
      if (supportedRes > 0)
      {
        tryWidth = supportedRes;
        platform->WriteDebugString("Widescreen side borders: enabled, attempting resolution %d x %d", tryWidth, gameHeight);
      }
      else
      {
        platform->WriteDebugString("Widescreen side borders: gfx card does not support suitable resolution. will attempt %d x %d anyway", tryWidth, gameHeight);
      }
      failed = init_gfx_mode(tryWidth, gameHeight, firstDepth);
    }
    else
    {
      platform->WriteDebugString("Widescreen side borders: disabled (not necessary, game and desktop aspect ratios match)", initasx, gameHeight, desktopWidth, desktopHeight);
    }
  }
  else 
  {
    platform->WriteDebugString("Widescreen side borders: disabled (unable to obtain desktop resolution)");
  }
  return failed;
}

int switch_to_graphics_mode(int initasx, int initasy, int scrnwid, int scrnhit, int firstDepth, int secondDepth) 
{
  int failed;
  int initasyLetterbox = (initasy * 12) / 10;

  // first of all, try 16-bit normal then letterboxed
  if (game.options[OPT_LETTERBOX] == 0) 
  {
    failed = try_widescreen_bordered_graphics_mode_if_appropriate(initasx, initasy, firstDepth);
    failed = init_gfx_mode(initasx,initasy, firstDepth);
  }
  failed = try_widescreen_bordered_graphics_mode_if_appropriate(initasx, initasyLetterbox, firstDepth);
  failed = init_gfx_mode(initasx, initasyLetterbox, firstDepth);

  if (secondDepth != firstDepth) {
    // now, try 15-bit normal then letterboxed
    if (game.options[OPT_LETTERBOX] == 0) 
    {
      failed = try_widescreen_bordered_graphics_mode_if_appropriate(initasx, initasy, secondDepth);
      failed = init_gfx_mode(initasx,initasy, secondDepth);
    }
    failed = try_widescreen_bordered_graphics_mode_if_appropriate(initasx, initasyLetterbox, secondDepth);
    failed = init_gfx_mode(initasx, initasyLetterbox, secondDepth);
  }

  if ((scrnwid != initasx) || (scrnhit != initasy))
  {
    // now, try the original resolution at 16 then 15 bit
    failed = init_gfx_mode(scrnwid,scrnhit,firstDepth);
    failed = init_gfx_mode(scrnwid,scrnhit, secondDepth);
  }
  
  if (failed)
    return -1;

  return 0;
}

void CreateBlankImage()
{
  // this is the first time that we try to use the graphics driver,
  // so it's the most likey place for a crash
  try
  {
    ALW_BITMAP *blank = alw_create_bitmap_ex(final_col_dep, 16, 16);
    blank = gfxDriver->ConvertBitmapToSupportedColourDepth(blank);
    alw_clear_bitmap(blank);
    blankImage = gfxDriver->CreateDDBFromBitmap(blank, false, true);
    blankSidebarImage = gfxDriver->CreateDDBFromBitmap(blank, false, true);
    alw_destroy_bitmap(blank);
  }
  catch (Ali3DException gfxException)
  {
    quit((char*)gfxException._message);
  }

}



// ============================================================================
// GRAPHICS - PRELOAD
// ============================================================================

void show_preload () {
  // ** Do the preload graphic if available
  color temppal[256];
  block splashsc = alw_load_pcx("preload.pcx",temppal);
  if (splashsc != NULL) {
    if (alw_bitmap_color_depth(splashsc) == 8)
      wsetpalette(0,255,temppal);
    block tsc = alw_create_bitmap_ex(alw_bitmap_color_depth(alw_screen),BMP_W(splashsc),BMP_H(splashsc));
    alw_blit(splashsc,tsc,0,0,0,0,BMP_W(tsc),BMP_H(tsc));
    alw_clear_bitmap(alw_screen);
    alw_stretch_sprite(alw_screen, tsc, 0, 0, scrnwid,scrnhit);

    gfxDriver->ClearDrawList();

    if (!gfxDriver->UsesMemoryBackBuffer())
    {
      IDriverDependantBitmap *ddb = gfxDriver->CreateDDBFromBitmap(alw_screen, false, true);
      gfxDriver->DrawSprite(0, 0, ddb);
      render_to_screen(alw_screen, 0, 0);
      gfxDriver->DestroyDDB(ddb);
    }
    else
      render_to_screen(alw_screen, 0, 0);

    wfreeblock(splashsc);
    wfreeblock(tsc);
    platform->Delay(500);
  }
}



// ============================================================================
// INIT
// ============================================================================



void initialise_game_file_name(int argc,char **argv)
{
#ifdef WINDOWS_VERSION
  WCHAR buffer[MAX_PATH];
  LPCWSTR dataFilePath = wArgv[datafile_argv];
  // Hack for Windows in case there are unicode chars in the path.
  // The normal argv[] array has ????? instead of the unicode chars
  // and fails, so instead we manually get the short file name, which
  // is always using ANSI chars.
  if (wcschr(dataFilePath, '\\') == NULL)
  {
    GetCurrentDirectoryW(MAX_PATH, buffer);
    wcscat(buffer, L"\\");
    wcscat(buffer, dataFilePath);
    dataFilePath = &buffer[0];
  }
  if (GetShortPathNameW(dataFilePath, directoryPathBuffer, MAX_PATH) == 0)
  {
    platform->DisplayAlert("Unable to determine startup path: GetShortPathNameW failed. The specified game file might be missing.");
    game_file_name = NULL;
    return;
  }
  game_file_name = (char*)malloc(MAX_PATH);
  WideCharToMultiByte(CP_ACP, 0, directoryPathBuffer, -1, game_file_name, MAX_PATH, NULL, NULL);
#else
  game_file_name = argv[datafile_argv];
  
#ifdef MAC_HACK
  game_file_name = "Game.exe";
#endif
  
#endif
}


void main_handle_command_line_arguments(int argc, char **argv) {
  
  debug_flags=0;

  for (int ee=1;ee<argc;ee++) {
    if (argv[ee][1]=='?') {help_required=1; return;}
    if (ac_stricmp(argv[ee],"-shelllaunch") == 0)
      change_to_game_dir = 1;
    else if (ac_stricmp(argv[ee],"-updatereg") == 0)
      debug_flags |= DBG_REGONLY;
    else if (ac_stricmp(argv[ee],"-windowed") == 0)
      force_window = 1;
    else if (ac_stricmp(argv[ee],"-fullscreen") == 0)
      force_window = 2;
    else if (ac_stricmp(argv[ee],"-hicolor") == 0)
      force_16bit = 1;
    else if (ac_stricmp(argv[ee],"-letterbox") == 0)
      force_letterbox = 1;
    else if (ac_stricmp(argv[ee],"-record") == 0)
      play.recording = 1;
    else if (ac_stricmp(argv[ee],"-playback") == 0)
      play.playback = 1;
#ifdef _DEBUG
    else if ((ac_stricmp(argv[ee],"--startr") == 0) && (ee < argc-1)) {
      override_start_room = atoi(argv[ee+1]);
      ee++;
    }
#endif
    else if ((ac_stricmp(argv[ee],"--testre") == 0) && (ee < argc-2)) {
      strcpy(return_to_roomedit, argv[ee+1]);
      strcpy(return_to_room, argv[ee+2]);
      ee+=2;
    }
    else if (ac_stricmp(argv[ee],"--15bit")==0) debug_15bit_mode = 1;
    else if (ac_stricmp(argv[ee],"--24bit")==0) debug_24bit_mode = 1;
    else if (ac_stricmp(argv[ee],"--fps")==0) display_fps = 2;
    else if (ac_stricmp(argv[ee],"--test")==0) debug_flags|=DBG_DEBUGMODE;
    else if (ac_stricmp(argv[ee],"-noiface")==0) debug_flags|=DBG_NOIFACE;
    else if (ac_stricmp(argv[ee],"-nosprdisp")==0) debug_flags|=DBG_NODRAWSPRITES;
    else if (ac_stricmp(argv[ee],"-nospr")==0) debug_flags|=DBG_NOOBJECTS;
    else if (ac_stricmp(argv[ee],"-noupdate")==0) debug_flags|=DBG_NOUPDATE;
    else if (ac_stricmp(argv[ee],"-nosound")==0) debug_flags|=DBG_NOSFX;
    else if (ac_stricmp(argv[ee],"-nomusic")==0) debug_flags|=DBG_NOMUSIC;
    else if (ac_stricmp(argv[ee],"-noscript")==0) debug_flags|=DBG_NOSCRIPT;
    else if (ac_stricmp(argv[ee],"-novideo")==0) debug_flags|=DBG_NOVIDEO;
    else if (ac_stricmp(argv[ee],"-noexceptionhandler")==0) usetup.disable_exception_handling = 1;
    else if (ac_stricmp(argv[ee],"-dbgscript")==0) debug_flags|=DBG_DBGSCRIPT;
    else if (ac_stricmp(argv[ee],"-registergame") == 0)
    {
      justRegisterGame = true;
    }
    else if (ac_stricmp(argv[ee],"-unregistergame") == 0)
    {
      justUnRegisterGame = true;
    }
    else if ((ac_stricmp(argv[ee],"-loadsavedgame") == 0) && (argc > ee + 1))
    {
      loadSaveGameOnStartup = argv[ee + 1];
      ee++;
    }
    else if ((ac_stricmp(argv[ee],"--enabledebugger") == 0) && (argc > ee + 1))
    {
      strcpy(editor_debugger_instance_token, argv[ee + 1]);
      editor_debugging_enabled = 1;
      force_window = 1;
      ee++;
    }
    else if (ac_stricmp(argv[ee],"--takeover")==0) {
      if (argc < ee+2)
        break;
      play.takeover_data = atoi (argv[ee + 1]);
      strncpy (play.takeover_from, argv[ee + 2], 49);
      play.takeover_from[49] = 0;
      ee += 2;
    }
    else if (argv[ee][0]!='-') datafile_argv=ee;
  }
}

void main_init_crt_debug() {
#ifdef _DEBUG
  /* logfile=fopen("g:\\ags.log","at");
   //_CrtSetReportHook( OurReportingFunction );
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    //tmpDbgFlag |= _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_DELAY_FREE_MEM_DF;

    tmpDbgFlag = (tmpDbgFlag & 0x0000FFFF) | _CRTDBG_CHECK_EVERY_16_DF | _CRTDBG_DELAY_FREE_MEM_DF;

    _CrtSetDbgFlag(tmpDbgFlag);

/*
//  _CrtMemState memstart,memnow;
  _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_WNDW );
  _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_WNDW );
  _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_WNDW );
/*
//   _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDERR );
//   _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDERR );
//   _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDERR );

//  _CrtMemCheckpoint(&memstart);
//  _CrtMemDumpStatistics( &memstart );*/
#endif
}

int main(int argc, char*argv[]) { 
  set_eip(-999);
  clib_set_open_priority(PR_FILEFIRST);

  // input recording
  play.recording = 0;
  play.playback = 0;

  // new game, no takeover data.
  play.takeover_data = 0;

  // init cross platform driver
  platform = AGSPlatformDriver::GetDriver();

#ifdef WINDOWS_VERSION
  wArgv = CommandLineToArgvW(GetCommandLineW(), &wArgc);
  if (wArgv == NULL)
  {
    platform->DisplayAlert("CommandLineToArgvW failed, unable to start the game.");
    return 9;
  }
#endif

  print_welcome_text(AC_VERSION_TEXT,ACI_VERSION_TEXT);
  if ((argc>1) && (argv[1][1]=='?'))
    return 0;

  write_log_debug("***** ENGINE STARTUP");

  install_debug_handlers();

  main_handle_command_line_arguments(argc, argv);
  if (help_required)
    return 0;

  main_init_crt_debug();

  if ((loadSaveGameOnStartup != NULL) && (argv[0] != NULL))
  {
    // When launched by double-clicking a save game file, the curdir will
    // be the save game folder unless we correct it
#ifdef WINDOWS_VERSION
    change_to_directory_of_file(wArgv[0]);
#else
    change_to_directory_of_file(argv[0]);
#endif
  }
 
#ifdef MAC_VERSION
  ac_getcwd(appDirectory, 512);
#endif
 
  //if (change_to_game_dir == 1)  {
  if (datafile_argv > 0) {
    // If launched by double-clicking .AGS file, change to that
    // folder; else change to this exe's folder
#ifdef WINDOWS_VERSION
    change_to_directory_of_file(wArgv[datafile_argv]);
#else
    change_to_directory_of_file(argv[datafile_argv]);
#endif
  }

#ifdef MAC_VERSION
  ac_getcwd(dataDirectory, 512);
#endif
 
  // Update shell associations and exit
  if (debug_flags & DBG_REGONLY)
    exit(0);

#ifndef USE_CUSTOM_EXCEPTION_HANDLER
  usetup.disable_exception_handling = 1;
#endif

  if (usetup.disable_exception_handling)
  {
    return initialize_engine(argc, argv);
  }
  else
  {
    return initialize_engine_with_exception_handling(argc, argv);
  }
}



// ============================================================================
// INIT ENGINE
// ============================================================================


int initeng_init_allegro() 
{
  alw_set_uformat(U_ASCII);

  write_log_debug("Initializing allegro");

  set_eip(-199);
  
  // Initialize allegro
  if (alw_allegro_init()) {
#ifdef WINDOWS_VERSION
    platform->DisplayAlert("Unable to initialize graphics subsystem. Make sure you have DirectX 5 or above installed.");
#else 
    platform->DisplayAlert("Unable to initialize graphics subsystem.");
#endif
    return EXIT_NORMAL;
  }

  return 0;
}

int initeng_init_window( int argc, char* * argv ) 
{

  write_log_debug("Setting up window");

  set_eip(-198);
#if (ALW_ALLEGRO_DATE > 19990103)
  alw_set_window_title("Adventure Game Studio");
#if (ALW_ALLEGRO_DATE > 20021115)
  alw_set_close_button_callback (winclosehook);
#else
  set_window_close_hook (winclosehook);
#endif

  set_eip(-197);
#endif

  platform->SetGameWindowIcon();

  set_eip(-196);

#if !defined(LINUX_VERSION) && !defined(MAC_VERSION)
  // check if Setup needs to be run instead
  if (argc>1) {
    if (ac_stricmp(argv[1],"--setup")==0) { 
      write_log_debug("Running Setup");

      if (!platform->RunSetup())
        return EXIT_NORMAL;

#ifndef WINDOWS_VERSION
#define _spawnl spawnl
#define _P_OVERLAY P_OVERLAY
#endif
      // Just re-reading the config file seems to cause a caching
      // problem on Win9x, so let's restart the process.
      alw_allegro_exit();
      char quotedpath[255];
      sprintf (quotedpath, "\"%s\"", argv[0]);
      _spawnl (_P_OVERLAY, argv[0], quotedpath, NULL);
      //read_config_file(argv[0]);
    }
  }
#endif

  // Force to run in a window, override the config file
  if (force_window == 1)
    usetup.windowed = 1;
  else if (force_window == 2)
    usetup.windowed = 0;

  return 0;
}

int initeng_init_game_data( int argc, char* * argv ) 
{

  set_eip(-195);

  write_log_debug("Initializing game data");

  // initialize the data file
  initialise_game_file_name(argc, argv);
  if (game_file_name == NULL) return EXIT_NORMAL;

  int errcod = csetlib(game_file_name,"");  // assume it's appended to exe

  set_eip(-194);
  //  char gamefilenamebuf[200];
  if ((errcod!=0) && (change_to_game_dir == 0)) {
    // it's not, so look for the file

    game_file_name = ci_find_file(usetup.data_files_dir, usetup.main_data_filename);

    errcod=csetlib(game_file_name,"");
    if (errcod) {
      //sprintf(gamefilenamebuf,"%s\\ac2game.ags",usetup.data_files_dir);
      free(game_file_name);
      game_file_name = ci_find_file(usetup.data_files_dir, "ac2game.ags");

      errcod = csetlib(game_file_name,"");
    }
  }
  else {
    // set the data filename to the EXE name

    usetup.main_data_filename = alw_get_filename(game_file_name);

    if (((strchr(game_file_name, '/') != NULL) ||
      (strchr(game_file_name, '\\') != NULL)) &&
      (ac_stricmp(usetup.data_files_dir, ".") == 0)) {
        // there is a path in the game file name (and the user
        // has not specified another one)
        // save the path, so that it can load the VOX files, etc
        usetup.data_files_dir = (char*)malloc(strlen(game_file_name) + 1);
        strcpy(usetup.data_files_dir, game_file_name);

        if (strrchr(usetup.data_files_dir, '/') != NULL)
          strrchr(usetup.data_files_dir, '/')[0] = 0;
        else if (strrchr(usetup.data_files_dir, '\\') != NULL)
          strrchr(usetup.data_files_dir, '\\')[0] = 0;
        else {
          platform->DisplayAlert("Error processing game file name: slash but no slash");
          return EXIT_NORMAL;
        }
    }

  }

  set_eip(-193);

  if (errcod!=0) {  // there's a problem
    if (errcod==-1) {  // file not found
      char emsg[STD_BUFFER_SIZE];
      sprintf (emsg,
        "You must create and save a game first in the AGS Editor before you can use "
        "this engine.\n\n"
        "If you have just downloaded AGS, you are probably running the wrong executable.\n"
        "Run AGSEditor.exe to launch the editor.\n\n"
        "(Unable to find '%s')\n", argv[datafile_argv]);
      platform->DisplayAlert(emsg);
    }
    else if (errcod==-4)
      platform->DisplayAlert("ERROR: Too many files in data file.");
    else platform->DisplayAlert("ERROR: The file is corrupt. Make sure you have the correct version of the\n"
      "editor, and that this really is an AGS game.\n");
    return EXIT_NORMAL;
  }

  return 0;
}

int initeng_init_mouse() 
{

  set_eip(-188);

  write_log_debug("Initializing mouse");

#ifdef _DEBUG
  // Quantify fails with the mouse for some reason
  minstalled();
#else
  if (minstalled()==0) {
    platform->DisplayAlert(platform->GetNoMouseErrorString());
    return EXIT_NORMAL;
  }
#endif // DEBUG

  return 0;
}

int initeng_mem_check() 
{
  set_eip(-187);

  write_log_debug("Checking memory");

  char*memcheck=(char*)malloc(4000000);
  if (memcheck==NULL) {
    platform->DisplayAlert("There is not enough memory available to run this game. You need 4 Mb free\n"
      "extended memory to run the game.\n"
      "If you are running from Windows, check the 'DPMI memory' setting on the DOS box\n"
      "properties.\n");
    return EXIT_NORMAL;
  }
  free(memcheck);

  return 0;
}

void initeng_init_rooms() 
{
  write_log_debug("Initializing rooms");

  roomstats=(RoomStatus*)calloc(sizeof(RoomStatus),MAX_ROOMS);
  for (int ee=0;ee<MAX_ROOMS;ee++) {
    roomstats[ee].beenhere=0;
    roomstats[ee].numobj=0;
    roomstats[ee].tsdatasize=0;
    roomstats[ee].tsdata=NULL;
  }
}

int initeng_init_speech() 
{

  play.want_speech=-2;

  set_eip(-186);
  if (usetup.no_speech_pack == 0) {
    /* Can't just use fopen here, since we need to change the filename
    so that pack functions, etc. will have the right case later */
    speech_file = ci_find_file(usetup.data_files_dir, "speech.vox");

    FILE *speech_fp = fopen(speech_file, "rb");

    if (speech_fp == NULL)
    {
      // In case they're running in debug, check Compiled folder
      free(speech_file);
      speech_file = ci_find_file("Compiled", "speech.vox");
      speech_fp = fopen(speech_file, "rb");
    }

    if (speech_fp!=NULL) {
      fclose(speech_fp);

      write_log_debug("Initializing speech vox");

      //if (csetlib(useloc,"")!=0) {
      if (csetlib(speech_file,"")!=0) {
        platform->DisplayAlert("Unable to initialize speech sample file - check for corruption and that\nit belongs to this game.\n");
        return EXIT_NORMAL;
      }
      FILE *speechsync = clibfopen("syncdata.dat", "rb");
      if (speechsync != NULL) {
        // this game has voice lip sync
        if (getw(speechsync) != 4)
          platform->DisplayAlert("Unknown speech lip sync format (might be from older or newer version); lip sync disabled");
        else {
          numLipLines = getw(speechsync);
          splipsync = (SpeechLipSyncLine*)malloc (sizeof(SpeechLipSyncLine) * numLipLines);
          for (int ee = 0; ee < numLipLines; ee++)
          {
            splipsync[ee].numPhenomes = getshort(speechsync);
            fread(splipsync[ee].filename, 1, 14, speechsync);
            splipsync[ee].endtimeoffs = (int*)malloc(splipsync[ee].numPhenomes * sizeof(int));
            fread(splipsync[ee].endtimeoffs, sizeof(int), splipsync[ee].numPhenomes, speechsync);
            splipsync[ee].frame = (short*)malloc(splipsync[ee].numPhenomes * sizeof(short));
            fread(splipsync[ee].frame, sizeof(short), splipsync[ee].numPhenomes, speechsync);
          }
        }
        fclose (speechsync);
      }
      csetlib(game_file_name,"");
      platform->WriteConsole("Speech sample file found and initialized.\n");
      play.want_speech=1;
    }
  }

  return 0;
}

int initeng_init_music() 
{

  set_eip(-185);
  play.seperate_music_lib = 0;

  /* Can't just use fopen here, since we need to change the filename
  so that pack functions, etc. will have the right case later */
  music_file = ci_find_file(usetup.data_files_dir, "audio.vox");

  /* Don't need to use ci_fopen here, because we've used ci_find_file to get
  the case insensitive matched filename already */
  FILE *music_fp = fopen(music_file, "rb");

  if (music_fp == NULL)
  {
    // In case they're running in debug, check Compiled folder
    free(music_file);
    music_file = ci_find_file("Compiled", "audio.vox");
    music_fp = fopen(music_file, "rb");
  }

  if (music_fp!=NULL) {
    fclose(music_fp);

    write_log_debug("Initializing audio vox");

    //if (csetlib(useloc,"")!=0) {
    if (csetlib(music_file,"")!=0) {
      platform->DisplayAlert("Unable to initialize music library - check for corruption and that\nit belongs to this game.\n");
      return EXIT_NORMAL;
    }
    csetlib(game_file_name,"");
    platform->WriteConsole("Audio vox found and initialized.\n");
    play.seperate_music_lib = 1;
  }

  return 0;
}



void initeng_init_sound() 
{
  platform->WriteConsole("Checking sound inits.\n");
  if (opts.mod_player) alw_reserve_voices(16,-1);
  // maybe this line will solve the sound volume?

#if ALW_ALLEGRO_DATE > 19991010
  alw_set_volume_per_voice(1);
#endif

  set_eip(-182);

#ifdef WINDOWS_VERSION
  // don't let it use the hardware mixer verion, crashes some systems
  //if ((usetup.digicard == ALW_DIGI_AUTODETECT) || (usetup.digicard == DIGI_DIRECTX(0)))
  //    usetup.digicard = DIGI_DIRECTAMX(0);

  if (usetup.digicard == DIGI_DIRECTX(0)) {
    // DirectX mixer seems to buffer an extra sample itself
    use_extra_sound_offset = 1;
  }

  // if the user clicked away to another app while we were
  // loading, DirectSound will fail to initialize. There doesn't
  // seem to be a solution to force us back to the foreground,
  // because we have no actual visible window at this time

#endif

  write_log_debug("Initialize sound drivers");

  if (alw_install_sound(usetup.digicard,usetup.midicard,NULL)!=0) {
    alw_reserve_voices(-1,-1);
    opts.mod_player=0;
    opts.mp3_player=0;
    if (alw_install_sound(usetup.digicard,usetup.midicard,NULL)!=0) {
      if ((usetup.digicard != ALW_DIGI_NONE) && (usetup.midicard != ALW_MIDI_NONE)) {
        // only flag an error if they wanted a sound card
        platform->DisplayAlert("\nUnable to initialize your audio hardware.\n"
          "[Problem: %s]\n",alw_allegro_error);
      }
      alw_reserve_voices(0,0);
      alw_install_sound(ALW_DIGI_NONE, ALW_MIDI_NONE, NULL);
      usetup.digicard = ALW_DIGI_NONE;
      usetup.midicard = ALW_MIDI_NONE;
    }
  }

  set_eip(-181);

  if (usetup.digicard == ALW_DIGI_NONE) {
    // disable speech and music if no digital sound
    // therefore the MIDI soundtrack will be used if present,
    // and the voice mode should not go to Voice Only
    play.want_speech = -2;
    play.seperate_music_lib = 0;
  }

  //alw_set_volume(255,-1);
}

void setup_sierra_interface() {
  int rr;
  game.numgui =0;
  for (rr=0;rr<42;rr++) game.paluses[rr]=PAL_GAMEWIDE;
  for (rr=42;rr<256;rr++) game.paluses[rr]=PAL_BACKGROUND;
}

int initeng_load_game_data() 
{
  write_log_debug("Load game data");

  set_eip(-17);
  int loaderror;
  if ((loaderror=load_game_file())!=0) {
    proper_exit=1;
    platform->FinishedUsingGraphicsMode();

    if (loaderror==-1)
      platform->DisplayAlert("Main game file not found. This may be from a different AGS version, or the file may have got corrupted.\n");
    else if (loaderror==-2)
      platform->DisplayAlert("Invalid file format. The file may be corrupt, or from a different\n"
      "version of AGS.\nThis engine can only run games made with AGS 3.2 or later.\n");
    else if (loaderror==-3)
      platform->DisplayAlert("Script link failed: %s\n",ccErrorString);
    return EXIT_NORMAL;
  }

  return 0;
}

void initeng_load_extra_game_data() 
{
  write_log_debug(game.gamename);

  set_eip(-189);

  if (alw_file_exists("Compiled", FA_ARCH | FA_DIREC, NULL))
  {
    // running in debugger
    use_compiled_folder_as_current_dir = 1;
    // don't redirect to the game exe folder (_Debug)
    usetup.data_files_dir = ".";
  }

  if (game.saveGameFolderName[0] != 0)
  {
    char newDirBuffer[MAX_PATH];
    sprintf(newDirBuffer, "$MYDOCS$/%s", game.saveGameFolderName);
    Game_SetSaveGameDirectory(newDirBuffer);
  }
  else if (use_compiled_folder_as_current_dir)
  {
    Game_SetSaveGameDirectory("Compiled");
  }
}

int initeng_check_disk_space() 
{
  set_eip(-178);

  write_log_debug("Checking for disk space");

  //init_language_text("en");
  if (check_write_access()==0) {
    platform->DisplayAlert("Unable to write to the current directory. Do not run this game off a\n"
      "network or CD-ROM drive. Also check drive free space (you need 1 Mb free).\n");
    proper_exit = 1;
    return EXIT_NORMAL; 
  }

  return 0;
}

void initeng_init_mod_player() 
{
  set_eip(-179);

  if (game.options[OPT_NOMODMUSIC])
    opts.mod_player = 0;

  if (opts.mod_player) {
    write_log_debug("Initializing MOD/XM player");

    if (init_mod_player(NUM_MOD_DIGI_VOICES) < 0) {
      platform->DisplayAlert("Warning: install_mod: MOD player failed to initialize.");
      opts.mod_player=0;
    }
  }
}

void initeng_init_screen_settings(int &initasx, int &initasy, int &firstDepth, int &secondDepth) 
{
  write_log_debug("Initializing screen settings");

  // default shifts for how we store the sprite data
  alw_set_rgb_r_shift_32(16);
  alw_set_rgb_g_shift_32(8);
  alw_set_rgb_b_shift_32(0);
  alw_set_rgb_r_shift_16(11);
  alw_set_rgb_g_shift_16(5);
  alw_set_rgb_b_shift_16(0);
  alw_set_rgb_r_shift_15(10);
  alw_set_rgb_g_shift_15(5);
  alw_set_rgb_b_shift_15(0);

  usetup.base_width = 320;
  usetup.base_height = 200;

  if (game.default_resolution >= 5)
  {
    if (game.default_resolution >= 6)
    {
      // 1024x768
      usetup.base_width = 512;
      usetup.base_height = 384;
    }
    else
    {
      // 800x600
      usetup.base_width = 400;
      usetup.base_height = 300;
    }
    // don't allow letterbox mode
    game.options[OPT_LETTERBOX] = 0;
    force_letterbox = 0;
    scrnwid = usetup.base_width * 2;
    scrnhit = usetup.base_height * 2;
    wtext_multiply = 2;
  }
  else if ((game.default_resolution == 4) ||
    (game.default_resolution == 3))
  {
    scrnwid = 640;
    scrnhit = 400;
    wtext_multiply = 2;
  }
  else if ((game.default_resolution == 2) ||
    (game.default_resolution == 1))
  {
    scrnwid = 320;
    scrnhit = 200;
    wtext_multiply = 1;
  }
  else
  {
    scrnwid = usetup.base_width;
    scrnhit = usetup.base_height;
    wtext_multiply = 1;
  }

  usetup.textheight = wgetfontheight(0) + 1;

  vesa_xres=scrnwid; vesa_yres=scrnhit;
  //scrnwto=scrnwid-1; scrnhto=scrnhit-1;
  current_screen_resolution_multiplier = scrnwid / BASEWIDTH;

  if ((game.default_resolution > 2) &&
    (game.options[OPT_NATIVECOORDINATES]))
  {
    usetup.base_width *= 2;
    usetup.base_height *= 2;
  }

  initasx=scrnwid;
  initasy=scrnhit;

  if (scrnwid==960) { initasx=1024; initasy=768; }

  // save this setting so we only do 640x480 full-screen if they want it
  usetup.want_letterbox = game.options[OPT_LETTERBOX];

  if (force_letterbox > 0)
    game.options[OPT_LETTERBOX] = 1;

  // don't allow them to force a 256-col game to hi-color
  if (game.color_depth < 2)
    usetup.force_hicolor_mode = 0;

  firstDepth = 8;
  secondDepth = 8;

  if ((game.color_depth == 2) || (force_16bit) || (usetup.force_hicolor_mode)) {
    firstDepth = 16;
    secondDepth = 15;
  }
  else if (game.color_depth > 2) {
    firstDepth = 32;
    secondDepth = 24;
  }

#ifndef MAC_HACK
#ifdef MAC_VERSION
  if (game.color_depth > 1)
  {
    // force true color - bug in hi color
    // only noticed in KQ2VGA, and haven't tracked down yet
    // (some gfx are corrupt)
    firstDepth = 32;
    secondDepth = 24;
  }
#endif
#endif

  adjust_sizes_for_resolution(loaded_game_file_version);
}

void create_gfx_driver() 
{
#ifdef ENABLE_THIS_LATER
#ifdef WINDOWS_VERSION
  if (ac_stricmp(usetup.gfxDriverID, "D3D9") == 0)
    gfxDriver = GetD3DGraphicsDriver(filter);
  else
#endif
    gfxDriver = GetSoftwareGraphicsDriver(filter);
#else
  gfxDriver = GetStubGraphicsDriver(filter);
#endif
  //gfxDriver = GetD3DGraphicsDriver(filter);
  gfxDriver = GetSoftwareGraphicsDriver(filter);
  gfxDriver->SetCallbackOnInit(GfxDriverOnInitCallback);
  gfxDriver->SetTintMethod(TintReColourise);
}

int initeng_switch_gfx_mode(int &initasx, int &initasy, int &firstDepth, int &secondDepth) 
{
  write_log_debug("Switching to graphics mode");

  if (switch_to_graphics_mode(initasx, initasy, scrnwid, scrnhit, firstDepth, secondDepth))
  {
    bool errorAndExit = true;

    if (((usetup.gfxFilterID == NULL) || 
      (ac_stricmp(usetup.gfxFilterID, "None") == 0)) &&
      (scrnwid == 320))
    {
      // If the game is 320x200 and no filter is being used, try using a 2x
      // filter automatically since many gfx drivers don't suport 320x200.
      write_log_debug("320x200 not supported, trying with 2x filter");
      delete filter;

      if (initialize_graphics_filter("StdScale2", initasx, initasy, firstDepth)) 
      {
        return EXIT_NORMAL;
      }

      create_gfx_driver();

      if (!switch_to_graphics_mode(initasx, initasy, scrnwid, scrnhit, firstDepth, secondDepth))
      {
        errorAndExit = false;
      }

    }

    if (errorAndExit)
    {
      proper_exit=1;
      platform->FinishedUsingGraphicsMode();

      // make sure the error message displays the true resolution
      if (game.options[OPT_LETTERBOX])
        initasy = (initasy * 12) / 10;

      if (filter != NULL)
        filter->GetRealResolution(&initasx, &initasy);

      platform->DisplayAlert("There was a problem initializing graphics mode %d x %d (%d-bit).\n"
        "(Problem: '%s')\n"
        "Try to correct the problem, or seek help from the AGS homepage.\n"
        "\nPossible causes:\n* your graphics card drivers do not support this resolution. "
        "Run the game setup program and try the other resolution.\n"
        "* the graphics driver you have selected does not work. Try switching between Direct3D and DirectDraw.\n"
        "* the graphics filter you have selected does not work. Try another filter.",
        initasx, initasy, firstDepth, alw_allegro_error);
      return EXIT_NORMAL;
    }
  }

  //alw_screen = _filter->ScreenInitialized(alw_screen, final_scrn_wid, final_scrn_hit);
  _old_screen = alw_screen;

  if (gfxDriver->HasAcceleratedStretchAndFlip()) 
  {
    walkBehindMethod = DrawAsSeparateSprite;

    CreateBlankImage();
  }

  return 0;
}

void initeng_prepare_gfx_screen(int &initasx, int &initasy, int &firstDepth, int &secondDepth) 
{

  write_log_debug("Preparing graphics mode screen");

  if ((final_scrn_hit != scrnhit) || (final_scrn_wid != scrnwid)) {
    initasx = final_scrn_wid;
    initasy = final_scrn_hit;
    alw_clear_bitmap(_old_screen);
    alw_screen = alw_create_sub_bitmap(_old_screen, initasx / 2 - scrnwid / 2, initasy/2-scrnhit/2, scrnwid, scrnhit);
    _sub_screen=alw_screen;

    scrnhit = BMP_H(alw_screen);
    vesa_yres = BMP_H(alw_screen);
    scrnwid = BMP_W(alw_screen);
    vesa_xres = BMP_W(alw_screen);
    gfxDriver->SetMemoryBackBuffer(alw_screen);

    platform->WriteDebugString("Screen resolution: %d x %d; game resolution %d x %d", BMP_W(_old_screen), BMP_H(_old_screen), scrnwid, scrnhit);
  }


  // Most cards do 5-6-5 RGB, which is the format the files are saved in
  // Some do 5-6-5 BGR, or  6-5-5 RGB, in which case convert the gfx
  if ((final_col_dep == 16) && ((alw_get_rgb_b_shift_16() != 0) || (alw_get_rgb_r_shift_16() != 11))) {
    convert_16bit_bgr = 1;
    if (alw_get_rgb_r_shift_16() == 10) {
      // some very old graphics cards lie about being 16-bit when they
      // are in fact 15-bit ... get around this
      _places_r = 3;
      _places_g = 3;
    }
  }
  if (final_col_dep > 16) {
    // when we're using 32-bit colour, it converts hi-color images
    // the wrong way round - so fix that
    alw_set_rgb_r_shift_16(11);
    alw_set_rgb_g_shift_16(5);
    alw_set_rgb_b_shift_16(0);
  }
  else if (final_col_dep <= 16) {
    // ensure that any 32-bit graphics displayed are converted
    // properly to the current depth
    alw_set_rgb_r_shift_32(16);
    alw_set_rgb_g_shift_32(8);
    alw_set_rgb_b_shift_32(0);
  }

  platform->PostAllegroInit((usetup.windowed > 0) ? true : false);

  gfxDriver->SetCallbackForPolling(update_polled_stuff);
  gfxDriver->SetCallbackToDrawScreen(draw_screen_callback);
  gfxDriver->SetCallbackForNullSprite(GfxDriverNullSpriteCallback);
}

void initeng_init_screen_buffers() 
{
  write_log_debug("Set up screen");

  virtual_screen=alw_create_bitmap_ex(final_col_dep,scrnwid,scrnhit);
  alw_clear_bitmap(virtual_screen);
  gfxDriver->SetMemoryBackBuffer(virtual_screen);
  //  ignore_mouseoff_bitmap = virtual_screen;
  abuf=alw_screen;

  set_eip(-7);
  for (int ee = 0; ee < MAX_INIT_SPR + game.numcharacters; ee++)
    actsps[ee] = NULL;
}

void initeng_fill_scsystem() 
{
  scsystem.width = final_scrn_wid;
  scsystem.height = final_scrn_hit;
  scsystem.coldepth = final_col_dep;
  scsystem.windowed = 0;
  scsystem.vsync = 0;
  scsystem.viewport_width = divide_down_coordinate(scrnwid);
  scsystem.viewport_height = divide_down_coordinate(scrnhit);
  strcpy(scsystem.aci_version, ACI_VERSION_TEXT);
  scsystem.os = platform->GetSystemOSID();

  if (usetup.windowed)
    scsystem.windowed = 1;
}

void initeng_configure_mouse() 
{
#if defined(WINDOWS_VERSION) || defined(LINUX_VERSION) || defined(MAC_VERSION)
  filter->SetMouseArea(0, 0, scrnwid-1, scrnhit-1);
#else
  filter->SetMouseArea(0,0,BASEWIDTH-1,BASEHEIGHT-1);
#endif
  //  mloadwcursor("mouse.spr");
  //mousecurs[0]=spriteset[2054];
  currentcursor=0;
  set_eip(-4);
  mousey=100;  // stop icon bar popping up
}

void initeng_configure_screen() 
{
  wsetscreen(virtual_screen);
  set_eip(-41);
  gfxDriver->SetRenderOffset(get_screen_x_adjustment(virtual_screen), get_screen_y_adjustment(virtual_screen));
}


int initialize_engine(int argc,char*argv[])
{
  int result;

  // CONFIG FILE ***********************************************************************************************

  write_log_debug("Reading config file");
  set_eip(-200);
  read_config_file(argv[0]);

  // ALLEGRO ***********************************************************************************************

  result = initeng_init_allegro();
  if (result) return result;

  // WINDOW ***********************************************************************************************

  result = initeng_init_window(argc, argv);
  if (result) return result;

  // GAME DATA ***********************************************************************************************

  result = initeng_init_game_data(argc, argv);
  if (result) return result;

  // FONTS ***********************************************************************************************

  set_eip(-192);
  write_log_debug("Initializing TTF renderer");
  init_font_renderer();

  // MOUSE ***********************************************************************************************

  result = initeng_init_mouse();
  if (result) return result;

  // MEM CHECK ***********************************************************************************************

  result = initeng_mem_check();
  if (result) return result;

  // REPLAY ***********************************************************************************************

  unlink (replayTempFile);

  // ROOMS ***********************************************************************************************

  initeng_init_rooms();

  // SPEECH ***********************************************************************************************

  result = initeng_init_speech();
  if (result) return result;

  // MUSIC ***********************************************************************************************

  result = initeng_init_music();
  if (result) return result;

  // KEYBOARD INPUT ***********************************************************************************************

  set_eip(-184);
  keyboard_input_initialise();

  // TIMER ***********************************************************************************************

  set_eip(-183);
  write_log_debug("Install timer");
  alw_install_timer();

  // SOUND INITS ***********************************************************************************************

  initeng_init_sound();

  // ENGINE DEBUG MSG ***********************************************************************************************

  if ((debug_flags & (~DBG_DEBUGMODE)) >0) {
    platform->DisplayAlert("Engine debugging enabled.\n"
     "\nNOTE: You have selected to enable one or more engine debugging options.\n"
     "These options cause many parts of the game to behave abnormally, and you\n"
     "may not see the game as you are used to it. The point is to test whether\n"
     "the engine passes a point where it is crashing on you normally.\n"
     "[Debug flags enabled: 0x%02X]\n"
     "Press a key to continue.\n",debug_flags);
    }

  // EXIT HANDLER ***********************************************************************************************

  set_eip(-10);
  write_log_debug("Install exit handler");
  atexit(atexit_handler);

  // DEBUG LOG ***********************************************************************************************

  unlink("warnings.log");

  // RAND ***********************************************************************************************

  play.randseed = time(NULL);
  srand (play.randseed);


  // PATH FINDER ***********************************************************************************************

  write_log_debug("Initialize path finder library");
  init_pathfinder();


  // GFX ***********************************************************************************************

  write_log_debug("Initialize gfx");
  platform->InitialiseAbufAtStartup();

  // TIMING ***********************************************************************************************

  ALW_LOCK_VARIABLE(timerloop);
  ALW_LOCK_FUNCTION(dj_timer_handler);
  set_game_speed(40);

  // UNUSED ***********************************************************************************************

  set_eip(-20);
  //thisroom.allocall();
  set_eip(-19);
  //setup_sierra_interface();   // take this out later

  // GAME DATA ***********************************************************************************************

  result = initeng_load_game_data();
  if (result) return result;

  // REGISTER GAME ***********************************************************************************************

  if (justRegisterGame) 
  {
    platform->RegisterGameWithGameExplorer();
    proper_exit = 1;
    return EXIT_NORMAL;
  }

  if (justUnRegisterGame) 
  {
    platform->UnRegisterGameWithGameExplorer();
    proper_exit = 1;
    return EXIT_NORMAL;
  }

  // WINDOW TITLE ***********************************************************************************************

  //platform->DisplayAlert("loaded game");
  set_eip(-91);
#if (ALW_ALLEGRO_DATE > 19990103)
  alw_set_window_title(game.gamename);
#endif

  // LOAD EXTRA GAME DATA ***********************************************************************************************

  initeng_load_extra_game_data();


  // DISK SPACE ***********************************************************************************************

  result = initeng_check_disk_space();
  if (result) return result;

  // FONTS ***********************************************************************************************

  if (fontRenderers[0] == NULL) 
  {
    platform->DisplayAlert("No fonts found. If you're trying to run the game "
      "from the Debug directory, this is not supported. Use the Build EXE "
      "command to create an executable in the Compiled folder.");
    proper_exit = 1;
    return EXIT_NORMAL;
  }

  // MOD PLAYER ***********************************************************************************************

  initeng_init_mod_player();

  // SCREEN ***********************************************************************************************

  int initasx,initasy;
  int firstDepth, secondDepth;
  initeng_init_screen_settings(initasx, initasy, firstDepth, secondDepth);

  // FILTERS ***********************************************************************************************

  write_log_debug("Init gfx filters");
  if (initialize_graphics_filter(usetup.gfxFilterID, initasx, initasy, firstDepth))
  {
    return EXIT_NORMAL;
  }
  
  // GFX DRIVER ***********************************************************************************************

  write_log_debug("Init gfx driver");
  create_gfx_driver();

  // GFX MODE ***********************************************************************************************

  result = initeng_switch_gfx_mode(initasx, initasy, firstDepth, secondDepth);
  if (result) return result;

  // GFX SCREEN ***********************************************************************************************

  initeng_prepare_gfx_screen(initasx, initasy, firstDepth, secondDepth);

  // COLOURS ***********************************************************************************************

  write_log_debug("Initializing colour conversion");
  alw_set_color_conversion(COLORCONV_MOST | COLORCONV_EXPAND_256 | COLORCONV_REDUCE_16_TO_15);

  // MULTITASKING ***********************************************************************************************

  SetMultitasking(0); // don't run in background

  // PRELOAD IMG ***********************************************************************************************

  write_log_debug("Check for preload image");
  show_preload ();

  // SPRITES INIT ***********************************************************************************************

  write_log_debug("Initialize sprites");

  if (spriteset.initFile ("acsprset.spr")) 
  {
    platform->FinishedUsingGraphicsMode();
    alw_allegro_exit();
    proper_exit=1;
    platform->DisplayAlert("Could not load sprite set file ACSPRSET.SPR\n"
      "This means that the file is missing or there is not enough free\n"
      "system memory to load the file.\n");
    return EXIT_NORMAL;
  }

  // SCREEN ***********************************************************************************************

  initeng_init_screen_buffers();

  // GAME SETTINGS ***********************************************************************************************

  write_log_debug("Initialize game settings");
  init_game_settings();

  // START ***********************************************************************************************

  write_log_debug("Prepare to start game");

  // SET SCSYSTEM ***********************************************************************************************

  initeng_fill_scsystem();

  // MOUSE CONFIG ***********************************************************************************************

  initeng_configure_mouse();

  // DIRTY RECTS ***********************************************************************************************

  init_invalid_regions(final_scrn_hit);

  // SETUP SCREEN ***********************************************************************************************

  initeng_configure_screen();

  // START ***********************************************************************************************

  initialize_start_and_play_game(override_start_room, loadSaveGameOnStartup);

  // END ***********************************************************************************************

  quit("|bye!");
  return 0;
}

#if defined(WINDOWS_VERSION) || defined(LINUX_VERSION) || defined(MAC_VERSION)
ALW_END_OF_MAIN()
#endif

int initialize_engine_with_exception_handling(int argc,char*argv[])
{
  write_log_debug("Installing exception handler");

#ifdef USE_CUSTOM_EXCEPTION_HANDLER
  __try 
  {
#endif

    return initialize_engine(argc, argv);

#ifdef USE_CUSTOM_EXCEPTION_HANDLER
  }
  __except (CustomExceptionHandler ( GetExceptionInformation() )) 
  {
    strcpy (tempmsg, "");
    sprintf (printfworkingspace, "An exception 0x%X occurred in ACWIN.EXE at EIP = 0x%08X %s; program pointer is %+d, ACI version " ACI_VERSION_TEXT ", gtags (%d,%d)\n\n"
      "AGS cannot continue, this exception was fatal. Please note down the numbers above, remember what you were doing at the time and post the details on the AGS Technical Forum.\n\n%s\n\n"
      "Most versions of Windows allow you to press Ctrl+C now to copy this entire message to the clipboard for easy reporting.\n\n%s (code %d)",
      excinfo.ExceptionCode, excinfo.ExceptionAddress, tempmsg, our_eip, eip_guinum, eip_guiobj, get_cur_script(5),
      (miniDumpResultCode == 0) ? "An error file CrashInfo.dmp has been created. You may be asked to upload this file when reporting this problem on the AGS Forums." : 
      "Unable to create an error dump file.", miniDumpResultCode);
    MessageBoxA(alw_get_allegro_wnd(), printfworkingspace, "Illegal exception", MB_ICONSTOP | MB_OK);
    proper_exit = 1;
  }
  return EXIT_CRASH;
#endif
}

