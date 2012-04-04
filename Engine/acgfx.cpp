/*
  AGS specific color blending routines for transparency and tinting
  effects

  Adventure Game Studio source code Copyright 1999-2011 Chris Jones.
  All rights reserved.

  The AGS Editor Source Code is provided under the Artistic License 2.0
  http://www.opensource.org/licenses/artistic-license-2.0.php

  You MAY NOT compile your own builds of the engine without making it EXPLICITLY
  CLEAR that the code has been altered from the Standard Version.

*/

#include "bmp.h"

#include "sdlwrap/allegro.h"

#include "wgt2allg.h"
#include "acgfx.h"
#ifdef WINDOWS_VERSION
#include <d3d9.h>
#endif

#define BYTES_PER_PIXEL(bpp)     (((int)(bpp) + 7) / 8)

#include "mousew32.h"

extern "C" {
unsigned long _blender_trans16(unsigned long x, unsigned long y, unsigned long n);
unsigned long _blender_trans15(unsigned long x, unsigned long y, unsigned long n);
}

// Take hue and saturation of blend colour, luminance of image
unsigned long _myblender_color15_light(unsigned long x, unsigned long y, unsigned long n)
{
   float xh, xs, xv;
   float yh, ys, yv;
   int r, g, b;

   alw_rgb_to_hsv(alw_getr15(x), alw_getg15(x), alw_getb15(x), &xh, &xs, &xv);
   alw_rgb_to_hsv(alw_getr15(y), alw_getg15(y), alw_getb15(y), &yh, &ys, &yv);

   // adjust luminance
   yv -= (1.0f - ((float)n / 250.0f));
   if (yv < 0.0) yv = 0.0;

   alw_hsv_to_rgb(xh, xs, yv, &r, &g, &b);

   return alw_makecol15(r, g, b);
}

// Take hue and saturation of blend colour, luminance of image
// n is the last parameter passed to draw_lit_sprite
unsigned long _myblender_color16_light(unsigned long x, unsigned long y, unsigned long n)
{
   float xh, xs, xv;
   float yh, ys, yv;
   int r, g, b;

   alw_rgb_to_hsv(alw_getr16(x), alw_getg16(x), alw_getb16(x), &xh, &xs, &xv);
   alw_rgb_to_hsv(alw_getr16(y), alw_getg16(y), alw_getb16(y), &yh, &ys, &yv);

   // adjust luminance
   yv -= (1.0f - ((float)n / 250.0f));
   if (yv < 0.0) yv = 0.0;

   alw_hsv_to_rgb(xh, xs, yv, &r, &g, &b);

   return alw_makecol16(r, g, b);
}

// Take hue and saturation of blend colour, luminance of image
unsigned long _myblender_color32_light(unsigned long x, unsigned long y, unsigned long n)
{
   float xh, xs, xv;
   float yh, ys, yv;
   int r, g, b;

   alw_rgb_to_hsv(alw_getr32(x), alw_getg32(x), alw_getb32(x), &xh, &xs, &xv);
   alw_rgb_to_hsv(alw_getr32(y), alw_getg32(y), alw_getb32(y), &yh, &ys, &yv);

   // adjust luminance
   yv -= (1.0f - ((float)n / 250.0f));
   if (yv < 0.0) yv = 0.0;

   alw_hsv_to_rgb(xh, xs, yv, &r, &g, &b);

   return alw_makeacol32(r, g, b, alw_geta32(y));
}

// Take hue and saturation of blend colour, luminance of image
unsigned long _myblender_color15(unsigned long x, unsigned long y, unsigned long n)
{
   float xh, xs, xv;
   float yh, ys, yv;
   int r, g, b;

   alw_rgb_to_hsv(alw_getr15(x), alw_getg15(x), alw_getb15(x), &xh, &xs, &xv);
   alw_rgb_to_hsv(alw_getr15(y), alw_getg15(y), alw_getb15(y), &yh, &ys, &yv);

   alw_hsv_to_rgb(xh, xs, yv, &r, &g, &b);

   return alw_makecol15(r, g, b);
}

