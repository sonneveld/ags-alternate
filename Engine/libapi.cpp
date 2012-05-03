

/* api for dll */

#define WGT2ALLEGRO_NOFUNCTIONS
#include "wgt2allg.h"
#include "sprcache.h"
#define CROOM_NOFUNCTIONS
#include "acroom.h"

extern roomstruct thisroom;
extern color palette[256];
extern SpriteCache spriteset;


#define DLLEXPORT extern "C" __declspec(dllexport)

static color *local_palette_ptr;  // actually palette
static GameSetupStruct thisgame;
static char editor_version[1024];
static int antiAliasFonts = 0;
static int sxmult = 1;
static int symult = 1;


DLLEXPORT bool ac_initialize_native()
{
  OutputDebugStringA("initialising\n");

  set_uformat('ASC8');
  install_allegro(SYSTEM_NONE, &errno, atexit);

  local_palette_ptr = thisgame.defpal;
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
  *r = local_palette_ptr[index];
}

DLLEXPORT void ac_set_local_palette_entry(int index, RGB r) {
  local_palette_ptr[index] = r;
}

DLLEXPORT void ac_set_palette_from_local() {
  set_palette(local_palette_ptr);
}

DLLEXPORT void ac_set_editor_version_number(char *version) {
  strcpy(editor_version, version);
}


DLLEXPORT void ac_copy_global_palette_to_room_palette() {
  for(int index=0; index < 0x100; index++) 
	{
		if (thisgame.paluses[index] != 2) {
			thisroom.bpalettes[0][index] =  local_palette_ptr[index];
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
//  DebugBreak();
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
        select_palette(local_palette_ptr);
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


// override for on in ac.cpp
void initialize_sprite (int ee) {
  fix_block(spriteset[ee]);
}

DLLEXPORT BITMAP* ac_get_sprite(int spritenum){
  //DebugBreak();
  OutputDebugStringA("1\n");
  if ( spritenum < 0 )
	  return 0;
  
  OutputDebugStringA("2\n");
	BITMAP* result = spriteset[spritenum];

  OutputDebugStringA("3\n");

	if (result == 0)
		result = spriteset[0];

  OutputDebugStringA("4\n");

	return result;
}


DLLEXPORT void ac_bmp_blit_line(unsigned char *destbuf, BITMAP* bmp, int linenum, int size) {
  memcpy(destbuf, bmp->line[linenum], size);
}