//
//  graphics.cpp
//  EngineMac
//
//  Created by Nick Sonneveld on 20/04/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>


#include "allegro.h"
#include "alw_to_allegro.h"


static int _sub_bitmap_id_count = 0;

#define BYTES_PER_PIXEL(bpp)     (((int)(bpp) + 7) / 8)

/* create_sub_bitmap:
 *  Creates a sub bitmap, ie. a bitmap sharing drawing memory with a
 *  pre-existing bitmap, but possibly with different clipping settings.
 *  Usually will be smaller, and positioned at some arbitrary point.
 *
 *  Mark Wodrich is the owner of the brain responsible this hugely useful 
 *  and beautiful function.
 */
BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height)
{
  BITMAP *bitmap;
  int nr_pointers;
  int i;
  
  ASSERT(parent);
  ASSERT((x >= 0) && (y >= 0) && (x < parent->w) && (y < parent->h));
  ASSERT((width > 0) && (height > 0));
  
  if (x+width > parent->w) 
    width = parent->w-x;
  
  if (y+height > parent->h) 
    height = parent->h-y;
  
  /* get memory for structure and line pointers */
  /* (see create_bitmap for the reason we need at least two) */
  nr_pointers = MAX(2, height);
  bitmap = (BITMAP*)_AL_MALLOC(sizeof(BITMAP) + (sizeof(char *) * nr_pointers));
  if (!bitmap)
    return NULL;
  
  acquire_bitmap(parent);
  
  bitmap->w = bitmap->cr = width;
  bitmap->h = bitmap->cb = height;
  bitmap->clip = TRUE;
  bitmap->cl = bitmap->ct = 0;
  bitmap->vtable = parent->vtable;
  bitmap->x_ofs = x + parent->x_ofs;
  bitmap->y_ofs = y + parent->y_ofs;
  bitmap->surf = 0;
  
  /* All bitmaps are created with zero ID's. When a sub-bitmap is created,
   * a unique ID is needed to identify the relationship when blitting from
   * one to the other. This is obtained from the global variable
   * _sub_bitmap_id_count, which provides a sequence of integers (yes I
   * know it will wrap eventually, but not for a long time :-) If the
   * parent already has an ID the sub-bitmap adopts it, otherwise a new
   * ID is given to both the parent and the child.
   */
  if (!(parent->id & BMP_ID_MASK)) {
    parent->id |= _sub_bitmap_id_count;
    _sub_bitmap_id_count = (_sub_bitmap_id_count+1) & BMP_ID_MASK;
  }
  
  bitmap->id = parent->id | BMP_ID_SUB;
  bitmap->id &= ~BMP_ID_LOCKED;
  
  x *= BYTES_PER_PIXEL(bitmap_color_depth(bitmap));
  
  /* setup line pointers: each line points to a line in the parent bitmap */
  for (i=0; i<height; i++) {
    unsigned char *ptr = parent->line[y+i] + x;
    bitmap->line[i] = ptr;
  }
    
  release_bitmap(parent);
  
  return bitmap;
}
