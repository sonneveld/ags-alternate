

/* api for dll */
// this replaces ac.cpp for the library version.

#include <stdio.h>
extern "C" {
 extern int  csetlib(char*,char*);
 extern FILE*clibfopen(char*,char*);
 extern int  cfopenpriority;
 }

#include "wgt2allg.h"
#include "sprcache.h"
#include "misc.h"
#include "acroom.h"
#include "acgui.h"

RGB *palette;
GameSetupStruct thisgame;
SpriteCache spriteset(1);
roomstruct thisroom;

bool outlineGuiObjects;
int mousex;
int mousey;

static char editor_version[1024];
static int antiAliasFonts = 0;
static int sxmult = 1;
static int symult = 1;
static int dsc_want_hires = 0;


// VT Fixups
// ============================================================================

// code that jumps back to managed code.  We might be able to get away with
// passing managed callbacks back to the dll?

void quit(char *message) {
  //throw new AGS.Types.AGSEditorException(message);
  exit(0);
}


// AC Shared
// ============================================================================

// this code was in ac.cpp as well, we link our own versions of these functions.

bool ShouldAntiAliasText() {
  return antiAliasFonts != 0;
}

int multiply_up_coordinate(int x) {
  return x * sxmult;
}

static void draw_stretched_sprite(int x, int y, BITMAP *bmp, int w, int h)
{
  int pixelsize;
  if ( bitmap_color_depth(bmp) == 15 )
    pixelsize = 2;
  else
    pixelsize = bitmap_color_depth(bmp) / 8;
	
  if ( pixelsize == thisgame.color_depth )
  {
    stretch_sprite(abuf, bmp, x, y, w, h);
  }
  else
  {
    BITMAP *tmpbmp = create_bitmap_ex(8 * thisgame.color_depth, bmp->w, bmp->h);
    blit(bmp, tmpbmp, 0, 0, 0, 0, bmp->w, bmp->h);  // convert to color depth
	// map mask colors to new colordepth's mask
    for (int bmpx = 0; bmpx < bmp->w; ++bmpx )
    {
      for (int bmpy = 0; bmpy < bmp->h; ++bmpy )
      {
        if ( getpixel(bmp, bmpx, bmpy) == bitmap_mask_color(bmp) )
          putpixel(tmpbmp, bmpx, bmpy, bitmap_mask_color(tmpbmp));
      }
    }
    stretch_sprite(abuf, tmpbmp, x, y, w, h);
    destroy_bitmap(tmpbmp);
  }
}

void draw_sprite_compensate(int sprnum, int x, int y, int xray) {

  BITMAP *sprbmp;
  int sprnum_loaded = sprnum;
  if ( sprnum >= 0 )
  {
    if ( !spriteset[sprnum] )
      sprnum_loaded = 0;
    sprbmp = spriteset[sprnum_loaded];
  }
  else
  {
    sprbmp = 0;
  }
  
  BITMAP *destbmp = sprbmp;
  bool destbmp_alloced = false;
  
  int pixelsize;
  if ( bitmap_color_depth(sprbmp) == 15 )
    pixelsize = 2;
  else
    pixelsize = bitmap_color_depth(sprbmp) / 8;
  if ( (pixelsize > 1) && (8 * thisgame.color_depth == 8) )
  {
    destbmp = create_bitmap_ex(8, sprbmp->w, sprbmp->h);
    destbmp_alloced = true;
    clear_to_color(destbmp, bitmap_mask_color(destbmp));

	  for (int spr_x = 0; spr_x < sprbmp->w; ++spr_x )
        for (int spr_y = 0; spr_y < sprbmp->h; ++spr_y )
        {
          int pixelin = getpixel(sprbmp, spr_x, spr_y);
          if ( pixelin != bitmap_mask_color(sprbmp)) 
          {
            int pixelout = makecol8(getr16(pixelin), getg16(pixelin), getb16(pixelin));
            putpixel(destbmp, spr_x, spr_y, pixelout);
          }
        }

  }
  
  int dest_w = destbmp->w;
  int dest_h = destbmp->h;
  if ( thisgame.spriteflags[sprnum] & 1 )
  {
    if ( !dsc_want_hires )
    {
      dest_w /= 2;
      dest_h /= 2;
    }
  }
  else
  {
    if ( dsc_want_hires )
    {
      dest_w *= 2;
      dest_h *= 2;
    }
  }
  draw_stretched_sprite(x, y, destbmp, dest_w, dest_h);
  
  if ( destbmp_alloced )
    destroy_bitmap(destbmp);
}

