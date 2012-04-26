/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Graphics routines: palette fading, circles, etc.
 *
 *      By Shawn Hargreaves.
 *
 *      Optimised line drawer by Michael Bukin.
 *
 *      Bresenham arc routine by Romano Signorelli.
 *
 *      Cohen-Sutherland line clipping by Jon Rafkind.
 *
 *      See readme.txt for copyright information.
 */


#include <math.h>
#include "allegro.h"
#include "alw_to_allegro.h"

extern ALW_PALETTE _current_palette; 
extern int _color_depth;
extern  int _rgb_scale_5[32];
extern  int _rgb_scale_6[32];

int _current_palette_changed = 0xFFFFFFFF;

int _palette_color8[256];               /* palette -> pixel mapping */
int _palette_color15[256];
int _palette_color16[256];
int _palette_color24[256];
int _palette_color32[256];

int *palette_color = _palette_color8; 

/* set_palette_range:
 *  Sets a part of the color palette.
 */
void set_palette_range(AL_CONST PALETTE p, int from, int to, int vsync)
{
   int c;

   ASSERT(from >= 0 && from < PAL_SIZE);
   ASSERT(to >= 0 && to < PAL_SIZE);

   for (c=from; c<=to; c++) {
      _current_palette[c] = p[c];

      if (_color_depth != 8)
	 palette_color[c] = makecol(_rgb_scale_6[p[c].r], _rgb_scale_6[p[c].g], _rgb_scale_6[p[c].b]);
   }

   _current_palette_changed = 0xFFFFFFFF & ~(1<<(_color_depth-1));

  alw_sys_set_palette_range(p, from, to, vsync);
  
#if 0
   if (gfx_driver) {
      if ((screen->vtable->color_depth == 8) && (!_dispsw_status))
	 gfx_driver->set_palette(p, from, to, vsync);
   }
   else if ((system_driver) && (system_driver->set_palette_range))
      system_driver->set_palette_range(p, from, to, vsync);
#endif
}

/* set_palette:
 *  Sets the entire color palette.
 */
void set_palette(AL_CONST PALETTE p)
{
  set_palette_range(p, 0, PAL_SIZE-1, TRUE);
}


/* previous palette, so the image loaders can restore it when they are done */
int _got_prev_current_palette = FALSE;
PALETTE _prev_current_palette;
static int prev_palette_color[PAL_SIZE];



/* select_palette:
 *  Sets the aspects of the palette tables that are used for converting
 *  between different image formats, without altering the display settings.
 *  The previous settings are copied onto a one-deep stack, from where they
 *  can be restored by calling unselect_palette().
 */
void select_palette(AL_CONST PALETTE p)
{
   int c;

   for (c=0; c<PAL_SIZE; c++) {
      _prev_current_palette[c] = _current_palette[c];
      _current_palette[c] = p[c];
   }

   if (_color_depth != 8) {
      for (c=0; c<PAL_SIZE; c++) {
	 prev_palette_color[c] = palette_color[c];
	 palette_color[c] = makecol(_rgb_scale_6[p[c].r], _rgb_scale_6[p[c].g], _rgb_scale_6[p[c].b]);
      }
   }

   _got_prev_current_palette = TRUE;

   _current_palette_changed = 0xFFFFFFFF & ~(1<<(_color_depth-1));
}



/* unselect_palette:
 *  Restores palette settings from before the last call to select_palette().
 */
void unselect_palette(void)
{
   int c;

   for (c=0; c<PAL_SIZE; c++)
      _current_palette[c] = _prev_current_palette[c];

   if (_color_depth != 8) {
      for (c=0; c<PAL_SIZE; c++)
	 palette_color[c] = prev_palette_color[c];
   }

   ASSERT(_got_prev_current_palette == TRUE);
   _got_prev_current_palette = FALSE;

   _current_palette_changed = 0xFFFFFFFF & ~(1<<(_color_depth-1));
}



/* _palette_expansion_table:
 *  Creates a lookup table for expanding 256->truecolor.
 */
