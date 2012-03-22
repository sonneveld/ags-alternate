#include "ac_hotspot.h"

#include "ac.h"
#include "ac_context.h"
#include "acgui.h"
#include "ac_script_gui.h"
#include "ac_string.h"
#include "ac_obj.h"

/* *** SCRIPT SYMBOL: [Game] MoveCharacterToHotspot *** */
void MoveCharacterToHotspot(int chaa,int hotsp) {
  if ((hotsp<0) || (hotsp>=MAX_HOTSPOTS))
    quit("!MovecharacterToHotspot: invalid hotspot");
  if (thisroom.hswalkto[hotsp].x<1) return;
  walk_character(chaa,thisroom.hswalkto[hotsp].x,thisroom.hswalkto[hotsp].y,0, true);
  do_main_cycle(UNTIL_MOVEEND,(int)&game.chars[chaa].walking);
  }

/* *** SCRIPT SYMBOL: [Hotspot] RunHotspotInteraction *** */
void RunHotspotInteraction (int hotspothere, int mood) {

  int passon=-1,cdata=-1;
  if (mood==MODE_TALK) passon=4;
  else if (mood==MODE_WALK) passon=0;
  else if (mood==MODE_LOOK) passon=1;
  else if (mood==MODE_HAND) passon=2;
  else if (mood==MODE_PICKUP) passon=7;
  else if (mood==MODE_CUSTOM1) passon = 8;
  else if (mood==MODE_CUSTOM2) passon = 9;
  else if (mood==MODE_USE) { passon=3;
    cdata=playerchar->activeinv;
    play.usedinv=cdata;
  }

  if ((game.options[OPT_WALKONLOOK]==0) & (mood==MODE_LOOK)) ;
  else if (play.auto_use_walkto_points == 0) ;
  else if ((mood!=MODE_WALK) && (play.check_interaction_only == 0))
    MoveCharacterToHotspot(game.playercharacter,hotspothere);

  // can't use the setevent functions because this ProcessClick is only
  // executed once in a eventlist
  char *oldbasename = evblockbasename;
  int   oldblocknum = evblocknum;

  evblockbasename="hotspot%d";
  evblocknum=hotspothere;

  if (thisroom.hotspotScripts != NULL) 
  {
    if (passon>=0)
      run_interaction_script(thisroom.hotspotScripts[hotspothere], passon, 5, (passon == 3));
    run_interaction_script(thisroom.hotspotScripts[hotspothere], 5);  // any click on hotspot
  }
  else
  {
    if (passon>=0) {
      if (run_interaction_event(&croom->intrHotspot[hotspothere],passon, 5, (passon == 3))) {
        evblockbasename = oldbasename;
        evblocknum = oldblocknum;
        return;
      }
    }
    // run the 'any click on hs' event
    run_interaction_event(&croom->intrHotspot[hotspothere],5);
  }

  evblockbasename = oldbasename;
  evblocknum = oldblocknum;
}

/* *** SCRIPT SYMBOL: [Hotspot] Hotspot::RunInteraction^1 *** */
void Hotspot_RunInteraction (ScriptHotspot *hss, int mood) {
  RunHotspotInteraction(hss->id, mood);
}

/* *** SCRIPT SYMBOL: [Hotspot] GetHotspotProperty *** */
int GetHotspotProperty (int hss, const char *property) {
  return get_int_property (&thisroom.hsProps[hss], property);
}
/* *** SCRIPT SYMBOL: [Hotspot] Hotspot::GetProperty^1 *** */
int Hotspot_GetProperty (ScriptHotspot *hss, const char *property) {
  return get_int_property (&thisroom.hsProps[hss->id], property);
}


/* *** SCRIPT SYMBOL: [Hotspot] GetHotspotPropertyText *** */
void GetHotspotPropertyText (int item, const char *property, char *bufer) {
  get_text_property (&thisroom.hsProps[item], property, bufer);
}
/* *** SCRIPT SYMBOL: [Hotspot] Hotspot::GetPropertyText^2 *** */
void Hotspot_GetPropertyText (ScriptHotspot *hss, const char *property, char *bufer) {
  get_text_property (&thisroom.hsProps[hss->id], property, bufer);
}
/* *** SCRIPT SYMBOL: [Hotspot] Hotspot::GetTextProperty^1 *** */
const char* Hotspot_GetTextProperty(ScriptHotspot *hss, const char *property) {
  return get_text_property_dynamic_string(&thisroom.hsProps[hss->id], property);
}


/* *** SCRIPT SYMBOL: [Hotspot] DisableHotspot *** */
void DisableHotspot(int hsnum) {
  if ((hsnum<1) | (hsnum>=MAX_HOTSPOTS))
    quit("!DisableHotspot: invalid hotspot specified");
  croom->hotspot_enabled[hsnum]=0;
  DEBUG_CONSOLE("Hotspot %d disabled", hsnum);
}