// Take hue and saturation of blend colour, luminance of image
unsigned long _myblender_color16(unsigned long x, unsigned long y, unsigned long n)
{
   float xh, xs, xv;
   float yh, ys, yv;
   int r, g, b;

   alw_rgb_to_hsv(alw_getr16(x), alw_getg16(x), alw_getb16(x), &xh, &xs, &xv);
   alw_rgb_to_hsv(alw_getr16(y), alw_getg16(y), alw_getb16(y), &yh, &ys, &yv);

   alw_hsv_to_rgb(xh, xs, yv, &r, &g, &b);

   return alw_makecol16(r, g, b);
}

// Take hue and saturation of blend colour, luminance of image
unsigned long _myblender_color32(unsigned long x, unsigned long y, unsigned long n)
{
   float xh, xs, xv;
   float yh, ys, yv;
   int r, g, b;

   alw_rgb_to_hsv(alw_getr32(x), alw_getg32(x), alw_getb32(x), &xh, &xs, &xv);
   alw_rgb_to_hsv(alw_getr32(y), alw_getg32(y), alw_getb32(y), &yh, &ys, &yv);

   alw_hsv_to_rgb(xh, xs, yv, &r, &g, &b);

   return alw_makeacol32(r, g, b, alw_geta32(y));
}



// trans24 blender, but preserve alpha channel from image
unsigned long _myblender_alpha_trans24(unsigned long x, unsigned long y, unsigned long n)
{
   unsigned long res, g, alph;

   if (n)
      n++;

   alph = y & 0xff000000;
   y &= 0x00ffffff;

   res = ((x & 0xFF00FF) - (y & 0xFF00FF)) * n / 256 + y;
   y &= 0xFF00;
   x &= 0xFF00;
   g = (x - y) * n / 256 + y;

   res &= 0xFF00FF;
   g &= 0xFF00;

   return res | g | alph;
}

void set_my_trans_blender(int r, int g, int b, int a) {
  // use standard allegro 15 and 16 bit blenders, but customize
  // the 32-bit one to preserve the alpha channel
  alw_set_blender_mode(_blender_trans15, _blender_trans16, _myblender_alpha_trans24, r, g, b, a);
}



// add the alpha values together, used for compositing alpha images
unsigned long _additive_alpha_blender(unsigned long x, unsigned long y, unsigned long n)
{
  unsigned long newAlpha = ((x & 0xff000000) >> 24) + ((y & 0xff000000) >> 24);

  if (newAlpha > 0xff) newAlpha = 0xff;

  return (newAlpha << 24) | (x & 0x00ffffff);
}

void set_additive_alpha_blender() {
  // add the alpha channels together
  alw_set_blender_mode(NULL, NULL, _additive_alpha_blender, 0, 0, 0, 0);
}

// sets the alpha channel to opaque. used when drawing a non-alpha sprite onto an alpha-sprite
unsigned long _opaque_alpha_blender(unsigned long x, unsigned long y, unsigned long n)
{
  return x | 0xff000000;
}

void set_opaque_alpha_blender() {
  alw_set_blender_mode(NULL, NULL, _opaque_alpha_blender, 0, 0, 0, 0);
}


// **** FILTERS *****

struct MouseGetPosCallbackImpl : IMouseGetPosCallback {
protected:
  ScalingGFXFilter *_callbackFilter;

public:
  MouseGetPosCallbackImpl(ScalingGFXFilter *filter)
  {
    _callbackFilter = filter;
  }

  virtual void AdjustPosition(int *x, int *y)
  {
    _callbackFilter->AdjustPosition(x, y);
  }
};



GFXFilter *filter;

//#include "sai.h"
#include "hq2x3x.h"


// Standard do-nothing filter

const char* GFXFilter::Initialize(int width, int height, int colDepth) {
  return NULL;  // always succeeds
}

void GFXFilter::UnInitialize() {
  // do nothing
}

void GFXFilter::GetRealResolution(int *wid, int *hit) {
  // no change
}

void GFXFilter::SetMouseArea(int x1, int y1, int x2, int y2) {
  mgraphconfine(x1, y1, x2, y2);
}

