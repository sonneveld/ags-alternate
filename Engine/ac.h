#ifndef _AC_H_HEADER
#define _AC_H_HEADER

#include "wgt2allg.h"
#include "ali3d.h"
#include "acroom.h"
#include "acruntim.h"
#include "sprcache.h"
#include "acgui.h"
#include "ac_mouse.h"

// Check that a supplied buffer from a text script function was not null
#define VALIDATE_STRING(strin) if ((unsigned long)strin <= 4096) quit("!String argument was null: make sure you pass a string, not an int, as a buffer")

extern void quit(char*quitmsg);
extern void put_sprite_256(int, int, block);
extern char alpha_blend_cursor ;
extern color palette[256];

extern void write_log(char*msg);
extern void initialize_sprite (int ee);
extern void pre_save_sprite(int ee);
extern void get_new_size_for_sprite (int ee, int ww, int hh, int &newwid, int &newhit);
extern int spritewidth[], spriteheight[];

// for acwavi.cpp
extern void update_polled_stuff_and_crossfade();
extern void update_polled_stuff();
extern int rec_mgetbutton();
extern int rec_kbhit();
extern int rec_getch();
extern void next_iteration();
extern void update_music_volume();
extern void render_to_screen(BITMAP *toRender, int atx, int aty);
extern int crossFading;
extern int crossFadeStep;
extern volatile char want_exit;
extern IGraphicsDriver *gfxDriver;

// for acfonts.cpp
extern bool ShouldAntiAliasText();
extern int our_eip;

// for routefnd.cpp
extern char ac_engine_copyright[];
extern void Display(char *, ...);
extern void write_log(char *);
extern void update_polled_stuff();
extern MoveList *mls;

// for acplwin.cpp
extern char* game_file_name;

// for acsound.cpp
extern int use_extra_sound_offset;
extern int our_eip;
extern void write_log(char*msg) ;
//extern void sample_update_callback(SAMPLE *sample, int voice);

// for acwavi3d.cpp
extern int rec_mgetbutton();
extern int rec_kbhit();
extern int rec_getch();
extern void update_polled_stuff_and_crossfade();
extern volatile char want_exit;
extern volatile int timerloop;
extern void next_iteration();

// for acdialog.cpp
extern int rec_getch();
extern int rec_kbhit();
extern int load_game(int,char*, int*);
extern void break_up_text_into_lines(int wii,int fonnt,char*todis);
extern void wouttext_outline(int xxp,int yyp,int usingfont,char*texx);
extern IGraphicsDriver *gfxDriver;
extern inline int get_fixed_pixel_size(int pixels);

// for acdialog.cpp
extern void rec_domouse(int);
extern int rec_misbuttondown(int);
extern int rec_mgetbutton();
extern void next_iteration();
extern void render_graphics(IDriverDependantBitmap *extraBitmap, int extraX, int extraY);
extern void update_polled_stuff();
extern int sgsiglen;
extern void update_polled_stuff_and_crossfade();
extern char *get_global_message(int);
extern int GetBaseWidth();
extern GameSetup usetup;
extern volatile int timerloop;
extern char saveGameSuffix[21];
extern char *get_language_text(int);
extern void get_save_game_path(int slotNum, char *buffer);
extern char saveGameDirectory[260];

// for acgui.cpp
extern SpriteCache spriteset;
extern void draw_sprite_compensate(int spr, int x, int y, int xray);
extern inline int divide_down_coordinate(int coord);
extern inline int multiply_up_coordinate(int coord);
extern inline void multiply_up_coordinates(int *x, int *y);
extern inline int get_fixed_pixel_size(int pixels);

// for acchars.cpp
extern int current_screen_resolution_multiplier;
extern int engineNeedsAsInt;
extern const char* Character_GetTextProperty(CharacterInfo *chaa, const char *property);
extern CharacterInfo *GetCharacterAtLocation(int xx, int yy);
extern int Character_GetSpeakingFrame(CharacterInfo *chaa);

extern void ccAddExternalSymbol(char *namof, void *addrof);
#define scAdd_External_Symbol ccAddExternalSymbol

// for ac_mouse.cpp
extern void multiply_up_coordinates_round_up(int *x, int *y);
extern void draw_sprite_support_alpha(int xpos, int ypos, block image, int slot);
extern void precache_view(int view) ;
extern int GetMaxScreenHeight ();
extern void GetLocationName(int xxx,int yyy,char*tempo);
extern int  GetLocationType(int,int);
extern void CheckViewFrame (int view, int loop, int frame);
extern void invalidate_sprite(int x1, int y1, IDriverDependantBitmap *pic);

extern struct GlobalMouseState global_mouse_state;
extern GUIMain*guis;

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
extern void setpal();
extern RoomStatus *roomstats;
extern int walk_behind_baselines_changed ;
extern int bg_just_changed;

// for ac_obj.cpp
extern void scAnimateCharacter (int chh, int loopn, int sppd, int rept);
extern block *actsps;
extern void tint_image (block srcimg, block destimg, int red, int grn, int blu, int light_level, int luminance);
extern block recycle_bitmap(block bimp, int coldep, int wid, int hit);
#define IS_ANTIALIAS_SPRITES usetup.enable_antialiasing && (play.disable_antialiasing == 0)
extern IDriverDependantBitmap* *actspsbmp;
extern int in_new_room;
extern int get_walkable_area_at_location(int xx, int yy);
extern int  run_interaction_event (NewInteraction *nint, int evnt, int chkAny = -1, int isInv = 0);
extern int  run_interaction_script(InteractionScripts *nint, int evnt, int chkAny = -1, int isInv = 0);
enum WalkBehindMethodEnum
{
  DrawOverCharSprite,
  DrawAsSeparateSprite,
  DrawAsSeparateCharSprite
};
extern WalkBehindMethodEnum walkBehindMethod;
extern void sort_out_char_sprite_walk_behind(int actspsIndex, int xx, int yy, int basel, int zoom, int width, int height);
extern int sort_out_walk_behinds(block sprit,int xx,int yy,int basel, block copyPixelsFrom = NULL, block checkPixelsFrom = NULL, int zoom=100);
extern void add_to_sprite_list(IDriverDependantBitmap* spp, int xx, int yy, int baseline, int trans, int sprNum, bool isWalkBehind = false);
extern int is_pos_in_sprite(int xx,int yy,int arx,int ary, block sprit, int spww,int sphh, int flipped = 0);
extern int char_lowest_yp, obj_lowest_yp;
extern ScriptObject scrObj[MAX_INIT_SPR];
extern char*evblockbasename;
extern int evblocknum;

#endif