/* *** SCRIPT SYMBOL: [Hotspot] EnableHotspot *** */
void EnableHotspot(int hsnum) {
  if ((hsnum<1) | (hsnum>=MAX_HOTSPOTS))
    quit("!EnableHotspot: invalid hotspot specified");
  croom->hotspot_enabled[hsnum]=1;
  DEBUG_CONSOLE("Hotspot %d re-enabled", hsnum);
}

/* *** SCRIPT SYMBOL: [Hotspot] Hotspot::set_Enabled *** */
void Hotspot_SetEnabled(ScriptHotspot *hss, int newval) {
  if (newval)
    EnableHotspot(hss->id);
  else
    DisableHotspot(hss->id);
}

/* *** SCRIPT SYMBOL: [Hotspot] Hotspot::get_Enabled *** */
int Hotspot_GetEnabled(ScriptHotspot *hss) {
  return croom->hotspot_enabled[hss->id];
}

/* *** SCRIPT SYMBOL: [Hotspot] Hotspot::get_ID *** */
int Hotspot_GetID(ScriptHotspot *hss) {
  return hss->id;
}


/* *** SCRIPT SYMBOL: [Hotspot] GetHotspotPointX *** */
int GetHotspotPointX (int hotspot) {
  if ((hotspot < 0) || (hotspot >= MAX_HOTSPOTS))
    quit("!GetHotspotPointX: invalid hotspot");

  if (thisroom.hswalkto[hotspot].x < 1)
    return -1;

  return thisroom.hswalkto[hotspot].x;
}

/* *** SCRIPT SYMBOL: [Hotspot] Hotspot::get_WalkToX *** */
int Hotspot_GetWalkToX(ScriptHotspot *hss) {
  return GetHotspotPointX(hss->id);
}

/* *** SCRIPT SYMBOL: [Hotspot] GetHotspotPointY *** */
int GetHotspotPointY (int hotspot) {
  if ((hotspot < 0) || (hotspot >= MAX_HOTSPOTS))
    quit("!GetHotspotPointY: invalid hotspot");

  if (thisroom.hswalkto[hotspot].x < 1)
    return -1;

  return thisroom.hswalkto[hotspot].y;
}

/* *** SCRIPT SYMBOL: [Hotspot] Hotspot::get_WalkToY *** */
int Hotspot_GetWalkToY(ScriptHotspot *hss) {
  return GetHotspotPointY(hss->id);
}

int get_hotspot_at(int xpp,int ypp) {
  int onhs=getpixel(thisroom.lookat, convert_to_low_res(xpp), convert_to_low_res(ypp));
  if (onhs<0) return 0;
  if (croom->hotspot_enabled[onhs]==0) return 0;
  return onhs;
}

/* *** SCRIPT SYMBOL: [Hotspot] GetHotspotAt *** */
int GetHotspotAt(int xxx,int yyy) {
  xxx += divide_down_coordinate(offsetx);
  yyy += divide_down_coordinate(offsety);
  if ((xxx>=thisroom.width) | (xxx<0) | (yyy<0) | (yyy>=thisroom.height))
    return 0;
  return get_hotspot_at(xxx,yyy);
}

/* *** SCRIPT SYMBOL: [Hotspot] Hotspot::GetAtScreenXY^2 *** */
ScriptHotspot *GetHotspotAtLocation(int xx, int yy) {
  int hsnum = GetHotspotAt(xx, yy);
  if (hsnum <= 0)
    return &scrHotspot[0];
  return &scrHotspot[hsnum];
}

// allowHotspot0 defines whether Hotspot 0 returns LOCTYPE_HOTSPOT
// or whether it returns 0
int __GetLocationType(int xxx,int yyy, int allowHotspot0) {
  getloctype_index = 0;
  // If it's not in ProcessClick, then return 0 when over a GUI
  if ((GetGUIAt(xxx, yyy) >= 0) && (getloctype_throughgui == 0))
    return 0;

  getloctype_throughgui = 0;

  xxx += divide_down_coordinate(offsetx);
  yyy += divide_down_coordinate(offsety);
  if ((xxx>=thisroom.width) | (xxx<0) | (yyy<0) | (yyy>=thisroom.height))
    return 0;

  // check characters, objects and walkbehinds, work out which is
  // foremost visible to the player
  int charat = is_pos_on_character(xxx,yyy);
  int hsat = get_hotspot_at(xxx,yyy);
  int objat = GetObjectAt(xxx - divide_down_coordinate(offsetx), yyy - divide_down_coordinate(offsety));

  multiply_up_coordinates(&xxx, &yyy);

  int wbat = getpixel(thisroom.object, xxx, yyy);

  if (wbat <= 0) wbat = 0;
  else wbat = croom->walkbehind_base[wbat];

  int winner = 0;
  // if it's an Ignore Walkbehinds object, then ignore the walkbehind
  if ((objat >= 0) && ((objs[objat].flags & OBJF_NOWALKBEHINDS) != 0))
    wbat = 0;
  if ((charat >= 0) && ((game.chars[charat].flags & CHF_NOWALKBEHINDS) != 0))
    wbat = 0;
  
  if ((charat >= 0) && (objat >= 0)) {
    if ((wbat > obj_lowest_yp) && (wbat > char_lowest_yp))
      winner = LOCTYPE_HOTSPOT;
    else if (obj_lowest_yp > char_lowest_yp)
      winner = LOCTYPE_OBJ;
    else
      winner = LOCTYPE_CHAR;
  }
  else if (charat >= 0) {
    if (wbat > char_lowest_yp)
      winner = LOCTYPE_HOTSPOT;
    else
      winner = LOCTYPE_CHAR;
  }
  else if (objat >= 0) {
    if (wbat > obj_lowest_yp)
      winner = LOCTYPE_HOTSPOT;
    else
      winner = LOCTYPE_OBJ;
  }

  if (winner == 0) {
    if (hsat >= 0)
      winner = LOCTYPE_HOTSPOT;
  }

  if ((winner == LOCTYPE_HOTSPOT) && (!allowHotspot0) && (hsat == 0))
    winner = 0;

  if (winner == LOCTYPE_HOTSPOT)
    getloctype_index = hsat;
  else if (winner == LOCTYPE_CHAR)
    getloctype_index = charat;
  else if (winner == LOCTYPE_OBJ)
    getloctype_index = objat;

  return winner;
}

