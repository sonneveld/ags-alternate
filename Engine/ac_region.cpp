#include "ac_region.h"

#include "allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "bmp.h"

void generate_light_table() {
  if (game.color_depth != 1)
    return;
  if (alw_has_color_map())
    return;

  // in 256-col mode, check if we need the light table this room
  for (int cc=0;cc < MAX_REGIONS;cc++) {
    if (thisroom.regionLightLevel[cc] < 0) {
      alw_create_light_table(&maincoltable,palette,0,0,0,NULL);
      alw_set_color_map(&maincoltable);
      break;
    }
  }
}

/* *** SCRIPT SYMBOL: [Region] SetAreaLightLevel *** */
void SetAreaLightLevel(int area, int brightness) {
  if ((area < 0) || (area > MAX_REGIONS))
    quit("!SetAreaLightLevel: invalid region");
  if (brightness < -100) brightness = -100;
  if (brightness > 100) brightness = 100;
  thisroom.regionLightLevel[area] = brightness;
  // disable RGB tint for this area
  thisroom.regionTintLevel[area] &= ~TINT_IS_ENABLED;
  generate_light_table();
  DEBUG_CONSOLE("Region %d light level set to %d", area, brightness);
}

/* *** SCRIPT SYMBOL: [Region] Region::set_LightLevel *** */
void Region_SetLightLevel(ScriptRegion *ssr, int brightness) {
  SetAreaLightLevel(ssr->id, brightness);
}

/* *** SCRIPT SYMBOL: [Region] Region::get_LightLevel *** */
int Region_GetLightLevel(ScriptRegion *ssr) {
  return thisroom.regionLightLevel[ssr->id];
}

/* *** SCRIPT SYMBOL: [Region] SetRegionTint *** */
void SetRegionTint (int area, int red, int green, int blue, int amount) {
  if ((area < 0) || (area > MAX_REGIONS))
    quit("!SetRegionTint: invalid region");

  if ((red < 0) || (red > 255) || (green < 0) || (green > 255) ||
      (blue < 0) || (blue > 255)) {
    quit("!SetRegionTint: RGB values must be 0-255");
  }

  // originally the value was passed as 0
  if (amount == 0)
    amount = 100;

  if ((amount < 1) || (amount > 100))
    quit("!SetRegionTint: amount must be 1-100");

  DEBUG_CONSOLE("Region %d tint set to %d,%d,%d", area, red, green, blue);

  /*red -= 100;
  green -= 100;
  blue -= 100;*/

  unsigned char rred = red;
  unsigned char rgreen = green;
  unsigned char rblue = blue;

  thisroom.regionTintLevel[area] = TINT_IS_ENABLED;
  thisroom.regionTintLevel[area] |= rred & 0x000000ff;
  thisroom.regionTintLevel[area] |= (int(rgreen) << 8) & 0x0000ff00;
  thisroom.regionTintLevel[area] |= (int(rblue) << 16) & 0x00ff0000;
  thisroom.regionLightLevel[area] = amount;
}

