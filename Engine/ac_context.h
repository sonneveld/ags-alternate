#ifndef _AC_CONTEXT_H_HEADER
#define _AC_CONTEXT_H_HEADER

#include "wgt2allg.h"
#include "ac_types.h"
#include "acruntim.h" // multiple types

//forward declarations:
struct AGSPlatformDriver;   // acplatfm.h
class IGraphicsDriver;      // ali3d.h
struct SOUNDCLIP;           // acsound.h
struct IAGSEditorDebugger;  // acdebug.h


// External variables:

extern int source_text_length;

extern int eip_guinum;
extern int eip_guiobj;
extern int trans_mode;
extern int engineNeedsAsInt;

extern struct GlobalMouseState global_mouse_state;
extern struct GlobalRoomState global_room_state;
extern char *music_file;
extern char *speech_file;
extern WCHAR directoryPathBuffer[];

#define REC_MOUSECLICK 1
#define REC_MOUSEMOVE  2
#define REC_MOUSEDOWN  3
#define REC_KBHIT      4
#define REC_GETCH      5
#define REC_KEYDOWN    6
#define REC_MOUSEWHEEL 7
#define REC_SPEECHFINISHED 8
#define REC_ENDOFFILE  0x6f
extern short *recordbuffer;
extern int  recbuffersize;
extern int recsize;
extern volatile int switching_away_from_game;

extern int musicPollIterator;

extern const char* sgnametemplate;
extern char saveGameSuffix[];

extern SOUNDCLIP *channels[];
extern SOUNDCLIP *cachedQueuedMusic;
extern int numSoundChannels;
#define SCHAN_SPEECH  0
#define SCHAN_AMBIENT 1
#define SCHAN_MUSIC   2
#define SCHAN_NORMAL  3
#define AUDIOTYPE_LEGACY_AMBIENT_SOUND 1
#define AUDIOTYPE_LEGACY_MUSIC 2
#define AUDIOTYPE_LEGACY_SOUND 3

#define MAX_ANIMATING_BUTTONS 15
#define RESTART_POINT_SAVE_GAME_NUMBER 999

extern AGSPlatformDriver *platform;
// crossFading is >0 (channel number of new track), or -1 (old
// track fading out, no new track)
extern int crossFading;
extern int crossFadeVolumePerStep;
extern int crossFadeStep;
extern int crossFadeVolumeAtStart;
extern int last_sound_played[];
extern char *heightTestString;
extern block virtual_screen;
extern int scrnwid;
extern int scrnhit;
extern int current_screen_resolution_multiplier;
extern roomstruct thisroom;
extern GameSetupStruct game;
extern RoomStatus *roomstats;
extern RoomStatus troom;
extern GameState play;
extern GameSetup usetup;
extern CharacterExtras *charextra;
extern int force_letterbox;
extern int game_paused;
extern int ifacepopped;
extern color palette[];
// initially size 1, this will be increased by the initFile function
extern SpriteCache spriteset;
extern long t1;
extern int cur_mode;
extern int cur_cursor;
extern int spritewidth[];
extern int spriteheight[];
extern char saveGameDirectory[];
extern int fps;
extern int display_fps;
extern DebugConsoleText debug_line[];
extern int first_debug_line;
extern int last_debug_line;
extern int display_console;
extern char *walkBehindExists;
extern int *walkBehindStartY;
extern int *walkBehindEndY;
extern char noWalkBehindsAtAll;
extern int walkBehindLeft[];
extern int walkBehindTop[];
extern int walkBehindRight[];
extern int walkBehindBottom[];
extern IDriverDependantBitmap *walkBehindBitmap[];
extern int walkBehindsCachedForBgNum;
extern WalkBehindMethodEnum walkBehindMethod;
extern unsigned long loopcounter;
extern unsigned long lastcounter;
extern volatile unsigned long globalTimerCounter;
extern char alpha_blend_cursor;
extern RoomObject*objs;
extern RoomStatus*croom;
extern CharacterInfo*playerchar;
extern long _sc_PlayerCharPtr;
extern int offsetx;
extern int offsety;
extern int use_extra_sound_offset;
extern GUIMain*guis;
extern block *guibg;
extern IDriverDependantBitmap **guibgbmp;
extern ccScript* gamescript;
extern ccScript* dialogScriptsScript;
extern ccInstance *gameinst;
extern ccInstance *roominst;
extern ccInstance *dialogScriptsInst;
extern ccInstance *gameinstFork;
extern ccInstance *roominstFork;
extern IGraphicsDriver *gfxDriver;
extern IDriverDependantBitmap *blankImage;
extern IDriverDependantBitmap *blankSidebarImage;
extern IDriverDependantBitmap *debugConsole;
extern block debugConsoleBuffer;

extern bool current_background_is_dirty;
extern int longestline;

extern PluginObjectReader pluginReaders[];
extern int numPluginReaders;

extern ccScript *scriptModules[];
extern ccInstance *moduleInst[];
extern ccInstance *moduleInstFork[];
extern char *moduleRepExecAddr[];
extern int numScriptModules;

