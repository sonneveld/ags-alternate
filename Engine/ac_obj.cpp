#include "ac_obj.h"

#include "sdlwrap/allegro.h"

//#include <aastr.h>

#include "ac.h"
#include "ac_context.h"
#include "routefnd.h"
#include "ac_script_room.h"
#include "ac_string.h"
#include "acgfx.h"
#include "sprcache.h"
#include "ali3d.h"
#include "bmp.h"


/* *** SCRIPT SYMBOL: [Object] SetObjectTint *** */
static void SetObjectTint(int obj, int red, int green, int blue, int opacity, int luminance) {
  if ((red < 0) || (green < 0) || (blue < 0) ||
      (red > 255) || (green > 255) || (blue > 255) ||
      (opacity < 0) || (opacity > 100) ||
      (luminance < 0) || (luminance > 100))
    quit("!SetObjectTint: invalid parameter. R,G,B must be 0-255, opacity & luminance 0-100");

  if (!is_valid_object(obj))
    quit("!SetObjectTint: invalid object number specified");

  DEBUG_CONSOLE("Set object %d tint RGB(%d,%d,%d) %d%%", obj, red, green, blue, opacity);

  objs[obj].tint_r = red;
  objs[obj].tint_g = green;
  objs[obj].tint_b = blue;
  objs[obj].tint_level = opacity;
  objs[obj].tint_light = (luminance * 25) / 10;
  objs[obj].flags |= OBJF_HASTINT;
}

/* *** SCRIPT SYMBOL: [Object] Object::Tint^5 *** */
static void Object_Tint(ScriptObject *objj, int red, int green, int blue, int saturation, int luminance) {
  SetObjectTint(objj->id, red, green, blue, saturation, luminance);
}

/* *** SCRIPT SYMBOL: [Object] RemoveObjectTint *** */
static void RemoveObjectTint(int obj) {
  if (!is_valid_object(obj))
    quit("!RemoveObjectTint: invalid object");
  
  if (objs[obj].flags & OBJF_HASTINT) {
    DEBUG_CONSOLE("Un-tint object %d", obj);
    objs[obj].flags &= ~OBJF_HASTINT;
  }
  else {
    debug_log("RemoveObjectTint called but object was not tinted");
  }
}

/* *** SCRIPT SYMBOL: [Object] Object::RemoveTint^0 *** */
static void Object_RemoveTint(ScriptObject *objj) {
  RemoveObjectTint(objj->id);
}


// [object]
// Get the local tint at the specified X & Y co-ordinates, based on
// room regions and SetAmbientTint
// tint_amnt will be set to 0 if there is no tint enabled
// if this is the case, then light_lev holds the light level (0=none)
void get_local_tint(int xpp, int ypp, int nolight,
                    int *tint_amnt, int *tint_r, int *tint_g,
                    int *tint_b, int *tint_lit,
                    int *light_lev) {

  int tint_level = 0, light_level = 0;
  int tint_amount = 0;
  int tint_red = 0;
  int tint_green = 0;
  int tint_blue = 0;
  int tint_light = 255;

  if (nolight == 0) {

    int onRegion = 0;

    if ((play.ground_level_areas_disabled & GLED_EFFECTS) == 0) {
      // check if the player is on a region, to find its
      // light/tint level
      onRegion = GetRegionAt (xpp, ypp);
      if (onRegion == 0) {
        // when walking, he might just be off a walkable area
        onRegion = GetRegionAt (xpp - 3, ypp);
        if (onRegion == 0)
          onRegion = GetRegionAt (xpp + 3, ypp);
        if (onRegion == 0)
          onRegion = GetRegionAt (xpp, ypp - 3);
        if (onRegion == 0)
          onRegion = GetRegionAt (xpp, ypp + 3);
      }
    }

    if ((onRegion > 0) && (onRegion <= MAX_REGIONS)) {
      light_level = thisroom.regionLightLevel[onRegion];
      tint_level = thisroom.regionTintLevel[onRegion];
    }
    else if (onRegion <= 0) {
      light_level = thisroom.regionLightLevel[0];
      tint_level = thisroom.regionTintLevel[0];
    }
    if ((game.color_depth == 1) || ((tint_level & 0x00ffffff) == 0) ||
        ((tint_level & TINT_IS_ENABLED) == 0))
      tint_level = 0;

    if (tint_level) {
      tint_red = (unsigned char)(tint_level & 0x000ff);
      tint_green = (unsigned char)((tint_level >> 8) & 0x000ff);
      tint_blue = (unsigned char)((tint_level >> 16) & 0x000ff);
      tint_amount = light_level;
      // older versions of the editor had a bug - work around it
      if (tint_amount < 0)
        tint_amount = 50;
      /*red = ((red + 100) * 25) / 20;
      grn = ((grn + 100) * 25) / 20;
      blu = ((blu + 100) * 25) / 20;*/
    }

    if (play.rtint_level > 0) {
      // override with room tint
      tint_level = 1;
      tint_red = play.rtint_red;
      tint_green = play.rtint_green;
      tint_blue = play.rtint_blue;
      tint_amount = play.rtint_level;
      tint_light = play.rtint_light;
    }
  }

  // copy to output parameters
  *tint_amnt = tint_amount;
  *tint_r = tint_red;
  *tint_g = tint_green;
  *tint_b = tint_blue;
  *tint_lit = tint_light;
  if (light_lev)
    *light_lev = light_level;
}

// [Object]
// Applies the specified RGB Tint or Light Level to the actsps
// sprite indexed with actspsindex
void apply_tint_or_light(int actspsindex, int light_level,
                         int tint_amount, int tint_red, int tint_green,
                         int tint_blue, int tint_light, int coldept,
                         block blitFrom) {

  // In a 256-colour game, we cannot do tinting or lightening
  // (but we can do darkening, if light_level < 0)
  if (game.color_depth == 1) {
    if ((light_level > 0) || (tint_amount != 0))
      return;
  }

  // we can only do tint/light if the colour depths match
  if (final_col_dep == alw_bitmap_color_depth(actsps[actspsindex])) {
    block oldwas;
    // if the caller supplied a source bitmap, blit from it
    // (used as a speed optimisation where possible)
    if (blitFrom) 
      oldwas = blitFrom;
    // otherwise, make a new target bmp
    else {
      oldwas = actsps[actspsindex];
      actsps[actspsindex] = alw_create_bitmap_ex(coldept, BMP_W(oldwas), BMP_H(oldwas));
    }

    if (tint_amount) {
      // It is an RGB tint

      tint_image (oldwas, actsps[actspsindex], tint_red, tint_green, tint_blue, tint_amount, tint_light);
    }
    else {
      // the RGB values passed to set_trans_blender decide whether it will darken
      // or lighten sprites ( <128=darken, >128=lighten). The parameter passed
      // to draw_lit_sprite defines how much it will be darkened/lightened by.
      int lit_amnt;
      alw_clear_to_color(actsps[actspsindex], alw_bitmap_mask_color(actsps[actspsindex]));
      // It's a light level, not a tint
      if (game.color_depth == 1) {
        // 256-col
        lit_amnt = (250 - ((-light_level) * 5)/2);
      }
      else {
        // hi-color
        if (light_level < 0)
          set_my_trans_blender(8,8,8,0);
        else
          set_my_trans_blender(248,248,248,0);
        lit_amnt = abs(light_level) * 2;
      }

      alw_draw_lit_sprite(actsps[actspsindex], oldwas, 0, 0, lit_amnt);
    }

    if (oldwas != blitFrom)
      wfreeblock(oldwas);

  }
  else if (blitFrom) {
    // sprite colour depth != game colour depth, so don't try and tint
    // but we do need to do something, so copy the source
    alw_blit(blitFrom, actsps[actspsindex], 0, 0, 0, 0, BMP_W(actsps[actspsindex]), BMP_H(actsps[actspsindex]));
  }

}

