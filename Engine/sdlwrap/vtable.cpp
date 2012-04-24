//
//  vtable.cpp
//  EngineMac
//
//  Created by Nick Sonneveld on 23/04/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>

#include "allegro.h"


extern void _linear_blit8(ALW_BITMAP *src, ALW_BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h);
extern void _linear_blit16(ALW_BITMAP *src, ALW_BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h);
extern void _linear_blit24(ALW_BITMAP *src, ALW_BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h);
extern void _linear_blit32(ALW_BITMAP *src, ALW_BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h);

extern void _linear_blit_backward8(ALW_BITMAP *src, ALW_BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h);
extern void _linear_blit_backward16(ALW_BITMAP *src, ALW_BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h);
extern void _linear_blit_backward24(ALW_BITMAP *src, ALW_BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h);
extern void _linear_blit_backward32(ALW_BITMAP *src, ALW_BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h);

extern void _linear_putpixel8(ALW_BITMAP *dst, int dx, int dy, int color, bool solid=true);
extern void _linear_putpixel15(ALW_BITMAP *dst, int dx, int dy, int color, bool solid=true);
extern void _linear_putpixel16(ALW_BITMAP *dst, int dx, int dy, int color, bool solid=true);
extern void _linear_putpixel24(ALW_BITMAP *dst, int dx, int dy, int color, bool solid=true);
extern void _linear_putpixel32(ALW_BITMAP *dst, int dx, int dy, int color, bool solid=true);

extern int _linear_getpixel8(ALW_BITMAP *src, int sx, int sy);
extern int _linear_getpixel15(ALW_BITMAP *src, int sx, int sy);
extern int _linear_getpixel16(ALW_BITMAP *src, int sx, int sy);
extern int _linear_getpixel24(ALW_BITMAP *src, int sx, int sy);
extern int _linear_getpixel32(ALW_BITMAP *src, int sx, int sy);

extern void _linear_hline8(ALW_BITMAP *dst, int dx1, int dy, int dx2, int color);
extern void _linear_hline15(ALW_BITMAP *dst, int dx1, int dy, int dx2, int color);
extern void _linear_hline16(ALW_BITMAP *dst, int dx1, int dy, int dx2, int color);
extern void _linear_hline24(ALW_BITMAP *dst, int dx1, int dy, int dx2, int color);
extern void _linear_hline32(ALW_BITMAP *dst, int dx1, int dy, int dx2, int color);

extern void _linear_vline8(ALW_BITMAP *dst, int dx1, int dy, int dx2, int color);
extern void _linear_vline15(ALW_BITMAP *dst, int dx1, int dy, int dx2, int color);
extern void _linear_vline16(ALW_BITMAP *dst, int dx1, int dy, int dx2, int color);
extern void _linear_vline24(ALW_BITMAP *dst, int dx1, int dy, int dx2, int color);
extern void _linear_vline32(ALW_BITMAP *dst, int dx1, int dy, int dx2, int color);

void _linear_clear_to_color8(ALW_BITMAP *dst, int color);
void _linear_clear_to_color15(ALW_BITMAP *dst, int color);
void _linear_clear_to_color16(ALW_BITMAP *dst, int color);
void _linear_clear_to_color24(ALW_BITMAP *dst, int color);
void _linear_clear_to_color32(ALW_BITMAP *dst, int color);