void GFXFilter::SetMouseLimit(int x1, int y1, int x2, int y2) {
  msetcursorlimit(x1, y1, x2, y2);
}

void GFXFilter::SetMousePosition(int x, int y) {
  msetgraphpos(x, y);
}

const char *GFXFilter::GetVersionBoxText() {
  return "";
}

const char *GFXFilter::GetFilterID() {
  return "None";
}

GFXFilter::~GFXFilter()
{
}

// Standard scaling filter

ScalingGFXFilter::ScalingGFXFilter(int multiplier, bool justCheckingForSetup) : GFXFilter() {
  MULTIPLIER = multiplier;
  mouseCallbackPtr = NULL;

  sprintf(filterName, "%d" "x nearest-neighbour filter[", multiplier);
  sprintf(filterID, "StdScale%d", multiplier);
}

  const char* ScalingGFXFilter::Initialize(int width, int height, int colDepth) {
    mouseCallbackPtr = new MouseGetPosCallbackImpl(this);
    msetcallback((MouseGetPosCallbackImpl*)mouseCallbackPtr);
    return NULL;
  }

  void ScalingGFXFilter::UnInitialize() {
    msetcallback(NULL);
  }

  void ScalingGFXFilter::GetRealResolution(int *wid, int *hit) {
    *wid *= MULTIPLIER;
    *hit *= MULTIPLIER;
  }

  void ScalingGFXFilter::SetMouseArea(int x1, int y1, int x2, int y2) {
    x1 *= MULTIPLIER;
    y1 *= MULTIPLIER;
    x2 *= MULTIPLIER;
    y2 *= MULTIPLIER;
    mgraphconfine(x1, y1, x2, y2);
  }

  void ScalingGFXFilter::SetMouseLimit(int x1, int y1, int x2, int y2) {
    // 199 -> 399
    x1 = x1 * MULTIPLIER + (MULTIPLIER - 1);
    y1 = y1 * MULTIPLIER + (MULTIPLIER - 1);
    x2 = x2 * MULTIPLIER + (MULTIPLIER - 1);
    y2 = y2 * MULTIPLIER + (MULTIPLIER - 1);
    msetcursorlimit(x1, y1, x2, y2);
  }

  void ScalingGFXFilter::SetMousePosition(int x, int y) {
    msetgraphpos(x * MULTIPLIER, y * MULTIPLIER);
  }

  void ScalingGFXFilter::AdjustPosition(int *x, int *y) {
    *x /= MULTIPLIER;
    *y /= MULTIPLIER;
  }

  const char *ScalingGFXFilter::GetVersionBoxText() {
    return filterName;
  }

  const char *ScalingGFXFilter::GetFilterID() {
    return filterID;
  }

  ScalingGFXFilter::~ScalingGFXFilter()
  {
    if (mouseCallbackPtr != NULL)
    {
      delete mouseCallbackPtr;
      mouseCallbackPtr = NULL;
    }
  }

// Standard 3D-accelerated filter

D3DGFXFilter::D3DGFXFilter(bool justCheckingForSetup) : ScalingGFXFilter(1, justCheckingForSetup) { 
  sprintf(filterName, "");
  sprintf(filterID, "None");
}

D3DGFXFilter::D3DGFXFilter(int multiplier, bool justCheckingForSetup) : ScalingGFXFilter(multiplier, justCheckingForSetup)
{
  sprintf(filterName, "%d" "x nearest-neighbour filter[", multiplier);
  sprintf(filterID, "StdScale%d", multiplier);
}

void D3DGFXFilter::SetSamplerStateForStandardSprite(void *direct3ddevice9)
{
#ifdef WINDOWS_VERSION
  IDirect3DDevice9* d3d9 = ((IDirect3DDevice9*)direct3ddevice9);
  d3d9->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
  d3d9->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
#endif
}

bool D3DGFXFilter::NeedToColourEdgeLines()
{
  return false;
}

// Anti-aliased D3D filter

struct AAD3DGFXFilter : D3DGFXFilter {