int get_adjusted_spritewidth(int spritenum) {
  if ( spritenum < 0 ) 	return 0;
	
  int actualspritenum = spritenum;
  if (spriteset[spritenum] == 0)
    actualspritenum = 0;
  BITMAP *bmp = spriteset[actualspritenum];
  if ( bmp  == 0) return 0;
	
    int width = bmp->w;
    if ( thisgame.spriteflags[spritenum] & 1 )
    {
      if ( sxmult == 1 )
        return width / 2;
    }
    else
    {
      if ( sxmult == 2 )
        return width * 2;
    }

  return width;
}

int get_adjusted_spriteheight(int spritenum){
  if ( spritenum < 0 ) 	return 0;
	
  int actualspritenum = spritenum;
  if (spriteset[spritenum] == 0)
    actualspritenum = 0;
  BITMAP *bmp = spriteset[actualspritenum];
  if ( bmp  == 0) return 0;
	
    int height = bmp->h;
    if ( thisgame.spriteflags[spritenum] & 1 )
    {
      if ( symult == 1 )
        return height / 2;
    }
    else
    {
      if ( symult == 2 )
        return height * 2;
    }

  return height;
}

int get_fixed_pixel_size(int x) {
  return multiply_up_coordinate(x);
}

void write_log(char *msg) {
   // DO NOTHING
}

static void fix_block(BITMAP *bmp);

void initialize_sprite (int spritenum) {
  fix_block(spriteset[spritenum]);
}

void pre_save_sprite(int spritenum) {
   initialize_sprite(spritenum);
}

long getlong(FILE * iii)
{
  long tmm;
  fread(&tmm, 4, 1, iii);
  return tmm;
}

#define SCRIPT_CONFIG_VERSION 1
void load_script_configuration(FILE * iii)
{
  int aa;
  if (getlong(iii) != SCRIPT_CONFIG_VERSION)
    quit("ScriptEdit: invliad config version");

  int numvarnames = getlong(iii);
  for (aa = 0; aa < numvarnames; aa++) {
    int lenoft = getc(iii);
    fseek(iii, lenoft, SEEK_CUR);
  }
}


void load_graphical_scripts(FILE * iii, roomstruct * rst)
{
  for (;;) {
    long ct;
    fread(&ct, 4, 1, iii);
    if ((ct == -1) | (feof(iii) != 0))
      break;

    long lee;
    fread(&lee, 4, 1, iii);

    fseek(iii, lee, SEEK_CUR);
  }
}

extern "C" {
extern PACKFILE *__old_pack_fopen(const char *, const char *);
}
PACKFILE *pack_fopen(const char *filnam1, const char *modd1) {
  return __old_pack_fopen(filnam1, modd1);// using original!
}

void GUIInv::Draw() {
  wsetcolor(15);
  wrectangle(x, y, x+wid, y+hit);
}

void NewInteractionCommand::remove() {
  // DO NOTHING
}


// DLL Exports
// ============================================================================

#define DLLEXPORT extern "C" __declspec(dllexport)

