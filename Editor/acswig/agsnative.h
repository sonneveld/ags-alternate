#define WINDOWS_VERSION
#define USE_CLIB
#define SWAP_RB_HICOL  // win32 uses BGR not RGB
#include <stdio.h>



#pragma unmanaged
#pragma warning (disable: 4996 4312)  // disable deprecation warnings




#include "misc.h"
#define WGT2ALLEGRO_NOFUNCTIONS
#include "wgt2allg.h"
#include "sprcache.h"
#define CROOM_NOFUNCTIONS
#include "acroom.h"
#include "acgui.h"

extern int antiAliasFonts;
#define SAVEBUFFERSIZE 5120

extern int mousex, mousey;

extern int sxmult, symult;
extern int dsc_want_hires;
extern bool enable_greyed_out_masks ;
extern bool outlineGuiObjects;
extern color*palette;
extern GameSetupStruct thisgame;
//extern SpriteCache spriteset;
extern GUIMain tempgui;

extern const char*sprsetname ;
extern const char*clibendsig ;
extern const char *old_editor_data_file ;
extern const char *new_editor_data_file ;
extern const char *old_editor_main_game_file;
extern const char *TEMPLATE_LOCK_FILE ;
extern const char *ROOM_TEMPLATE_ID_FILE ;
//extern const int ROOM_TEMPLATE_ID_FILE_SIGNATURE;
extern bool spritesModified ;
extern roomstruct thisroom;
extern bool roomModified ;
extern block drawBuffer ;
extern block undoBuffer;
extern int loaded_room_number;

// stuff for importing old games
extern int numScriptModules;
extern ScriptModule* scModules ;
extern DialogTopic *dialog;
extern char*dlgscript[MAX_DIALOG];
extern GUIMain *guis;
extern ViewStruct272 *oldViews;
extern ViewStruct *newViews;
extern int numNewViews ;



extern "C" {
  extern FILE *clibfopen(char *filnamm, char *fmt);
  extern int csetlib(char *fileName, char *password);
  extern int clibGetNumFiles();
  extern const char *clibGetFileName(int);
  extern const char *clibgetoriginalfilename();
  extern int cfopenpriority;
}
extern bool Scintilla_RegisterClasses(void *hInstance);
extern int Scintilla_LinkLexers();
extern bool ShouldAntiAliasText();


extern void serialize_room_interactions(FILE *);
extern void ThrowManagedException(const char *message);

extern void Cstretch_blit(BITMAP *src, BITMAP *dst, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh);
extern void Cstretch_sprite(BITMAP *dst, BITMAP *src, int x, int y, int w, int h);


extern bool reload_font(int curFont);
extern void drawBlockScaledAt(int hdc, block todraw ,int x, int y, int scaleFactor);

// this is to shut up the linker, it's used by CSRUN.CPP
extern void write_log(char *);


extern bool initialize_native();
extern void update_font_sizes();

extern void copy_global_palette_to_room_palette();

extern bool reset_sprite_file();

enum RoomAreaMask
{
    None,
    Hotspots,
    WalkBehinds,
    WalkableAreas,
    Regions
};


extern block get_bitmap_for_mask(roomstruct *roomptr, RoomAreaMask maskType) ;

extern void validate_mask(block toValidate, const char *name, int maxColour);

 void set_rgb_mask_from_alpha_channel(block image);
 void set_opaque_alpha_channel(block image);

 #define fix_sprite(num) fix_block(spriteset[num])
void fix_block (block todraw);

extern void sort_out_transparency(block toimp, int sprite_import_method, color*itspal, bool useBgSlots, int importedColourDepth) ;

extern void SetNewSprite(int slot, block sprit);

extern block get_sprite (int spnr);