// [Object]
// Draws the specified 'sppic' sprite onto actsps[useindx] at the
// specified width and height, and flips the sprite if necessary.
// Returns 1 if something was drawn to actsps; returns 0 if no
// scaling or stretching was required, in which case nothing was done
int scale_and_flip_sprite(int useindx, int coldept, int zoom_level,
                          int sppic, int newwidth, int newheight,
                          int isMirrored) {

  int actsps_used = 1;

  // create and blank out the new sprite
  actsps[useindx] = recycle_bitmap(actsps[useindx], coldept, newwidth, newheight);
  alw_clear_to_color(actsps[useindx],alw_bitmap_mask_color(actsps[useindx]));

  if (zoom_level != 100) {
    // Scaled character

    our_eip = 334;

    // Ensure that anti-aliasing routines have a palette to
    // use for mapping while faded out
    if (in_new_room)
      alw_select_palette (palette);

    
    if (isMirrored) {
      block tempspr = alw_create_bitmap_ex (coldept, newwidth, newheight);
      alw_clear_to_color (tempspr, alw_bitmap_mask_color(actsps[useindx]));
      if ((IS_ANTIALIAS_SPRITES) && ((game.spriteflags[sppic] & SPF_ALPHACHANNEL) == 0))
        aa_stretch_sprite (tempspr, spriteset[sppic], 0, 0, newwidth, newheight);
      else
        alw_stretch_sprite (tempspr, spriteset[sppic], 0, 0, newwidth, newheight);
      alw_draw_sprite_h_flip (actsps[useindx], tempspr, 0, 0);
      wfreeblock (tempspr);
    }
    else if ((IS_ANTIALIAS_SPRITES) && ((game.spriteflags[sppic] & SPF_ALPHACHANNEL) == 0))
      aa_stretch_sprite(actsps[useindx],spriteset[sppic],0,0,newwidth,newheight);
    else
      alw_stretch_sprite(actsps[useindx],spriteset[sppic],0,0,newwidth,newheight);

/*  AASTR2 version of code (doesn't work properly, gives black borders)
    if (IS_ANTIALIAS_SPRITES) {
      int aa_mode = AA_MASKED; 
      if (game.spriteflags[sppic] & SPF_ALPHACHANNEL)
        aa_mode |= AA_ALPHA | AA_RAW_ALPHA;
      if (isMirrored)
        aa_mode |= AA_HFLIP;

      aa_set_mode(aa_mode);
      aa_stretch_sprite(actsps[useindx],spriteset[sppic],0,0,newwidth,newheight);
    }
    else if (isMirrored) {
      block tempspr = alw_create_bitmap_ex (coldept, newwidth, newheight);
      alw_clear_to_color (tempspr, alw_bitmap_mask_color(actsps[useindx]));
      alw_stretch_sprite (tempspr, spriteset[sppic], 0, 0, newwidth, newheight);
      alw_draw_sprite_h_flip (actsps[useindx], tempspr, 0, 0);
      wfreeblock (tempspr);
    }
    else
      alw_stretch_sprite(actsps[useindx],spriteset[sppic],0,0,newwidth,newheight);
*/
    if (in_new_room)
      alw_unselect_palette();

  } 
  else {
    // Not a scaled character, draw at normal size

    our_eip = 339;

    if (isMirrored)
      alw_draw_sprite_h_flip (actsps[useindx], spriteset[sppic], 0, 0);
    else
      actsps_used = 0;
      //alw_blit (spriteset[sppic], actsps[useindx], 0, 0, 0, 0, BMP_W(actsps[useindx]), BMP_H(actsps[useindx]));
  }

  return actsps_used;
}

// [Object]
int get_area_scaling (int onarea, int xx, int yy) {

  int zoom_level = 100;
  xx = convert_to_low_res(xx);
  yy = convert_to_low_res(yy);

  if ((onarea >= 0) && (onarea <= MAX_WALK_AREAS) &&
      (thisroom.walk_area_zoom2[onarea] != NOT_VECTOR_SCALED)) {
    // We have vector scaling!
    // In case the character is off the screen, limit the Y co-ordinate
    // to within the area range (otherwise we get silly zoom levels
    // that cause Out Of Memory crashes)
    if (yy > thisroom.walk_area_bottom[onarea])
      yy = thisroom.walk_area_bottom[onarea];
    if (yy < thisroom.walk_area_top[onarea])
      yy = thisroom.walk_area_top[onarea];
    // Work it all out without having to use floats
    // Percent = ((y - top) * 100) / (areabottom - areatop)
    // Zoom level = ((max - min) * Percent) / 100
    int percent = ((yy - thisroom.walk_area_top[onarea]) * 100)
          / (thisroom.walk_area_bottom[onarea] - thisroom.walk_area_top[onarea]);

    zoom_level = ((thisroom.walk_area_zoom2[onarea] - thisroom.walk_area_zoom[onarea]) * (percent)) / 100 + thisroom.walk_area_zoom[onarea];
    zoom_level += 100;
  }
  else if ((onarea >= 0) & (onarea <= MAX_WALK_AREAS))
    zoom_level = thisroom.walk_area_zoom[onarea] + 100;

  if (zoom_level == 0)
    zoom_level = 100;

  return zoom_level;
}

//[Object]
void scale_sprite_size(int sppic, int zoom_level, int *newwidth, int *newheight) {
  newwidth[0] = (spritewidth[sppic] * zoom_level) / 100;
  newheight[0] = (spriteheight[sppic] * zoom_level) / 100;
  if (newwidth[0] < 1)
    newwidth[0] = 1;
  if (newheight[0] < 1)
    newheight[0] = 1;
}

