// from allegro

#include <math.h>
#include <assert.h>

#define ASSERT assert
#undef MIN
#define MIN(x,y)     (((x) < (y)) ? (x) : (y))

// taken from allegro.

/* hsv_to_rgb:
 *  Converts from HSV colorspace to RGB values.
 */
void alw_hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b)
{
   float f, x, y, z;
   int i;

   ASSERT(s >= 0 && s <= 1);
   ASSERT(v >= 0 && v <= 1);

   v *= 255.0f;

   if (s == 0.0f) { /* ok since we don't divide by s, and faster */
      *r = *g = *b = v + 0.5f;
   }
   else {
      h = fmod(h, 360.0f) / 60.0f;
      if (h < 0.0f)
	 h += 6.0f;

      i = (int)h;
      f = h - i;
      x = v * s;
      y = x * f;
      v += 0.5f; /* round to the nearest integer below */
      z = v - x;

      switch (i) {

	 case 6:
	 case 0:
	    *r = v;
	    *g = z + y;
	    *b = z;
	    break;

	 case 1:
	    *r = v - y;
	    *g = v;
	    *b = z;
	    break;

	 case 2:
	    *r = z;
	    *g = v;
	    *b = z + y;
	    break;

	 case 3:
	    *r = z;
	    *g = v - y;
	    *b = v;
	    break;

	 case 4:
	    *r = z + y;
	    *g = z;
	    *b = v;
	    break;

	 case 5:
	    *r = v;
	    *g = z;
	    *b = v - y;
	    break;
      }
   }
}



/* rgb_to_hsv:
 *  Converts an RGB value into the HSV colorspace.
 */
void alw_rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v)
{
   int delta;

   ASSERT(r >= 0 && r <= 255);
   ASSERT(g >= 0 && g <= 255);
   ASSERT(b >= 0 && b <= 255);

   if (r > g) {
      if (b > r) {
	 /* b>r>g */
	 delta = b-g;
	 *h = 240.0f + ((r-g) * 60) / (float)delta;
	 *s = (float)delta / (float)b;
	 *v = (float)b * (1.0f/255.0f);
      }
      else {
	 /* r>g and r>b */
	 delta = r - MIN(g, b);
	 *h = ((g-b) * 60) / (float)delta;
	 if (*h < 0.0f)
	    *h += 360.0f;
	 *s = (float)delta / (float)r;
	 *v = (float)r * (1.0f/255.0f);
      }
   }
   else {
      if (b > g) {
	 /* b>g>=r */
	 delta = b-r;
	 *h = 240.0f + ((r-g) * 60) / (float)delta;
	 *s = (float)delta / (float)b;
	 *v = (float)b * (1.0f/255.0f);
      }
      else {
	 /* g>=b and g>=r */
	 delta = g - MIN(r, b);
	 if (delta == 0) {
	    *h = 0.0f;
	    if (g == 0)
	       *s = *v = 0.0f;
	    else {
	       *s = (float)delta / (float)g;
	       *v = (float)g * (1.0f/255.0f);
	    }
	 }
	 else {
	    *h = 120.0f + ((b-r) * 60) / (float)delta;
	    *s = (float)delta / (float)g;
	    *v = (float)g * (1.0f/255.0f);
	 }
      }
   }
}
