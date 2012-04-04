#include "ac_script_room.h"

#include "sdlwrap/allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "acroom.h"
#include "ac_string.h"
#include "ac_obj.h"
#include "ac_palette.h"
#include "dynobj/script_drawing_surface.h"
#include "cscomp.h"


/* *** SCRIPT SYMBOL: [Room] GetScalingAt *** */
int GetScalingAt (int x, int y) {
  int onarea = get_walkable_area_pixel(x, y);
  if (onarea < 0)
    return 100;

  return get_area_scaling (onarea, x, y);
}

/* *** SCRIPT SYMBOL: [Room] SetAreaScaling *** */
static void SetAreaScaling(int area, int min, int max) {
  if ((area < 0) || (area > MAX_WALK_AREAS))
    quit("!SetAreaScaling: invalid walkalbe area");

  if (min > max)
    quit("!SetAreaScaling: min > max");

  if ((min < 5) || (max < 5) || (min > 200) || (max > 200))
    quit("!SetAreaScaling: min and max must be in range 5-200");

  // the values are stored differently
  min -= 100;
  max -= 100;
  
  if (min == max) {
    thisroom.walk_area_zoom[area] = min;
    thisroom.walk_area_zoom2[area] = NOT_VECTOR_SCALED;
  }
  else {
    thisroom.walk_area_zoom[area] = min;
    thisroom.walk_area_zoom2[area] = max;
  }
}

/* *** SCRIPT SYMBOL: [Room] GetMessageText *** */
static void GetMessageText (int msg, char *buffer) {
  VALIDATE_STRING(buffer);
  get_message_text (msg, buffer, 0);
}

/* *** SCRIPT SYMBOL: [Room] Room::geti_Messages *** */
static const char* Room_GetMessages(int index) {
  if ((index < 0) || (index >= thisroom.nummes)) {
    return NULL;
  }
  char buffer[STD_BUFFER_SIZE];
  buffer[0]=0;
  replace_tokens(get_translation(thisroom.message[index]), buffer, STD_BUFFER_SIZE);
  return CreateNewScriptString(buffer);
}

struct Rect {
  int x1,y1,x2,y2;
};

static int GetThingRect(int thing, Rect *rect) {
  if (is_valid_character(thing)) {
    if (game.chars[thing].room != displayed_room)
      return 0;
    
    int charwid = divide_down_coordinate(GetCharacterWidth(thing));
    rect->x1 = game.chars[thing].x - (charwid / 2);
    rect->x2 = rect->x1 + charwid;
    rect->y1 = game.chars[thing].get_effective_y() - divide_down_coordinate(GetCharacterHeight(thing));
    rect->y2 = game.chars[thing].get_effective_y();
  }
  else if (is_valid_object(thing - OVERLAPPING_OBJECT)) {
    int objid = thing - OVERLAPPING_OBJECT;
    if (objs[objid].on != 1)
      return 0;
    rect->x1 = objs[objid].x;
    rect->x2 = objs[objid].x + divide_down_coordinate(objs[objid].get_width());
    rect->y1 = objs[objid].y - divide_down_coordinate(objs[objid].get_height());
    rect->y2 = objs[objid].y;
  }
  else
    quit("!AreThingsOverlapping: invalid parameter");

  return 1;
}

/* *** SCRIPT SYMBOL: [Room] AreThingsOverlapping *** */
int AreThingsOverlapping(int thing1, int thing2) {
  Rect r1, r2;
  // get the bounding rectangles, and return 0 if the object/char
  // is currently turned off
  if (GetThingRect(thing1, &r1) == 0)
    return 0;
  if (GetThingRect(thing2, &r2) == 0)
    return 0;

  if ((r1.x2 > r2.x1) && (r1.x1 < r2.x2) &&
      (r1.y2 > r2.y1) && (r1.y1 < r2.y2)) {
    // determine how far apart they are
    // take the smaller of the X distances as the overlapping amount
    int xdist = abs(r1.x2 - r2.x1);
    if (abs(r1.x1 - r2.x2) < xdist)
      xdist = abs(r1.x1 - r2.x2);
    // take the smaller of the Y distances
    int ydist = abs(r1.y2 - r2.y1);
    if (abs(r1.y1 - r2.y2) < ydist)
      ydist = abs(r1.y1 - r2.y2);
    // the overlapping amount is the smaller of the X and Y ovrlap
    if (xdist < ydist)
      return xdist;
    else
      return ydist;
//    return 1;
  }
  return 0;
}