int *_palette_expansion_table(int bpp)
{
   int *table;
   int c;

   switch (bpp) {
      case 15: table = _palette_color15; break;
      case 16: table = _palette_color16; break;
      case 24: table = _palette_color24; break;
      case 32: table = _palette_color32; break;
      default: ASSERT(FALSE); return NULL;
   }

   if (_current_palette_changed & (1<<(bpp-1))) {
      for (c=0; c<PAL_SIZE; c++) {
	 table[c] = makecol_depth(bpp,
				  _rgb_scale_6[_current_palette[c].r], 
				  _rgb_scale_6[_current_palette[c].g], 
				  _rgb_scale_6[_current_palette[c].b]);
      }

      _current_palette_changed &= ~(1<<(bpp-1));
   } 

   return table;
}


/* generate_332_palette:
 *  Used when loading a truecolor image into an 8 bit bitmap, to generate
 *  a 3.3.2 RGB palette.
 */
void generate_332_palette(PALETTE pal)
{
   int c;

   for (c=0; c<PAL_SIZE; c++) {
      pal[c].r = ((c>>5)&7) * 63/7;
      pal[c].g = ((c>>2)&7) * 63/7;
      pal[c].b = (c&3) * 63/3;
   }

   pal[0].r = 63;
   pal[0].g = 0;
   pal[0].b = 63;

   pal[254].r = pal[254].g = pal[254].b = 0;
}


/* get_palette_range:
 *  Retrieves a part of the color palette.
 */
void get_palette_range(PALETTE p, int from, int to)
{
   int c;

   ASSERT(from >= 0 && from < PAL_SIZE);
   ASSERT(to >= 0 && to < PAL_SIZE);

#if 0
  // writes to _current_palette
   if ((system_driver) && (system_driver->read_hardware_palette))
      system_driver->read_hardware_palette();
#endif
  
   for (c=from; c<=to; c++)
      p[c] = _current_palette[c];
}

/* get_palette:
 *  Retrieves the entire color palette.
 */
void get_palette(PALETTE p)
{
  get_palette_range(p, 0, PAL_SIZE-1);
}


/* fade_interpolate: 
 *  Calculates a palette part way between source and dest, returning it
 *  in output. The pos indicates how far between the two extremes it should
 *  be: 0 = return source, 64 = return dest, 32 = return exactly half way.
 *  Only affects colors between from and to (inclusive).
 */
void fade_interpolate(AL_CONST PALETTE source, AL_CONST PALETTE dest, PALETTE output, int pos, int from, int to)
{
   int c;

   ASSERT(pos >= 0 && pos <= 64);
   ASSERT(from >= 0 && from < PAL_SIZE);
   ASSERT(to >= 0 && to < PAL_SIZE);

   for (c=from; c<=to; c++) { 
      output[c].r = ((int)source[c].r * (63-pos) + (int)dest[c].r * pos) / 64;
      output[c].g = ((int)source[c].g * (63-pos) + (int)dest[c].g * pos) / 64;
      output[c].b = ((int)source[c].b * (63-pos) + (int)dest[c].b * pos) / 64;
   }
}


/* rect:
 *  Draws an outline rectangle.
 */
void _soft_rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   int t;

   if (x2 < x1) {
      t = x1;
      x1 = x2;
      x2 = t;
   }

   if (y2 < y1) {
      t = y1;
      y1 = y2;
      y2 = t;
   }

   acquire_bitmap(bmp);

   hline(bmp, x1, y1, x2, color);

   if (y2 > y1)
      hline(bmp, x1, y2, x2, color);

   if (y2-1 >= y1+1) {
      vline(bmp, x1, y1+1, y2-1, color);

      if (x2 > x1)
	 vline(bmp, x2, y1+1, y2-1, color);
   }

   release_bitmap(bmp);
}



/* _normal_rectfill:
 *  Draws a solid filled rectangle, using hfill() to do the work.
 */
