#ifndef _AC_H_HEADER
#define _AC_H_HEADER

// forward declarations:
#include <stdio.h>          // for FILE
#include "allegro.h"        // for ALW_BITMAP
typedef ALW_BITMAP *block;      // wgt2allh.h
class IDriverDependantBitmap; // ali3d.h
struct CharacterInfo;       // acroom.h
struct CustomProperties;    // acroom.h
struct NewInteraction;      // acroom.h
struct InteractionScripts;  // acroom.h
struct ccInstance;          // cscomp.h
struct EventHappened;       // acruntim.h
struct ViewFrame;           // acroom.h
struct InteractionVariable; // acroom.h

#ifdef NO_MP3_PLAYER
#define _AC_SPECIAL_VERSION "NMP"
#else
#define _AC_SPECIAL_VERSION ""
#endif
// Version and build numbers
#define AC_VERSION_TEXT "3.21 "
#define ACI_VERSION_TEXT "3.21.1115"_AC_SPECIAL_VERSION
// this needs to be updated if the "play" struct changes
#define LOWEST_SGVER_COMPAT "3.20.1103"_AC_SPECIAL_VERSION

// Check that a supplied buffer from a text script function was not null
#define VALIDATE_STRING(strin) if ((unsigned long)strin <= 4096) quit("!String argument was null: make sure you pass a string, not an int, as a buffer")

#define IS_ANTIALIAS_SPRITES usetup.enable_antialiasing && (play.disable_antialiasing == 0)

#define write_log_debug(msg) platform->WriteDebugString(msg)

#define Random __Rand

#define DEBUG_CONSOLE if (play.debug_mode) debug_write_console

#define scAdd_External_Symbol ccAddExternalSymbol


extern void quit(const char*quitmsg);
extern void quitprintf(const char*texx, ...);
extern void put_sprite_256(int, int, block);
extern void write_log(char*msg);
extern void initialize_sprite (int ee);
extern void pre_save_sprite(int ee);

// for acwavi.cpp
extern void update_polled_stuff_and_crossfade();
extern void update_polled_stuff();
extern void next_iteration();
extern void update_music_volume();
extern void render_to_screen(ALW_BITMAP *toRender, int atx, int aty);

// for acfonts.cpp
extern bool ShouldAntiAliasText();

// for routefnd.cpp
extern char ac_engine_copyright[];
extern void Display(char *, ...);
extern void write_log(char *);
extern void update_polled_stuff();

// for acsound.cpp
extern void write_log(char*msg) ;
//extern void sample_update_callback(ALW_SAMPLE *sample, int voice);

// for acwavi3d.cpp
extern void update_polled_stuff_and_crossfade();
extern void next_iteration();

// for acdialog.cpp
extern int load_game(int,char*, int*);
extern void break_up_text_into_lines(int wii,int fonnt,char*todis);
extern void wouttext_outline(int xxp,int yyp,int usingfont,char*texx);
extern inline int get_fixed_pixel_size(int pixels);

// for acdialog.cpp
extern void next_iteration();
extern void update_polled_stuff();
extern void update_polled_stuff_and_crossfade();
extern char *get_global_message(int);
extern char saveGameSuffix[21];
extern char *get_language_text(int);
extern void get_save_game_path(int slotNum, char *buffer);
extern char saveGameDirectory[260];

// for acgui.cpp
extern void draw_sprite_compensate(int spr, int x, int y, int xray);
extern inline int divide_down_coordinate(int coord);
extern inline int multiply_up_coordinate(int coord);
extern inline void multiply_up_coordinates(int *x, int *y);
extern inline int get_fixed_pixel_size(int pixels);

// for acchars.cpp
extern const char* Character_GetTextProperty(CharacterInfo *chaa, const char *property);
extern CharacterInfo *GetCharacterAtLocation(int xx, int yy);
extern int Character_GetSpeakingFrame(CharacterInfo *chaa);

extern void ccAddExternalSymbol(const char *namof, void *addrof);


// for ac_mouse.cpp
extern void multiply_up_coordinates_round_up(int *x, int *y);
extern void draw_sprite_support_alpha(int xpos, int ypos, block image, int slot);
extern void precache_view(int view) ;
extern int GetMaxScreenHeight ();
extern void GetLocationName(int xxx,int yyy,char*tempo);
extern int  GetLocationType(int,int);
extern void CheckViewFrame (int view, int loop, int frame);
extern void invalidate_sprite(int x1, int y1, IDriverDependantBitmap *pic);

// for ac_room.cpp
extern int get_walkable_area_pixel(int x, int y);
extern void get_message_text (int msnum, char *buffer, char giveErr);
extern void replace_tokens(char*srcmes,char*destm, int maxlen = 99999);
extern int GetCharacterHeight(int charid);
extern int get_int_property (CustomProperties *cprop, const char *property);
extern void get_text_property (CustomProperties *cprop, const char *property, char *bufer) ;
extern const char* get_text_property_dynamic_string(CustomProperties *cprop, const char *property) ;
extern void redo_walkable_areas() ;
extern void check_viewport_coords() ;
extern void mark_current_background_dirty();
extern void invalidate_cached_walkbehinds() ;