// [Object]
// create the actsps[aa] image with the object drawn correctly
// returns 1 if nothing at all has changed and actsps is still
// intact from last time; 0 otherwise
static int construct_object_gfx(int aa, int *drawnWidth, int *drawnHeight, bool alwaysUseSoftware) {
  int useindx = aa;
  bool hardwareAccelerated = gfxDriver->HasAcceleratedStretchAndFlip();

  if (alwaysUseSoftware)
    hardwareAccelerated = false;

  if (spriteset[objs[aa].num] == NULL)
    quitprintf("There was an error drawing object %d. Its current sprite, %d, is invalid.", aa, objs[aa].num);

  int coldept = alw_bitmap_color_depth(spriteset[objs[aa].num]);
  int sprwidth = spritewidth[objs[aa].num];
  int sprheight = spriteheight[objs[aa].num];

  int tint_red, tint_green, tint_blue;
  int tint_level, tint_light, light_level;
  int zoom_level = 100;

  // calculate the zoom level
  if (objs[aa].flags & OBJF_USEROOMSCALING) {
    int onarea = get_walkable_area_at_location(objs[aa].x, objs[aa].y);

    if ((onarea <= 0) && (thisroom.walk_area_zoom[0] == 0)) {
      // just off the edge of an area -- use the scaling we had
      // while on the area
      zoom_level = objs[aa].last_zoom;
    }
    else
      zoom_level = get_area_scaling(onarea, objs[aa].x, objs[aa].y);

    if (zoom_level != 100)
      scale_sprite_size(objs[aa].num, zoom_level, &sprwidth, &sprheight);
    
  }
  // save the zoom level for next time
  objs[aa].last_zoom = zoom_level;

  // save width/height into parameters if requested
  if (drawnWidth)
    *drawnWidth = sprwidth;
  if (drawnHeight)
    *drawnHeight = sprheight;

  objs[aa].last_width = sprwidth;
  objs[aa].last_height = sprheight;

  if (objs[aa].flags & OBJF_HASTINT) {
    // object specific tint, use it
    tint_red = objs[aa].tint_r;
    tint_green = objs[aa].tint_g;
    tint_blue = objs[aa].tint_b;
    tint_level = objs[aa].tint_level;
    tint_light = objs[aa].tint_light;
    light_level = 0;
  }
  else {
    // get the ambient or region tint
    int ignoreRegionTints = 1;
    if (objs[aa].flags & OBJF_USEREGIONTINTS)
      ignoreRegionTints = 0;

    get_local_tint(objs[aa].x, objs[aa].y, ignoreRegionTints,
      &tint_level, &tint_red, &tint_green, &tint_blue,
      &tint_light, &light_level);
  }

  // check whether the image should be flipped
  int isMirrored = 0;
  if ( (objs[aa].view >= 0) &&
       (views[objs[aa].view].loops[objs[aa].loop].frames[objs[aa].frame].pic == objs[aa].num) &&
      ((views[objs[aa].view].loops[objs[aa].loop].frames[objs[aa].frame].flags & VFLG_FLIPSPRITE) != 0)) {
    isMirrored = 1;
  }

  if ((objcache[aa].image != NULL) &&
      (objcache[aa].sppic == objs[aa].num) &&
      (walkBehindMethod != DrawOverCharSprite) &&
      (actsps[useindx] != NULL) &&
      (hardwareAccelerated))
  {
    // HW acceleration
    objcache[aa].tintamntwas = tint_level;
    objcache[aa].tintredwas = tint_red;
    objcache[aa].tintgrnwas = tint_green;
    objcache[aa].tintbluwas = tint_blue;
    objcache[aa].tintlightwas = tint_light;
    objcache[aa].lightlevwas = light_level;
    objcache[aa].zoomWas = zoom_level;
    objcache[aa].mirroredWas = isMirrored;

    return 1;
  }

  if ((!hardwareAccelerated) && (gfxDriver->HasAcceleratedStretchAndFlip()))
  {
    // They want to draw it in software mode with the D3D driver,
    // so force a redraw
    objcache[aa].sppic = -389538;
  }

  // If we have the image cached, use it
  if ((objcache[aa].image != NULL) &&
      (objcache[aa].sppic == objs[aa].num) &&
      (objcache[aa].tintamntwas == tint_level) &&
      (objcache[aa].tintlightwas == tint_light) &&
      (objcache[aa].tintredwas == tint_red) &&
      (objcache[aa].tintgrnwas == tint_green) &&
      (objcache[aa].tintbluwas == tint_blue) &&
      (objcache[aa].lightlevwas == light_level) &&
      (objcache[aa].zoomWas == zoom_level) &&
      (objcache[aa].mirroredWas == isMirrored)) {
    // the image is the same, we can use it cached!
    if ((walkBehindMethod != DrawOverCharSprite) &&
        (actsps[useindx] != NULL))
      return 1;
    // Check if the X & Y co-ords are the same, too -- if so, there
    // is scope for further optimisations
    if ((objcache[aa].xwas == objs[aa].x) &&
        (objcache[aa].ywas == objs[aa].y) &&
        (actsps[useindx] != NULL) &&
        (walk_behind_baselines_changed == 0))
      return 1;
    actsps[useindx] = recycle_bitmap(actsps[useindx], coldept, sprwidth, sprheight);
    alw_blit(objcache[aa].image, actsps[useindx], 0, 0, 0, 0, BMP_W(objcache[aa].image), BMP_H(objcache[aa].image));
    return 0;
  }

  // Not cached, so draw the image

  int actspsUsed = 0;
  if (!hardwareAccelerated)
  {
    // draw the base sprite, scaled and flipped as appropriate
    actspsUsed = scale_and_flip_sprite(useindx, coldept, zoom_level,
                         objs[aa].num, sprwidth, sprheight, isMirrored);
  }
  else
  {
    // ensure actsps exists
    actsps[useindx] = recycle_bitmap(actsps[useindx], coldept, spritewidth[objs[aa].num], spriteheight[objs[aa].num]);
  }

  // direct read from source bitmap, where possible
  block comeFrom = NULL;
  if (!actspsUsed)
    comeFrom = spriteset[objs[aa].num];

  // apply tints or lightenings where appropriate, else just copy
  // the source bitmap
  if (((tint_level > 0) || (light_level != 0)) &&
      (!hardwareAccelerated))
  {
    apply_tint_or_light(useindx, light_level, tint_level, tint_red,
                        tint_green, tint_blue, tint_light, coldept,
                        comeFrom);
  }
  else if (!actspsUsed)
    alw_blit(spriteset[objs[aa].num],actsps[useindx],0,0,0,0,spritewidth[objs[aa].num],spriteheight[objs[aa].num]);

  // Re-use the bitmap if it's the same size
  objcache[aa].image = recycle_bitmap(objcache[aa].image, coldept, sprwidth, sprheight);

  // Create the cached image and store it
  alw_blit(actsps[useindx], objcache[aa].image, 0, 0, 0, 0, sprwidth, sprheight);

  objcache[aa].sppic = objs[aa].num;
  objcache[aa].tintamntwas = tint_level;
  objcache[aa].tintredwas = tint_red;
  objcache[aa].tintgrnwas = tint_green;
  objcache[aa].tintbluwas = tint_blue;
  objcache[aa].tintlightwas = tint_light;
  objcache[aa].lightlevwas = light_level;
  objcache[aa].zoomWas = zoom_level;
  objcache[aa].mirroredWas = isMirrored;
  return 0;
}

// [Object]
// This is only called from draw_screen_background, but it's seperated
// to help with profiling the program
void prepare_objects_for_drawing() {
  int aa,atxp,atyp,useindx;
  our_eip=32;

  for (aa=0;aa<croom->numobj;aa++) {
    if (objs[aa].on != 1) continue;
    // offscreen, don't draw
    if ((objs[aa].x >= thisroom.width) || (objs[aa].y < 1))
      continue;

    useindx = aa;
    int tehHeight;

    int actspsIntact = construct_object_gfx(aa, NULL, &tehHeight, false);

    // update the cache for next time
    objcache[aa].xwas = objs[aa].x;
    objcache[aa].ywas = objs[aa].y;

    atxp = multiply_up_coordinate(objs[aa].x) - offsetx;
    atyp = (multiply_up_coordinate(objs[aa].y) - tehHeight) - offsety;

    int usebasel = objs[aa].get_baseline();

    if (objs[aa].flags & OBJF_NOWALKBEHINDS) {
      // ignore walk-behinds, do nothing
      if (walkBehindMethod == DrawAsSeparateSprite)
      {
        usebasel += thisroom.height;
      }
    }
    else if (walkBehindMethod == DrawAsSeparateCharSprite) 
    {
      sort_out_char_sprite_walk_behind(useindx, atxp+offsetx, atyp+offsety, usebasel, objs[aa].last_zoom, objs[aa].last_width, objs[aa].last_height);
    }
    else if ((!actspsIntact) && (walkBehindMethod == DrawOverCharSprite))
    {
      sort_out_walk_behinds(actsps[useindx],atxp+offsetx,atyp+offsety,usebasel);
    }

    if ((!actspsIntact) || (actspsbmp[useindx] == NULL))
    {
      bool hasAlpha = (game.spriteflags[objs[aa].num] & SPF_ALPHACHANNEL) != 0;

      if (actspsbmp[useindx] != NULL)
        gfxDriver->DestroyDDB(actspsbmp[useindx]);
      actspsbmp[useindx] = gfxDriver->CreateDDBFromBitmap(actsps[useindx], hasAlpha);
    }

    if (gfxDriver->HasAcceleratedStretchAndFlip())
    {
      actspsbmp[useindx]->SetFlippedLeftRight(objcache[aa].mirroredWas != 0);
      actspsbmp[useindx]->SetStretch(objs[aa].last_width, objs[aa].last_height);
      actspsbmp[useindx]->SetTint(objcache[aa].tintredwas, objcache[aa].tintgrnwas, objcache[aa].tintbluwas, (objcache[aa].tintamntwas * 256) / 100);

      if (objcache[aa].tintamntwas > 0)
      {
        if (objcache[aa].tintlightwas == 0)  // luminance of 0 -- pass 1 to enable
          actspsbmp[useindx]->SetLightLevel(1);
        else if (objcache[aa].tintlightwas < 250)
          actspsbmp[useindx]->SetLightLevel(objcache[aa].tintlightwas);
        else
          actspsbmp[useindx]->SetLightLevel(0);
      }
      else if (objcache[aa].lightlevwas != 0)
        actspsbmp[useindx]->SetLightLevel((objcache[aa].lightlevwas * 25) / 10 + 256);
      else
        actspsbmp[useindx]->SetLightLevel(0);
    }

    add_to_sprite_list(actspsbmp[useindx],atxp,atyp,usebasel,objs[aa].transparent,objs[aa].num);
  }

}






