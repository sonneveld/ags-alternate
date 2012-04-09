#include "allegro.h"
#include "wgt2allg.h"
#include "bmp.h"

// copied over from ac.cpp
#define USE_CLIB

const char *spindexid = "SPRINDEX";
const char *spindexfilename = "sprindex.dat";
int currentcolor;
int vesa_xres, vesa_yres;
block abuf;

#ifdef USE_CLIB
#define CLIB32_REDEFINE_FOPEN
#include "clib32.h"
#endif

  void wsetscreen(block nss)
  {
    if (nss == NULL)
      abuf = alw_screen;
    else
      abuf = nss;
  }

  void wsetrgb(int coll, int r, int g, int b, color * pall)
  {
    pall[coll].r = r;
    pall[coll].g = g;
    pall[coll].b = b;
  }

  void wcolrotate(unsigned char start, unsigned char finish, int dir, color * pall)
  {
    int jj;
    if (dir == 0) {
      // rotate left
      color tempp = pall[start];

      for (jj = start; jj < finish; jj++)
        pall[jj] = pall[jj + 1];

      pall[finish] = tempp;
    }
    else {
      // rotate right
      color tempp = pall[finish];

      for (jj = finish - 1; jj >= start; jj--)
        pall[jj + 1] = pall[jj];

      pall[start] = tempp;
    }
  }

  static block tempbitm;
  block wnewblock(int x1, int y1, int x2, int y2)
  {
    int twid = (x2 - x1) + 1, thit = (y2 - y1) + 1;

    if (twid < 1)
      twid = 1;

    if (thit < 1)
      thit = 1;

    tempbitm = alw_create_bitmap(twid, thit);

    if (tempbitm == NULL)
      return NULL;

    alw_blit(abuf, tempbitm, x1, y1, 0, 0, BMP_W(tempbitm), BMP_H(tempbitm));
    return tempbitm;
  }

  short getshort(FILE * fff)
  {
    short sss;
    fread(&sss, 2, 1, fff);
    return sss;
  }

  void putshort(short num, FILE *fff)
  {
    fwrite(&num, 2, 1, fff);
  }

  void wputblock(int xx, int yy, block bll, int xray)
  {
    if (xray)
      alw_draw_sprite(abuf, bll, xx, yy);
    else
      alw_blit(bll, abuf, 0, 0, xx, yy, BMP_W(bll), BMP_H(bll));
  }

  static const int col_lookups[32] = {
    0x000000, 0x0000A0, 0x00A000, 0x00A0A0, 0xA00000,   // 4
    0xA000A0, 0xA05000, 0xA0A0A0, 0x505050, 0x5050FF, 0x50FF50, 0x50FFFF,       // 11
    0xFF5050, 0xFF50FF, 0xFFFF50, 0xFFFFFF, 0x000000, 0x101010, 0x202020,       // 18
    0x303030, 0x404040, 0x505050, 0x606060, 0x707070, 0x808080, 0x909090,       // 25
    0xA0A0A0, 0xB0B0B0, 0xC0C0C0, 0xD0D0D0, 0xE0E0E0, 0xF0F0F0
  };

  void __my_setcolor(int *ctset, int newcol)
  {
    int wantColDep = alw_bitmap_color_depth(abuf);
    if (wantColDep == 8)
      ctset[0] = newcol;
    else if (newcol & 0x40000000) // already calculated it
      ctset[0] = newcol;
    else if ((newcol >= 32) && (wantColDep > 16)) {
      // true-color
#ifdef SWAP_RB_HICOL
      ctset[0] = alw_makeacol32(alw_getb16(newcol), alw_getg16(newcol), alw_getr16(newcol), 255);
#else
      ctset[0] = alw_makeacol32(alw_getr16(newcol), alw_getg16(newcol), alw_getb16(newcol), 255);
#endif
    }
    else if (newcol >= 32) {
#ifdef SWAP_RB_HICOL
      ctset[0] = alw_makecol16(alw_getb16(newcol), alw_getg16(newcol), alw_getr16(newcol));
#else
      // If it's 15-bit, convert the color
      if (wantColDep == 15)
        ctset[0] = (newcol & 0x001f) | ((newcol >> 1) & 0x7fe0);
      else
        ctset[0] = newcol;
#endif
    } 
    else
    {
      ctset[0] = alw_makecol_depth(wantColDep, col_lookups[newcol] >> 16,
                               (col_lookups[newcol] >> 8) & 0x000ff, col_lookups[newcol] & 0x000ff);

      // in case it's used on an alpha-channel sprite, make sure it's visible
      if (wantColDep > 16)
        ctset[0] |= 0xff000000;
    }

    // if it's 32-bit color, signify that the colour has been calculated
    //if (wantColDep >= 24)
//      ctset[0] |= 0x40000000;
  }

  void wsetcolor(int nval)
  {
    __my_setcolor(&currentcolor, nval);
  }

  int get_col8_lookup(int nval)
  {
    int tmpv;
    __my_setcolor(&tmpv, nval);
    return tmpv;
  }

#ifdef USE_CLIB
#undef fopen
#endif