DLLEXPORT bool ac_initialize_native()
{
  OutputDebugStringA("initialising\n");

  set_uformat('ASC8');
  install_allegro(SYSTEM_NONE, &errno, atexit);

  palette = thisgame.defpal;
  thisgame.color_depth = 2;
  abuf = create_bitmap_ex(32, 10, 10);

  thisgame.numfonts = 0;
  wloadfont_size(0, 0);
  thisgame.fontflags[thisgame.numfonts] = 0;
  thisgame.fontoutline[thisgame.numfonts] = -1;
  thisgame.numfonts += 1;

  spriteset.reset();
  if (spriteset.initFile("acsprset.spr"))
    return false;
  spriteset.maxCacheSize = 100000000;

  // we don't need to register scintilla window class
#if 0
  HMODULE module; 
  module = GetModuleHandleA(0);
  if(!Scintilla_RegisterClasses(module))
    return false;
#endif

  init_font_renderer();

  return true;
}


DLLEXPORT void ac_get_local_palette_entry(int index, RGB *r) {
  *r = palette[index];
}

DLLEXPORT void ac_set_local_palette_entry(int index, RGB r) {
  palette[index] = r;
}

DLLEXPORT void ac_set_palette_from_local() {
  set_palette(palette);
}

DLLEXPORT void ac_set_editor_version_number(char *version) {
  strcpy(editor_version, version);
}


DLLEXPORT void ac_copy_global_palette_to_room_palette() {
  for(int index=0; index < 0x100; index++) 
	{
		if (thisgame.paluses[index] != 2) {
			thisroom.bpalettes[0][index] =  palette[index];
		}
	}
}


static char reload_font(int fontnum)
{
  int fontsize; 

  wfreefont(fontnum);
  fontsize = thisgame.fontflags[fontnum] & 0x3F;
  if ( thisgame.options[OPT_NOSCALEFNT] )
  {
    if ( thisgame.default_resolution <= 2 )
      return wloadfont_size(fontnum, (thisgame.fontflags[fontnum] & 0x3F) / 2);
  }
  else
  {
    if ( thisgame.default_resolution >= 3 )
      fontsize *= 2;
  }
  return wloadfont_size(fontnum, fontsize);
}

static char  update_font_sizes()
{
  signed int result; // eax@1
  int i; // esi@5
  
  result = 1;
  wtext_multiply = 1;
  if ( !thisgame.options[OPT_NOSCALEFNT] && thisgame.default_resolution >= 3 )
  {
    result = 2;
    wtext_multiply = 2;
  }
  if ( wtext_multiply != result )
  {
    for ( i = 0; i < thisgame.numfonts; ++i )
      result = reload_font(i) & 0xFF;
  }
  if ( thisgame.default_resolution < 3 )
  {
    sxmult = 1;
    symult = 1;
  }
  else
  {
    sxmult = 2;
    symult = 2;
  }
  return result;
}

DLLEXPORT void ac_thisgame_init_graphics(int colourdepth, int resolution, bool fonts_for_hi_res, bool anti_alias_fonts) {
  thisgame.color_depth = colourdepth;
	thisgame.default_resolution = resolution;
	thisgame.options[OPT_NOSCALEFNT] = fonts_for_hi_res;
	thisgame.options[OPT_ANTIALIASFONTS] = anti_alias_fonts;
	antiAliasFonts = anti_alias_fonts;
	update_font_sizes();
	
	destroy_bitmap(abuf);
	abuf = create_bitmap_ex(thisgame.color_depth*8, 32, 32);
}
DLLEXPORT void ac_thisgame_set_paluse(int index, int paluse) {
  thisgame.paluses[index] = paluse;
}
DLLEXPORT void ac_thisgame_clear_fonts() {
  thisgame.numfonts = 0;
}
DLLEXPORT void ac_thisgame_add_font(int fontnum, int pointsize) {
  thisgame.numfonts += 1;
	thisgame.fontflags[fontnum] &= 0xc0;
	thisgame.fontflags[fontnum] |= pointsize & 0xFF;
	reload_font(fontnum);
}
DLLEXPORT void ac_thisgame_set_sprite_flags(int spriteNum, int flags){ 
  thisgame.spriteflags[spriteNum] = flags;
}


DLLEXPORT int ac_reset_sprite_file()
{
  spriteset.reset();
  if ( spriteset.initFile("acsprset.spr") )
	  return 0;

	spriteset.maxCacheSize = 100000000;
	return 1;
}


