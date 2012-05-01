

/* api for dll */

#define WGT2ALLEGRO_NOFUNCTIONS
#include "wgt2allg.h"
#include "sprcache.h"
#define CROOM_NOFUNCTIONS
#include "acroom.h"

extern roomstruct thisroom;
extern color palette[256];
extern SpriteCache spriteset;

color *palette_ptr;  // actually palette
GameSetupStruct thisgame;


  




extern "C" __declspec(dllexport)  bool initialize_native()
{
  set_uformat('ASC8');
  install_allegro(SYSTEM_NONE, &errno, atexit);

  palette_ptr = thisgame.defpal;
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
  HMODULE v2; // eax@3
  v2 = GetModuleHandleA(0);
  if(!(unsigned __int8)Scintilla_RegisterClasses(v2))
    return false;
#endif

  init_font_renderer();

  return true;
}