void _normal_rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   int t;

   if (y1 > y2) {
      t = y1;
      y1 = y2;
      y2 = t;
   }

   if (bmp->clip) {
      if (x1 > x2) {
	 t = x1;
	 x1 = x2;
	 x2 = t;
      }

      if (x1 < bmp->cl)
	 x1 = bmp->cl;

      if (x2 >= bmp->cr)
	 x2 = bmp->cr-1;

      if (x2 < x1)
	 return;

      if (y1 < bmp->ct)
	 y1 = bmp->ct;

      if (y2 >= bmp->cb)
	 y2 = bmp->cb-1;

      if (y2 < y1)
	 return;

      bmp->clip = FALSE;
      t = TRUE;
   }
   else
      t = FALSE;

   acquire_bitmap(bmp);

   while (y1 <= y2) {
      _vtable_hfill(bmp, x1, y1, x2, color);
      y1++;
   };

   release_bitmap(bmp);

   bmp->clip = t;
}



/* do_line:
 *  Calculates all the points along a line between x1, y1 and x2, y2, 
 *  calling the supplied function for each one. This will be passed a 
 *  copy of the bmp parameter, the x and y position, and a copy of the 
 *  d parameter (so do_line() can be used with putpixel()).
 */
void do_line(BITMAP *bmp, int x1, int y1, int x2, int y2, int d, void (*proc)(BITMAP *, int, int, int))
{
   int dx = x2-x1;
   int dy = y2-y1;
   int i1, i2;
   int x, y;
   int dd;

   /* worker macro */
   #define DO_LINE(pri_sign, pri_c, pri_cond, sec_sign, sec_c, sec_cond)     \
   {                                                                         \
      if (d##pri_c == 0) {                                                   \
	 proc(bmp, x1, y1, d);                                               \
	 return;                                                             \
      }                                                                      \
									     \
      i1 = 2 * d##sec_c;                                                     \
      dd = i1 - (sec_sign (pri_sign d##pri_c));                              \
      i2 = dd - (sec_sign (pri_sign d##pri_c));                              \
									     \
      x = x1;                                                                \
      y = y1;                                                                \
									     \
      while (pri_c pri_cond pri_c##2) {                                      \
	 proc(bmp, x, y, d);                                                 \
									     \
	 if (dd sec_cond 0) {                                                \
	    sec_c = sec_c sec_sign 1;                                        \
	    dd += i2;                                                        \
	 }                                                                   \
	 else                                                                \
	    dd += i1;                                                        \
									     \
	 pri_c = pri_c pri_sign 1;                                           \
      }                                                                      \
   }

   if (dx >= 0) {
      if (dy >= 0) {
	 if (dx >= dy) {
	    /* (x1 <= x2) && (y1 <= y2) && (dx >= dy) */
	    DO_LINE(+, x, <=, +, y, >=);
	 }
	 else {
	    /* (x1 <= x2) && (y1 <= y2) && (dx < dy) */
	    DO_LINE(+, y, <=, +, x, >=);
	 }
      }
      else {
	 if (dx >= -dy) {
	    /* (x1 <= x2) && (y1 > y2) && (dx >= dy) */
	    DO_LINE(+, x, <=, -, y, <=);
	 }
	 else {
	    /* (x1 <= x2) && (y1 > y2) && (dx < dy) */
	    DO_LINE(-, y, >=, +, x, >=);
	 }
      }
   }
   else {
      if (dy >= 0) {
	 if (-dx >= dy) {
	    /* (x1 > x2) && (y1 <= y2) && (dx >= dy) */
	    DO_LINE(-, x, >=, +, y, >=);
	 }
	 else {
	    /* (x1 > x2) && (y1 <= y2) && (dx < dy) */
	    DO_LINE(+, y, <=, -, x, <=);
	 }
      }
      else {
	 if (-dx >= -dy) {
	    /* (x1 > x2) && (y1 > y2) && (dx >= dy) */
	    DO_LINE(-, x, >=, -, y, <=);
	 }
	 else {
	    /* (x1 > x2) && (y1 > y2) && (dx < dy) */
	    DO_LINE(-, y, >=, -, x, <=);
	 }
      }
   }

   #undef DO_LINE
}



/* _normal_line:
 *  Draws a line from x1, y1 to x2, y2, using putpixel() to do the work.
 */
void _normal_line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   int sx, sy, dx, dy, t;

   if (x1 == x2) {
      vline(bmp, x1, y1, y2, color);
      return;
   }

   if (y1 == y2) {
      hline(bmp, x1, y1, x2, color);
      return;
   }

   /* use a bounding box to check if the line needs clipping */
   if (bmp->clip) {
      sx = x1;
      sy = y1;
      dx = x2;
      dy = y2;

      if (sx > dx) {
	 t = sx;
	 sx = dx;
	 dx = t;
      }

      if (sy > dy) {
	 t = sy;
	 sy = dy;
	 dy = t;
      }

      if ((sx >= bmp->cr) || (sy >= bmp->cb) || (dx < bmp->cl) || (dy < bmp->ct))
	 return;

      if ((sx >= bmp->cl) && (sy >= bmp->ct) && (dx < bmp->cr) && (dy < bmp->cb))
	 bmp->clip = FALSE;

      t = TRUE;
   }
   else
      t= FALSE;

   acquire_bitmap(bmp);

   do_line(bmp, x1, y1, x2, y2, color, _vtable_putpixel);

   release_bitmap(bmp);

   bmp->clip = t;
}



/* _fast_line:
 *  Draws a line from x1, y1 to x2, y2, using putpixel() to do the work.
 *  This is an implementation of the Cohen-Sutherland line clipping algorithm.
 *  Loops over the line until it can be either trivially rejected or trivially
 *  accepted. If it is neither rejected nor accepted, subdivide it into two
 *  segments, one of which can be rejected.
 */
void _fast_line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   int code0, code1;
   int outcode;
   int x, y;
   int xmax, xmin, ymax, ymin;
   int done = 0, accept = 0;
   int clip_orig;
   ASSERT(bmp);

   if ((clip_orig = bmp->clip) != 0) {  /* save clipping state */
      #define TOP     0x8
      #define BOTTOM  0x4
      #define LEFT    0x2
      #define RIGHT   0x1

      #define COMPCLIP(code, x, y)  \
      {                             \
	 code = 0;                  \
	 if (y < ymin)              \
	    code |= TOP;            \
	 else if (y > ymax)         \
	    code |= BOTTOM;         \
	 if (x < xmin)              \
	    code |= LEFT;           \
	 else if (x > xmax)         \
	    code |= RIGHT;          \
      }

      xmin = bmp->cl;
      xmax = bmp->cr-1;
      ymin = bmp->ct;
      ymax = bmp->cb-1;

      COMPCLIP(code0, x1, y1);
      COMPCLIP(code1, x2, y2);

      do {

	 if (!(code0 | code1)) {
	    /* Trivially accept. */
	    accept = done = 1;
	 }
	 else if (code0 & code1) {
	    /* Trivially reject. */
	    done = 1;
	 }
	 else {
	    /* Didn't reject or accept, so do some calculations. */
	    outcode = code0 ? code0 : code1;  /* pick one endpoint */

	    if (outcode & TOP) {
	       if (y2 == y1)
		  x = x1;
	       else
		  x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1);
	       y = ymin;
	    }
	    else if (outcode & BOTTOM) {
	       if (y2 == y1)
		  x = x1;
	       else
		  x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1);
	       y = ymax;
	    }
	    else if (outcode & LEFT) {
	       if (x2 == x1)
		  y = y1;
	       else
		  y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1);
	       x = xmin;
	    }
	    else {  /* outcode & RIGHT */
	       if (x2 == x1)
		  y = y1;
	       else
		  y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1);
	       x = xmax;
	    }

	    if (outcode == code0) {
	       x1 = x;
	       y1 = y;
	       COMPCLIP(code0, x1, y1);
	    }
	    else {
	       x2 = x;
	       y2 = y;
	       COMPCLIP(code1, x2, y2);
	    }
	 }
      } while (!done);

      #undef COMPCLIP
      #undef TOP
      #undef BOTTOM
      #undef LEFT
      #undef RIGHT

      if (!accept)
	 return;

      /* We have already done the clipping, no need to do it again. */
      bmp->clip = FALSE;
   }

   if (x1 == x2) {
      _vtable_vline(bmp, x1, y1, y2, color);
   }
   else if (y1 == y2) {
      _vtable_hline(bmp, x1, y1, x2, color);
   }
   else {
      acquire_bitmap(bmp);
      do_line(bmp, x1, y1, x2, y2, color, _vtable_putpixel);
      release_bitmap(bmp);
   }

   /* Restore original clipping state. */
   bmp->clip = clip_orig;
}



