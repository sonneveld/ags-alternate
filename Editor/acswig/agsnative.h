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


block get_bitmap_for_mask(roomstruct *roomptr, RoomAreaMask maskType) ;

void validate_mask(block toValidate, const char *name, int maxColour);

void set_rgb_mask_from_alpha_channel(block image);
void set_opaque_alpha_channel(block image);

#define fix_sprite(num) fix_block(spriteset[num])
void fix_block (block todraw);

void sort_out_transparency(block toimp, int sprite_import_method, color*itspal, bool useBgSlots, int importedColourDepth) ;
void SetNewSprite(int slot, block sprit);
block get_sprite (int spnr);
void drawSprite(int hdc, int x, int y, int spriteNum, bool flipImage);
bool reload_font(int curFont);
void drawBlockScaledAt(int hdc, block todraw ,int x, int y, int scaleFactor);
int multiply_up_coordinate(int coord);
int get_fixed_pixel_size(int coord);
void initialize_sprite(int spnum);
void pre_save_sprite(int spnum);
block get_sprite (int spnr);

bool initialize_native();
void shutdown_native();
// AGS::Types::Game^ load_old_game_dta_file(const char *fileName);
void free_old_game_data();
// AGS::Types::Room^ load_crm_file(UnloadedRoom ^roomToLoad);
// void save_crm_file(Room ^roomToSave);
const char* import_sci_font(const char*fnn,int fslot);
bool reload_font(int curFont);
void drawFontAt (int hdc, int fontnum, int x,int y);
// Dictionary<int, Sprite^>^ load_sprite_dimensions();
// void drawGUI(int hdc, int x,int y, GUI^ gui, int scaleFactor, int selectedControl);
void drawSprite(int hdc, int x,int y, int spriteNum, bool flipImage);
void drawSpriteStretch(int hdc, int x,int y, int width, int height, int spriteNum);
void drawBlockOfColour(int hdc, int x,int y, int width, int height, int colNum);
// void drawViewLoop (int hdc, ViewLoop^ loopToDraw, int x, int y, int size, int cursel);
void SetNewSpriteFromHBitmap(int slot, int hBmp);
// int SetNewSpriteFromBitmap(int slot, Bitmap^ bmp, int spriteImportMethod, bool remapColours, bool useRoomBackgroundColours, bool alphaChannel);
int GetSpriteAsHBitmap(int spriteSlot);
// Bitmap^ getSpriteAsBitmap32bit(int spriteNum, int width, int height);
// Bitmap^ getSpriteAsBitmap(int spriteNum);
// Bitmap^ getBackgroundAsBitmap(Room ^room, int backgroundNumber);
unsigned char* GetRawSpriteData(int spriteSlot);
int find_free_sprite_slot();
int crop_sprite_edges(int numSprites, int *sprites, bool symmetric);
void deleteSprite(int sprslot);
int GetSpriteWidth(int slot);
int GetSpriteHeight(int slot);
int GetRelativeSpriteWidth(int slot);
int GetRelativeSpriteHeight(int slot);
int GetSpriteColorDepth(int slot);
int GetPaletteAsHPalette();
bool DoesSpriteExist(int slot);
int GetMaxSprites();
// int GetCurrentlyLoadedRoomNumber();
int load_template_file(const char *fileName, char **iconDataBuffer, long *iconDataSize, bool isRoomTemplate);
int extract_template_files(const char *templateFileName);
int extract_room_template_files(const char *templateFileName, int newRoomNumber);
void change_sprite_number(int oldNumber, int newNumber);
void update_sprite_resolution(int spriteNum, bool isHighRes);
// void save_game(bool compressSprites);
bool reset_sprite_file();
int GetSpriteResolutionMultiplier(int slot);
// void PaletteUpdated(cli::array<PaletteEntry^>^ newPalette);
// void GameUpdated(Game ^game);
// void UpdateSpriteFlags(SpriteFolder ^folder) ;
void draw_room_background(void *roomptr, int hdc, int x, int y, int bgnum, float scaleFactor, int maskType, int selectedArea, int maskTransparency);
// void ImportBackground(Room ^room, int backgroundNumber, Bitmap ^bmp, bool useExactPalette, bool sharePalette);
// void DeleteBackground(Room ^room, int backgroundNumber);
// void CreateBuffer(int width, int height);
// void RenderBufferToHDC(int hdc);
// void DrawSpriteToBuffer(int sprNum, int x, int y, int scaleFactor);
void draw_line_onto_mask(void *roomptr, int maskType, int x1, int y1, int x2, int y2, int color);
void draw_filled_rect_onto_mask(void *roomptr, int maskType, int x1, int y1, int x2, int y2, int color);
void draw_fill_onto_mask(void *roomptr, int maskType, int x1, int y1, int color);
void copy_walkable_to_regions(void *roomptr);
int get_mask_pixel(void *roomptr, int maskType, int x, int y);
// void import_area_mask(void *roomptr, int maskType, Bitmap ^bmp);
void create_undo_buffer(void *roomptr, int maskType) ;
bool does_undo_buffer_exist();
void clear_undo_buffer() ;
void restore_from_undo_buffer(void *roomptr, int maskType);
// System::String ^load_room_script(System::String ^fileName);
void transform_string(char *text);
extern bool enable_greyed_out_masks;
extern bool spritesModified;

const char* save_sprites(bool compressSprites) ;

void doDrawViewLoop (int hdc, int numFrames, ViewFrame *frames, int x, int y, int size, int cursel);