  AAD3DGFXFilter(int multiplier, bool justCheckingForSetup) : D3DGFXFilter(multiplier, justCheckingForSetup)
  {
    sprintf(filterName, "%d" "x anti-aliasing filter[", multiplier);
    sprintf(filterID, "AAx%d", multiplier);
  }

  virtual void SetSamplerStateForStandardSprite(void *direct3ddevice9)
  {
#ifdef WINDOWS_VERSION
    IDirect3DDevice9* d3d9 = ((IDirect3DDevice9*)direct3ddevice9);
    d3d9->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    d3d9->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
#endif
  }

  virtual bool NeedToColourEdgeLines()
  {
    return true;
  }
};

// Standard Allegro filter

AllegroGFXFilter::AllegroGFXFilter(bool justCheckingForSetup) : ScalingGFXFilter(1, justCheckingForSetup) { 
  lastBlitX = 0;
  lastBlitY = 0;

  sprintf(filterName, "");
  sprintf(filterID, "None");
}

AllegroGFXFilter::AllegroGFXFilter(int multiplier, bool justCheckingForSetup) : ScalingGFXFilter(multiplier, justCheckingForSetup)
{
  lastBlitX = 0;
  lastBlitY = 0;
}

ALW_BITMAP* AllegroGFXFilter::ScreenInitialized(ALW_BITMAP *screen, int fakeWidth, int fakeHeight) {
  realScreen = screen;
  return alw_screen;
}

ALW_BITMAP *AllegroGFXFilter::ShutdownAndReturnRealScreen(ALW_BITMAP *currentScreen) {
  return currentScreen;
}

void AllegroGFXFilter::RenderScreen(ALW_BITMAP *toRender, int x, int y) {

  if (toRender != realScreen) 
    alw_blit(toRender, realScreen, 0, 0, x, y, BMP_W(toRender), BMP_H(toRender));

  lastBlitX = x;
  lastBlitY = y;
}

void AllegroGFXFilter::RenderScreenFlipped(ALW_BITMAP *toRender, int x, int y, int flipType) {

  if (toRender == realScreen) 
    return;

  if (flipType == SCR_HFLIP)
    alw_draw_sprite_h_flip(realScreen, toRender, 0, 0);
  else if (flipType == SCR_VFLIP)
    alw_draw_sprite_v_flip(realScreen, toRender, 0, 0);
  else if (flipType == SCR_VHFLIP)
    alw_draw_sprite_vh_flip(realScreen, toRender, 0, 0);
}

void AllegroGFXFilter::ClearRect(int x1, int y1, int x2, int y2, int color) {
  alw_rectfill(realScreen, x1, y1, x2, y2, color);
}

void AllegroGFXFilter::GetCopyOfScreenIntoBitmap(ALW_BITMAP *copyBitmap) 
{
  GetCopyOfScreenIntoBitmap(copyBitmap, true);
}

void AllegroGFXFilter::GetCopyOfScreenIntoBitmap(ALW_BITMAP *copyBitmap, bool copyWithOffset)
{
  if (copyBitmap != realScreen)
    alw_blit(realScreen, copyBitmap, (copyWithOffset ? lastBlitX : 0), (copyWithOffset ? lastBlitY : 0), 0, 0, BMP_W(copyBitmap), BMP_H(copyBitmap));
}


struct ScalingAllegroGFXFilter : public AllegroGFXFilter {
protected:
  ALW_BITMAP *fakeScreen;
  ALW_BITMAP *realScreenSizedBuffer;
  ALW_BITMAP *lastBlitFrom;

public:

  ScalingAllegroGFXFilter(int multiplier, bool justCheckingForSetup) : 
      AllegroGFXFilter(multiplier, justCheckingForSetup) {

    lastBlitFrom = NULL;
  }

  virtual ALW_BITMAP* ScreenInitialized(ALW_BITMAP *alw_screen, int fakeWidth, int fakeHeight) {
    realScreen = alw_screen;
    realScreenSizedBuffer = alw_create_bitmap_ex(alw_bitmap_color_depth(alw_screen), BMP_W(alw_screen), BMP_H(alw_screen));
    fakeScreen = alw_create_bitmap_ex(alw_bitmap_color_depth(alw_screen), fakeWidth, fakeHeight);
    return fakeScreen;
  }