/* do_circle:
 *  Helper function for the circle drawing routines. Calculates the points
 *  in a circle of radius r around point x, y, and calls the specified 
 *  routine for each one. The output proc will be passed first a copy of
 *  the bmp parameter, then the x, y point, then a copy of the d parameter
 *  (so putpixel() can be used as the callback).
 */
void do_circle(BITMAP *bmp, int x, int y, int radius, int d, void (*proc)(BITMAP *, int, int, int))
{
   int cx = 0;
   int cy = radius;
   int df = 1 - radius; 
   int d_e = 3;
   int d_se = -2 * radius + 5;

   do {
      proc(bmp, x+cx, y+cy, d); 

      if (cx) 
	 proc(bmp, x-cx, y+cy, d); 

      if (cy) 
	 proc(bmp, x+cx, y-cy, d);

      if ((cx) && (cy)) 
	 proc(bmp, x-cx, y-cy, d); 

      if (cx != cy) {
	 proc(bmp, x+cy, y+cx, d); 

	 if (cx) 
	    proc(bmp, x+cy, y-cx, d);

	 if (cy) 
	    proc(bmp, x-cy, y+cx, d); 

	 if (cx && cy) 
	    proc(bmp, x-cy, y-cx, d); 
      }

      if (df < 0)  {
	 df += d_e;
	 d_e += 2;
	 d_se += 2;
      }
      else { 
	 df += d_se;
	 d_e += 2;
	 d_se += 4;
	 cy--;
      } 

      cx++; 

   } while (cx <= cy);
}