/* *** SCRIPT SYMBOL: [Room] Room::GetDrawingSurfaceForBackground^1 *** */
static ScriptDrawingSurface* Room_GetDrawingSurfaceForBackground(int backgroundNumber)
{
  if (displayed_room < 0)
    quit("!Room.GetDrawingSurfaceForBackground: no room is currently loaded");

  if (backgroundNumber == SCR_NO_VALUE)
  {
    backgroundNumber = play.bg_frame;
  }

  if ((backgroundNumber < 0) || (backgroundNumber >= thisroom.num_bscenes))
    quit("!Room.GetDrawingSurfaceForBackground: invalid background number specified");


  ScriptDrawingSurface *surface = new ScriptDrawingSurface();
  surface->roomBackgroundNumber = backgroundNumber;
  ccRegisterManagedObject(surface, surface);
  return surface;
}


/* *** SCRIPT SYMBOL: [Room] Room::get_ObjectCount *** */
static int Room_GetObjectCount() {
  return croom->numobj;
}

/* *** SCRIPT SYMBOL: [Room] Room::get_Width *** */
static int Room_GetWidth() {
  return thisroom.width;
}

/* *** SCRIPT SYMBOL: [Room] Room::get_Height *** */
static int Room_GetHeight() {
  return thisroom.height;
}

/* *** SCRIPT SYMBOL: [Room] Room::get_ColorDepth *** */
static int Room_GetColorDepth() {
  return alw_bitmap_color_depth(thisroom.ebscene[0]);
}

/* *** SCRIPT SYMBOL: [Room] Room::get_LeftEdge *** */
static int Room_GetLeftEdge() {
  return thisroom.left;
}

/* *** SCRIPT SYMBOL: [Room] Room::get_RightEdge *** */
static int Room_GetRightEdge() {
  return thisroom.right;
}

/* *** SCRIPT SYMBOL: [Room] Room::get_TopEdge *** */
static int Room_GetTopEdge() {
  return thisroom.top;
}

/* *** SCRIPT SYMBOL: [Room] Room::get_BottomEdge *** */
static int Room_GetBottomEdge() {
  return thisroom.bottom;
}

/* *** SCRIPT SYMBOL: [Room] Room::get_MusicOnLoad *** */
static int Room_GetMusicOnLoad() {
  return thisroom.options[ST_TUNE];
}


/* *** SCRIPT SYMBOL: [Room] GetRoomProperty *** */
static int GetRoomProperty (const char *property) {
  return get_int_property (&thisroom.roomProps, property);
}

/* *** SCRIPT SYMBOL: [Room] GetRoomPropertyText *** */
static void GetRoomPropertyText (const char *property, char *bufer) {
  get_text_property (&thisroom.roomProps, property, bufer);
}
/* *** SCRIPT SYMBOL: [Room] Room::GetTextProperty^1 *** */
static const char* Room_GetTextProperty(const char *property) {
  return get_text_property_dynamic_string(&thisroom.roomProps, property);
}


/* *** SCRIPT SYMBOL: [Room] RemoveWalkableArea *** */
static void RemoveWalkableArea(int areanum) {
  if ((areanum<1) | (areanum>15))
    quit("!RemoveWalkableArea: invalid area number specified (1-15).");
  play.walkable_areas_on[areanum]=0;
  redo_walkable_areas();
  DEBUG_CONSOLE("Walkable area %d removed", areanum);
}

/* *** SCRIPT SYMBOL: [Room] RestoreWalkableArea *** */
static void RestoreWalkableArea(int areanum) {
  if ((areanum<1) | (areanum>15))
    quit("!RestoreWalkableArea: invalid area number specified (1-15).");
  play.walkable_areas_on[areanum]=1;
  redo_walkable_areas();
  DEBUG_CONSOLE("Walkable area %d restored", areanum);
}