extern ViewStruct*views;
extern COLOR_MAP maincoltable;
extern ScriptSystem scsystem;
extern block _old_screen;
extern block _sub_screen;
extern MoveList *mls;
extern DialogTopic *dialog;
extern block walkareabackup;
extern block walkable_areas_temp;
extern ExecutingScript scripts[];
extern ExecutingScript*curscript;
extern AnimatingGUIButton animbuts[];
extern int numAnimButs;
extern int num_scripts;
extern int eventClaimed;
extern int getloctype_index;
extern int getloctype_throughgui;
extern int user_disabled_for;
extern int user_disabled_data;
extern int user_disabled_data2;
extern int user_disabled_data3;
extern int is_complete_overlay;
extern int is_text_overlay;
// Sierra-style speech settings
extern int face_talking;
extern int facetalkview;
extern int facetalkwait;
extern int facetalkframe;
extern int facetalkloop;
extern int facetalkrepeat;
extern int facetalkAllowBlink;
extern int facetalkBlinkLoop;
extern CharacterInfo *facetalkchar;
// lip-sync speech settings
extern int loops_per_character;
extern int text_lips_offset;
extern int char_speaking;
extern char *text_lips_text;
extern SpeechLipSyncLine *splipsync;
extern int numLipLines;
extern int curLipLine;
extern int curLipLinePhenome;
extern int gameHasBeenRestored;
extern char **characterScriptObjNames;
extern char objectScriptObjNames[];
extern char **guiScriptObjNames;

// set to 0 once successful
extern int working_gfx_mode_status;

extern int said_speech_line;

extern int restrict_until;
extern int gs_to_newroom;
extern ScreenOverlay screenover[];
extern int proper_exit;
extern int our_eip;
extern int numscreenover;
extern int scaddr;
extern int walk_behind_baselines_changed;
extern int displayed_room;
extern int starting_room;
extern int mouse_on_iface;
extern int mouse_on_iface_button;
extern int mouse_pushed_iface;
extern int mouse_ifacebut_xoffs;
extern int mouse_ifacebut_yoffs;
extern int debug_flags;
extern IDriverDependantBitmap* roomBackgroundBmp;

extern int use_compiled_folder_as_current_dir;
extern int editor_debugging_enabled;
extern int editor_debugging_initialized;
extern char editor_debugger_instance_token[];
extern IAGSEditorDebugger *editor_debugger;
extern int break_on_next_script_step;
extern volatile int game_paused_in_debugger;
extern HWND editor_window_handle;

extern int in_enters_screen;
extern int done_es_error;
extern int in_leaves_screen;
extern int need_to_stop_cd;
extern int debug_15bit_mode;
extern int debug_24bit_mode;
extern int said_text;
extern int convert_16bit_bgr;
extern int mouse_z_was;
extern int bg_just_changed;
extern int loaded_game_file_version;
extern volatile char want_exit;
extern volatile char abort_engine;
extern char check_dynamic_sprites_at_exit;
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
#define MAXEVENTS 15
extern EventHappened event[];
extern int numevents;
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
extern char ac_engine_copyright[];
extern int current_music_type;

#define LOCTYPE_HOTSPOT 1
#define LOCTYPE_CHAR 2
#define LOCTYPE_OBJ  3

#define MAX_DYNAMIC_SURFACES 20
#define REP_EXEC_ALWAYS_NAME "repeatedly_execute_always"
#define REP_EXEC_NAME "repeatedly_execute"

extern char*tsnames[];
extern char*evblockbasename;
extern int evblocknum;
extern int frames_per_second;
extern int in_new_room;
extern int new_room_was;
extern int new_room_pos;
extern int new_room_x;
extern int new_room_y;
extern unsigned int load_new_game;
extern int load_new_game_restore;
extern int inside_script;
extern int in_graph_script;
extern int no_blocking_functions;
extern int in_inv_screen;
extern int inv_screen_newroom;
extern int new_room_flags;
#define MAX_SPRITES_ON_SCREEN 76
extern SpriteListEntry sprlist[];
extern int sprlistsize;
#define MAX_THINGS_TO_DRAW 125
extern SpriteListEntry thingsToDrawList[];
extern int thingsToDrawSize;
extern int use_cdplayer;
extern bool triedToUseCdAudioCommand;
extern int final_scrn_wid;
extern int final_scrn_hit;
extern int final_col_dep;
extern int post_script_cleanup_stack;
// actsps is used for temporary storage of the bitamp image
// of the latest version of the sprite
extern int actSpsCount;
extern block *actsps;
extern IDriverDependantBitmap* *actspsbmp;
// temporary cache of walk-behind for this actsps image
extern block *actspswb;
extern IDriverDependantBitmap* *actspswbbmp;
extern CachedActSpsData* actspswbcache;

extern CharacterCache *charcache;
extern ObjectCache objcache[];

extern ScriptObject scrObj[];
extern ScriptGUI *scrGui;
extern ScriptHotspot scrHotspot[];
extern ScriptRegion scrRegion[];
extern ScriptInvItem scrInv[];
extern ScriptDialog scrDialog[];

extern RGB_MAP rgb_table;
extern char* game_file_name;
extern int want_quit;
extern int screen_reset;
extern block raw_saved_screen;
extern block dynamicallyCreatedSurfaces[];
extern int scrlockWasDown;
// whether there are currently remnants of a DisplaySpeech
extern int screen_is_dirty;
extern char replayfile[];
extern int replay_time;
extern unsigned long replay_last_second;
extern int replay_start_this_time;
extern char pexbuf[];

extern int pluginsWantingDebugHooks;

extern const char *replayTempFile;

extern NonBlockingScriptFunction repExecAlways;
extern NonBlockingScriptFunction getDialogOptionsDimensionsFunc;
extern NonBlockingScriptFunction renderDialogOptionsFunc;
extern NonBlockingScriptFunction getDialogOptionUnderCursorFunc;
extern NonBlockingScriptFunction runDialogOptionMouseClickHandlerFunc;

// set by dj_timer_handler
extern volatile int timerloop;
extern volatile int mvolcounter;
extern int update_music_at;
extern int time_between_timers;

extern int char_lowest_yp;
extern int obj_lowest_yp;

#endif