/* circle:
 *  Draws a circle.
 */
void _soft_circle(BITMAP *bmp, int x, int y, int radius, int color)
{
   int clip, sx, sy, dx, dy;
   ASSERT(bmp);

   if (bmp->clip) {
      sx = x-radius-1;
      sy = y-radius-1;
      dx = x+radius+1;
      dy = y+radius+1;

      if ((sx >= bmp->cr) || (sy >= bmp->cb) || (dx < bmp->cl) || (dy < bmp->ct))
	 return;

      if ((sx >= bmp->cl) && (sy >= bmp->ct) && (dx < bmp->cr) && (dy < bmp->cb))
	 bmp->clip = FALSE;

      clip = TRUE;
   }
   else
      clip = FALSE;

   acquire_bitmap(bmp);

   do_circle(bmp, x, y, radius, color, _vtable_putpixel);

   release_bitmap(bmp);

   bmp->clip = clip;
}



/* circlefill:
 *  Draws a filled circle.
 */
void _soft_circlefill(BITMAP *bmp, int x, int y, int radius, int color)
{
   int cx = 0;
   int cy = radius;
   int df = 1 - radius; 
   int d_e = 3;
   int d_se = -2 * radius + 5;
   int clip, sx, sy, dx, dy;
   ASSERT(bmp);

   if (bmp->clip) {
      sx = x-radius-1;
      sy = y-radius-1;
      dx = x+radius+1;
      dy = y+radius+1;

      if ((sx >= bmp->cr) || (sy >= bmp->cb) || (dx < bmp->cl) || (dy < bmp->ct))
	 return;

      if ((sx >= bmp->cl) && (sy >= bmp->ct) && (dx < bmp->cr) && (dy < bmp->cb))
	 bmp->clip = FALSE;

      clip = TRUE;
   }
   else
      clip = FALSE;

   acquire_bitmap(bmp);

   do {
      _vtable_hfill(bmp, x-cy, y-cx, x+cy, color);

      if (cx)
	 _vtable_hfill(bmp, x-cy, y+cx, x+cy, color);

      if (df < 0)  {
	 df += d_e;
	 d_e += 2;
	 d_se += 2;
      }
      else { 
	 if (cx != cy) {
	    _vtable_hfill(bmp, x-cx, y-cy, x+cx, color);

	    if (cy)
	       _vtable_hfill(bmp, x-cx, y+cy, x+cx, color);
	 }

	 df += d_se;
	 d_e += 2;
	 d_se += 4;
	 cy--;
      } 

      cx++; 

   } while (cx <= cy);

   release_bitmap(bmp);

   bmp->clip = clip;
}
