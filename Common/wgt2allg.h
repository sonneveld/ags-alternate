/*
** WGT -> Allegro portability interface
** Copyright (C) 1998, Chris Jones
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE;
** the contents of this file may not be disclosed to third parties,
** copied or duplicated in any form, in whole or in part, without
** prior express permission from Chris Jones.
**
** wsavesprites and wloadsprites are hi-color compliant
*/
#define _WGT45_

#ifndef __WGT4_H
#define __WGT4_H

#ifdef _MSC_VER
#ifndef WINDOWS_VERSION
#define WINDOWS_VERSION
#endif
#endif

#include <stdio.h>
#include <string.h>

#include "allegro.h"

#if defined(LINUX_VERSION) || defined(MAC_VERSION)
//#include "djcompat.h"
#else
#include <dos.h>
#include <io.h>
#endif


#include <stdarg.h>

#include "bigend.h"

typedef ALW_BITMAP *block;

#define color ALW_RGB
#define TEXTFG    0
#define TEXTBG    1

#define fpos_t unsigned long
#ifdef __cplusplus
extern "C"
{
#endif

extern const char *spindexid;
extern const char *spindexfilename ;

extern int currentcolor;
extern block abuf;
extern int vesa_xres, vesa_yres;

extern short getshort(FILE * fff);
extern void putshort(short num, FILE *fff);

extern void wsetscreen(block);
extern void wsetrgb(int, int, int, int, color *);
extern void wcolrotate(unsigned char, unsigned char, int, color *);
extern block wnewblock(int, int, int, int);
extern void wputblock(int, int, block, int);
extern void wsetcolor(int);
extern void __my_setcolor(int *ctset, int newcol);
extern int get_col8_lookup(int nval);

#ifdef __cplusplus
}
#endif

#define wallocblock(wii,hii)  alw_create_bitmap(wii,hii)
#define wbar(x1, y1, x2, y2)  alw_rectfill(abuf, x1, y1, x2, y2, currentcolor)

#define wfreeblock(bll)                 alw_destroy_bitmap(bll)
#define wgetblockheight(bll)            BMP_H(bll)
#define wgetblockwidth(bll)             BMP_W(bll)
#define whline(x1, x2, yy)              alw_hline(abuf, x1, yy, x2, currentcolor)
#define wline(x1, y1, x2, y2)           alw_line(abuf,x1,y1,x2,y2,currentcolor)
#define wputpixel(x1, y1)               alw_putpixel(abuf, x1, y1, currentcolor)
#define wreadpalette(from, to, dd)      alw_get_palette_range(dd, from, to)
#define wrectangle(x1, y1, x2, y2)      alw_rect(abuf, x1, y1, x2, y2, currentcolor)
#define wsetpalette(from, to, pall)     alw_set_palette_range(pall, from, to, 0)

struct IMouseGetPosCallback {
public:
  virtual void AdjustPosition(int *x, int *y) = 0;
};

// Font/text rendering
extern void init_font_renderer();
extern void shutdown_font_renderer();
extern bool wloadfont_size(int fontNumber, int fontSize);
extern void wfreefont(int fontNumber);
extern void wouttextxy(int, int, int fontNumber, const char *);
extern void wgtprintf(int, int, int fontNumber, char *, ...);
extern int wgettextheight(const char *, int fontNumber);
extern int wgettextwidth(const char *, int fontNumber);
extern void wtextcolor(int);
extern int textcol;
extern int wtext_multiply;
extern void ensure_text_valid_for_font(char *text, int fontnum);
extern void adjust_y_coordinate_for_text(int* ypos, int fontnum);
extern void wtexttransparent(int);
/* Temp code to test for memory leaks
#define destroy_bitmap my_destroy_bitmap
#undef wfreeblock
#define wfreeblock my_destroy_bitmap
extern void my_destroy_bitmap(BITMAP *bitmap);
*/
#endif
