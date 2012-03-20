/* MOUSELIBW32.CPP

  Library of mouse functions for graphics and text mode

  (c) 1994 Chris Jones

  Win32 (allegro) update (c) 1999 Chris Jones

  This is UNPUBLISHED PROPRIETARY SOURCE CODE;
  the contents of this file may not be disclosed to third parties,
  copied or duplicated in any form, in whole or in part, without
  prior express permission from Chris Jones.
*/

#include "bmp.h"

#if !defined(LINUX_VERSION) && !defined(MAC_VERSION)
#include <dos.h>
#include <conio.h>
#include <process.h>
#endif

#include <stdio.h>

#define WGT2ALLEGRO_NOFUNCTIONS
#include "wgt2allg.h"
#if !defined(WINDOWS_VERSION) && !defined(LINUX_VERSION) && !defined(MAC_VERSION)
#error This is the Windows 32-bit version
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define MAXCURSORS 20

void msetgraphpos(int,int);

#include "clib32.h"
#include "ac.h"

char currentcursor = 0;
int mousex = 0;
int mousey = 0;
int hotx = 0;
int hoty = 0;
int disable_mgetgraphpos = 0;
char ignore_bounds = 0;
block mousecurs[MAXCURSORS];

static char *mouselibcopyr = "MouseLib32 (c) 1994, 1998 Chris Jones";

static int aa;
static int boundx1 = 0, boundx2 = 99999, boundy1 = 0, boundy2 = 99999;
static char mouseturnedon = FALSE;
static int numcurso = -1;
static block savebk = NULL;
static IMouseGetPosCallback *callback = NULL;

void msetcallback(IMouseGetPosCallback *gpCallback) {
  callback = gpCallback;
}

void mgraphconfine(int x1, int y1, int x2, int y2)
{
  set_mouse_range(x1, y1, x2, y2);
}

void mgetgraphpos()
{
  poll_mouse();
  if (!disable_mgetgraphpos) {
    mousex = mouse_x;
    mousey = mouse_y;
  }

  if (!ignore_bounds) {

    if (mousex < boundx1) {
      mousex = boundx1;
      msetgraphpos(mousex, mousey);
    }
    if (mousey < boundy1) {
      mousey = boundy1;
      msetgraphpos(mousex, mousey);
    }
    if (mousex > boundx2) {
      mousex = boundx2;
      msetgraphpos(mousex, mousey);
    }
    if (mousey > boundy2) {
      mousey = boundy2;
      msetgraphpos(mousex, mousey);
    }

  }

  if ((callback) && (!disable_mgetgraphpos))
    callback->AdjustPosition(&mousex, &mousey);
}

void msetcursorlimit(int x1, int y1, int x2, int y2)
{
  // like graphconfine, but don't actually pass it to the driver
  // - stops the Windows cursor showing when out of the area
  boundx1 = x1;
  boundy1 = y1;
  boundx2 = x2;
  boundy2 = y2;
}

static void drawCursor() {
  if (alpha_blend_cursor) {
    set_alpha_blender();
    draw_trans_sprite(abuf, mousecurs[currentcursor], mousex, mousey);
  }
  else
    put_sprite_256(mousex, mousey, mousecurs[currentcursor]);
}

static int hotxwas = 0, hotywas = 0;
// graphics mode cursor
void domouse(int str)
{
  /*
     TO USE THIS ROUTINE YOU MUST LOAD A MOUSE CURSOR USING mloadcursor.
     YOU MUST ALSO REMEMBER TO CALL mfreemem AT THE END OF THE PROGRAM.
  */
  int poow = wgetblockwidth(mousecurs[currentcursor]);
  int pooh = wgetblockheight(mousecurs[currentcursor]);
  int smx = mousex - hotxwas, smy = mousey - hotywas;

  mgetgraphpos();
  mousex -= hotx;
  mousey -= hoty;

  if (mousex + poow >= vesa_xres)
    poow = vesa_xres - mousex;

  if (mousey + pooh >= vesa_yres)
    pooh = vesa_yres - mousey;

  wclip(0, 0, vesa_xres - 1, vesa_yres - 1);
  if ((str == 0) & (mouseturnedon == TRUE)) {
    if ((mousex != smx) | (mousey != smy)) {    // the mouse has moved
      wputblock(smx, smy, savebk, 0);
      wfreeblock(savebk);
      savebk = wnewblock(mousex, mousey, mousex + poow, mousey + pooh);
      drawCursor();
    }
  }
  else if ((str == 1) & (mouseturnedon == FALSE)) {
    // the mouse is just being turned on
    savebk = wnewblock(mousex, mousey, mousex + poow, mousey + pooh);
    drawCursor();
    mouseturnedon = TRUE;
  }
  else if ((str == 2) & (mouseturnedon == TRUE)) {    // the mouse is being turned off
    if (savebk != NULL) {
      wputblock(smx, smy, savebk, 0);
      wfreeblock(savebk);
    }

    savebk = NULL;
    mouseturnedon = FALSE;
  }

  mousex += hotx;
  mousey += hoty;
  hotxwas = hotx;
  hotywas = hoty;
}

static int ismouseinbox(int lf, int tp, int rt, int bt)
{
  if ((mousex >= lf) & (mousex <= rt) & (mousey >= tp) & (mousey <= bt))
    return TRUE;
  else
    return FALSE;
}

static void mfreemem()
{
  for (int re = 0; re < numcurso; re++) {
    if (mousecurs[re] != NULL)
      wfreeblock(mousecurs[re]);
  }
}

static void mnewcursor(char cursno)
{
  domouse(2);
  currentcursor = cursno;
  domouse(1);
}


static void mloadwcursor(char *namm)
{
  color dummypal[256];
  if (wloadsprites(&dummypal[0], namm, mousecurs, 0, MAXCURSORS)) {
    //printf("C_Load_wCursor: Error reading mouse cursor file\n"); 
    exit(1);
  }
}

static int butwas = 0;
int mgetbutton()
{
  int toret = NONE;
  poll_mouse();
  int butis = mouse_b;

  if ((butis > 0) & (butwas > 0))
    return NONE;  // don't allow holding button down

  if (butis & 1)
    toret = LEFT;
  else if (butis & 2)
    toret = RIGHT;
  else if (butis & 4)
    toret = MIDDLE;

  butwas = butis;
  return toret;
}

static const int MB_ARRAY[3] = { 1, 2, 4 };
int misbuttondown(int buno)
{
  poll_mouse();
  if (mouse_b & MB_ARRAY[buno])
    return TRUE;
  return FALSE;
}

 void msetgraphpos(int xa, int ya)
{ 
  position_mouse(xa, ya); // xa -= hotx; ya -= hoty;
}

// Graphics mode only. Useful for crosshair.
void msethotspot(int xx, int yy)
{
  hotx = xx;  // mousex -= hotx; mousey -= hoty;
  hoty = yy;  // mousex += hotx; mousey += hoty;
}

// this returns number of buttons (or 0)
int minstalled()
{
  int nbuts;
  if ((nbuts = install_mouse()) < 1)
    return 0;

  mgraphconfine(0, 0, 319, 199);  // use 320x200 co-ord system
  if (nbuts < 2)
    nbuts = 2;

  return nbuts;
}