  virtual ALW_BITMAP *ShutdownAndReturnRealScreen(ALW_BITMAP *currentScreen) {
    alw_destroy_bitmap(fakeScreen);
    alw_destroy_bitmap(realScreenSizedBuffer);
    fakeScreen = NULL;
    realScreenSizedBuffer = NULL;
    return realScreen;
  }

  virtual void RenderScreen(ALW_BITMAP *toRender, int x, int y) 
  {
    alw_stretch_blit(toRender, realScreen, 0, 0, BMP_W(toRender), BMP_H(toRender), x * MULTIPLIER, y * MULTIPLIER, BMP_W(toRender) * MULTIPLIER, BMP_H(toRender) * MULTIPLIER);
    lastBlitX = x;
    lastBlitY = y;
    lastBlitFrom = toRender;
  }

  virtual void RenderScreenFlipped(ALW_BITMAP *toRender, int x, int y, int flipType) {

    if (toRender == fakeScreen)
      return;

    if (flipType == SCR_HFLIP)
      alw_draw_sprite_h_flip(fakeScreen, toRender, 0, 0);
    else if (flipType == SCR_VFLIP)
      alw_draw_sprite_v_flip(fakeScreen, toRender, 0, 0);
    else if (flipType == SCR_VHFLIP)
      alw_draw_sprite_vh_flip(fakeScreen, toRender, 0, 0);

    RenderScreen(fakeScreen, x, y);
  }

  virtual void ClearRect(int x1, int y1, int x2, int y2, int color) {
    x1 *= MULTIPLIER;
    y1 *= MULTIPLIER;
    x2 = x2 * MULTIPLIER + (MULTIPLIER - 1);
    y2 = y2 * MULTIPLIER + (MULTIPLIER - 1);
    alw_rectfill(realScreen, x1, y1, x2, y2, color);
  }

  virtual void GetCopyOfScreenIntoBitmap(ALW_BITMAP *copyBitmap) 
  {
    GetCopyOfScreenIntoBitmap(copyBitmap, true);
  }

  virtual void GetCopyOfScreenIntoBitmap(ALW_BITMAP *copyBitmap, bool copyWithYOffset)
  {
    if (!copyWithYOffset)
    {
      // Can't stretch_blit from Video Memory to normal memory,
      // so copy the screen to a buffer first.
      alw_blit(realScreen, realScreenSizedBuffer, 0, 0, 0, 0, BMP_W(realScreen), BMP_H(realScreen));
      alw_stretch_blit(realScreenSizedBuffer, copyBitmap, 0, 0, 
                  BMP_W(realScreenSizedBuffer), BMP_H(realScreenSizedBuffer), 
                  0, 0, BMP_W(copyBitmap), BMP_H(copyBitmap));
    }
    else if (lastBlitFrom == NULL)
      alw_clear_bitmap(copyBitmap);
    else
      alw_stretch_blit(lastBlitFrom, copyBitmap, 0, 0, 
                  BMP_W(lastBlitFrom), BMP_H(lastBlitFrom), 
                  0, 0, BMP_W(copyBitmap), BMP_H(copyBitmap));
  }

};