/* *** SCRIPT SYMBOL: [Room] DisableGroundLevelAreas *** */
static void DisableGroundLevelAreas(int alsoEffects) {
  if ((alsoEffects < 0) || (alsoEffects > 1))
    quit("!DisableGroundLevelAreas: invalid parameter: must be 0 or 1");

  play.ground_level_areas_disabled = GLED_INTERACTION;

  if (alsoEffects)
    play.ground_level_areas_disabled |= GLED_EFFECTS;

  DEBUG_CONSOLE("Ground-level areas disabled");
}

/* *** SCRIPT SYMBOL: [Room] EnableGroundLevelAreas *** */
static void EnableGroundLevelAreas() {
  play.ground_level_areas_disabled = 0;

  DEBUG_CONSOLE("Ground-level areas re-enabled");
}

/* *** SCRIPT SYMBOL: [Room] SetWalkBehindBase *** */
static void SetWalkBehindBase(int wa,int bl) {
  if ((wa < 1) || (wa >= MAX_OBJ))
    quit("!SetWalkBehindBase: invalid walk-behind area specified");

  if (bl != croom->walkbehind_base[wa]) {
    walk_behind_baselines_changed = 1;
    invalidate_cached_walkbehinds();
    croom->walkbehind_base[wa] = bl;
    DEBUG_CONSOLE("Walk-behind %d baseline changed to %d", wa, bl);
  }
}


/* *** SCRIPT SYMBOL: [Room] ResetRoom *** */
static void ResetRoom(int nrnum) {
  if (nrnum == displayed_room)
    quit("!ResetRoom: cannot reset current room");
  if ((nrnum<0) | (nrnum>=MAX_ROOMS))
    quit("!ResetRoom: invalid room number");
  if (roomstats[nrnum].beenhere) {
    if (roomstats[nrnum].tsdata!=NULL)
      free(roomstats[nrnum].tsdata);
    roomstats[nrnum].tsdata=NULL;
    roomstats[nrnum].tsdatasize=0;
    }
  roomstats[nrnum].beenhere=0;
  DEBUG_CONSOLE("Room %d reset to original state", nrnum);
}

/* *** SCRIPT SYMBOL: [Room] HasPlayerBeenInRoom *** */
static int HasPlayerBeenInRoom(int roomnum) {
  if ((roomnum < 0) || (roomnum >= MAX_ROOMS))
    return 0;
  return roomstats[roomnum].beenhere;
}


/* *** SCRIPT SYMBOL: [Room] GetWalkableAreaAt *** */
static int GetWalkableAreaAt(int xxx,int yyy) {
  xxx += divide_down_coordinate(offsetx);
  yyy += divide_down_coordinate(offsety);
  if ((xxx>=thisroom.width) | (xxx<0) | (yyy<0) | (yyy>=thisroom.height))
    return 0;
  int result = get_walkable_area_pixel(xxx, yyy);
  if (result <= 0)
    return 0;
  return result;
}


/* *** SCRIPT SYMBOL: [Room] GetPlayerCharacter *** */
int GetPlayerCharacter() {
  return game.playercharacter;
  }


/* *** SCRIPT SYMBOL: [Room] SetViewport *** */
static void SetViewport(int offsx,int offsy) {
  DEBUG_CONSOLE("Viewport locked to %d,%d", offsx, offsy);
  offsetx = multiply_up_coordinate(offsx);
  offsety = multiply_up_coordinate(offsy);
  check_viewport_coords();
  play.offsets_locked = 1;
}
/* *** SCRIPT SYMBOL: [Room] ReleaseViewport *** */
static void ReleaseViewport() {
  play.offsets_locked = 0;
  DEBUG_CONSOLE("Viewport released back to engine control");
}
/* *** SCRIPT SYMBOL: [Room] GetViewportX *** */
static int GetViewportX () {
  return divide_down_coordinate(offsetx);
  }
/* *** SCRIPT SYMBOL: [Room] GetViewportY *** */
static int GetViewportY () {
  return divide_down_coordinate(offsety);
  }


void on_background_frame_change () {

  invalidate_screen();
  mark_current_background_dirty();
  invalidate_cached_walkbehinds();

  // get the new frame's palette
  memcpy (palette, thisroom.bpalettes[play.bg_frame], sizeof(color) * 256);

  // hi-colour, update the palette. It won't have an immediate effect
  // but will be drawn properly when the screen fades in
  if (game.color_depth > 1)
    setpal();

  if (in_enters_screen)
    return;

  // Don't update the palette if it hasn't changed
  if (thisroom.ebpalShared[play.bg_frame])
    return;

  // 256-colours, tell it to update the palette (will actually be done as
  // close as possible to the screen update to prevent flicker problem)
  if (game.color_depth == 1)
    bg_just_changed = 1;
}


