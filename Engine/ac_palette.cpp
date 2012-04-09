#include "ac_palette.h"

#include "allegro.h"

#include "ac.h"
#include "ac_context.h"


void setpal() {
  wsetpalette(0,255,palette);
}

/* *** SCRIPT SYMBOL: [Palette] CyclePalette *** */
void CyclePalette(int strt,int eend) {
  // hi-color game must invalidate screen since the palette changes
  // the effect of the drawing operations
  if (game.color_depth > 1)
    invalidate_screen();

  if ((strt < 0) || (strt > 255) || (eend < 0) || (eend > 255))
    quit("!CyclePalette: start and end must be 0-255");

  if (eend > strt) {
    // forwards
    wcolrotate(strt, eend, 0, palette);
    wsetpalette(strt, eend, palette);
  }
  else {
    // backwards
    wcolrotate(eend, strt, 1, palette);
    wsetpalette(eend, strt, palette);
  }
  
}
/* *** SCRIPT SYMBOL: [Palette] SetPalRGB *** */
void SetPalRGB(int inndx,int rr,int gg,int bb) {
  if (game.color_depth > 1)
    invalidate_screen();

  wsetrgb(inndx,rr,gg,bb,palette);
  wsetpalette(inndx,inndx,palette);
}
/*void scSetPal(color*pptr) {
  wsetpalette(0,255,pptr);
  }
void scGetPal(color*pptr) {
  alw_get_palette(pptr);
  }*/


/* *** SCRIPT SYMBOL: [Palette] UpdatePalette *** */
void UpdatePalette() {
  if (game.color_depth > 1)
    invalidate_screen();

  if (!play.fast_forward)  
    setpal();
}


/* PASTE:
*/
void register_palette_script_functions() {
  scAdd_External_Symbol("CyclePalette",(void *)CyclePalette);
//  scAdd_External_Symbol("GetPalette",(void *)scGetPal);
//  scAdd_External_Symbol("SetPalette",scSetPal);
  scAdd_External_Symbol("SetPalRGB",(void *)SetPalRGB);
  scAdd_External_Symbol("UpdatePalette",(void *)UpdatePalette);
}