/* 
 *** UNFORTUNATELY THE 2xSAI SCALER IS GPL AND SO WE CANNOT
 *** USE IT IN RELEASE CODE

struct Super2xSAIGFXFilter : public ScalingGFXFilter {
private:
  ALW_BITMAP *realScreenBuffer;

public:

  Super2xSAIGFXFilter(bool justCheckingForSetup) : ScalingGFXFilter(2, justCheckingForSetup) { }

  virtual const char* Initialize(int width, int height, int colDepth) {
    if (colDepth == 8)
      return "8-bit colour not supported";

    return ScalingGFXFilter::Initialize(width, height, colDepth);
  }

  virtual ALW_BITMAP* ScreenInitialized(ALW_BITMAP *screen, int fakeWidth, int fakeHeight) {
    realScreen = screen;
    realScreenBuffer = alw_create_bitmap(BMP_W( screen), BMP_H( screen));
    fakeScreen = alw_create_bitmap_ex(alw_bitmap_color_depth( screen), fakeWidth, fakeHeight);
    Init_2xSaI(alw_bitmap_color_depth( screen));
    return fakeScreen;
  }

  virtual ALW_BITMAP *ShutdownAndReturnRealScreen(ALW_BITMAP *currentScreen) {
    alw_destroy_bitmap(fakeScreen);
    alw_destroy_bitmap(realScreenBuffer);
    return realScreen;
  }

  virtual void RenderScreen(ALW_BITMAP *toRender, int x, int y) {

    alw_acquire_bitmap(realScreenBuffer);
    Super2xSaI(toRender, realScreenBuffer, 0, 0, 0, 0, BMP_W(toRender), BMP_H(toRender));
    alw_release_bitmap(realScreenBuffer);

    alw_blit(realScreenBuffer, realScreen, 0, 0, x * MULTIPLIER, y * MULTIPLIER, BMP_W(realScreen), BMP_H(realScreen));

    lastBlitFrom = toRender;
  }

  virtual const char *GetVersionBoxText() {
    return "Super2xSaI filter[";
  }

  virtual const char *GetFilterID() {
    return "Super2xSaI";
  }
};
*/


struct Hq2xGFXFilter : public ScalingAllegroGFXFilter {
private:
  ALW_BITMAP *realScreenBuffer;

public:

  Hq2xGFXFilter(bool justCheckingForSetup) : ScalingAllegroGFXFilter(2, justCheckingForSetup) { }

  virtual const char* Initialize(int width, int height, int colDepth) {
    if (colDepth < 32)
      return "Only supports 32-bit colour games";

    return ScalingGFXFilter::Initialize(width, height, colDepth);
  }


  virtual ALW_BITMAP* ScreenInitialized(ALW_BITMAP *screen, int fakeWidth, int fakeHeight) {
    realScreen = screen;
    realScreenBuffer = alw_create_bitmap(BMP_W( screen), BMP_H( screen));
    realScreenSizedBuffer = alw_create_bitmap_ex(alw_bitmap_color_depth( screen), BMP_W( screen), BMP_H( screen));
    fakeScreen = alw_create_bitmap_ex(alw_bitmap_color_depth( screen), fakeWidth, fakeHeight);
    InitLUTs();
    return fakeScreen;
  }

  virtual ALW_BITMAP *ShutdownAndReturnRealScreen(ALW_BITMAP *currentScreen) {
    alw_destroy_bitmap(fakeScreen);
    alw_destroy_bitmap(realScreenBuffer);
    alw_destroy_bitmap(realScreenSizedBuffer);
    return realScreen;
  }

  virtual void RenderScreen(ALW_BITMAP *toRender, int x, int y) {

    alw_acquire_bitmap(realScreenBuffer);
    hq2x_32(&BMP_LINE(toRender)[0][0], &BMP_LINE(realScreenBuffer)[0][0], BMP_W(toRender), BMP_H(toRender), BMP_W(realScreenBuffer) * BYTES_PER_PIXEL(alw_bitmap_color_depth(realScreenBuffer)));
    alw_release_bitmap(realScreenBuffer);

    alw_blit(realScreenBuffer, realScreen, 0, 0, x * MULTIPLIER, y * MULTIPLIER, BMP_W(realScreen), BMP_H(realScreen));

    lastBlitFrom = toRender;
  }

  virtual const char *GetVersionBoxText() {
    return "Hq2x filter (32-bit only)[";
  }

  virtual const char *GetFilterID() {
    return "Hq2x";
  }
};

struct Hq3xGFXFilter : public ScalingAllegroGFXFilter {
private:
  ALW_BITMAP *realScreenBuffer;

public:

  Hq3xGFXFilter(bool justCheckingForSetup) : ScalingAllegroGFXFilter(3, justCheckingForSetup) { }