void _linear_draw_sprite8(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_v_flip8(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_h_flip8(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_vh_flip8(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_trans_sprite8(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_lit_sprite8(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy, int color);

void _linear_draw_sprite15(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_v_flip15(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_h_flip15(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_vh_flip15(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_trans_sprite15(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_lit_sprite15(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy, int color);

void _linear_draw_sprite16(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_v_flip16(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_h_flip16(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_vh_flip16(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_trans_sprite16(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_lit_sprite16(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy, int color);

void _linear_draw_sprite24(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_v_flip24(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_h_flip24(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_vh_flip24(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_trans_sprite24(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_lit_sprite24(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy, int color);

void _linear_draw_sprite32(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_v_flip32(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_h_flip32(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_vh_flip32(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_trans_sprite32(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_lit_sprite32(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy, int color);

// blit other thing to THIS, in this format
void _vtable_blit_to_self(int for_depth, ALW_BITMAP *src, ALW_BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
  switch (for_depth) {
    case 8: _linear_blit8(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 15:
    case 16:_linear_blit16(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 24:_linear_blit24(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 32:_linear_blit32(src, dest, s_x, s_y, d_x, d_y, w, h); break;
  }
}


void _vtable_blit_to_self_forward(int for_depth, ALW_BITMAP *src, ALW_BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
  switch (for_depth) {
    case 8: _linear_blit8(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 15:
    case 16:_linear_blit16(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 24:_linear_blit24(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 32:_linear_blit32(src, dest, s_x, s_y, d_x, d_y, w, h); break;
  }
}

void _vtable_blit_to_self_backward(int for_depth, ALW_BITMAP *src, ALW_BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
  switch (for_depth) {
    case 8: _linear_blit_backward8(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 15:
    case 16:_linear_blit_backward16(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 24:_linear_blit_backward24(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 32:_linear_blit_backward32(src, dest, s_x, s_y, d_x, d_y, w, h); break;
  }
}


void _vtable_putpixel_ex(ALW_BITMAP *dst, int dx, int dy, int color, bool solid) {
  switch (alw_bitmap_color_depth(dst)){
    case 8: _linear_putpixel8(dst, dx, dy, color, solid); break;
    case 15:_linear_putpixel15(dst, dx, dy, color, solid); break;
    case 16:_linear_putpixel16(dst, dx, dy, color, solid); break;
    case 24:_linear_putpixel24(dst, dx, dy, color, solid); break;
    case 32:_linear_putpixel32(dst, dx, dy, color, solid); break;
  }
}

void _vtable_putpixel(ALW_BITMAP *dst, int dx, int dy, int color) {
  _vtable_putpixel_ex(dst, dx, dy, color, true);
}


int _vtable_getpixel(ALW_BITMAP *src, int sx, int sy) {
  switch (alw_bitmap_color_depth(src)){
    case 8: return _linear_getpixel8(src, sx, sy);
    case 15: return _linear_getpixel15(src, sx, sy);
    case 16: return _linear_getpixel16(src, sx, sy);
    case 24: return _linear_getpixel24(src, sx, sy);
    case 32: return _linear_getpixel32(src, sx, sy);
  }
  return -1;
}

void _vtable_hline(ALW_BITMAP *dst, int dx1, int dy, int dx2, int color) {
  switch (alw_bitmap_color_depth(dst)){
    case 8: _linear_hline8(dst,dx1,dy,dx2,color); break;
    case 15: _linear_hline15(dst,dx1,dy,dx2,color); break;
    case 16: _linear_hline16(dst,dx1,dy,dx2,color); break;
    case 24: _linear_hline24(dst,dx1,dy,dx2,color); break;
    case 32: _linear_hline32(dst,dx1,dy,dx2,color); break;
  }
}

void _vtable_hfill(ALW_BITMAP *dst, int dx1, int dy, int dx2, int color) {
  _vtable_hline(dst, dx1, dy, dx2, color);
}
void _vtable_vline(ALW_BITMAP *dst, int dx, int dy1, int dy2, int color){
  switch (alw_bitmap_color_depth(dst)){
    case 8: _linear_vline8(dst,dx,dy1,dy2,color); break;
    case 15: _linear_vline15(dst,dx,dy1,dy2,color); break;
    case 16: _linear_vline16(dst,dx,dy1,dy2,color); break;
    case 24: _linear_vline24(dst,dx,dy1,dy2,color); break;
    case 32: _linear_vline32(dst,dx,dy1,dy2,color); break;
  }
}


void _vtable_clear_to_color(ALW_BITMAP *bitmap, int color) 
{
  switch (alw_bitmap_color_depth(bitmap)) {
    case 8:
      _linear_clear_to_color8(bitmap, color);
      break;
    case 15:
    case 16:
      _linear_clear_to_color16(bitmap, color);
      break;
    case 24:
      _linear_clear_to_color24(bitmap, color);
      break;
    case 32:
      _linear_clear_to_color32(bitmap, color);
      break;
  }
}

void _vtable_draw_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
  switch (alw_bitmap_color_depth(sprite)) {
    case 8:
      _linear_draw_sprite8(bmp, sprite, x, y);
      break;
    case 15:
      _linear_draw_sprite15(bmp, sprite, x, y);
      break;
    case 16:
      _linear_draw_sprite16(bmp, sprite, x, y);
      break;
    case 24:
      _linear_draw_sprite24(bmp, sprite, x, y);
      break;
    case 32:
      _linear_draw_sprite32(bmp, sprite, x, y);
      break;
  }
}


void _vtable_draw_lit_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y, int color) { 
  // we assume that bmp and sprite have same color depth
  switch (alw_bitmap_color_depth(sprite)) {
    case 8:
      _linear_draw_lit_sprite8(bmp, sprite, x, y, color);
      break;
    case 15:
      _linear_draw_lit_sprite15(bmp, sprite, x, y, color);
      break;
    case 16:
      _linear_draw_lit_sprite16(bmp, sprite, x, y, color);
      break;
    case 24:
      _linear_draw_lit_sprite24(bmp, sprite, x, y, color);
      break;
    case 32:
      _linear_draw_lit_sprite32(bmp, sprite, x, y, color);
      break;
  }
}
void _vtable_draw_trans_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
  switch (alw_bitmap_color_depth(sprite)) {
    case 8:
      _linear_draw_trans_sprite8(bmp, sprite, x, y);
      break;
    case 15:
      _linear_draw_trans_sprite15(bmp, sprite, x, y);
      break;
    case 16:
      _linear_draw_trans_sprite16(bmp, sprite, x, y);
      break;
    case 24:
      _linear_draw_trans_sprite24(bmp, sprite, x, y);
      break;
    case 32:
      _linear_draw_trans_sprite32(bmp, sprite, x, y);
      break;
  }
}
void _vtable_draw_sprite_h_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
  // we assume that bmp and sprite have same color depth
  switch (alw_bitmap_color_depth(sprite)) {
    case 8:
      _linear_draw_sprite_h_flip8(bmp, sprite, x, y);
      break;
    case 15:
      _linear_draw_sprite_h_flip15(bmp, sprite, x, y);
      break;
    case 16:
      _linear_draw_sprite_h_flip16(bmp, sprite, x, y);
      break;
    case 24:
      _linear_draw_sprite_h_flip24(bmp, sprite, x, y);
      break;
    case 32:
      _linear_draw_sprite_h_flip32(bmp, sprite, x, y);
      break;
  }
}
void _vtable_draw_sprite_v_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
  // we assume that bmp and sprite have same color depth
  switch (alw_bitmap_color_depth(sprite)) {
    case 8:
      _linear_draw_sprite_v_flip8(bmp, sprite, x, y);
      break;
    case 15:
      _linear_draw_sprite_v_flip15(bmp, sprite, x, y);
      break;
    case 16:
      _linear_draw_sprite_v_flip16(bmp, sprite, x, y);
      break;
    case 24:
      _linear_draw_sprite_v_flip24(bmp, sprite, x, y);
      break;
    case 32:
      _linear_draw_sprite_v_flip32(bmp, sprite, x, y);
      break;
  }
}

void _vtable_draw_sprite_vh_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
  // we assume that bmp and sprite have same color depth
  switch (alw_bitmap_color_depth(sprite)) {
    case 8:
      _linear_draw_sprite_vh_flip8(bmp, sprite, x, y);
      break;
    case 15:
      _linear_draw_sprite_vh_flip15(bmp, sprite, x, y);
      break;
    case 16:
      _linear_draw_sprite_vh_flip16(bmp, sprite, x, y);
      break;
    case 24:
      _linear_draw_sprite_vh_flip24(bmp, sprite, x, y);
      break;
    case 32:
      _linear_draw_sprite_vh_flip32(bmp, sprite, x, y);
      break;
  }
}