/* *** SCRIPT SYMBOL: [Object] SetObjectView *** */
void SetObjectView(int obn,int vii) {
  if (!is_valid_object(obn)) quit("!SetObjectView: invalid object number specified");
  DEBUG_CONSOLE("Object %d set to view %d", obn, vii);
  if ((vii < 1) || (vii > game.numviews)) {
    char buffer[150];
    sprintf (buffer, "!SetObjectView: invalid view number (You said %d, max is %d)", vii, game.numviews);
    quit(buffer);
  }
  vii--;

  objs[obn].view=vii;
  objs[obn].frame=0;
  if (objs[obn].loop >= views[vii].numLoops)
    objs[obn].loop=0;
  objs[obn].cycling=0;
  objs[obn].num = views[vii].loops[0].frames[0].pic;
  }

/* *** SCRIPT SYMBOL: [Object] SetObjectFrame *** */
static void SetObjectFrame(int obn,int viw,int lop,int fra) {
  if (!is_valid_object(obn)) quit("!SetObjectFrame: invalid object number specified");
  viw--;
  if (viw>=game.numviews) quit("!SetObjectFrame: invalid view number used");
  if (lop>=views[viw].numLoops) quit("!SetObjectFrame: invalid loop number used");
  objs[obn].view=viw;
  if (fra >= 0)
    objs[obn].frame=fra;
  if (lop >= 0)
    objs[obn].loop=lop;

  if (objs[obn].loop >= views[viw].numLoops)
    objs[obn].loop = 0;
  if (objs[obn].frame >= views[viw].loops[objs[obn].loop].numFrames)
    objs[obn].frame = 0;

  if (views[viw].loops[objs[obn].loop].numFrames == 0) 
    quit("!SetObjectFrame: specified loop has no frames");

  objs[obn].cycling=0;
  objs[obn].num = views[viw].loops[objs[obn].loop].frames[objs[obn].frame].pic;
  CheckViewFrame(viw, objs[obn].loop, objs[obn].frame);
}

/* *** SCRIPT SYMBOL: [Object] Object::SetView^3 *** */
static void Object_SetView(ScriptObject *objj, int view, int loop, int frame) {
  SetObjectFrame(objj->id, view, loop, frame);
}

// pass trans=0 for fully solid, trans=100 for fully transparent
/* *** SCRIPT SYMBOL: [Object] SetObjectTransparency *** */
static void SetObjectTransparency(int obn,int trans) {
  if (!is_valid_object(obn)) quit("!SetObjectTransparent: invalid object number specified");
  if ((trans < 0) || (trans > 100)) quit("!SetObjectTransparent: transparency value must be between 0 and 100");
  if (trans == 0)
    objs[obn].transparent=0;
  else if (trans == 100)
    objs[obn].transparent = 255;
  else
    objs[obn].transparent=((100-trans) * 25) / 10;
}

/* *** SCRIPT SYMBOL: [Object] Object::set_Transparency *** */
static void Object_SetTransparency(ScriptObject *objj, int trans) {
  SetObjectTransparency(objj->id, trans);
}

/* *** SCRIPT SYMBOL: [Object] Object::get_Transparency *** */
static int Object_GetTransparency(ScriptObject *objj) {
  if (!is_valid_object(objj->id))
    quit("!Object.Transparent: invalid object number specified");

  if (objj->obj->transparent == 0)
    return 0;
  if (objj->obj->transparent == 255)
    return 100;

  return 100 - ((objj->obj->transparent * 10) / 25);

}

/* *** SCRIPT SYMBOL: [Object] SetObjectBaseline *** */
static void SetObjectBaseline (int obn, int basel) {
  if (!is_valid_object(obn)) quit("!SetObjectBaseline: invalid object number specified");
  // baseline has changed, invalidate the cache
  if (objs[obn].baseline != basel) {
    objcache[obn].ywas = -9999;
    objs[obn].baseline = basel;
  }
}

/* *** SCRIPT SYMBOL: [Object] Object::set_Baseline *** */
static void Object_SetBaseline(ScriptObject *objj, int basel) {
  SetObjectBaseline(objj->id, basel);
}