  virtual const char* Initialize(int width, int height, int colDepth) {
    if (colDepth < 32)
      return "Only supports 32-bit colour games";

    return ScalingGFXFilter::Initialize(width, height, colDepth);
  }


  virtual ALW_BITMAP* ScreenInitialized(ALW_BITMAP *screen, int fakeWidth, int fakeHeight) {
    realScreen = screen;
    realScreenBuffer = alw_create_bitmap(BMP_W(screen), BMP_H(screen));
    realScreenSizedBuffer = alw_create_bitmap_ex(alw_bitmap_color_depth(screen), BMP_W(screen), BMP_H(screen));
    fakeScreen = alw_create_bitmap_ex(alw_bitmap_color_depth(screen), fakeWidth, fakeHeight);
    InitLUTs();
    return fakeScreen;
  }

  virtual ALW_BITMAP *ShutdownAndReturnRealScreen(ALW_BITMAP *currentScreen) {
    alw_destroy_bitmap(fakeScreen);
    alw_destroy_bitmap(realScreenBuffer);
    alw_destroy_bitmap(realScreenSizedBuffer);
    return realScreen;
  }

  virtual void RenderScreen(ALW_BITMAP *toRender, int x, int y) {

    alw_acquire_bitmap(realScreenBuffer);
    hq3x_32(&BMP_LINE(toRender)[0][0], &BMP_LINE(realScreenBuffer)[0][0], BMP_W(toRender), BMP_H(toRender), BMP_W(realScreenBuffer) * BYTES_PER_PIXEL(alw_bitmap_color_depth(realScreenBuffer)));
    alw_release_bitmap(realScreenBuffer);

    alw_blit(realScreenBuffer, realScreen, 0, 0, x * MULTIPLIER, y * MULTIPLIER, BMP_W(realScreen), BMP_H(realScreen));

    lastBlitFrom = toRender;
  }

  virtual const char *GetVersionBoxText() {
    return "Hq3x filter (32-bit only)[";
  }

  virtual const char *GetFilterID() {
    return "Hq3x";
  }
};

GFXFilter *gfxFilterList[10];
GFXFilter *gfxFilterListD3D[10];

GFXFilter **get_allegro_gfx_filter_list(bool checkingForSetup) {

  gfxFilterList[0] = new AllegroGFXFilter(checkingForSetup);
  gfxFilterList[1] = new ScalingAllegroGFXFilter(2, checkingForSetup);
  gfxFilterList[2] = new ScalingAllegroGFXFilter(3, checkingForSetup);
  gfxFilterList[3] = new ScalingAllegroGFXFilter(4, checkingForSetup);
  gfxFilterList[4] = new Hq2xGFXFilter(checkingForSetup);
  gfxFilterList[5] = new Hq3xGFXFilter(checkingForSetup);
  gfxFilterList[6] = NULL;

  return gfxFilterList;
}

GFXFilter **get_d3d_gfx_filter_list(bool checkingForSetup) {

  gfxFilterListD3D[0] = new D3DGFXFilter(checkingForSetup);
  gfxFilterListD3D[1] = new D3DGFXFilter(2, checkingForSetup);
  gfxFilterListD3D[2] = new D3DGFXFilter(3, checkingForSetup);
  gfxFilterListD3D[3] = new D3DGFXFilter(4, checkingForSetup);
  gfxFilterListD3D[4] = new AAD3DGFXFilter(2, checkingForSetup);
  gfxFilterListD3D[5] = new AAD3DGFXFilter(3, checkingForSetup);
  gfxFilterListD3D[6] = new AAD3DGFXFilter(4, checkingForSetup);
  gfxFilterListD3D[7] = NULL;

  return gfxFilterListD3D;
}

/*
const char *initialize_gfx_filter(int width, int height, int depth) {

  //filter = new ScalingGFXFilter(2, false);

  filter = new Super2xSAIGFXFilter(false);

  //filter = new Hq2xGFXFilter(false);

  //filter = new GFXFilter();

  return filter->Initialize(width, height, depth);
}

*/
