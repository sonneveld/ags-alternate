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


// blit other thing to THIS, in this format
void _vtable_blit_to_self(int for_depth, ALW_BITMAP *src, ALW_BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
  switch (for_depth) {
    case 8: _linear_blit8(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 15:_linear_blit16(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 16:_linear_blit16(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 24:_linear_blit24(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 32:_linear_blit32(src, dest, s_x, s_y, d_x, d_y, w, h); break;
  }
}


void _vtable_blit_to_self_forward(int for_depth, ALW_BITMAP *src, ALW_BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
  switch (for_depth) {
    case 8: _linear_blit8(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 15:_linear_blit16(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 16:_linear_blit16(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 24:_linear_blit24(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 32:_linear_blit32(src, dest, s_x, s_y, d_x, d_y, w, h); break;
  }
}

void _vtable_blit_to_self_backward(int for_depth, ALW_BITMAP *src, ALW_BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
  switch (for_depth) {
    case 8: _linear_blit_backward8(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 15:_linear_blit_backward16(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 16:_linear_blit_backward16(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 24:_linear_blit_backward24(src, dest, s_x, s_y, d_x, d_y, w, h); break;
    case 32:_linear_blit_backward32(src, dest, s_x, s_y, d_x, d_y, w, h); break;
  }
}
