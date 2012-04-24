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
 *      Linear graphics functions.
 *
 *      By Michael Bukin.
 *
 *      See readme.txt for copyright information.
 */

#ifndef __bma_cgfx_h
#define __bma_cgfx_h

/* _linear_putpixel:
 *  Draws a pixel onto a linear bitmap.
 */
void FUNC_LINEAR_PUTPIXEL(BITMAP *dst, int dx, int dy, int color, bool solid=true)
{
   ASSERT(dst);

   if (dst->clip && ((dx < dst->cl) || (dx >= dst->cr) || (dy < dst->ct) || (dy >= dst->cb)))
      return;

   bmp_select(dst);

   if (solid) {
      PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy), dx);
      PUT_PIXEL(d, color);
   }
   else {
      PIXEL_PTR s = OFFSET_PIXEL_PTR(bmp_read_line(dst, dy), dx);
      PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy), dx);
      PP_BLENDER blender = MAKE_PP_BLENDER(color);
     unsigned long other = GET_PIXEL(s);
      unsigned long c = PP_BLEND(blender, other, color);
      PUT_PIXEL(d, c);
   }

   bmp_unwrite_line(dst);
}



/* _linear_getpixel:
 *  Reads a pixel from a linear bitmap.
 */
int FUNC_LINEAR_GETPIXEL(BITMAP *src, int sx, int sy)
{
   ASSERT(src);

   if ((sx < 0) || (sx >= src->w) || (sy < 0) || (sy >= src->h))
      return -1;
   else {
      PIXEL_PTR s = OFFSET_PIXEL_PTR(bmp_read_line(src, sy), sx);
      unsigned long c;

      bmp_select(src);
      c = GET_PIXEL(s);
      bmp_unwrite_line(src);

      return c;
   }
}



/* _linear_hline:
 *  Draws a horizontal line onto a linear bitmap.
 */
void FUNC_LINEAR_HLINE(BITMAP *dst, int dx1, int dy, int dx2, int color)
{
   int w;

   ASSERT(dst);

   if (dx1 > dx2) {
      int tmp = dx1;
      dx1 = dx2;
      dx2 = tmp;
   }
   if (dst->clip) {
      if (dx1 < dst->cl)
	 dx1 = dst->cl;
      if (dx2 >= dst->cr)
	 dx2 = dst->cr - 1;
      if ((dx1 > dx2) || (dy < dst->ct) || (dy >= dst->cb))
	 return;
   }

   w = dx2 - dx1;

   bmp_select(dst);

      PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy), dx1);
      do {
	 PUT_PIXEL(d, color);
	 INC_PIXEL_PTR(d);
      } while (--w >= 0);
   
   bmp_unwrite_line(dst);
}



/* _linear_vline:
 *  Draws a vertical line onto a linear bitmap.
 */
void FUNC_LINEAR_VLINE(BITMAP *dst, int dx, int dy1, int dy2, int color)
{
   int y;

   ASSERT(dst);

   if (dy1 > dy2) {
      int tmp = dy1;
      dy1 = dy2;
      dy2 = tmp;
   }
   if (dst->clip) {
      if (dy1 < dst->ct)
	 dy1 = dst->ct;
      if (dy2 >= dst->cb)
	 dy2 = dst->cb - 1;
      if ((dx < dst->cl) || (dx >= dst->cr) || (dy1 > dy2))
	 return;
   }

      bmp_select(dst);
      for (y = dy1; y <= dy2; y++) {
	 PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, y), dx);
	 PUT_PIXEL(d, color);
      }
      bmp_unwrite_line(dst);
}

#endif /* !__bma_cgfx_h */