// GetLocationType exported function - just call through
// to the main function with default 0
/* *** SCRIPT SYMBOL: [Game] GetLocationType *** */
int GetLocationType(int xxx,int yyy) {
  return __GetLocationType(xxx, yyy, 0);
}


/* *** SCRIPT SYMBOL: [Hotspot] GetHotspotName *** */
void GetHotspotName(int hotspot, char *buffer) {
  VALIDATE_STRING(buffer);
  if ((hotspot < 0) || (hotspot >= MAX_HOTSPOTS))
    quit("!GetHotspotName: invalid hotspot number");

  strcpy(buffer, get_translation(thisroom.hotspotnames[hotspot]));
}

/* *** SCRIPT SYMBOL: [Hotspot] Hotspot::GetName^1 *** */
void Hotspot_GetName(ScriptHotspot *hss, char *buffer) {
  GetHotspotName(hss->id, buffer);
}

/* *** SCRIPT SYMBOL: [Hotspot] Hotspot::get_Name *** */
const char* Hotspot_GetName_New(ScriptHotspot *hss) {
  return CreateNewScriptString(get_translation(thisroom.hotspotnames[hss->id]));
}


void register_hotspot_script_functions() {
  scAdd_External_Symbol("Hotspot::GetAtScreenXY^2",(void *)GetHotspotAtLocation);
  scAdd_External_Symbol("Hotspot::GetName^1", (void*)Hotspot_GetName);
  scAdd_External_Symbol("Hotspot::GetProperty^1", (void*)Hotspot_GetProperty);
  scAdd_External_Symbol("Hotspot::GetPropertyText^2", (void*)Hotspot_GetPropertyText);
  scAdd_External_Symbol("Hotspot::GetTextProperty^1",(void *)Hotspot_GetTextProperty);
  scAdd_External_Symbol("Hotspot::RunInteraction^1", (void*)Hotspot_RunInteraction);
  scAdd_External_Symbol("Hotspot::get_Enabled", (void*)Hotspot_GetEnabled);
  scAdd_External_Symbol("Hotspot::set_Enabled", (void*)Hotspot_SetEnabled);
  scAdd_External_Symbol("Hotspot::get_ID", (void*)Hotspot_GetID);
  scAdd_External_Symbol("Hotspot::get_Name", (void*)Hotspot_GetName_New);
  scAdd_External_Symbol("Hotspot::get_WalkToX", (void*)Hotspot_GetWalkToX);
  scAdd_External_Symbol("Hotspot::get_WalkToY", (void*)Hotspot_GetWalkToY);
  scAdd_External_Symbol("DisableHotspot",(void *)DisableHotspot);
  scAdd_External_Symbol("EnableHotspot",(void *)EnableHotspot);
  scAdd_External_Symbol("GetHotspotAt",(void *)GetHotspotAt);
  scAdd_External_Symbol("GetHotspotName",(void *)GetHotspotName);
  scAdd_External_Symbol("GetHotspotPointX",(void *)GetHotspotPointX);
  scAdd_External_Symbol("GetHotspotPointY",(void *)GetHotspotPointY);
  scAdd_External_Symbol("GetHotspotProperty",(void *)GetHotspotProperty);
  scAdd_External_Symbol("GetHotspotPropertyText",(void *)GetHotspotPropertyText);
  scAdd_External_Symbol("RunHotspotInteraction", (void *)RunHotspotInteraction);
  scAdd_External_Symbol("MoveCharacterToHotspot",(void *)MoveCharacterToHotspot);
  scAdd_External_Symbol("GetLocationType",(void *)GetLocationType);
}