DLLEXPORT int ac_bitmap_color_depth(BITMAP* bmp){   return bitmap_color_depth(bmp); }
DLLEXPORT int ac_bitmap_width(BITMAP* bmp) { return bmp->w; };
DLLEXPORT int ac_bitmap_height(BITMAP* bmp) { return bmp->h; };
DLLEXPORT void ac_destroy_bitmap(BITMAP* bmp) {   destroy_bitmap(bmp); }
DLLEXPORT int ac_thisgame_get_sprite_flags(int spriteNum) {   return thisgame.spriteflags[spriteNum]; }

static void fix_block(BITMAP *bmp) {

  if ( !bmp )
    return;

  if ( bmp->w <= 0 )
    return;

  if ( bitmap_color_depth(bmp) == 16 )
  {

    for (int bmp16_x =0; bmp16_x < bmp->w; ++bmp16_x)
      for ( int bmp16_y = 0; bmp16_y < bmp->h; ++bmp16_y )
      {
        int pxintptr = bmp_read_line(bmp, bmp16_y) + 2*bmp16_x;
        int bmp16_pixel = *(  (uint16_t*)pxintptr  );
        bmp_unwrite_line(bmp);

        pxintptr = bmp_write_line(bmp, bmp16_y) + 2*bmp16_x;
        *( (uint16_t*)pxintptr  ) = 
          getb16(bmp16_pixel) << _rgb_r_shift_16 |
          getr16(bmp16_pixel) << _rgb_g_shift_16 |
          getg16(bmp16_pixel) << _rgb_b_shift_16;
        bmp_unwrite_line(bmp);
      }
  }
  else if ( bitmap_color_depth(bmp) == 32 )
  {
    for (int bmp32_x =0; bmp32_x < bmp->w; ++bmp32_x)
      for ( int bmp32_y = 0; bmp32_y < bmp->h; ++bmp32_y )
      {
        int pxintptr =  bmp_read_line(bmp, bmp32_y) + 4*bmp32_x;

        int bmp32_pixel = *(  (uint32_t*)pxintptr );
        bmp_unwrite_line(bmp);

        pxintptr = bmp_write_line(bmp, bmp32_y) + 4*bmp32_x;

        *(  (uint32_t*)pxintptr  ) = 
          geta32(bmp32_pixel) << _rgb_a_shift_32 |
          getr32(bmp32_pixel) << _rgb_b_shift_32 |
          getg32(bmp32_pixel) << _rgb_g_shift_32 |
          getb32(bmp32_pixel) << _rgb_r_shift_32;
        bmp_unwrite_line(bmp);
      }
  }

}

DLLEXPORT BITMAP* ac_get_block_as_32bit(BITMAP* todraw, int width, int height) 
{
    BITMAP* todrawConverted = create_bitmap_ex(32, todraw->w, todraw->h);
    if (bitmap_color_depth(todraw) == 8)
    {
        select_palette(palette);
    }
    blit(todraw, todrawConverted, 0, 0, 0, 0, todraw->w, todraw->h);
    if (bitmap_color_depth(todraw) == 8)
    {
        unselect_palette();
    }
    if ((width != todraw->w) || (height != todraw->h))
    {
        BITMAP* stretchedbmp = create_bitmap_ex(32, width, height);
        stretch_blit(todrawConverted, stretchedbmp, 0, 0, todraw->w, todraw->h, 0, 0, width, height);
        destroy_bitmap(todrawConverted);
        todrawConverted = stretchedbmp;
    }
    fix_block(todrawConverted);

    return todrawConverted;
}




DLLEXPORT BITMAP* ac_get_sprite(int spritenum){
  if ( spritenum < 0 )
	  return 0;
  
	BITMAP* result = spriteset[spritenum];
	if (result == 0)
		result = spriteset[0];

	return result;
}


DLLEXPORT void ac_bmp_blit_line(unsigned char *destbuf, BITMAP* bmp, int linenum, int size) {
  memcpy(destbuf, bmp->line[linenum], size);
}