// for ac_obj.cpp
extern void do_main_cycle(int untilwhat,int daaa);
extern void scAnimateCharacter (int chh, int loopn, int sppd, int rept);
extern void tint_image (block srcimg, block destimg, int red, int grn, int blu, int light_level, int luminance);
extern block recycle_bitmap(block bimp, int coldep, int wid, int hit);
extern int get_walkable_area_at_location(int xx, int yy);
extern int  run_interaction_event (NewInteraction *nint, int evnt, int chkAny = -1, int isInv = 0);
extern int  run_interaction_script(InteractionScripts *nint, int evnt, int chkAny = -1, int isInv = 0);
extern void sort_out_char_sprite_walk_behind(int actspsIndex, int xx, int yy, int basel, int zoom, int width, int height);
extern int sort_out_walk_behinds(block sprit,int xx,int yy,int basel, block copyPixelsFrom = NULL, block checkPixelsFrom = NULL, int zoom=100);
extern void add_to_sprite_list(IDriverDependantBitmap* spp, int xx, int yy, int baseline, int trans, int sprNum, bool isWalkBehind = false);
extern int is_pos_in_sprite(int xx,int yy,int arx,int ary, block sprit, int spww,int sphh, int flipped = 0);

// for sprcache.cpp
extern void get_new_size_for_sprite (int ee, int ww, int hh, int &newwid, int &newhit);

// for ac_main.cpp
extern void get_script_name(ccInstance *rinst, char *curScrName);
extern void process_event(EventHappened*evp);
extern int run_text_script(ccInstance*sci,char*tsname);
extern void setevent(int evtyp,int ev1,int ev2,int ev3);

extern void render_graphics(IDriverDependantBitmap *extraBitmap = NULL, int extraX = 0, int extraY = 0);

// for ac_dialog.h
extern void can_run_delayed_command();
extern void do_conversation(int dlgnum);
extern int show_dialog_options(int dlgnum, int sayChosenOption, bool runGameLoopsInBackground);

// for ac_overlay
extern void get_overlay_position(int overlayidx, int *x, int *y);
extern int GetTextDisplayTime (char *text, int canberel=0);
extern int _display_main(int xx,int yy,int wii,char*todis,int blocking,int usingfont,int asspch, int isThought, int allowShrink, bool overlayPositionFixed) ;

// for ac_gui_inv_window
extern void write_screen();
extern void mainloop(bool checkControls = false, IDriverDependantBitmap *extraBitmap = NULL, int extraX = 0, int extraY = 0);
extern void construct_virtual_screen(bool fullRedraw);


// for ac_hotspot
extern void walk_character(int chac,int tox,int toy,int ignwal, bool autoWalkAnims);
extern int is_pos_on_character(int xx,int yy);

// for ac_drawsurf
extern void get_message_text (int msnum, char *buffer, char giveErr = 1);
extern void tint_image (block source, block dest, int red, int grn, int blu, int light_level, int luminance=255);
extern void push_screen ();
extern void pop_screen();

// for acchars
extern int play_speech(int charid,int sndid) ;
extern void stop_speech() ;
extern int adjust_y_for_guis ( int yy);
extern int get_textwindow_top_border_height (int twgui) ;
extern void DrawViewFrame(block target, ViewFrame *vframe, int x, int y);
extern int get_textwindow_border_width (int twgui);
extern void _display_at(int xx,int yy,int wii,char*todis,int blocking,int asspch, int isThought, int allowShrink, bool overlayPositionFixed);
extern void new_room(int newnum,CharacterInfo*forchar);
extern void setevent(int evtyp,int ev1=0,int ev2=-1000,int ev3=0);


// for ac_multimedia.h
extern int calculate_max_volume();
extern void update_ambient_sound_vol ();
extern int check_for_messages_from_editor();

// for ac_text
extern int GetGlobalInt(int index);

// for ac_Screen
extern void clear_letterbox_borders();

// for ac_game
extern int load_game_and_print_error(int toload);
extern void unload_old_room();
extern void unload_game_file();
extern void show_preload ();
extern int load_game_file();
extern void init_game_settings();
extern void start_game();
extern void set_game_speed(int fps);
extern InteractionVariable *FindGraphicalVariable(const char *varName);
extern int convert_gui_disabled_style(int oldStyle);
extern void convert_guid_from_text_to_binary(const char *guidText, unsigned char *buffer) ;
extern void safeguard_string (unsigned char *descript);
extern void save_game_data (FILE *ooo, block screenshot);
extern uint64_t write_screen_shot_for_vista(FILE *ooo, block screenshot) ;
extern void initialize_skippable_cutscene();
extern void stop_fast_forwarding();

#endif