#include "ac_drawsurf.h"

#include "allegro_wrapper.h"

#include "ac.h"
#include "ac_context.h"
#include "sprcache.h"
#include "bmp.h"
#include "acgui.h"
#include "ac_string.h"
#include "dynobj/script_drawing_surface.h"
#include "cscomp.h"

extern int Game_GetColorFromRGB(int red, int grn, int blu); // eventually ac_game.h

// ** SCRIPT DRAWINGSURFACE OBJECT

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::Release^0 *** */
void DrawingSurface_Release(ScriptDrawingSurface* sds)
{
  if (sds->roomBackgroundNumber >= 0)
  {
    if (sds->modified)
    {
      if (sds->roomBackgroundNumber == play.bg_frame)
      {
        invalidate_screen();
        mark_current_background_dirty();
      }
      play.raw_modified[sds->roomBackgroundNumber] = 1;
    }

    sds->roomBackgroundNumber = -1;
  }
  if (sds->dynamicSpriteNumber >= 0)
  {
    if (sds->modified)
    {
      int tt;
      // force a refresh of any cached object or character images
      if (croom != NULL) 
      {
        for (tt = 0; tt < croom->numobj; tt++) 
        {
          if (objs[tt].num == sds->dynamicSpriteNumber)
            objcache[tt].sppic = -31999;
        }
      }
      for (tt = 0; tt < game.numcharacters; tt++) 
      {
        if (charcache[tt].sppic == sds->dynamicSpriteNumber)
          charcache[tt].sppic = -31999;
      }
      for (tt = 0; tt < game.numgui; tt++) 
      {
        if ((guis[tt].bgpic == sds->dynamicSpriteNumber) &&
            (guis[tt].on == 1))
        {
          guis_need_update = 1;
          break;
        }
      }
    }

    sds->dynamicSpriteNumber = -1;
  }
  if (sds->dynamicSurfaceNumber >= 0)
  {
    alw_destroy_bitmap(dynamicallyCreatedSurfaces[sds->dynamicSurfaceNumber]);
    dynamicallyCreatedSurfaces[sds->dynamicSurfaceNumber] = NULL;
    sds->dynamicSurfaceNumber = -1;
  }
  sds->modified = 0;
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::CreateCopy^0 *** */
ScriptDrawingSurface* DrawingSurface_CreateCopy(ScriptDrawingSurface *sds)
{
  ALW_BITMAP *sourceBitmap = sds->GetBitmapSurface();

  for (int i = 0; i < MAX_DYNAMIC_SURFACES; i++)
  {
    if (dynamicallyCreatedSurfaces[i] == NULL)
    {
      dynamicallyCreatedSurfaces[i] = alw_create_bitmap_ex(alw_bitmap_color_depth(sourceBitmap), BMP_W(sourceBitmap), BMP_H(sourceBitmap));
      alw_blit(sourceBitmap, dynamicallyCreatedSurfaces[i], 0, 0, 0, 0, BMP_W(sourceBitmap), BMP_H(sourceBitmap));
      ScriptDrawingSurface *newSurface = new ScriptDrawingSurface();
      newSurface->dynamicSurfaceNumber = i;
      newSurface->hasAlphaChannel = sds->hasAlphaChannel;
      ccRegisterManagedObject(newSurface, newSurface);
      return newSurface;
    }
  }

  quit("!DrawingSurface.CreateCopy: too many copied surfaces created");
  return NULL;
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::DrawSurface^2 *** */
void DrawingSurface_DrawSurface(ScriptDrawingSurface* target, ScriptDrawingSurface* source, int translev) {
  if ((translev < 0) || (translev > 99))
    quit("!DrawingSurface.DrawSurface: invalid parameter (transparency must be 0-99)");

  target->StartDrawing();
  ALW_BITMAP *surfaceToDraw = source->GetBitmapSurface();

  if (surfaceToDraw == abuf)
    quit("!DrawingSurface.DrawSurface: cannot draw surface onto itself");

  if (translev == 0) {
    // just draw it over the top, no transparency
    alw_blit(surfaceToDraw, abuf, 0, 0, 0, 0, BMP_W(surfaceToDraw), BMP_H(surfaceToDraw));
    target->FinishedDrawing();
    return;
  }

  if (alw_bitmap_color_depth(surfaceToDraw) <= 8)
    quit("!DrawingSurface.DrawSurface: 256-colour surfaces cannot be drawn transparently");

  // Draw it transparently
  trans_mode = ((100-translev) * 25) / 10;
  put_sprite_256(0, 0, surfaceToDraw);
  target->FinishedDrawing();
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::DrawImage^6 *** */
void DrawingSurface_DrawImage(ScriptDrawingSurface* sds, int xx, int yy, int slot, int trans, int width, int height)
{
  if ((slot < 0) || (slot >= MAX_SPRITES) || (spriteset[slot] == NULL))
    quit("!DrawingSurface.DrawImage: invalid sprite slot number specified");

  if ((trans < 0) || (trans > 100))
    quit("!DrawingSurface.DrawImage: invalid transparency setting");

  // 100% transparency, don't draw anything
  if (trans == 100)
    return;

  ALW_BITMAP *sourcePic = spriteset[slot];
  bool needToFreeBitmap = false;

  if (width != SCR_NO_VALUE)
  {
    // Resize specified

    if ((width < 1) || (height < 1))
      return;

    sds->MultiplyCoordinates(&width, &height);

    // resize the sprite to the requested size
    block newPic = alw_create_bitmap_ex(alw_bitmap_color_depth(sourcePic), width, height);

    alw_stretch_blit(sourcePic, newPic,
                 0, 0, spritewidth[slot], spriteheight[slot],
                 0, 0, width, height);

    sourcePic = newPic;
    needToFreeBitmap = true;
    update_polled_stuff();
  }

  sds->StartDrawing();
  sds->MultiplyCoordinates(&xx, &yy);

  if (alw_bitmap_color_depth(sourcePic) != alw_bitmap_color_depth(abuf)) {
    debug_log("RawDrawImage: Sprite %d colour depth %d-bit not same as background depth %d-bit", slot, alw_bitmap_color_depth(spriteset[slot]), alw_bitmap_color_depth(abuf));
  }

  if (trans > 0)
  {
    trans_mode = ((100 - trans) * 255) / 100;
  }

  draw_sprite_support_alpha(xx, yy, sourcePic, slot);

  sds->FinishedDrawing();

  if (needToFreeBitmap)
    alw_destroy_bitmap(sourcePic);
}


/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::set_DrawingColor *** */
void DrawingSurface_SetDrawingColor(ScriptDrawingSurface *sds, int newColour) 
{
  sds->currentColourScript = newColour;
  // StartDrawing to set up abuf to set the colour at the appropriate
  // depth for the background
  sds->StartDrawing();
  if (newColour == SCR_COLOR_TRANSPARENT)
  {
    sds->currentColour = alw_bitmap_mask_color(abuf);
  }
  else
  {
    sds->currentColour = get_col8_lookup(newColour);
  }
  sds->FinishedDrawingReadOnly();
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::get_DrawingColor *** */
int DrawingSurface_GetDrawingColor(ScriptDrawingSurface *sds) 
{
  return sds->currentColourScript;
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::set_UseHighResCoordinates *** */
void DrawingSurface_SetUseHighResCoordinates(ScriptDrawingSurface *sds, int highRes) 
{
  sds->highResCoordinates = (highRes) ? 1 : 0;
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::get_UseHighResCoordinates *** */
int DrawingSurface_GetUseHighResCoordinates(ScriptDrawingSurface *sds) 
{
  return sds->highResCoordinates;
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::get_Height *** */
int DrawingSurface_GetHeight(ScriptDrawingSurface *sds) 
{
  sds->StartDrawing();
  int height = BMP_H(abuf);
  sds->FinishedDrawingReadOnly();
  sds->UnMultiplyThickness(&height);
  return height;
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::get_Width *** */
int DrawingSurface_GetWidth(ScriptDrawingSurface *sds) 
{
  sds->StartDrawing();
  int width = BMP_W(abuf);
  sds->FinishedDrawingReadOnly();
  sds->UnMultiplyThickness(&width);
  return width;
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::Clear^1 *** */
void DrawingSurface_Clear(ScriptDrawingSurface *sds, int colour) 
{
  sds->StartDrawing();
  int allegroColor;
  if ((colour == -SCR_NO_VALUE) || (colour == SCR_COLOR_TRANSPARENT))
  {
    allegroColor = alw_bitmap_mask_color(abuf);
  }
  else
  {
    allegroColor = get_col8_lookup(colour);
  }
  alw_clear_to_color(abuf, allegroColor);
  sds->FinishedDrawing();
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::DrawCircle^3 *** */
void DrawingSurface_DrawCircle(ScriptDrawingSurface *sds, int x, int y, int radius) 
{
  sds->MultiplyCoordinates(&x, &y);
  sds->MultiplyThickness(&radius);

  sds->StartDrawing();
  alw_circlefill(abuf, x, y, radius, sds->currentColour);
  sds->FinishedDrawing();
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::DrawRectangle^4 *** */
void DrawingSurface_DrawRectangle(ScriptDrawingSurface *sds, int x1, int y1, int x2, int y2) 
{
  sds->MultiplyCoordinates(&x1, &y1);
  sds->MultiplyCoordinates(&x2, &y2);

  sds->StartDrawing();
  alw_rectfill(abuf, x1,y1,x2,y2, sds->currentColour);
  sds->FinishedDrawing();
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::DrawTriangle^6 *** */
void DrawingSurface_DrawTriangle(ScriptDrawingSurface *sds, int x1, int y1, int x2, int y2, int x3, int y3) 
{
  sds->MultiplyCoordinates(&x1, &y1);
  sds->MultiplyCoordinates(&x2, &y2);
  sds->MultiplyCoordinates(&x3, &y3);

  sds->StartDrawing();
  alw_triangle(abuf, x1,y1,x2,y2,x3,y3, sds->currentColour);
  sds->FinishedDrawing();
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::DrawString^104 *** */
void DrawingSurface_DrawString(ScriptDrawingSurface *sds, int xx, int yy, int font, const char* texx, ...) 
{
  char displbuf[STD_BUFFER_SIZE];
  va_list ap;
  va_start(ap,texx);
  my_sprintf(displbuf,get_translation(texx),ap);
  va_end(ap);
  // don't use wtextcolor because it will do a 16->32 conversion
  textcol = sds->currentColour;

  sds->MultiplyCoordinates(&xx, &yy);
  sds->StartDrawing();
  wtexttransparent(TEXTFG);
  if ((alw_bitmap_color_depth(abuf) <= 8) && (play.raw_color > 255)) {
    wtextcolor(1);
    debug_log ("RawPrint: Attempted to use hi-color on 256-col background");
  }
  wouttext_outline(xx, yy, font, displbuf);
  sds->FinishedDrawing();
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::DrawStringWrapped^6 *** */
void DrawingSurface_DrawStringWrapped(ScriptDrawingSurface *sds, int xx, int yy, int wid, int font, int alignment, const char *msg) {
  int texthit = wgetfontheight(font);
  sds->MultiplyCoordinates(&xx, &yy);
  sds->MultiplyThickness(&wid);

  break_up_text_into_lines(wid, font, (char*)msg);

  textcol = sds->currentColour;
  sds->StartDrawing();

  wtexttransparent(TEXTFG);
  for (int i = 0; i < numlines; i++)
  {
    int drawAtX = xx;

    if (alignment == SCALIGN_CENTRE)
    {
      drawAtX = xx + ((wid / 2) - wgettextwidth(lines[i], font) / 2);
    }
    else if (alignment == SCALIGN_RIGHT)
    {
      drawAtX = (xx + wid) - wgettextwidth(lines[i], font);
    }

    wouttext_outline(drawAtX, yy + texthit*i, font, lines[i]);
  }

  sds->FinishedDrawing();
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::DrawMessageWrapped^5 *** */
void DrawingSurface_DrawMessageWrapped(ScriptDrawingSurface *sds, int xx, int yy, int wid, int font, int msgm)
{
  char displbuf[3000];
  get_message_text(msgm, displbuf);
  // it's probably too late but check anyway
  if (strlen(displbuf) > 2899)
    quit("!RawPrintMessageWrapped: message too long");

  DrawingSurface_DrawStringWrapped(sds, xx, yy, wid, font, SCALIGN_LEFT, displbuf);
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::DrawLine^5 *** */
void DrawingSurface_DrawLine(ScriptDrawingSurface *sds, int fromx, int fromy, int tox, int toy, int thickness) {
  sds->MultiplyCoordinates(&fromx, &fromy);
  sds->MultiplyCoordinates(&tox, &toy);
  sds->MultiplyThickness(&thickness);
  int ii,jj,xx,yy;
  sds->StartDrawing();
  // draw several lines to simulate the thickness
  for (ii = 0; ii < thickness; ii++) 
  {
    xx = (ii - (thickness / 2));
    for (jj = 0; jj < thickness; jj++)
    {
      yy = (jj - (thickness / 2));
      alw_line (abuf, fromx + xx, fromy + yy, tox + xx, toy + yy, sds->currentColour);
    }
  }
  sds->FinishedDrawing();
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::DrawPixel^2 *** */
void DrawingSurface_DrawPixel(ScriptDrawingSurface *sds, int x, int y) {
  sds->MultiplyCoordinates(&x, &y);
  int thickness = 1;
  sds->MultiplyThickness(&thickness);
  int ii,jj;
  sds->StartDrawing();
  // draw several pixels to simulate the thickness
  for (ii = 0; ii < thickness; ii++) 
  {
    for (jj = 0; jj < thickness; jj++)
    {
      alw_putpixel(abuf, x + ii, y + jj, sds->currentColour);
    }
  }
  sds->FinishedDrawing();
}

/* *** SCRIPT SYMBOL: [DrawingSurface] DrawingSurface::GetPixel^2 *** */
int DrawingSurface_GetPixel(ScriptDrawingSurface *sds, int x, int y) {
  sds->MultiplyCoordinates(&x, &y);
  sds->StartDrawing();
  unsigned int rawPixel = alw_getpixel(abuf, x, y);
  unsigned int maskColor = alw_bitmap_mask_color(abuf);
  int colDepth = alw_bitmap_color_depth(abuf);

  if (rawPixel == maskColor)
  {
    rawPixel = SCR_COLOR_TRANSPARENT;
  }
  else if (colDepth > 8)
  {
    int r = alw_getr_depth(colDepth, rawPixel);
    int g = alw_getg_depth(colDepth, rawPixel);
    int b = alw_getb_depth(colDepth, rawPixel);

    rawPixel = Game_GetColorFromRGB(r, g, b);
  }

  sds->FinishedDrawingReadOnly();

  return rawPixel;
}


// Raw screen writing routines - similar to old CapturedStuff
#define RAW_START() block oldabuf=abuf; abuf=thisroom.ebscene[play.bg_frame]; play.raw_modified[play.bg_frame] = 1
#define RAW_END() abuf = oldabuf
// RawSaveScreen: copy the current screen to a backup bitmap
/* *** SCRIPT SYMBOL: [DrawingSurface] RawSaveScreen *** */
void RawSaveScreen () {
  if (raw_saved_screen != NULL)
    wfreeblock(raw_saved_screen);
  block source = thisroom.ebscene[play.bg_frame];
  raw_saved_screen = wallocblock(BMP_W(source), BMP_H(source));
  alw_blit(source, raw_saved_screen, 0, 0, 0, 0, BMP_W(source), BMP_H(source));
}
// RawRestoreScreen: copy backup bitmap back to screen; we
// deliberately don't free the block cos they can multiple restore
// and it gets freed on room exit anyway
/* *** SCRIPT SYMBOL: [DrawingSurface] RawRestoreScreen *** */
void RawRestoreScreen() {
  if (raw_saved_screen == NULL) {
    debug_log("RawRestoreScreen: unable to restore, since the screen hasn't been saved previously.");
    return;
  }
  block deston = thisroom.ebscene[play.bg_frame];
  alw_blit(raw_saved_screen, deston, 0, 0, 0, 0, BMP_W(deston), BMP_H(deston));
  invalidate_screen();
  mark_current_background_dirty();
}
// Restores the backup bitmap, but tints it to the specified level
/* *** SCRIPT SYMBOL: [DrawingSurface] RawRestoreScreenTinted *** */
void RawRestoreScreenTinted(int red, int green, int blue, int opacity) {
  if (raw_saved_screen == NULL) {
    debug_log("RawRestoreScreenTinted: unable to restore, since the screen hasn't been saved previously.");
    return;
  }
  if ((red < 0) || (green < 0) || (blue < 0) ||
      (red > 255) || (green > 255) || (blue > 255) ||
      (opacity < 1) || (opacity > 100))
    quit("!RawRestoreScreenTinted: invalid parameter. R,G,B must be 0-255, opacity 1-100");

  DEBUG_CONSOLE("RawRestoreTinted RGB(%d,%d,%d) %d%%", red, green, blue, opacity);

  block deston = thisroom.ebscene[play.bg_frame];
  tint_image(raw_saved_screen, deston, red, green, blue, opacity);
  invalidate_screen();
  mark_current_background_dirty();
}

/* *** SCRIPT SYMBOL: [DrawingSurface] RawDrawFrameTransparent *** */
void RawDrawFrameTransparent (int frame, int translev) {
  if ((frame < 0) || (frame >= thisroom.num_bscenes) ||
      (translev < 0) || (translev > 99))
    quit("!RawDrawFrameTransparent: invalid parameter (transparency must be 0-99, frame a valid BG frame)");

  if (alw_bitmap_color_depth(thisroom.ebscene[frame]) <= 8)
    quit("!RawDrawFrameTransparent: 256-colour backgrounds not supported");

  if (frame == play.bg_frame)
    quit("!RawDrawFrameTransparent: cannot draw current background onto itself");

  if (translev == 0) {
    // just draw it over the top, no transparency
    alw_blit(thisroom.ebscene[frame], thisroom.ebscene[play.bg_frame], 0, 0, 0, 0, BMP_W(thisroom.ebscene[frame]), BMP_H(thisroom.ebscene[frame]));
    play.raw_modified[play.bg_frame] = 1;
    return;
  }
  // Draw it transparently
  RAW_START();
  trans_mode = ((100-translev) * 25) / 10;
  put_sprite_256 (0, 0, thisroom.ebscene[frame]);
  invalidate_screen();
  mark_current_background_dirty();
  RAW_END();
}

/* *** SCRIPT SYMBOL: [DrawingSurface] RawClearScreen *** */
void RawClear (int clr) {
  play.raw_modified[play.bg_frame] = 1;
  clr = get_col8_lookup(clr);
  alw_clear_to_color (thisroom.ebscene[play.bg_frame], clr);
  invalidate_screen();
  mark_current_background_dirty();
}
/* *** SCRIPT SYMBOL: [DrawingSurface] RawSetColor *** */
void RawSetColor (int clr) {
  push_screen();
  wsetscreen(thisroom.ebscene[play.bg_frame]);
  // set the colour at the appropriate depth for the background
  play.raw_color = get_col8_lookup(clr);
  pop_screen();
}


/* *** SCRIPT SYMBOL: [DrawingSurface] RawPrint *** */
void RawPrint (int xx, int yy, char*texx, ...) {
  char displbuf[STD_BUFFER_SIZE];
  va_list ap;
  va_start(ap,texx);
  my_sprintf(displbuf,get_translation(texx),ap);
  va_end(ap);
  // don't use wtextcolor because it will do a 16->32 conversion
  textcol = play.raw_color;
  RAW_START();
  wtexttransparent(TEXTFG);
  if ((alw_bitmap_color_depth(abuf) <= 8) && (play.raw_color > 255)) {
    wtextcolor(1);
    debug_log ("RawPrint: Attempted to use hi-color on 256-col background");
  }
  multiply_up_coordinates(&xx, &yy);
  wouttext_outline(xx, yy, play.normal_font, displbuf);
  // we must invalidate the entire screen because these are room
  // co-ordinates, not screen co-ords which it works with
  invalidate_screen();
  mark_current_background_dirty();
  RAW_END();
}
/* *** SCRIPT SYMBOL: [DrawingSurface] RawPrintMessageWrapped *** */
void RawPrintMessageWrapped (int xx, int yy, int wid, int font, int msgm) {
  char displbuf[3000];
  int texthit = wgetfontheight(font);
  multiply_up_coordinates(&xx, &yy);
  wid = multiply_up_coordinate(wid);

  get_message_text (msgm, displbuf);
  // it's probably too late but check anyway
  if (strlen(displbuf) > 2899)
    quit("!RawPrintMessageWrapped: message too long");
  break_up_text_into_lines (wid, font, displbuf);

  textcol = play.raw_color;
  RAW_START();
  wtexttransparent(TEXTFG);
  for (int i = 0; i < numlines; i++)
    wouttext_outline(xx, yy + texthit*i, font, lines[i]);
  invalidate_screen();
  mark_current_background_dirty();
  RAW_END();
}

void RawDrawImageCore(int xx, int yy, int slot) {
  if ((slot < 0) || (slot >= MAX_SPRITES) || (spriteset[slot] == NULL))
    quit("!RawDrawImage: invalid sprite slot number specified");
  RAW_START();

  if (alw_bitmap_color_depth(spriteset[slot]) != alw_bitmap_color_depth(abuf)) {
    debug_log("RawDrawImage: Sprite %d colour depth %d-bit not same as background depth %d-bit", slot, alw_bitmap_color_depth(spriteset[slot]), alw_bitmap_color_depth(abuf));
  }

  draw_sprite_support_alpha(xx, yy, spriteset[slot], slot);
  invalidate_screen();
  mark_current_background_dirty();
  RAW_END();
}

/* *** SCRIPT SYMBOL: [DrawingSurface] RawDrawImage *** */
void RawDrawImage(int xx, int yy, int slot) {
  multiply_up_coordinates(&xx, &yy);
  RawDrawImageCore(xx, yy, slot);
}

/* *** SCRIPT SYMBOL: [DrawingSurface] RawDrawImageOffset *** */
void RawDrawImageOffset(int xx, int yy, int slot) {

  if ((current_screen_resolution_multiplier == 1) && (game.default_resolution >= 3)) {
    // running a 640x400 game at 320x200, adjust
    xx /= 2;
    yy /= 2;
  }
  else if ((current_screen_resolution_multiplier > 1) && (game.default_resolution <= 2)) {
    // running a 320x200 game at 640x400, adjust
    xx *= 2;
    yy *= 2;
  }

  RawDrawImageCore(xx, yy, slot);
}

/* *** SCRIPT SYMBOL: [DrawingSurface] RawDrawImageTransparent *** */
void RawDrawImageTransparent(int xx, int yy, int slot, int trans) {
  if ((trans < 0) || (trans > 100))
    quit("!RawDrawImageTransparent: invalid transparency setting");

  // since RawDrawImage uses putsprite256, we can just select the
  // transparency mode and call it
  trans_mode = (trans * 255) / 100;
  RawDrawImage(xx, yy, slot);

  update_polled_stuff();  // this operation can be slow so stop music skipping
}
/* *** SCRIPT SYMBOL: [DrawingSurface] RawDrawImageResized *** */
void RawDrawImageResized(int xx, int yy, int gotSlot, int width, int height) {
  if ((gotSlot < 0) || (gotSlot >= MAX_SPRITES) || (spriteset[gotSlot] == NULL))
    quit("!RawDrawImageResized: invalid sprite slot number specified");
  // very small, don't draw it
  if ((width < 1) || (height < 1))
    return;

  multiply_up_coordinates(&xx, &yy);
  multiply_up_coordinates(&width, &height);

  // resize the sprite to the requested size
  block newPic = alw_create_bitmap_ex(alw_bitmap_color_depth(spriteset[gotSlot]), width, height);

  alw_stretch_blit(spriteset[gotSlot], newPic,
               0, 0, spritewidth[gotSlot], spriteheight[gotSlot],
               0, 0, width, height);

  RAW_START();
  if (alw_bitmap_color_depth(newPic) != alw_bitmap_color_depth(abuf))
    quit("!RawDrawImageResized: image colour depth mismatch: the background image must have the same colour depth as the sprite being drawn");

  put_sprite_256(xx, yy, newPic);
  alw_destroy_bitmap(newPic);
  invalidate_screen();
  mark_current_background_dirty();
  update_polled_stuff();  // this operation can be slow so stop music skipping
  RAW_END();
}
/* *** SCRIPT SYMBOL: [DrawingSurface] RawDrawLine *** */
void RawDrawLine (int fromx, int fromy, int tox, int toy) {
  multiply_up_coordinates(&fromx, &fromy);
  multiply_up_coordinates(&tox, &toy);

  play.raw_modified[play.bg_frame] = 1;
  int ii,jj;
  // draw a line thick enough to look the same at all resolutions
  for (ii = 0; ii < get_fixed_pixel_size(1); ii++) {
    for (jj = 0; jj < get_fixed_pixel_size(1); jj++)
      alw_line (thisroom.ebscene[play.bg_frame], fromx+ii, fromy+jj, tox+ii, toy+jj, play.raw_color);
  }
  invalidate_screen();
  mark_current_background_dirty();
}
/* *** SCRIPT SYMBOL: [DrawingSurface] RawDrawCircle *** */
void RawDrawCircle (int xx, int yy, int rad) {
  multiply_up_coordinates(&xx, &yy);
  rad = multiply_up_coordinate(rad);

  play.raw_modified[play.bg_frame] = 1;
  alw_circlefill (thisroom.ebscene[play.bg_frame], xx, yy, rad, play.raw_color);
  invalidate_screen();
  mark_current_background_dirty();
}
/* *** SCRIPT SYMBOL: [DrawingSurface] RawDrawRectangle *** */
void RawDrawRectangle(int x1, int y1, int x2, int y2) {
  play.raw_modified[play.bg_frame] = 1;
  multiply_up_coordinates(&x1, &y1);
  multiply_up_coordinates_round_up(&x2, &y2);

  alw_rectfill(thisroom.ebscene[play.bg_frame], x1,y1,x2,y2, play.raw_color);
  invalidate_screen();
  mark_current_background_dirty();
}
/* *** SCRIPT SYMBOL: [DrawingSurface] RawDrawTriangle *** */
void RawDrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
  play.raw_modified[play.bg_frame] = 1;
  multiply_up_coordinates(&x1, &y1);
  multiply_up_coordinates(&x2, &y2);
  multiply_up_coordinates(&x3, &y3);

  alw_triangle (thisroom.ebscene[play.bg_frame], x1,y1,x2,y2,x3,y3, play.raw_color);
  invalidate_screen();
  mark_current_background_dirty();
}
/* *** SCRIPT SYMBOL: [Game] RawSetColorRGB *** */
void RawSetColorRGB(int red, int grn, int blu) {
  if ((red < 0) || (red > 255) || (grn < 0) || (grn > 255) ||
      (blu < 0) || (blu > 255))
    quit("!RawSetColorRGB: colour values must be 0-255");

  play.raw_color = alw_makecol_depth(alw_bitmap_color_depth(thisroom.ebscene[play.bg_frame]), red, grn, blu);
}

void register_drawing_surface_script_functions() {
  scAdd_External_Symbol("DrawingSurface::Clear^1", (void *)DrawingSurface_Clear);
  scAdd_External_Symbol("DrawingSurface::CreateCopy^0", (void *)DrawingSurface_CreateCopy);
  scAdd_External_Symbol("DrawingSurface::DrawCircle^3", (void *)DrawingSurface_DrawCircle);
  scAdd_External_Symbol("DrawingSurface::DrawImage^6", (void *)DrawingSurface_DrawImage);
  scAdd_External_Symbol("DrawingSurface::DrawLine^5", (void *)DrawingSurface_DrawLine);
  scAdd_External_Symbol("DrawingSurface::DrawMessageWrapped^5", (void *)DrawingSurface_DrawMessageWrapped);
  scAdd_External_Symbol("DrawingSurface::DrawPixel^2", (void *)DrawingSurface_DrawPixel);
  scAdd_External_Symbol("DrawingSurface::DrawRectangle^4", (void *)DrawingSurface_DrawRectangle);
  scAdd_External_Symbol("DrawingSurface::DrawString^104", (void *)DrawingSurface_DrawString);
  scAdd_External_Symbol("DrawingSurface::DrawStringWrapped^6", (void *)DrawingSurface_DrawStringWrapped);
  scAdd_External_Symbol("DrawingSurface::DrawSurface^2", (void *)DrawingSurface_DrawSurface);
  scAdd_External_Symbol("DrawingSurface::DrawTriangle^6", (void *)DrawingSurface_DrawTriangle);
  scAdd_External_Symbol("DrawingSurface::GetPixel^2", (void *)DrawingSurface_GetPixel);
  scAdd_External_Symbol("DrawingSurface::Release^0", (void *)DrawingSurface_Release);
  scAdd_External_Symbol("DrawingSurface::get_DrawingColor", (void *)DrawingSurface_GetDrawingColor);
  scAdd_External_Symbol("DrawingSurface::set_DrawingColor", (void *)DrawingSurface_SetDrawingColor);
  scAdd_External_Symbol("DrawingSurface::get_Height", (void *)DrawingSurface_GetHeight);
  scAdd_External_Symbol("DrawingSurface::get_UseHighResCoordinates", (void *)DrawingSurface_GetUseHighResCoordinates);
  scAdd_External_Symbol("DrawingSurface::set_UseHighResCoordinates", (void *)DrawingSurface_SetUseHighResCoordinates);
  scAdd_External_Symbol("DrawingSurface::get_Width", (void *)DrawingSurface_GetWidth);
  scAdd_External_Symbol("RawClearScreen", (void *)RawClear);
  scAdd_External_Symbol("RawDrawCircle",(void *)RawDrawCircle);
  scAdd_External_Symbol("RawDrawFrameTransparent",(void *)RawDrawFrameTransparent);
  scAdd_External_Symbol("RawDrawImage", (void *)RawDrawImage);
  scAdd_External_Symbol("RawDrawImageOffset", (void *)RawDrawImageOffset);
  scAdd_External_Symbol("RawDrawImageResized", (void *)RawDrawImageResized);
  scAdd_External_Symbol("RawDrawImageTransparent", (void *)RawDrawImageTransparent);
  scAdd_External_Symbol("RawDrawLine", (void *)RawDrawLine);
  scAdd_External_Symbol("RawDrawRectangle", (void *)RawDrawRectangle);
  scAdd_External_Symbol("RawDrawTriangle", (void *)RawDrawTriangle);
  scAdd_External_Symbol("RawPrint", (void *)RawPrint);
  scAdd_External_Symbol("RawPrintMessageWrapped", (void *)RawPrintMessageWrapped);
  scAdd_External_Symbol("RawRestoreScreen", (void *)RawRestoreScreen);
  scAdd_External_Symbol("RawRestoreScreenTinted", (void *)RawRestoreScreenTinted);
  scAdd_External_Symbol("RawSaveScreen", (void *)RawSaveScreen);
  scAdd_External_Symbol("RawSetColor", (void *)RawSetColor);
  scAdd_External_Symbol("RawSetColorRGB", (void *)RawSetColorRGB);
}