/* *** SCRIPT SYMBOL: [Room] SetBackgroundFrame *** */
static void SetBackgroundFrame(int frnum) {
  if ((frnum<-1) | (frnum>=thisroom.num_bscenes))
    quit("!SetBackgrondFrame: invalid frame number specified");
  if (frnum<0) {
    play.bg_frame_locked=0;
    return;
  }

  play.bg_frame_locked = 1;

  if (frnum == play.bg_frame)
  {
    // already on this frame, do nothing
    return;
  }

  play.bg_frame = frnum;
  on_background_frame_change ();
}

/* *** SCRIPT SYMBOL: [Room] GetBackgroundFrame *** */
static int GetBackgroundFrame() {
  return play.bg_frame;
  }

void register_room_script_functions() {
  scAdd_External_Symbol("Room::GetDrawingSurfaceForBackground^1", (void *)Room_GetDrawingSurfaceForBackground);
  scAdd_External_Symbol("Room::GetTextProperty^1",(void *)Room_GetTextProperty);
  scAdd_External_Symbol("Room::get_BottomEdge", (void *)Room_GetBottomEdge);
  scAdd_External_Symbol("Room::get_ColorDepth", (void *)Room_GetColorDepth);
  scAdd_External_Symbol("Room::get_Height", (void *)Room_GetHeight);
  scAdd_External_Symbol("Room::get_LeftEdge", (void *)Room_GetLeftEdge);
  scAdd_External_Symbol("Room::geti_Messages",(void *)Room_GetMessages);
  scAdd_External_Symbol("Room::get_MusicOnLoad", (void *)Room_GetMusicOnLoad);
  scAdd_External_Symbol("Room::get_ObjectCount", (void *)Room_GetObjectCount);
  scAdd_External_Symbol("Room::get_RightEdge", (void *)Room_GetRightEdge);
  scAdd_External_Symbol("Room::get_TopEdge", (void *)Room_GetTopEdge);
  scAdd_External_Symbol("Room::get_Width", (void *)Room_GetWidth);
  scAdd_External_Symbol("AreThingsOverlapping",(void *)AreThingsOverlapping);
  scAdd_External_Symbol("DisableGroundLevelAreas",(void *)DisableGroundLevelAreas);
  scAdd_External_Symbol("EnableGroundLevelAreas",(void *)EnableGroundLevelAreas);
  scAdd_External_Symbol("GetBackgroundFrame",(void *)GetBackgroundFrame);
  scAdd_External_Symbol("GetMessageText", (void *)GetMessageText);
  scAdd_External_Symbol("GetPlayerCharacter",(void *)GetPlayerCharacter);
  scAdd_External_Symbol("GetRoomProperty",(void *)GetRoomProperty);
  scAdd_External_Symbol("GetRoomPropertyText",(void *)GetRoomPropertyText);
  scAdd_External_Symbol("GetViewportX",(void *)GetViewportX);
  scAdd_External_Symbol("GetViewportY",(void *)GetViewportY);
  scAdd_External_Symbol("GetWalkableAreaAt",(void *)GetWalkableAreaAt);
  scAdd_External_Symbol("HasPlayerBeenInRoom",(void *)HasPlayerBeenInRoom);
  scAdd_External_Symbol("ReleaseViewport",(void *)ReleaseViewport);
  scAdd_External_Symbol("RemoveWalkableArea",(void *)RemoveWalkableArea);
  scAdd_External_Symbol("ResetRoom",(void *)ResetRoom);
  scAdd_External_Symbol("RestoreWalkableArea",(void *)RestoreWalkableArea);
  scAdd_External_Symbol("SetAreaScaling",(void *)SetAreaScaling);
  scAdd_External_Symbol("SetBackgroundFrame",(void *)SetBackgroundFrame);
  scAdd_External_Symbol("SetViewport",(void *)SetViewport);
  scAdd_External_Symbol("SetWalkBehindBase",(void *)SetWalkBehindBase);
  scAdd_External_Symbol("GetScalingAt",(void *)GetScalingAt);
}