/* *** SCRIPT SYMBOL: [Object] GetObjectBaseline *** */
static int GetObjectBaseline(int obn) {
  if (!is_valid_object(obn)) quit("!GetObjectBaseline: invalid object number specified");

  if (objs[obn].baseline < 1)
    return 0;
  
  return objs[obn].baseline;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_Baseline *** */
static int Object_GetBaseline(ScriptObject *objj) {
  return GetObjectBaseline(objj->id);
}


/* *** SCRIPT SYMBOL: [Object] AnimateObjectEx *** */
static void AnimateObjectEx(int obn,int loopn,int spdd,int rept, int direction, int blocking) {
  if (obn>=MANOBJNUM) {
    scAnimateCharacter(obn - 100,loopn,spdd,rept);
    return;
  }
  if (!is_valid_object(obn))
    quit("!AnimateObject: invalid object number specified");
  if (objs[obn].view < 0)
    quit("!AnimateObject: object has not been assigned a view");
  if (loopn >= views[objs[obn].view].numLoops)
    quit("!AnimateObject: invalid loop number specified");
  if ((direction < 0) || (direction > 1))
    quit("!AnimateObjectEx: invalid direction");
  if ((rept < 0) || (rept > 2))
    quit("!AnimateObjectEx: invalid repeat value");
  if (views[objs[obn].view].loops[loopn].numFrames < 1)
    quit("!AnimateObject: no frames in the specified view loop");

  DEBUG_CONSOLE("Obj %d start anim view %d loop %d, speed %d, repeat %d", obn, objs[obn].view+1, loopn, spdd, rept);

  objs[obn].cycling = rept+1 + (direction * 10);
  objs[obn].loop=loopn;
  if (direction == 0)
    objs[obn].frame = 0;
  else {
    objs[obn].frame = views[objs[obn].view].loops[loopn].numFrames - 1;
  }

  objs[obn].overall_speed=spdd;
  objs[obn].wait = spdd+views[objs[obn].view].loops[loopn].frames[objs[obn].frame].speed;
  objs[obn].num = views[objs[obn].view].loops[loopn].frames[objs[obn].frame].pic;
  CheckViewFrame (objs[obn].view, loopn, objs[obn].frame);

  if (blocking)
    do_main_cycle(UNTIL_CHARIS0,(int)&objs[obn].cycling);
}

/* *** SCRIPT SYMBOL: [Object] Object::Animate^5 *** */
static void Object_Animate(ScriptObject *objj, int loop, int delay, int repeat, int blocking, int direction) {
  if (direction == FORWARDS)
    direction = 0;
  else if (direction == BACKWARDS)
    direction = 1;
  else
    quit("!Object.Animate: Invalid DIRECTION parameter");

  if ((blocking == BLOCKING) || (blocking == 1))
    blocking = 1;
  else if ((blocking == IN_BACKGROUND) || (blocking == 0))
    blocking = 0;
  else
    quit("!Object.Animate: Invalid BLOCKING parameter");

  AnimateObjectEx(objj->id, loop, delay, repeat, direction, blocking);
}

/* *** SCRIPT SYMBOL: [Object] Object::StopAnimating^0 *** */
static void Object_StopAnimating(ScriptObject *objj) {
  if (!is_valid_object(objj->id))
    quit("!Object.StopAnimating: invalid object number");

  if (objs[objj->id].cycling) {
    objs[objj->id].cycling = 0;
    objs[objj->id].wait = 0;
  }
}

/* *** SCRIPT SYMBOL: [Object] AnimateObject *** */
void AnimateObject(int obn,int loopn,int spdd,int rept) {
  AnimateObjectEx (obn, loopn, spdd, rept, 0, 0);
}

/* *** SCRIPT SYMBOL: [Object] MergeObject *** */
void MergeObject(int obn) {
  if (!is_valid_object(obn)) quit("!MergeObject: invalid object specified");
  int theHeight;

  construct_object_gfx(obn, NULL, &theHeight, true);

  block oldabuf = abuf;
  abuf = thisroom.ebscene[play.bg_frame];
  if (alw_bitmap_color_depth(abuf) != alw_bitmap_color_depth(actsps[obn]))
    quit("!MergeObject: unable to merge object due to color depth differences");

  int xpos = multiply_up_coordinate(objs[obn].x);
  int ypos = (multiply_up_coordinate(objs[obn].y) - theHeight);

  draw_sprite_support_alpha(xpos, ypos, actsps[obn], objs[obn].num);
  invalidate_screen();
  mark_current_background_dirty();

  abuf = oldabuf;
  // mark the sprite as merged
  objs[obn].on = 2;
  DEBUG_CONSOLE("Object %d merged into background", obn);
}

/* *** SCRIPT SYMBOL: [Object] Object::MergeIntoBackground^0 *** */
static void Object_MergeIntoBackground(ScriptObject *objj) {
  MergeObject(objj->id);
}

/* *** SCRIPT SYMBOL: [Object] StopObjectMoving *** */
static void StopObjectMoving(int objj) {
  if (!is_valid_object(objj))
    quit("!StopObjectMoving: invalid object number");
  objs[objj].moving = 0;

  DEBUG_CONSOLE("Object %d stop moving", objj);
}

/* *** SCRIPT SYMBOL: [Object] Object::StopMoving^0 *** */
static void Object_StopMoving(ScriptObject *objj) {
  StopObjectMoving(objj->id);
}

/* *** SCRIPT SYMBOL: [Object] ObjectOff *** */
void ObjectOff(int obn) {
  if (!is_valid_object(obn)) quit("!ObjectOff: invalid object specified");
  // don't change it if on == 2 (merged)
  if (objs[obn].on == 1) {
    objs[obn].on = 0;
    DEBUG_CONSOLE("Object %d turned off", obn);
    StopObjectMoving(obn);
  }
}

/* *** SCRIPT SYMBOL: [Object] ObjectOn *** */
void ObjectOn(int obn) {
  if (!is_valid_object(obn)) quit("!ObjectOn: invalid object specified");
  if (objs[obn].on == 0) {
    objs[obn].on = 1;
    DEBUG_CONSOLE("Object %d turned on", obn);
  }
}

/* *** SCRIPT SYMBOL: [Object] Object::set_Visible *** */
static void Object_SetVisible(ScriptObject *objj, int onoroff) {
  if (onoroff)
    ObjectOn(objj->id);
  else
    ObjectOff(objj->id);
}

/* *** SCRIPT SYMBOL: [Object] IsObjectOn *** */
static int IsObjectOn (int objj) {
  if (!is_valid_object(objj)) quit("!IsObjectOn: invalid object number");
  
  // ==1 is on, ==2 is merged into background
  if (objs[objj].on == 1)
    return 1;

  return 0;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_View *** */
static int Object_GetView(ScriptObject *objj) {
  if (objj->obj->view < 0)
    return 0;
  return objj->obj->view + 1;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_Loop *** */
static int Object_GetLoop(ScriptObject *objj) {
  if (objj->obj->view < 0)
    return 0;
  return objj->obj->loop;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_Frame *** */
static int Object_GetFrame(ScriptObject *objj) {
  if (objj->obj->view < 0)
    return 0;
  return objj->obj->frame;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_Visible *** */
static int Object_GetVisible(ScriptObject *objj) {
  return IsObjectOn(objj->id);
}

/* *** SCRIPT SYMBOL: [Object] SetObjectGraphic *** */
static void SetObjectGraphic(int obn,int slott) {
  if (!is_valid_object(obn)) quit("!SetObjectGraphic: invalid object specified");

  if (objs[obn].num != slott) {
    objs[obn].num = slott;
    DEBUG_CONSOLE("Object %d graphic changed to slot %d", obn, slott);
  }
  objs[obn].cycling=0;
  objs[obn].frame = 0;
  objs[obn].loop = 0;
  objs[obn].view = -1;
}

/* *** SCRIPT SYMBOL: [Object] Object::set_Graphic *** */
static void Object_SetGraphic(ScriptObject *objj, int slott) {
  SetObjectGraphic(objj->id, slott);
}

/* *** SCRIPT SYMBOL: [Object] GetObjectGraphic *** */
static int GetObjectGraphic(int obn) {
  if (!is_valid_object(obn)) quit("!GetObjectGraphic: invalid object specified");
  return objs[obn].num;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_Graphic *** */
static int Object_GetGraphic(ScriptObject *objj) {
  return GetObjectGraphic(objj->id);
}

/* *** SCRIPT SYMBOL: [Object] AreObjectsColliding *** */
static int AreObjectsColliding(int obj1,int obj2) {
  if ((!is_valid_object(obj1)) | (!is_valid_object(obj2)))
    quit("!AreObjectsColliding: invalid object specified");

  return (AreThingsOverlapping(obj1 + OVERLAPPING_OBJECT, obj2 + OVERLAPPING_OBJECT)) ? 1 : 0;
}

/* *** SCRIPT SYMBOL: [Object] Object::IsCollidingWithObject^1 *** */
static int Object_IsCollidingWithObject(ScriptObject *objj, ScriptObject *obj2) {
  return AreObjectsColliding(objj->id, obj2->id);
}


/* *** SCRIPT SYMBOL: [Object] GetObjectAt *** */
int GetObjectAt(int xx,int yy) {
  int aa,bestshotyp=-1,bestshotwas=-1;
  // translate screen co-ordinates to room co-ordinates
  xx += divide_down_coordinate(offsetx);
  yy += divide_down_coordinate(offsety);
  // Iterate through all objects in the room
  for (aa=0;aa<croom->numobj;aa++) {
    if (objs[aa].on != 1) continue;
    if (objs[aa].flags & OBJF_NOINTERACT)
      continue;
    int xxx=objs[aa].x,yyy=objs[aa].y;
    int isflipped = 0;
    int spWidth = divide_down_coordinate(objs[aa].get_width());
    int spHeight = divide_down_coordinate(objs[aa].get_height());
    if (objs[aa].view >= 0)
      isflipped = views[objs[aa].view].loops[objs[aa].loop].frames[objs[aa].frame].flags & VFLG_FLIPSPRITE;

    block theImage = GetObjectImage(aa, &isflipped);

    if (is_pos_in_sprite(xx, yy, xxx, yyy - spHeight, theImage,
                         spWidth, spHeight, isflipped) == FALSE)
      continue;

    int usebasel = objs[aa].get_baseline();   
    if (usebasel < bestshotyp) continue;

    bestshotwas = aa;
    bestshotyp = usebasel;
  }
  obj_lowest_yp = bestshotyp;
  return bestshotwas;
}

/* *** SCRIPT SYMBOL: [Object] Object::GetAtScreenXY^2 *** */
static ScriptObject *GetObjectAtLocation(int xx, int yy) {
  int hsnum = GetObjectAt(xx, yy);
  if (hsnum < 0)
    return NULL;
  return &scrObj[hsnum];
}

/* *** SCRIPT SYMBOL: [Object] RunObjectInteraction *** */
static void RunObjectInteraction (int aa, int mood) {
  if (!is_valid_object(aa))
    quit("!RunObjectInteraction: invalid object number for current room");
  int passon=-1,cdata=-1;
  if (mood==MODE_LOOK) passon=0;
  else if (mood==MODE_HAND) passon=1;
  else if (mood==MODE_TALK) passon=2;
  else if (mood==MODE_PICKUP) passon=5;
  else if (mood==MODE_CUSTOM1) passon = 6;
  else if (mood==MODE_CUSTOM2) passon = 7;
  else if (mood==MODE_USE) { passon=3;
    cdata=playerchar->activeinv;
    play.usedinv=cdata; }
  evblockbasename="object%d"; evblocknum=aa;

  if (thisroom.objectScripts != NULL) 
  {
    if (passon>=0) 
    {
      if (run_interaction_script(thisroom.objectScripts[aa], passon, 4, (passon == 3)))
        return;
    }
    run_interaction_script(thisroom.objectScripts[aa], 4);  // any click on obj
  }
  else
  {
    if (passon>=0) {
      if (run_interaction_event(&croom->intrObject[aa],passon, 4, (passon == 3)))
        return;
    }
    run_interaction_event(&croom->intrObject[aa],4);  // any click on obj
  }
}

/* *** SCRIPT SYMBOL: [Object] Object::RunInteraction^1 *** */
static void Object_RunInteraction(ScriptObject *objj, int mode) {
  RunObjectInteraction(objj->id, mode);
}

// X and Y co-ordinates must be in 320x200 format
int check_click_on_object(int xx,int yy,int mood) {
  int aa = GetObjectAt(xx - divide_down_coordinate(offsetx), yy - divide_down_coordinate(offsety));
  if (aa < 0) return 0;
  RunObjectInteraction(aa, mood);
  return 1;
  }


/* *** SCRIPT SYMBOL: [Object] GetObjectX *** */
static int GetObjectX (int objj) {
  if (!is_valid_object(objj)) quit("!GetObjectX: invalid object number");
  return objs[objj].x;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_X *** */
static int Object_GetX(ScriptObject *objj) {
  return GetObjectX(objj->id);
}

/* *** SCRIPT SYMBOL: [Object] GetObjectY *** */
static int GetObjectY (int objj) {
  if (!is_valid_object(objj)) quit("!GetObjectY: invalid object number");
  return objs[objj].y;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_Y *** */
static int Object_GetY(ScriptObject *objj) {
  return GetObjectY(objj->id);
}

/* *** SCRIPT SYMBOL: [Object] IsObjectAnimating *** */
static int IsObjectAnimating(int objj) {
  if (!is_valid_object(objj)) quit("!IsObjectAnimating: invalid object number");
  return (objs[objj].cycling != 0) ? 1 : 0;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_Animating *** */
static int Object_GetAnimating(ScriptObject *objj) {
  return IsObjectAnimating(objj->id);
}

/* *** SCRIPT SYMBOL: [Object] IsObjectMoving *** */
static int IsObjectMoving(int objj) {
  if (!is_valid_object(objj)) quit("!IsObjectMoving: invalid object number");
  return (objs[objj].moving > 0) ? 1 : 0;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_Moving *** */
static int Object_GetMoving(ScriptObject *objj) {
  return IsObjectMoving(objj->id);
}


/* *** SCRIPT SYMBOL: [Object] SetObjectPosition *** */
static void SetObjectPosition(int objj, int tox, int toy) {
  if (!is_valid_object(objj))
    quit("!SetObjectPosition: invalid object number");

  if (objs[objj].moving > 0)
    quit("!Object.SetPosition: cannot set position while object is moving");

  objs[objj].x = tox;
  objs[objj].y = toy;
}

/* *** SCRIPT SYMBOL: [Object] Object::SetPosition^2 *** */
static void Object_SetPosition(ScriptObject *objj, int xx, int yy) {
  SetObjectPosition(objj->id, xx, yy);
}

/* *** SCRIPT SYMBOL: [Object] Object::set_X *** */
static void Object_SetX(ScriptObject *objj, int xx) {
  SetObjectPosition(objj->id, xx, objj->obj->y);
}

/* *** SCRIPT SYMBOL: [Object] Object::set_Y *** */
static void Object_SetY(ScriptObject *objj, int yy) {
  SetObjectPosition(objj->id, objj->obj->x, yy);
}


void convert_move_path_to_high_res(MoveList *ml)
{
  ml->fromx *= current_screen_resolution_multiplier;
  ml->fromy *= current_screen_resolution_multiplier;
  ml->lastx *= current_screen_resolution_multiplier;
  ml->lasty *= current_screen_resolution_multiplier;

  for (int i = 0; i < ml->numstage; i++)
  {
    short lowPart = (ml->pos[i] & 0x0000ffff) * current_screen_resolution_multiplier;
    short highPart = ((ml->pos[i] >> 16) & 0x0000ffff) * current_screen_resolution_multiplier;
    ml->pos[i] = (highPart << 16) | lowPart;

    ml->xpermove[i] *= current_screen_resolution_multiplier;
    ml->ypermove[i] *= current_screen_resolution_multiplier;
  }
}

static void move_object(int objj,int tox,int toy,int spee,int ignwal) {

  if (!is_valid_object(objj))
    quit("!MoveObject: invalid object number");

  DEBUG_CONSOLE("Object %d start move to %d,%d", objj, tox, toy);

  int objX = convert_to_low_res(objs[objj].x);
  int objY = convert_to_low_res(objs[objj].y);
  tox = convert_to_low_res(tox);
  toy = convert_to_low_res(toy);

  set_route_move_speed(spee, spee);
  alw_set_color_depth(8);
  int mslot=find_route(objX, objY, tox, toy, prepare_walkable_areas(-1), objj+1, 1, ignwal);
  alw_set_color_depth(final_col_dep);
  if (mslot>0) {
    objs[objj].moving = mslot;
    mls[mslot].direct = ignwal;

    if ((game.options[OPT_NATIVECOORDINATES] != 0) &&
        (game.default_resolution > 2))
    {
      convert_move_path_to_high_res(&mls[mslot]);
    }
  }
}


/* *** SCRIPT SYMBOL: [Object] GetObjectProperty *** */
static int GetObjectProperty (int hss, const char *property) {
  if (!is_valid_object(hss))
    quit("!GetObjectProperty: invalid object");
  return get_int_property (&thisroom.objProps[hss], property);
}
/* *** SCRIPT SYMBOL: [Object] Object::GetProperty^1 *** */
static int Object_GetProperty (ScriptObject *objj, const char *property) {
  return GetObjectProperty(objj->id, property);
}
/* *** SCRIPT SYMBOL: [Object] GetObjectPropertyText *** */
static void GetObjectPropertyText (int item, const char *property, char *bufer) {
  get_text_property (&thisroom.objProps[item], property, bufer);
}
/* *** SCRIPT SYMBOL: [Object] Object::GetPropertyText^2 *** */
static void Object_GetPropertyText(ScriptObject *objj, const char *property, char *bufer) {
  GetObjectPropertyText(objj->id, property, bufer);
}
/* *** SCRIPT SYMBOL: [Object] Object::GetTextProperty^1 *** */
static const char* Object_GetTextProperty(ScriptObject *objj, const char *property) {
  return get_text_property_dynamic_string(&thisroom.objProps[objj->id], property);
}


/* *** SCRIPT SYMBOL: [Object] GetObjectName *** */
static void GetObjectName(int obj, char *buffer) {
  VALIDATE_STRING(buffer);
  if (!is_valid_object(obj))
    quit("!GetObjectName: invalid object number");

  strcpy(buffer, get_translation(thisroom.objectnames[obj]));
}

/* *** SCRIPT SYMBOL: [Object] Object::GetName^1 *** */
static void Object_GetName(ScriptObject *objj, char *buffer) {
  GetObjectName(objj->id, buffer);
}

/* *** SCRIPT SYMBOL: [Object] Object::get_Name *** */
static const char* Object_GetName_New(ScriptObject *objj) {
  if (!is_valid_object(objj->id))
    quit("!Object.Name: invalid object number");

  return CreateNewScriptString(get_translation(thisroom.objectnames[objj->id]));
}


/* *** SCRIPT SYMBOL: [Object] MoveObject *** */
void MoveObject(int objj,int xx,int yy,int spp) {
  move_object(objj,xx,yy,spp,0);
  }
/* *** SCRIPT SYMBOL: [Object] MoveObjectDirect *** */
static void MoveObjectDirect(int objj,int xx,int yy,int spp) {
  move_object(objj,xx,yy,spp,1);
  }

/* *** SCRIPT SYMBOL: [Object] Object::Move^5 *** */
static void Object_Move(ScriptObject *objj, int x, int y, int speed, int blocking, int direct) {
  if ((direct == ANYWHERE) || (direct == 1))
    direct = 1;
  else if ((direct == WALKABLE_AREAS) || (direct == 0))
    direct = 0;
  else
    quit("Object.Move: invalid DIRECT parameter");

  move_object(objj->id, x, y, speed, direct);

  if ((blocking == BLOCKING) || (blocking == 1))
    do_main_cycle(UNTIL_SHORTIS0,(int)&objj->obj->moving);
  else if ((blocking != IN_BACKGROUND) && (blocking != 0))
    quit("Object.Move: invalid BLOCKING paramter");
}


/* *** SCRIPT SYMBOL: [Object] SetObjectClickable *** */
static void SetObjectClickable (int cha, int clik) {
  if (!is_valid_object(cha))
    quit("!SetObjectClickable: Invalid object specified");
  objs[cha].flags&=~OBJF_NOINTERACT;
  if (clik == 0)
    objs[cha].flags|=OBJF_NOINTERACT;
  }

/* *** SCRIPT SYMBOL: [Object] Object::set_Clickable *** */
static void Object_SetClickable(ScriptObject *objj, int clik) {
  SetObjectClickable(objj->id, clik);
}

/* *** SCRIPT SYMBOL: [Object] Object::get_Clickable *** */
static int Object_GetClickable(ScriptObject *objj) {
  if (!is_valid_object(objj->id))
    quit("!Object.Clickable: Invalid object specified");

  if (objj->obj->flags & OBJF_NOINTERACT)
    return 0;
  return 1;
}

/* *** SCRIPT SYMBOL: [Object] Object::set_IgnoreScaling *** */
static void Object_SetIgnoreScaling(ScriptObject *objj, int newval) {
  if (!is_valid_object(objj->id))
    quit("!Object.IgnoreScaling: Invalid object specified");

  objj->obj->flags &= ~OBJF_USEROOMSCALING;
  if (!newval)
    objj->obj->flags |= OBJF_USEROOMSCALING;

  // clear the cache
  objcache[objj->id].ywas = -9999;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_IgnoreScaling *** */
static int Object_GetIgnoreScaling(ScriptObject *objj) {
  if (!is_valid_object(objj->id))
    quit("!Object.IgnoreScaling: Invalid object specified");

  if (objj->obj->flags & OBJF_USEROOMSCALING)
    return 0;
  return 1;
}

/* *** SCRIPT SYMBOL: [Object] Object::set_Solid *** */
static void Object_SetSolid(ScriptObject *objj, int solid) {
  objj->obj->flags &= ~OBJF_SOLID;
  if (solid)
    objj->obj->flags |= OBJF_SOLID;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_Solid *** */
static int Object_GetSolid(ScriptObject *objj) {
  if (objj->obj->flags & OBJF_SOLID)
    return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [Object] Object::set_BlockingWidth *** */
static void Object_SetBlockingWidth(ScriptObject *objj, int bwid) {
  objj->obj->blocking_width = bwid;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_BlockingWidth *** */
static int Object_GetBlockingWidth(ScriptObject *objj) {
  return objj->obj->blocking_width;
}

/* *** SCRIPT SYMBOL: [Object] Object::set_BlockingHeight *** */
static void Object_SetBlockingHeight(ScriptObject *objj, int bhit) {
  objj->obj->blocking_height = bhit;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_BlockingHeight *** */
static int Object_GetBlockingHeight(ScriptObject *objj) {
  return objj->obj->blocking_height;
}

/* *** SCRIPT SYMBOL: [Object] SetObjectIgnoreWalkbehinds *** */
static void SetObjectIgnoreWalkbehinds (int cha, int clik) {
  if (!is_valid_object(cha))
    quit("!SetObjectIgnoreWalkbehinds: Invalid object specified");
  objs[cha].flags&=~OBJF_NOWALKBEHINDS;
  if (clik)
    objs[cha].flags|=OBJF_NOWALKBEHINDS;
  // clear the cache
  objcache[cha].ywas = -9999;
}

/* *** SCRIPT SYMBOL: [Object] Object::get_ID *** */
static int Object_GetID(ScriptObject *objj) {
  return objj->id;
}

/* *** SCRIPT SYMBOL: [Object] Object::set_IgnoreWalkbehinds *** */
static void Object_SetIgnoreWalkbehinds(ScriptObject *chaa, int clik) {
  SetObjectIgnoreWalkbehinds(chaa->id, clik);
}

/* *** SCRIPT SYMBOL: [Object] Object::get_IgnoreWalkbehinds *** */
static int Object_GetIgnoreWalkbehinds(ScriptObject *chaa) {
  if (!is_valid_object(chaa->id))
    quit("!Object.IgnoreWalkbehinds: Invalid object specified");

  if (chaa->obj->flags & OBJF_NOWALKBEHINDS)
    return 1;
  return 0;
}




void register_object_script_functions() {

  scAdd_External_Symbol("Object::Animate^5", (void *)Object_Animate);
  scAdd_External_Symbol("Object::IsCollidingWithObject^1", (void *)Object_IsCollidingWithObject);
  scAdd_External_Symbol("Object::GetName^1", (void *)Object_GetName);
  scAdd_External_Symbol("Object::GetProperty^1", (void *)Object_GetProperty);
  scAdd_External_Symbol("Object::GetPropertyText^2", (void *)Object_GetPropertyText);
  scAdd_External_Symbol("Object::GetTextProperty^1",(void *)Object_GetTextProperty);
  scAdd_External_Symbol("Object::MergeIntoBackground^0", (void *)Object_MergeIntoBackground);
  scAdd_External_Symbol("Object::Move^5", (void *)Object_Move);
  scAdd_External_Symbol("Object::RemoveTint^0", (void *)Object_RemoveTint);
  scAdd_External_Symbol("Object::RunInteraction^1", (void *)Object_RunInteraction);
  scAdd_External_Symbol("Object::SetPosition^2", (void *)Object_SetPosition);
  scAdd_External_Symbol("Object::SetView^3", (void *)Object_SetView);
  scAdd_External_Symbol("Object::StopAnimating^0", (void *)Object_StopAnimating);
  scAdd_External_Symbol("Object::StopMoving^0", (void *)Object_StopMoving);
  scAdd_External_Symbol("Object::Tint^5", (void *)Object_Tint);

  // static
  scAdd_External_Symbol("Object::GetAtScreenXY^2", (void *)GetObjectAtLocation);

  scAdd_External_Symbol("Object::get_Animating", (void *)Object_GetAnimating);
  scAdd_External_Symbol("Object::get_Baseline", (void *)Object_GetBaseline);
  scAdd_External_Symbol("Object::set_Baseline", (void *)Object_SetBaseline);
  scAdd_External_Symbol("Object::get_BlockingHeight",(void *)Object_GetBlockingHeight);
  scAdd_External_Symbol("Object::set_BlockingHeight",(void *)Object_SetBlockingHeight);
  scAdd_External_Symbol("Object::get_BlockingWidth",(void *)Object_GetBlockingWidth);
  scAdd_External_Symbol("Object::set_BlockingWidth",(void *)Object_SetBlockingWidth);
  scAdd_External_Symbol("Object::get_Clickable", (void *)Object_GetClickable);
  scAdd_External_Symbol("Object::set_Clickable", (void *)Object_SetClickable);
  scAdd_External_Symbol("Object::get_Frame", (void *)Object_GetFrame);
  scAdd_External_Symbol("Object::get_Graphic", (void *)Object_GetGraphic);
  scAdd_External_Symbol("Object::set_Graphic", (void *)Object_SetGraphic);
  scAdd_External_Symbol("Object::get_ID", (void *)Object_GetID);
  scAdd_External_Symbol("Object::get_IgnoreScaling", (void *)Object_GetIgnoreScaling);
  scAdd_External_Symbol("Object::set_IgnoreScaling", (void *)Object_SetIgnoreScaling);
  scAdd_External_Symbol("Object::get_IgnoreWalkbehinds", (void *)Object_GetIgnoreWalkbehinds);
  scAdd_External_Symbol("Object::set_IgnoreWalkbehinds", (void *)Object_SetIgnoreWalkbehinds);
  scAdd_External_Symbol("Object::get_Loop", (void *)Object_GetLoop);
  scAdd_External_Symbol("Object::get_Moving", (void *)Object_GetMoving);
  scAdd_External_Symbol("Object::get_Name", (void *)Object_GetName_New);
  scAdd_External_Symbol("Object::get_Solid", (void *)Object_GetSolid);
  scAdd_External_Symbol("Object::set_Solid", (void *)Object_SetSolid);
  scAdd_External_Symbol("Object::get_Transparency", (void *)Object_GetTransparency);
  scAdd_External_Symbol("Object::set_Transparency", (void *)Object_SetTransparency);
  scAdd_External_Symbol("Object::get_View", (void *)Object_GetView);
  scAdd_External_Symbol("Object::get_Visible", (void *)Object_GetVisible);
  scAdd_External_Symbol("Object::set_Visible", (void *)Object_SetVisible);
  scAdd_External_Symbol("Object::get_X", (void *)Object_GetX);
  scAdd_External_Symbol("Object::set_X", (void *)Object_SetX);
  scAdd_External_Symbol("Object::get_Y", (void *)Object_GetY);
  scAdd_External_Symbol("Object::set_Y", (void *)Object_SetY);


  scAdd_External_Symbol("AnimateObjectEx",(void *)AnimateObjectEx);
  scAdd_External_Symbol("AreObjectsColliding",(void *)AreObjectsColliding);
  scAdd_External_Symbol("GetObjectAt",(void *)GetObjectAt);
  scAdd_External_Symbol("GetObjectBaseline",(void *)GetObjectBaseline);
  scAdd_External_Symbol("GetObjectGraphic",(void *)GetObjectGraphic);
  scAdd_External_Symbol("GetObjectName",(void *)GetObjectName);
  scAdd_External_Symbol("GetObjectProperty",(void *)GetObjectProperty);
  scAdd_External_Symbol("GetObjectPropertyText",(void *)GetObjectPropertyText);
  scAdd_External_Symbol("GetObjectX",(void *)GetObjectX);
  scAdd_External_Symbol("GetObjectY",(void *)GetObjectY);
  scAdd_External_Symbol("IsObjectAnimating",(void *)IsObjectAnimating);
  scAdd_External_Symbol("IsObjectMoving",(void *)IsObjectMoving);
  scAdd_External_Symbol("IsObjectOn",(void *)IsObjectOn);
  scAdd_External_Symbol("MoveObject",(void *)MoveObject);
  scAdd_External_Symbol("MoveObjectDirect",(void *)MoveObjectDirect);
  scAdd_External_Symbol("ObjectOff",(void *)ObjectOff);
  scAdd_External_Symbol("ObjectOn",(void *)ObjectOn);
  scAdd_External_Symbol("RemoveObjectTint",(void *)RemoveObjectTint);
  scAdd_External_Symbol("RunObjectInteraction", (void *)RunObjectInteraction);
  scAdd_External_Symbol("SetObjectBaseline",(void *)SetObjectBaseline);
  scAdd_External_Symbol("SetObjectClickable",(void *)SetObjectClickable);
  scAdd_External_Symbol("SetObjectFrame",(void *)SetObjectFrame);
  scAdd_External_Symbol("SetObjectGraphic",(void *)SetObjectGraphic);
  scAdd_External_Symbol("SetObjectIgnoreWalkbehinds",(void *)SetObjectIgnoreWalkbehinds);
  scAdd_External_Symbol("SetObjectPosition",(void *)SetObjectPosition);
  scAdd_External_Symbol("SetObjectTint",(void *)SetObjectTint);
  scAdd_External_Symbol("SetObjectTransparency",(void *)SetObjectTransparency);
  scAdd_External_Symbol("SetObjectView",(void *)SetObjectView);
  scAdd_External_Symbol("StopObjectMoving",(void *)StopObjectMoving);
  scAdd_External_Symbol("MergeObject",(void *)MergeObject);
  scAdd_External_Symbol("AnimateObject",(void *)AnimateObject);

}