/* *** SCRIPT SYMBOL: [Region] Region::get_TintEnabled *** */
int Region_GetTintEnabled(ScriptRegion *srr) {
  if (thisroom.regionTintLevel[srr->id] & TINT_IS_ENABLED)
    return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [Region] Region::get_TintRed *** */
int Region_GetTintRed(ScriptRegion *srr) {
  
  return thisroom.regionTintLevel[srr->id] & 0x000000ff;
}

/* *** SCRIPT SYMBOL: [Region] Region::get_TintGreen *** */
int Region_GetTintGreen(ScriptRegion *srr) {
  
  return (thisroom.regionTintLevel[srr->id] >> 8) & 0x000000ff;
}

/* *** SCRIPT SYMBOL: [Region] Region::get_TintBlue *** */
int Region_GetTintBlue(ScriptRegion *srr) {
  
  return (thisroom.regionTintLevel[srr->id] >> 16) & 0x000000ff;
}

/* *** SCRIPT SYMBOL: [Region] Region::get_TintSaturation *** */
int Region_GetTintSaturation(ScriptRegion *srr) {
  
  return thisroom.regionLightLevel[srr->id];
}

/* *** SCRIPT SYMBOL: [Region] Region::Tint^4 *** */
void Region_Tint(ScriptRegion *srr, int red, int green, int blue, int amount) {
  SetRegionTint(srr->id, red, green, blue, amount);
}



/* *** SCRIPT SYMBOL: [Region] GetRegionAt *** */
int GetRegionAt (int xxx, int yyy) {
  // if the co-ordinates are off the edge of the screen,
  // correct them to be just within
  // this fixes walk-off-screen problems
  xxx = convert_to_low_res(xxx);
  yyy = convert_to_low_res(yyy);

  if (xxx >= BMP_W(thisroom.regions))
	  xxx = BMP_W(thisroom.regions) - 1;
  if (yyy >= BMP_H(thisroom.regions))
	  yyy = BMP_H(thisroom.regions) - 1;
  if (xxx < 0)
	  xxx = 0;
  if (yyy < 0)
	  yyy = 0;

  int hsthere = alw_getpixel (thisroom.regions, xxx, yyy);
  if (hsthere < 0)
    hsthere = 0;

  if (hsthere >= MAX_REGIONS) {
    char tempmsg[300];
    sprintf(tempmsg, "!An invalid pixel was found on the room region mask (colour %d, location: %d, %d)", hsthere, xxx, yyy);
    quit(tempmsg);
  }

  if (croom->region_enabled[hsthere] == 0)
    return 0;
  return hsthere;
}

/* *** SCRIPT SYMBOL: [Region] Region::GetAtRoomXY^2 *** */
ScriptRegion *GetRegionAtLocation(int xx, int yy) {
  int hsnum = GetRegionAt(xx, yy);
  if (hsnum <= 0)
    return &scrRegion[0];
  return &scrRegion[hsnum];
}



/* *** SCRIPT SYMBOL: [Region] RunRegionInteraction *** */
void RunRegionInteraction (int regnum, int mood) {
  if ((regnum < 0) || (regnum >= MAX_REGIONS))
    quit("!RunRegionInteraction: invalid region speicfied");
  if ((mood < 0) || (mood > 2))
    quit("!RunRegionInteraction: invalid event specified");

  // We need a backup, because region interactions can run
  // while another interaction (eg. hotspot) is in a Wait
  // command, and leaving our basename would call the wrong
  // script later on
  char *oldbasename = evblockbasename;
  int   oldblocknum = evblocknum;

  evblockbasename = "region%d";
  evblocknum = regnum;

  if (thisroom.regionScripts != NULL)
  {
    run_interaction_script(thisroom.regionScripts[regnum], mood);
  }
  else
  {
    run_interaction_event(&croom->intrRegion[regnum], mood);
  }

  evblockbasename = oldbasename;
  evblocknum = oldblocknum;
}

/* *** SCRIPT SYMBOL: [Region] Region::RunInteraction^1 *** */
void Region_RunInteraction(ScriptRegion *ssr, int mood) {
  RunRegionInteraction(ssr->id, mood);
}



/* *** SCRIPT SYMBOL: [Region] DisableRegion *** */
void DisableRegion(int hsnum) {
  if ((hsnum < 0) || (hsnum >= MAX_REGIONS))
    quit("!DisableRegion: invalid region specified");

  croom->region_enabled[hsnum] = 0;
  DEBUG_CONSOLE("Region %d disabled", hsnum);
}

/* *** SCRIPT SYMBOL: [Region] EnableRegion *** */
void EnableRegion(int hsnum) {
  if ((hsnum < 0) || (hsnum >= MAX_REGIONS))
    quit("!EnableRegion: invalid region specified");

  croom->region_enabled[hsnum] = 1;
  DEBUG_CONSOLE("Region %d enabled", hsnum);
}

/* *** SCRIPT SYMBOL: [Region] Region::set_Enabled *** */
void Region_SetEnabled(ScriptRegion *ssr, int enable) {
  if (enable)
    EnableRegion(ssr->id);
  else
    DisableRegion(ssr->id);
}

/* *** SCRIPT SYMBOL: [Region] Region::get_Enabled *** */
int Region_GetEnabled(ScriptRegion *ssr) {
  return croom->region_enabled[ssr->id];
}

/* *** SCRIPT SYMBOL: [Region] Region::get_ID *** */
int Region_GetID(ScriptRegion *ssr) {
  return ssr->id;
}



void register_region_script_functions() {
  scAdd_External_Symbol("Region::GetAtRoomXY^2",(void *)GetRegionAtLocation);
  scAdd_External_Symbol("Region::Tint^4", (void*)Region_Tint);
  scAdd_External_Symbol("Region::RunInteraction^1", (void*)Region_RunInteraction);
  scAdd_External_Symbol("Region::get_Enabled", (void*)Region_GetEnabled);
  scAdd_External_Symbol("Region::set_Enabled", (void*)Region_SetEnabled);
  scAdd_External_Symbol("Region::get_ID", (void*)Region_GetID);
  scAdd_External_Symbol("Region::get_LightLevel", (void*)Region_GetLightLevel);
  scAdd_External_Symbol("Region::set_LightLevel", (void*)Region_SetLightLevel);
  scAdd_External_Symbol("Region::get_TintEnabled", (void*)Region_GetTintEnabled);
  scAdd_External_Symbol("Region::get_TintBlue", (void*)Region_GetTintBlue);
  scAdd_External_Symbol("Region::get_TintGreen", (void*)Region_GetTintGreen);
  scAdd_External_Symbol("Region::get_TintRed", (void*)Region_GetTintRed);
  scAdd_External_Symbol("Region::get_TintSaturation", (void*)Region_GetTintSaturation);
  scAdd_External_Symbol("DisableRegion",(void *)DisableRegion);
  scAdd_External_Symbol("EnableRegion",(void *)EnableRegion);
  scAdd_External_Symbol("RunRegionInteraction", (void *)RunRegionInteraction);
  scAdd_External_Symbol("SetAreaLightLevel",(void *)SetAreaLightLevel);
  scAdd_External_Symbol("SetRegionTint",(void *)SetRegionTint);
  scAdd_External_Symbol("GetRegionAt",(void *)GetRegionAt);
}

