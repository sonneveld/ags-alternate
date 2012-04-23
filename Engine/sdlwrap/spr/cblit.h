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
 *      Bitmap blitting functions.
 *
 *      By Michael Bukin.
 *
 *      See readme.txt for copyright information.
 */

#ifndef __bma_cblit_h
#define __bma_cblit_h


#include <string.h>



/* _linear_clear_to_color:
 *   Fills a linear bitmp with the specified color.
 */
void FUNC_LINEAR_CLEAR_TO_COLOR(BITMAP *dst, int color)
{
   int x, y;
   int w;

   ASSERT(dst);

   w = dst->cr - dst->cl;

   bmp_select(dst);

   for (y = dst->ct; y < dst->cb; y++) {
      PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, y), dst->cl);

      for (x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
	 PUT_PIXEL(d, color);
      }
   }

   bmp_unwrite_line(dst);
}



/* _linear_blit:
 *  Normal forward blitting for linear bitmaps.
 */
void FUNC_LINEAR_BLIT(BITMAP *src, BITMAP *dst, int sx, int sy,
		      int dx, int dy, int w, int h)
{
   int y;

   ASSERT(src);
   ASSERT(dst);

   for (y = 0; y < h; y++) {
      PIXEL_PTR s = OFFSET_PIXEL_PTR(bmp_read_line(src, sy + y), sx);
      PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy + y), dx);

      memmove(d, s, w * sizeof(*s) * PTR_PER_PIXEL);
   }

   bmp_unwrite_line(src);
   bmp_unwrite_line(dst);
}



/* _linear_blit_backward:
 *  Reverse blitting routine, for overlapping linear bitmaps.
 */
void FUNC_LINEAR_BLIT_BACKWARD(BITMAP *src, BITMAP *dst, int sx, int sy,
			       int dx, int dy, int w, int h)
{
   int y;

   ASSERT(src);
   ASSERT(dst);

   for (y = h - 1; y >= 0; y--) {
      PIXEL_PTR s = OFFSET_PIXEL_PTR(bmp_read_line(src, sy + y), sx);
      PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy + y), dx);

      memmove(d, s, w * sizeof(*s) * PTR_PER_PIXEL);
   }

   bmp_unwrite_line(src);
   bmp_unwrite_line(dst);
}

void FUNC_LINEAR_BLIT_END(void) { }



#endif /* !__bma_cblit_h */

