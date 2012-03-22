/*
  AGS Character functions

  Adventure Game Studio source code Copyright 1999-2011 Chris Jones.
  All rights reserved.

  The AGS Editor Source Code is provided under the Artistic License 2.0
  http://www.opensource.org/licenses/artistic-license-2.0.php

  You MAY NOT compile your own builds of the engine without making it EXPLICITLY
  CLEAR that the code has been altered from the Standard Version.

*/

#include "acchars.h"

#include "ac.h"
#include "ac_context.h"
#include "bmp.h"
#include "wgt2allg.h"
#include "acroom.h"
#include "acruntim.h"
#include <math.h>
#include "routefnd.h"
#include "ac_context.h"
#include "ac_string.h"
#include "ac_mouse.h"
#include "ac_obj.h"
#include "sprcache.h"
#include "ac_overlay.h"
#include "ac_viewframe.h"
#include "acgfx.h"
#include "ali3d.h"

// order of loops to turn character in circle from down to down
int turnlooporder[8] = {0, 6, 1, 7, 3, 5, 2, 4};


void fix_player_sprite(MoveList*cmls,CharacterInfo*chinf);


/* *** SCRIPT SYMBOL: [Character] StopMoving *** */
void StopMoving(int chaa) {

  Character_StopMoving(&game.chars[chaa]);
}

/* *** SCRIPT SYMBOL: [Character] ReleaseCharacterView *** */
void ReleaseCharacterView(int chat) {
  if (!is_valid_character(chat))
    quit("!ReleaseCahracterView: invalid character supplied");

  Character_UnlockView(&game.chars[chat]);
}

void walk_character(int chac,int tox,int toy,int ignwal, bool autoWalkAnims) {
  CharacterInfo*chin=&game.chars[chac];
  if (chin->room!=displayed_room)
    quit("!MoveCharacter: character not in current room");

  chin->flags &= ~CHF_MOVENOTWALK;

  int toxPassedIn = tox, toyPassedIn = toy;
  int charX = convert_to_low_res(chin->x);
  int charY = convert_to_low_res(chin->y);
  tox = convert_to_low_res(tox);
  toy = convert_to_low_res(toy);

  if ((tox == charX) && (toy == charY)) {
    StopMoving(chac);
    DEBUG_CONSOLE("%s already at destination, not moving", chin->scrname);
    return;
  }

  if ((chin->animating) && (autoWalkAnims))
    chin->animating = 0;

  if (chin->idleleft < 0) {
    ReleaseCharacterView(chac);
    chin->idleleft=chin->idletime;
  }
  // stop them to make sure they're on a walkable area
  // but save their frame first so that if they're already
  // moving it looks smoother
  int oldframe = chin->frame;
  int waitWas = 0, animWaitWas = 0;
  // if they are currently walking, save the current Wait
  if (chin->walking)
  {
    waitWas = chin->walkwait;
    animWaitWas = charextra[chac].animwait;
  }

  StopMoving (chac);
  chin->frame = oldframe;
  // use toxPassedIn cached variable so the hi-res co-ordinates
  // are still displayed as such
  DEBUG_CONSOLE("%s: Start move to %d,%d", chin->scrname, toxPassedIn, toyPassedIn);

  int move_speed_x = chin->walkspeed;
  int move_speed_y = chin->walkspeed;

  if (chin->walkspeed_y != UNIFORM_WALK_SPEED)
    move_speed_y = chin->walkspeed_y;

  if ((move_speed_x == 0) && (move_speed_y == 0)) {
    debug_log("Warning: MoveCharacter called for '%s' with walk speed 0", chin->name);
  }

  set_route_move_speed(move_speed_x, move_speed_y);
  set_color_depth(8);
  int mslot=find_route(charX, charY, tox, toy, prepare_walkable_areas(chac), chac+CHMLSOFFS, 1, ignwal);
  set_color_depth(final_col_dep);
  if (mslot>0) {
    chin->walking = mslot;
    mls[mslot].direct = ignwal;

    if ((game.options[OPT_NATIVECOORDINATES] != 0) &&
        (game.default_resolution > 2))
    {
      convert_move_path_to_high_res(&mls[mslot]);
    }
    // cancel any pending waits on current animations
    // or if they were already moving, keep the current wait - 
    // this prevents a glitch if MoveCharacter is called when they
    // are already moving
    if (autoWalkAnims)
    {
      chin->walkwait = waitWas;
      charextra[chac].animwait = animWaitWas;

      if (mls[mslot].pos[0] != mls[mslot].pos[1]) {
        fix_player_sprite(&mls[mslot],chin);
      }
    }
    else
      chin->flags |= CHF_MOVENOTWALK;
  }
  else if (autoWalkAnims) // pathfinder couldn't get a route, stand them still
    chin->frame = 0;
}

int find_looporder_index (int curloop) {
  int rr;
  for (rr = 0; rr < 8; rr++) {
    if (turnlooporder[rr] == curloop)
      return rr;
  }
  return 0;
}

// returns 0 to use diagonal, 1 to not
int useDiagonal (CharacterInfo *char1) {
  if ((views[char1->view].numLoops < 8) || ((char1->flags & CHF_NODIAGONAL)!=0))
    return 1;
  // If they have just provided standing frames for loops 4-7, to
  // provide smoother turning
  if (views[char1->view].loops[4].numFrames < 2)
    return 2;
  return 0;
}

// returns 1 normally, or 0 if they only have horizontal animations
int hasUpDownLoops(CharacterInfo *char1) {
  // if no loops in the Down animation
  // or no loops in the Up animation
  if ((views[char1->view].loops[0].numFrames < 1) ||
      (views[char1->view].numLoops < 4) ||
      (views[char1->view].loops[3].numFrames < 1))
  {
    return 0;
  }

  return 1;
}

void start_character_turning (CharacterInfo *chinf, int useloop, int no_diagonal) {
  // work out how far round they have to turn 
  int fromidx = find_looporder_index (chinf->loop);
  int toidx = find_looporder_index (useloop);
  //Display("Curloop: %d, needloop: %d",chinf->loop, useloop);
  int ii, go_anticlock = 0;
  // work out whether anticlockwise is quicker or not
  if ((toidx > fromidx) && ((toidx - fromidx) > 4))
    go_anticlock = 1;
  if ((toidx < fromidx) && ((fromidx - toidx) < 4))
    go_anticlock = 1;
  // strip any current turning_around stages
  chinf->walking = chinf->walking % TURNING_AROUND;
  if (go_anticlock)
    chinf->walking += TURNING_BACKWARDS;
  else
    go_anticlock = -1;

  // Allow the diagonal frames just for turning
  if (no_diagonal == 2)
    no_diagonal = 0;

  for (ii = fromidx; ii != toidx; ii -= go_anticlock) {
    if (ii < 0)
      ii = 7;
    if (ii >= 8)
      ii = 0;
    if (ii == toidx)
      break;
    if ((turnlooporder[ii] >= 4) && (no_diagonal > 0))
      continue;
    if (views[chinf->view].loops[turnlooporder[ii]].numFrames < 1)
      continue;
    if (turnlooporder[ii] < views[chinf->view].numLoops)
      chinf->walking += TURNING_AROUND;
  }
  
}


#define CHECK_DIAGONAL(maindir,othdir,codea,codeb) \
  if (no_diagonal) ;\
  else if (abs(maindir) > abs(othdir) / 2) {\
    if (maindir < 0) useloop=codea;\
    else useloop=codeb;\
    }
// loops: 0=down, 1=left, 2=right, 3=up, 4=down-right, 5=up-right,
// 6=down-left, 7=up-left
void fix_player_sprite(MoveList*cmls,CharacterInfo*chinf) {
  int view_num=chinf->view;
  int want_horiz=1,useloop = 1,no_diagonal=0;
  fixed xpmove = cmls->xpermove[cmls->onstage];
  fixed ypmove = cmls->ypermove[cmls->onstage];

  // if not moving, do nothing
  if ((xpmove == 0) && (ypmove == 0))
    return;

  if (hasUpDownLoops(chinf) == 0)
    want_horiz = 1;
  else if (abs(ypmove) > abs(xpmove))
    want_horiz = 0;

  no_diagonal = useDiagonal (chinf);

  if ((want_horiz==1) && (xpmove > 0)) {
    // right
    useloop=2;
    // diagonal up-right/down-right
    CHECK_DIAGONAL(ypmove,xpmove,5,4)
    }
  else if ((want_horiz==1) && (xpmove <= 0)) {
    // left
    useloop=1;
    // diagonal up-left/down-left
    CHECK_DIAGONAL(ypmove,xpmove,7,6)
    }
  else if (ypmove < 0) {
    // up
    useloop=3;
    // diagonal up-left/up-right
    CHECK_DIAGONAL(xpmove,ypmove,7,5)
    }
  else {
    // down
    useloop=0;
    // diagonal down-left/down-right
    CHECK_DIAGONAL(xpmove,ypmove,6,4)
    }

  if ((game.options[OPT_ROTATECHARS] == 0) || ((chinf->flags & CHF_NOTURNING) != 0)) {
    chinf->loop = useloop;
    return;
  }
  if ((chinf->loop > 3) && ((chinf->flags & CHF_NODIAGONAL)!=0)) {
    // They've just been playing an animation with an extended loop number,
    // so don't try and rotate using it
    chinf->loop = useloop;
    return;
  }
  if ((chinf->loop >= views[chinf->view].numLoops) ||
      (views[chinf->view].loops[chinf->loop].numFrames < 1) ||
      (hasUpDownLoops(chinf) == 0)) {
    // Character is not currently on a valid loop, so don't try to rotate
    // eg. left/right only view, but current loop 0
    chinf->loop = useloop;
    return;
  }
  start_character_turning (chinf, useloop, no_diagonal);
}

// Check whether two characters have walked into each other
int has_hit_another_character(int sourceChar) {

  // if the character who's moving doesn't block, don't bother checking
  if (game.chars[sourceChar].flags & CHF_NOBLOCKING)
    return -1;

  for (int ww = 0; ww < game.numcharacters; ww++) {
    if (game.chars[ww].on != 1) continue;
    if (game.chars[ww].room != displayed_room) continue;
    if (ww == sourceChar) continue;
    if (game.chars[ww].flags & CHF_NOBLOCKING) continue;

    if (is_char_on_another (sourceChar, ww, NULL, NULL)) {
      // we are now overlapping character 'ww'
      if ((game.chars[ww].walking) && 
          ((game.chars[ww].flags & CHF_AWAITINGMOVE) == 0))
        return ww;
    }

  }
  return -1;
}

// Does the next move from the character's movelist.
// Returns 1 if they are now waiting for another char to move,
// otherwise returns 0
int doNextCharMoveStep (int aa, CharacterInfo *chi) {
  int ntf=0, xwas = chi->x, ywas = chi->y;

  if (do_movelist_move(&chi->walking,&chi->x,&chi->y) == 2) 
  {
    if ((chi->flags & CHF_MOVENOTWALK) == 0)
      fix_player_sprite(&mls[chi->walking], chi);
  }

  ntf = has_hit_another_character(aa);
  if (ntf >= 0) {
    chi->walkwait = 30;
    if (game.chars[ntf].walkspeed < 5)
      chi->walkwait += (5 - game.chars[ntf].walkspeed) * 5;
    // we are now waiting for the other char to move, so
    // make sure he doesn't stop for us too

    chi->flags |= CHF_AWAITINGMOVE;

    if ((chi->flags & CHF_MOVENOTWALK) == 0)
    {
      chi->frame = 0;
      charextra[aa].animwait = chi->walkwait;
    }

    if ((chi->walking < 1) || (chi->walking >= TURNING_AROUND)) ;
    else if (mls[chi->walking].onpart > 0) {
      mls[chi->walking].onpart --;
      chi->x = xwas;
      chi->y = ywas;
    }
    DEBUG_CONSOLE("%s: Bumped into %s, waiting for them to move", chi->scrname, game.chars[ntf].scrname);
    return 1;
  }
  return 0;
}

int find_nearest_walkable_area_within(int *xx, int *yy, int range, int step)
{
    int ex, ey, nearest = 99999, thisis, nearx = 0, neary = 0;
    int startx = 0, starty = 14;
    int roomWidthLowRes = convert_to_low_res(thisroom.width);
    int roomHeightLowRes = convert_to_low_res(thisroom.height);
    int xwidth = roomWidthLowRes, yheight = roomHeightLowRes;

    int xLowRes = convert_to_low_res(xx[0]);
    int yLowRes = convert_to_low_res(yy[0]);
    int rightEdge = convert_to_low_res(thisroom.right);
    int leftEdge = convert_to_low_res(thisroom.left);
    int topEdge = convert_to_low_res(thisroom.top);
    int bottomEdge = convert_to_low_res(thisroom.bottom);

    // tweak because people forget to move the edges sometimes
    // if the player is already over the edge, ignore it
    if (xLowRes >= rightEdge) rightEdge = roomWidthLowRes;
    if (xLowRes <= leftEdge) leftEdge = 0;
    if (yLowRes >= bottomEdge) bottomEdge = roomHeightLowRes;
    if (yLowRes <= topEdge) topEdge = 0;

    if (range > 0) 
    {
      startx = xLowRes - range;
      starty = yLowRes - range;
      xwidth = startx + range * 2;
      yheight = starty + range * 2;
      if (startx < 0) startx = 0;
      if (starty < 10) starty = 10;
      if (xwidth > roomWidthLowRes) xwidth = roomWidthLowRes;
      if (yheight > roomHeightLowRes) yheight = roomHeightLowRes;
    }

    for (ex = startx; ex < xwidth; ex += step) {
      for (ey = starty; ey < yheight; ey += step) {
        // non-walkalbe, so don't go here
        if (getpixel(thisroom.walls,ex,ey) == 0) continue;
        // off a screen edge, don't move them there
        if ((ex <= leftEdge) || (ex >= rightEdge) ||
           (ey <= topEdge) || (ey >= bottomEdge))
          continue;
        // otherwise, calculate distance from target
        thisis=(int) ::sqrt((double)((ex - xLowRes) * (ex - xLowRes) + (ey - yLowRes) * (ey - yLowRes)));
        if (thisis<nearest) { nearest=thisis; nearx=ex; neary=ey; }
      }
    }
    if (nearest < 90000) 
    {
      xx[0] = convert_back_to_high_res(nearx);
      yy[0] = convert_back_to_high_res(neary);
      return 1;
    }

    return 0;
}

void find_nearest_walkable_area (int *xx, int *yy) {
  

  int pixValue = getpixel(thisroom.walls, convert_to_low_res(xx[0]), convert_to_low_res(yy[0]));
  // only fix this code if the game was built with 2.61 or above
  if (pixValue == 0 || (engineNeedsAsInt >=261 && pixValue < 1))
  {
    // First, check every 2 pixels within immediate area
    if (!find_nearest_walkable_area_within(xx, yy, 20, 2))
    {
      // If not, check whole screen at 5 pixel intervals
      find_nearest_walkable_area_within(xx, yy, -1, 5);
    }
  }

}

/* *** SCRIPT SYMBOL: [Character] MoveToWalkableArea *** */
void MoveToWalkableArea(int charid) {
  if (!is_valid_character(charid))
    quit("!MoveToWalkableArea: invalid character specified");
  
  Character_PlaceOnWalkableArea(&game.chars[charid]);
}

/* *** SCRIPT SYMBOL: [Character] FaceLocation *** */
void FaceLocation(int cha, int xx, int yy) {
  if (!is_valid_character(cha))
    quit("!FaceLocation: Invalid character specified");

  Character_FaceLocation(&game.chars[cha], xx, yy, BLOCKING);
}

/* *** SCRIPT SYMBOL: [Character] FaceCharacter *** */
void FaceCharacter(int cha,int toface) {
  if (!is_valid_character(cha))
    quit("!FaceCharacter: Invalid character specified");
  if (!is_valid_character(toface)) 
    quit("!FaceCharacter: invalid character specified");

  Character_FaceCharacter(&game.chars[cha], &game.chars[toface], BLOCKING);
}




// **** CHARACTER: FUNCTIONS ****


/* *** SCRIPT SYMBOL: [Character] Character::AddInventory^2 *** */
void Character_AddInventory(CharacterInfo *chaa, ScriptInvItem *invi, int addIndex) {
  int ee;

  if (invi == NULL)
    quit("!AddInventoryToCharacter: invalid invnetory number");

  int inum = invi->id;

  if (chaa->inv[inum] >= 32000)
    quit("!AddInventory: cannot carry more than 32000 of one inventory item");

  chaa->inv[inum]++;

  int charid = chaa->index_id;

  if (game.options[OPT_DUPLICATEINV] == 0) {
    // Ensure it is only in the list once
    for (ee = 0; ee < charextra[charid].invorder_count; ee++) {
      if (charextra[charid].invorder[ee] == inum) {
        // They already have the item, so don't add it to the list
        if (chaa == playerchar)
          run_on_event (GE_ADD_INV, inum);
        return;
      }
    }
  }
  if (charextra[charid].invorder_count >= MAX_INVORDER)
    quit("!Too many inventory items added, max 500 display at one time");

  if ((addIndex == SCR_NO_VALUE) ||
      (addIndex >= charextra[charid].invorder_count) ||
      (addIndex < 0)) {
    // add new item at end of list
    charextra[charid].invorder[charextra[charid].invorder_count] = inum;
  }
  else {
    // insert new item at index
    for (ee = charextra[charid].invorder_count - 1; ee >= addIndex; ee--)
      charextra[charid].invorder[ee + 1] = charextra[charid].invorder[ee];

    charextra[charid].invorder[addIndex] = inum;
  }
  charextra[charid].invorder_count++;
  guis_need_update = 1;
  if (chaa == playerchar)
    run_on_event (GE_ADD_INV, inum);

}

/* *** SCRIPT SYMBOL: [Character] Character::AddWaypoint^2 *** */
void Character_AddWaypoint(CharacterInfo *chaa, int x, int y) {

  if (chaa->room != displayed_room)
    quit("!MoveCharacterPath: specified character not in current room");

  // not already walking, so just do a normal move
  if (chaa->walking <= 0) {
    Character_Walk(chaa, x, y, IN_BACKGROUND, ANYWHERE);
    return;
  }

  MoveList *cmls = &mls[chaa->walking % TURNING_AROUND];
  if (cmls->numstage >= MAXNEEDSTAGES)
    quit("!MoveCharacterPath: move is too complex, cannot add any further paths");

  cmls->pos[cmls->numstage] = (x << 16) + y;
  // They're already walking there anyway
  if (cmls->pos[cmls->numstage] == cmls->pos[cmls->numstage - 1])
    return;

  calculate_move_stage (cmls, cmls->numstage-1);
  cmls->numstage ++;

}

/* *** SCRIPT SYMBOL: [Character] Character::Animate^5 *** */
void Character_Animate(CharacterInfo *chaa, int loop, int delay, int repeat, int blocking, int direction) {

  if (direction == FORWARDS)
    direction = 0;
  else if (direction == BACKWARDS)
    direction = 1;
  else
    quit("!Character.Animate: Invalid DIRECTION parameter");

  animate_character(chaa, loop, delay, repeat, 0, direction);

  if ((blocking == BLOCKING) || (blocking == 1))
    do_main_cycle(UNTIL_SHORTIS0,(int)&chaa->animating);
  else if ((blocking != IN_BACKGROUND) && (blocking != 0))
    quit("!Character.Animate: Invalid BLOCKING parameter");
}

/* *** SCRIPT SYMBOL: [Character] Character::ChangeRoomAutoPosition^2 *** */
void Character_ChangeRoomAutoPosition(CharacterInfo *chaa, int room, int newPos) 
{
  if (chaa->index_id != game.playercharacter) 
  {
    quit("!Character.ChangeRoomAutoPosition can only be used with the player character.");
  }

  new_room_pos = newPos;

  if (new_room_pos == 0) {
    // auto place on other side of screen
    if (chaa->x <= thisroom.left + 10)
      new_room_pos = 2000;
    else if (chaa->x >= thisroom.right - 10)
      new_room_pos = 1000;
    else if (chaa->y <= thisroom.top + 10)
      new_room_pos = 3000;
    else if (chaa->y >= thisroom.bottom - 10)
      new_room_pos = 4000;
    
    if (new_room_pos < 3000)
      new_room_pos += chaa->y;
    else
      new_room_pos += chaa->x;
  }
  NewRoom(room);
}

/* *** SCRIPT SYMBOL: [Character] Character::ChangeRoom^3 *** */
void Character_ChangeRoom(CharacterInfo *chaa, int room, int x, int y) {

  if (chaa->index_id != game.playercharacter) {
    // NewRoomNPC
    if ((x != SCR_NO_VALUE) && (y != SCR_NO_VALUE)) {
      chaa->x = x;
      chaa->y = y;
    }
    chaa->prevroom = chaa->room;
    chaa->room = room;

    DEBUG_CONSOLE("%s moved to room %d, location %d,%d",
                  chaa->scrname, room, chaa->x, chaa->y);

    return;
  }

  if ((x != SCR_NO_VALUE) && (y != SCR_NO_VALUE)) {
    new_room_pos = 0;
    // don't check X or Y bounds, so that they can do a
    // walk-in animation if they want
    new_room_x = x;
    new_room_y = y;
  }
  
  NewRoom(room);
}

void FindReasonableLoopForCharacter(CharacterInfo *chap) {

  if (chap->loop >= views[chap->view].numLoops)
    chap->loop=0;
  if (views[chap->view].numLoops < 1)
    quitprintf("!View %d does not have any loops", chap->view + 1);

  // if the current loop has no frames, find one that does
  if (views[chap->view].loops[chap->loop].numFrames < 1) 
  {
    for (int i = 0; i < views[chap->view].numLoops; i++) 
    {
      if (views[chap->view].loops[i].numFrames > 0) {
        chap->loop = i;
        break;
      }
    }
  }

}

/* *** SCRIPT SYMBOL: [Character] Character::ChangeView^1 *** */
void Character_ChangeView(CharacterInfo *chap, int vii) {
  vii--;

  if ((vii < 0) || (vii >= game.numviews))
    quit("!ChangeCharacterView: invalid view number specified");

  // if animating, but not idle view, give warning message
  if ((chap->flags & CHF_FIXVIEW) && (chap->idleleft >= 0))
    debug_log("Warning: ChangeCharacterView was used while the view was fixed - call ReleaseCharView first");

  DEBUG_CONSOLE("%s: Change view to %d", chap->scrname, vii+1);
  chap->defview = vii;
  chap->view = vii;
  chap->animating = 0;
  chap->frame = 0;
  chap->wait = 0;
  chap->walkwait = 0;
  charextra[chap->index_id].animwait = 0;
  FindReasonableLoopForCharacter(chap);
}

/* *** SCRIPT SYMBOL: [Character] Character::FaceCharacter^2 *** */
void Character_FaceCharacter(CharacterInfo *char1, CharacterInfo *char2, int blockingStyle) {
  if (char2 == NULL) 
    quit("!FaceCharacter: invalid character specified");
  
  if (char1->room != char2->room)
    quit("!FaceCharacter: characters are in different rooms");

  Character_FaceLocation(char1, char2->x, char2->y, blockingStyle);
}

/* *** SCRIPT SYMBOL: [Character] Character::FaceLocation^3 *** */
void Character_FaceLocation(CharacterInfo *char1, int xx, int yy, int blockingStyle) {
  DEBUG_CONSOLE("%s: Face location %d,%d", char1->scrname, xx, yy);

  int diffrx = xx - char1->x;
  int diffry = yy - char1->y;
  int useloop = 1, wanthoriz=0, no_diagonal = 0;
  int highestLoopForTurning = 3;

  if ((diffrx == 0) && (diffry == 0)) {
    // FaceLocation called on their current position - do nothing
    return;
  }

  no_diagonal = useDiagonal (char1);

  if (no_diagonal != 1) {
    highestLoopForTurning = 7;
  }

  if (hasUpDownLoops(char1) == 0)
    wanthoriz = 1;
  else if (abs(diffry) < abs(diffrx))
    wanthoriz = 1;

  if ((wanthoriz==1) && (diffrx > 0)) {
    useloop=2;
    CHECK_DIAGONAL(diffry, diffrx, 5, 4)
  }
  else if ((wanthoriz==1) && (diffrx <= 0)) {
    useloop=1;
    CHECK_DIAGONAL(diffry, diffrx,7,6)
  }
  else if (diffry>0) {
    useloop=0;
    CHECK_DIAGONAL(diffrx ,diffry ,6,4)
  }
  else if (diffry<0) {
    useloop=3;
    CHECK_DIAGONAL(diffrx, diffry,7,5)
  }


  if ((game.options[OPT_TURNTOFACELOC] != 0) &&
      (useloop != char1->loop) &&
      (char1->loop <= highestLoopForTurning) &&
      (in_enters_screen == 0)) {
    // Turn to face new direction
    Character_StopMoving(char1);
    if (char1->on == 1) {
      // only do the turning if the character is not hidden
      // (otherwise do_main_cycle will never return)
      start_character_turning (char1, useloop, no_diagonal);

      if ((blockingStyle == BLOCKING) || (blockingStyle == 1))
        do_main_cycle(UNTIL_MOVEEND,(int)&char1->walking);
    }
    else
      char1->loop = useloop;
  }
  else
    char1->loop=useloop;

  char1->frame=0;
}

/* *** SCRIPT SYMBOL: [Character] Character::FaceObject^2 *** */
void Character_FaceObject(CharacterInfo *char1, ScriptObject *obj, int blockingStyle) {
  if (obj == NULL) 
    quit("!FaceObject: invalid object specified");
  

  Character_FaceLocation(char1, obj->obj->x, obj->obj->y, blockingStyle);
}

/* *** SCRIPT SYMBOL: [Character] Character::FollowCharacter^3 *** */
void Character_FollowCharacter(CharacterInfo *chaa, CharacterInfo *tofollow, int distaway, int eagerness) {

  if ((eagerness < 0) || (eagerness > 250))
    quit("!FollowCharacterEx: invalid eagerness: must be 0-250");

  if ((chaa->index_id == game.playercharacter) && (tofollow != NULL) && 
      (tofollow->room != chaa->room))
    quit("!FollowCharacterEx: you cannot tell the player character to follow a character in another room");

  if (tofollow != NULL) {
    DEBUG_CONSOLE("%s: Start following %s (dist %d, eager %d)", chaa->scrname, tofollow->scrname, distaway, eagerness);
  }
  else {
    DEBUG_CONSOLE("%s: Stop following other character", chaa->scrname);
  }

  if ((chaa->following >= 0) &&
      (chaa->followinfo == FOLLOW_ALWAYSONTOP)) {
    // if this character was following always-on-top, its baseline will
    // have been changed, so release it.
    chaa->baseline = -1;
  }

  if (tofollow == NULL)
    chaa->following = -1;
  else
    chaa->following = tofollow->index_id;

  chaa->followinfo=(distaway << 8) | eagerness;

  chaa->flags &= ~CHF_BEHINDSHEPHERD;

  // special case for Always On Other Character
  if (distaway == FOLLOW_ALWAYSONTOP) {
    chaa->followinfo = FOLLOW_ALWAYSONTOP;
    if (eagerness == 1)
      chaa->flags |= CHF_BEHINDSHEPHERD;
  }

  if (chaa->animating & CHANIM_REPEAT)
    debug_log("Warning: FollowCharacter called but the sheep is currently animating looped. It may never start to follow.");

}

/* *** SCRIPT SYMBOL: [Character] Character::IsCollidingWithChar^1 *** */
int Character_IsCollidingWithChar(CharacterInfo *char1, CharacterInfo *char2) {
  if (char2 == NULL)
    quit("!AreCharactersColliding: invalid char2");

  if (char1->room != char2->room) return 0; // not colliding

  if ((char1->y > char2->y - 5) && (char1->y < char2->y + 5)) ;
  else return 0;

  int w1 = divide_down_coordinate(GetCharacterWidth(char1->index_id));
  int w2 = divide_down_coordinate(GetCharacterWidth(char2->index_id));

  int xps1=char1->x - w1/2;
  int xps2=char2->x - w2/2;

  if ((xps1 >= xps2 - w1) & (xps1 <= xps2 + w2)) return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [Character] Character::IsCollidingWithObject^1 *** */
int Character_IsCollidingWithObject(CharacterInfo *chin, ScriptObject *objid) {
  if (objid == NULL)
    quit("!AreCharObjColliding: invalid object number");

  if (chin->room != displayed_room)
    return 0;
  if (objs[objid->id].on != 1)
    return 0;

  block checkblk = GetObjectImage(objid->id, NULL);
  int objWidth = BMP_W(checkblk);
  int objHeight = BMP_H(checkblk);
  int o1x = objs[objid->id].x;
  int o1y = objs[objid->id].y - divide_down_coordinate(objHeight);

  block charpic = GetCharacterImage(chin->index_id, NULL);
  
  int charWidth = BMP_W(charpic);
  int charHeight = BMP_H(charpic);
  int o2x = chin->x - divide_down_coordinate(charWidth) / 2;
  int o2y = chin->get_effective_y() - 5;  // only check feet

  if ((o2x >= o1x - divide_down_coordinate(charWidth)) &&
    (o2x <= o1x + divide_down_coordinate(objWidth)) &&
    (o2y >= o1y - 8) &&
    (o2y <= o1y + divide_down_coordinate(objHeight))) {
    // the character's feet are on the object
    if (game.options[OPT_PIXPERFECT] == 0)
      return 1;
    // check if they're on a transparent bit of the object
    int stxp = multiply_up_coordinate(o2x - o1x);
    int styp = multiply_up_coordinate(o2y - o1y);
    int maskcol = bitmap_mask_color (checkblk);
    int maskcolc = bitmap_mask_color (charpic);
    int thispix, thispixc;
    // check each pixel of the object along the char's feet
    for (int i = 0; i < charWidth; i += get_fixed_pixel_size(1)) {
      for (int j = 0; j < get_fixed_pixel_size(6); j += get_fixed_pixel_size(1)) {
        thispix = my_getpixel(checkblk, i + stxp, j + styp);
        thispixc = my_getpixel(charpic, i, j + (charHeight - get_fixed_pixel_size(5)));

        if ((thispix != -1) && (thispix != maskcol) &&
            (thispixc != -1) && (thispixc != maskcolc))
          return 1;
      }
    }

  }
  return 0;
}

/* *** SCRIPT SYMBOL: [Character] Character::LockView^1 *** */
void Character_LockView(CharacterInfo *chap, int vii) {

  if ((vii < 1) || (vii > game.numviews)) {
    char buffer[150];
    sprintf (buffer, "!SetCharacterView: invalid view number (You said %d, max is %d)", vii, game.numviews);
    quit(buffer);
  }
  vii--;
  
  DEBUG_CONSOLE("%s: View locked to %d", chap->scrname, vii+1);
  if (chap->idleleft < 0) {
    Character_UnlockView(chap);
    chap->idleleft = chap->idletime;
  }
  Character_StopMoving(chap);
  chap->view=vii;
  chap->animating=0;
  FindReasonableLoopForCharacter(chap);
  chap->frame=0;
  chap->wait=0;
  chap->flags|=CHF_FIXVIEW;
  chap->pic_xoffs = 0;
  chap->pic_yoffs = 0;
}


/* *** SCRIPT SYMBOL: [Character] Character::LockViewAligned^3 *** */
void Character_LockViewAligned(CharacterInfo *chap, int vii, int loop, int align) {
  if (chap->view < 0)
    quit("!SetCharacterLoop: character has invalid old view number");

  int sppic = views[chap->view].loops[chap->loop].frames[chap->frame].pic;
  int leftSide = multiply_up_coordinate(chap->x) - spritewidth[sppic] / 2;

  Character_LockView(chap, vii);

  if ((loop < 0) || (loop >= views[chap->view].numLoops))
    quit("!SetCharacterViewEx: invalid loop specified");

  chap->loop = loop;
  chap->frame = 0;
  int newpic = views[chap->view].loops[chap->loop].frames[chap->frame].pic;
  int newLeft = multiply_up_coordinate(chap->x) - spritewidth[newpic] / 2;
  int xdiff = 0;

  if (align == SCALIGN_LEFT)
    xdiff = leftSide - newLeft;
  else if (align == SCALIGN_CENTRE)
    xdiff = 0;
  else if (align == SCALIGN_RIGHT)
    xdiff = (leftSide + spritewidth[sppic]) - (newLeft + spritewidth[newpic]);
  else
    quit("!SetCharacterViewEx: invalid alignment type specified");

  chap->pic_xoffs = xdiff;
  chap->pic_yoffs = 0;
}

/* *** SCRIPT SYMBOL: [Character] Character::LockViewFrame^3 *** */
void Character_LockViewFrame(CharacterInfo *chaa, int view, int loop, int frame) {

  Character_LockView(chaa, view);
  
  view--;
  if ((loop < 0) || (loop >= views[view].numLoops))
    quit("!SetCharacterFrame: invalid loop specified");
  if ((frame < 0) || (frame >= views[view].loops[loop].numFrames))
    quit("!SetCharacterFrame: invalid frame specified");

  chaa->loop = loop;
  chaa->frame = frame;
}

/* *** SCRIPT SYMBOL: [Character] Character::LockViewOffset^3 *** */
void Character_LockViewOffset(CharacterInfo *chap, int vii, int xoffs, int yoffs) {
  Character_LockView(chap, vii);

  if ((current_screen_resolution_multiplier == 1) && (game.default_resolution >= 3)) {
    // running a 640x400 game at 320x200, adjust
    xoffs /= 2;
    yoffs /= 2;
  }
  else if ((current_screen_resolution_multiplier > 1) && (game.default_resolution <= 2)) {
    // running a 320x200 game at 640x400, adjust
    xoffs *= 2;
    yoffs *= 2;
  }

  chap->pic_xoffs = xoffs;
  chap->pic_yoffs = yoffs;
}

/* *** SCRIPT SYMBOL: [Character] Character::LoseInventory^1 *** */
void Character_LoseInventory(CharacterInfo *chap, ScriptInvItem *invi) {

  if (invi == NULL)
    quit("!LoseInventoryFromCharacter: invalid invnetory number");

  int inum = invi->id;

  if (chap->inv[inum] > 0)
    chap->inv[inum]--;

  if ((chap->activeinv == inum) & (chap->inv[inum] < 1)) {
    chap->activeinv = -1;
    if ((chap == playerchar) && (GetCursorMode() == MODE_USE))
      set_cursor_mode(0);
  }

  int charid = chap->index_id;

  if ((chap->inv[inum] == 0) || (game.options[OPT_DUPLICATEINV] > 0)) {
    int xx,tt;
    for (xx = 0; xx < charextra[charid].invorder_count; xx++) {
      if (charextra[charid].invorder[xx] == inum) {
        charextra[charid].invorder_count--;
        for (tt = xx; tt < charextra[charid].invorder_count; tt++)
          charextra[charid].invorder[tt] = charextra[charid].invorder[tt+1];
        break;
      }
    }
  }
  guis_need_update = 1;

  if (chap == playerchar)
    run_on_event (GE_LOSE_INV, inum);
}

/* *** SCRIPT SYMBOL: [Character] Character::PlaceOnWalkableArea^0 *** */
void Character_PlaceOnWalkableArea(CharacterInfo *chap) 
{
  if (displayed_room < 0)
    quit("!Character.PlaceOnWalkableArea: no room is currently loaded");

  find_nearest_walkable_area(&chap->x, &chap->y);
}

/* *** SCRIPT SYMBOL: [Character] Character::RemoveTint^0 *** */
void Character_RemoveTint(CharacterInfo *chaa) {
  
  if (chaa->flags & CHF_HASTINT) {
    DEBUG_CONSOLE("Un-tint %s", chaa->scrname);
    chaa->flags &= ~CHF_HASTINT;
  }
  else {
    debug_log("Character.RemoveTint called but character was not tinted");
  }
}

/* *** SCRIPT SYMBOL: [Character] Character::get_HasExplicitTint *** */
int Character_GetHasExplicitTint(CharacterInfo *chaa) {
  
  if (chaa->flags & CHF_HASTINT)
    return 1;

  return 0;
}

/* *** SCRIPT SYMBOL: [Character] Character::Say^101 *** */
void Character_Say(CharacterInfo *chaa, const char *texx, ...) {
  
  char displbuf[STD_BUFFER_SIZE];
  va_list ap;
  va_start(ap,texx);
  my_sprintf(displbuf,get_translation(texx),ap);
  va_end(ap);

  _DisplaySpeechCore(chaa->index_id, displbuf);

}

/* *** SCRIPT SYMBOL: [Character] Character::SayAt^4 *** */
void Character_SayAt(CharacterInfo *chaa, int x, int y, int width, const char *texx) {

  DisplaySpeechAt(x, y, width, chaa->index_id, (char*)texx);
}

/* *** SCRIPT SYMBOL: [Character] Character::SayBackground^1 *** */
ScriptOverlay* Character_SayBackground(CharacterInfo *chaa, const char *texx) {

  int ovltype = DisplaySpeechBackground(chaa->index_id, (char*)texx);
  int ovri = find_overlay_of_type(ovltype);
  if (ovri<0)
    quit("!SayBackground internal error: no overlay");

  // Convert the overlay ID to an Overlay object
  ScriptOverlay *scOver = new ScriptOverlay();
  scOver->overlayId = ovltype;
  scOver->borderHeight = 0;
  scOver->borderWidth = 0;
  scOver->isBackgroundSpeech = 1;
  int handl = ccRegisterManagedObject(scOver, scOver);
  screenover[ovri].associatedOverlayHandle = handl;

  return scOver;
}

/* *** SCRIPT SYMBOL: [Character] Character::SetAsPlayer^0 *** */
void Character_SetAsPlayer(CharacterInfo *chaa) {

    // set to same character, so ignore
  if (game.playercharacter == chaa->index_id)
    return;

  setup_player_character(chaa->index_id);

  //update_invorder();

  DEBUG_CONSOLE("%s is new player character", playerchar->scrname);

  // Within game_start, return now
  if (displayed_room < 0)
    return;

  if (displayed_room != playerchar->room)
    NewRoom(playerchar->room);
  else   // make sure it doesn't run the region interactions
    play.player_on_region = GetRegionAt (playerchar->x, playerchar->y);

  if ((playerchar->activeinv >= 0) && (playerchar->inv[playerchar->activeinv] < 1))
    playerchar->activeinv = -1;

  // They had inv selected, so change the cursor
  if (cur_mode == MODE_USE) {
    if (playerchar->activeinv < 0)
      SetNextCursor ();
    else
      SetActiveInventory (playerchar->activeinv);
  }

}


/* *** SCRIPT SYMBOL: [Character] Character::SetIdleView^2 *** */
void Character_SetIdleView(CharacterInfo *chaa, int iview, int itime) {

  if (iview == 1) 
    quit("!SetCharacterIdle: view 1 cannot be used as an idle view, sorry.");

  // if an idle anim is currently playing, release it
  if (chaa->idleleft < 0)
    Character_UnlockView(chaa);
    
  chaa->idleview = iview - 1;
  // make sure they don't appear idle while idle anim is disabled
  if (iview < 1)
    itime = 10;
  chaa->idletime = itime;
  chaa->idleleft = itime;

  // if not currently animating, reset the wait counter
  if ((chaa->animating == 0) && (chaa->walking == 0))
    chaa->wait = 0;

  if (iview >= 1) {
    DEBUG_CONSOLE("Set %s idle view to %d (time %d)", chaa->scrname, iview, itime);
  }
  else {
    DEBUG_CONSOLE("%s idle view disabled", chaa->scrname);
  }
  if (chaa->flags & CHF_FIXVIEW) {
    debug_log("SetCharacterIdle called while character view locked with SetCharacterView; idle ignored");
    DEBUG_CONSOLE("View locked, idle will not kick in until Released");
  }
  // if they switch to a swimming animation, kick it off immediately
  if (itime == 0)
    charextra[chaa->index_id].process_idle_this_time = 1;

}

void Character_SetOption(CharacterInfo *chaa, int flag, int yesorno) {

  if ((yesorno < 0) || (yesorno > 1))
    quit("!SetCharacterProperty: last parameter must be 0 or 1");

  if (flag & CHF_MANUALSCALING) {
    // backwards compatibility fix
    Character_SetIgnoreScaling(chaa, yesorno);
  }
  else {
    chaa->flags &= ~flag;
    if (yesorno)
      chaa->flags |= flag;
  }

}

/* *** SCRIPT SYMBOL: [Character] Character::SetWalkSpeed^2 *** */
void Character_SetSpeed(CharacterInfo *chaa, int xspeed, int yspeed) {

  if ((xspeed == 0) || (xspeed > 50) || (yspeed == 0) || (yspeed > 50))
    quit("!SetCharacterSpeedEx: invalid speed value");
  if (chaa->walking)
    quit("!SetCharacterSpeedEx: cannot change speed while walking");

  chaa->walkspeed = xspeed;
  
  if (yspeed == xspeed) 
    chaa->walkspeed_y = UNIFORM_WALK_SPEED;
  else
    chaa->walkspeed_y = yspeed;
}


/* *** SCRIPT SYMBOL: [Character] Character::StopMoving^0 *** */
void Character_StopMoving(CharacterInfo *charp) {

  int chaa = charp->index_id;
  if (chaa == play.skip_until_char_stops)
    EndSkippingUntilCharStops();

  if (charextra[chaa].xwas != INVALID_X) {
    charp->x = charextra[chaa].xwas;
    charp->y = charextra[chaa].ywas;
    charextra[chaa].xwas = INVALID_X;
  }
  if ((charp->walking > 0) && (charp->walking < TURNING_AROUND)) {
    // if it's not a MoveCharDirect, make sure they end up on a walkable area
    if ((mls[charp->walking].direct == 0) && (charp->room == displayed_room))
      Character_PlaceOnWalkableArea(charp);

    DEBUG_CONSOLE("%s: stop moving", charp->scrname);

    charp->idleleft = charp->idletime;
    // restart the idle animation straight away
    charextra[chaa].process_idle_this_time = 1;
  }
  if (charp->walking) {
    // If the character is currently moving, stop them and reset their frame
    charp->walking = 0;
    if ((charp->flags & CHF_MOVENOTWALK) == 0)
      charp->frame = 0;
  }
}

/* *** SCRIPT SYMBOL: [Character] Character::Tint^5 *** */
void Character_Tint(CharacterInfo *chaa, int red, int green, int blue, int opacity, int luminance) {
  if ((red < 0) || (green < 0) || (blue < 0) ||
      (red > 255) || (green > 255) || (blue > 255) ||
      (opacity < 0) || (opacity > 100) ||
      (luminance < 0) || (luminance > 100))
    quit("!Character.Tint: invalid parameter. R,G,B must be 0-255, opacity & luminance 0-100");

  DEBUG_CONSOLE("Set %s tint RGB(%d,%d,%d) %d%%", chaa->scrname, red, green, blue, opacity);

  charextra[chaa->index_id].tint_r = red;
  charextra[chaa->index_id].tint_g = green;
  charextra[chaa->index_id].tint_b = blue;
  charextra[chaa->index_id].tint_level = opacity;
  charextra[chaa->index_id].tint_light = (luminance * 25) / 10;
  chaa->flags |= CHF_HASTINT;
}

/* *** SCRIPT SYMBOL: [Character] Character::Think^101 *** */
void Character_Think(CharacterInfo *chaa, const char *texx, ...) {

  char displbuf[STD_BUFFER_SIZE];
  va_list ap;
  va_start(ap,texx);
  my_sprintf(displbuf,get_translation(texx),ap);
  va_end(ap);

  _DisplayThoughtCore(chaa->index_id, displbuf);
}

/* *** SCRIPT SYMBOL: [Character] Character::UnlockView^0 *** */
void Character_UnlockView(CharacterInfo *chaa) {
  if (chaa->flags & CHF_FIXVIEW) {
    DEBUG_CONSOLE("%s: Released view back to default", chaa->scrname);
  }
  chaa->flags &= ~CHF_FIXVIEW;
  chaa->view = chaa->defview;
  chaa->frame = 0;
  Character_StopMoving(chaa);
  if (chaa->view >= 0) {
    int maxloop = views[chaa->view].numLoops;
    if (((chaa->flags & CHF_NODIAGONAL)!=0) && (maxloop > 4))
      maxloop = 4;
    FindReasonableLoopForCharacter(chaa);
  }
  chaa->animating = 0;
  chaa->idleleft = chaa->idletime;
  chaa->pic_xoffs = 0;
  chaa->pic_yoffs = 0;
  // restart the idle animation straight away
  charextra[chaa->index_id].process_idle_this_time = 1;

}

void walk_or_move_character(CharacterInfo *chaa, int x, int y, int blocking, int direct, bool isWalk)
{
  if (chaa->on != 1)
    quit("!MoveCharacterBlocking: character is turned off and cannot be moved");

  if ((direct == ANYWHERE) || (direct == 1))
    walk_character(chaa->index_id, x, y, 1, isWalk);
  else if ((direct == WALKABLE_AREAS) || (direct == 0))
    walk_character(chaa->index_id, x, y, 0, isWalk);
  else
    quit("!Character.Walk: Direct must be ANYWHERE or WALKABLE_AREAS");

  if ((blocking == BLOCKING) || (blocking == 1))
    do_main_cycle(UNTIL_MOVEEND,(int)&chaa->walking);
  else if ((blocking != IN_BACKGROUND) && (blocking != 0))
    quit("!Character.Walk: Blocking must be BLOCKING or IN_BACKGRUOND");

}

/* *** SCRIPT SYMBOL: [Character] Character::Walk^4 *** */
void Character_Walk(CharacterInfo *chaa, int x, int y, int blocking, int direct) 
{
  walk_or_move_character(chaa, x, y, blocking, direct, true);
}

/* *** SCRIPT SYMBOL: [Character] Character::Move^4 *** */
void Character_Move(CharacterInfo *chaa, int x, int y, int blocking, int direct) 
{
  walk_or_move_character(chaa, x, y, blocking, direct, false);
}

/* *** SCRIPT SYMBOL: [Character] Character::WalkStraight^3 *** */
void Character_WalkStraight(CharacterInfo *chaa, int xx, int yy, int blocking) {

  if (chaa->room != displayed_room)
    quit("!MoveCharacterStraight: specified character not in current room");

  Character_StopMoving(chaa);
  int movetox = xx, movetoy = yy;
  
  wallscreen = prepare_walkable_areas(chaa->index_id);

  int fromXLowres = convert_to_low_res(chaa->x);
  int fromYLowres = convert_to_low_res(chaa->y);
  int toXLowres = convert_to_low_res(xx);
  int toYLowres = convert_to_low_res(yy);

  if (!can_see_from(fromXLowres, fromYLowres, toXLowres, toYLowres)) {
    movetox = convert_back_to_high_res(lastcx);
    movetoy = convert_back_to_high_res(lastcy);
  }

  walk_character(chaa->index_id, movetox, movetoy, 1, true);

  if ((blocking == BLOCKING) || (blocking == 1))
    do_main_cycle(UNTIL_MOVEEND,(int)&chaa->walking);
  else if ((blocking != IN_BACKGROUND) && (blocking != 0))
    quit("!Character.Walk: Blocking must be BLOCKING or IN_BACKGRUOND");

}


// **** CHARACTER: PROPERTIES ****


/* *** SCRIPT SYMBOL: [Character] Character::get_ActiveInventory *** */
ScriptInvItem* Character_GetActiveInventory(CharacterInfo *chaa) {

  if (chaa->activeinv <= 0)
    return NULL;

  return &scrInv[chaa->activeinv];
}

/* *** SCRIPT SYMBOL: [Character] Character::set_ActiveInventory *** */
void Character_SetActiveInventory(CharacterInfo *chaa, ScriptInvItem* iit) {
  guis_need_update = 1;

  if (iit == NULL) {
    chaa->activeinv = -1;

    if (chaa->index_id == game.playercharacter) {

      if (GetCursorMode()==MODE_USE)
        set_cursor_mode(0);
    }
    return;
  }

  if (chaa->inv[iit->id] < 1)
    quit("!SetActiveInventory: character doesn't have any of that inventory");

  chaa->activeinv = iit->id;

  if (chaa->index_id == game.playercharacter) {
    // if it's the player character, update mouse cursor
    update_inv_cursor(iit->id);
    set_cursor_mode(MODE_USE);
  }
}

/* *** SCRIPT SYMBOL: [Character] Character::get_Animating *** */
int Character_GetAnimating(CharacterInfo *chaa) {
  if (chaa->animating)
    return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_AnimationSpeed *** */
int Character_GetAnimationSpeed(CharacterInfo *chaa) {
  return chaa->animspeed;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_AnimationSpeed *** */
void Character_SetAnimationSpeed(CharacterInfo *chaa, int newval) {

  chaa->animspeed = newval;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_Baseline *** */
int Character_GetBaseline(CharacterInfo *chaa) {

  if (chaa->baseline < 1)
    return 0;

  return chaa->baseline;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_Baseline *** */
void Character_SetBaseline(CharacterInfo *chaa, int basel) {

  chaa->baseline = basel;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_BlinkInterval *** */
int Character_GetBlinkInterval(CharacterInfo *chaa) {

  return chaa->blinkinterval;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_BlinkInterval *** */
void Character_SetBlinkInterval(CharacterInfo *chaa, int interval) {

  if (interval < 0)
    quit("!SetCharacterBlinkView: invalid blink interval");

  chaa->blinkinterval = interval;

  if (chaa->blinktimer > 0)
    chaa->blinktimer = chaa->blinkinterval;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_BlinkView *** */
int Character_GetBlinkView(CharacterInfo *chaa) {

  return chaa->blinkview + 1;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_BlinkView *** */
void Character_SetBlinkView(CharacterInfo *chaa, int vii) {
  
  if (((vii < 2) || (vii > game.numviews)) && (vii != -1))
    quit("!SetCharacterBlinkView: invalid view number");

  chaa->blinkview = vii - 1;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_BlinkWhileThinking *** */
int Character_GetBlinkWhileThinking(CharacterInfo *chaa) {
  if (chaa->flags & CHF_NOBLINKANDTHINK)
    return 0;
  return 1;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_BlinkWhileThinking *** */
void Character_SetBlinkWhileThinking(CharacterInfo *chaa, int yesOrNo) {
  chaa->flags &= ~CHF_NOBLINKANDTHINK;
  if (yesOrNo == 0)
    chaa->flags |= CHF_NOBLINKANDTHINK;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_BlockingHeight *** */
int Character_GetBlockingHeight(CharacterInfo *chaa) {

  return chaa->blocking_height;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_BlockingHeight *** */
void Character_SetBlockingHeight(CharacterInfo *chaa, int hit) {

  chaa->blocking_height = hit;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_BlockingWidth *** */
int Character_GetBlockingWidth(CharacterInfo *chaa) {

  return chaa->blocking_width;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_BlockingWidth *** */
void Character_SetBlockingWidth(CharacterInfo *chaa, int wid) {

  chaa->blocking_width = wid;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_DiagonalLoops *** */
int Character_GetDiagonalWalking(CharacterInfo *chaa) {

  if (chaa->flags & CHF_NODIAGONAL)
    return 0;
  return 1;  
}

/* *** SCRIPT SYMBOL: [Character] Character::set_DiagonalLoops *** */
void Character_SetDiagonalWalking(CharacterInfo *chaa, int yesorno) {

  chaa->flags &= ~CHF_NODIAGONAL;
  if (!yesorno)
    chaa->flags |= CHF_NODIAGONAL;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_Clickable *** */
int Character_GetClickable(CharacterInfo *chaa) {
  
  if (chaa->flags & CHF_NOINTERACT)
    return 0;
  return 1;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_Clickable *** */
void Character_SetClickable(CharacterInfo *chaa, int clik) {
  
  chaa->flags &= ~CHF_NOINTERACT;
  // if they don't want it clickable, set the relevant bit
  if (clik == 0)
    chaa->flags |= CHF_NOINTERACT;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_ID *** */
int Character_GetID(CharacterInfo *chaa) {

  return chaa->index_id;

}

/* *** SCRIPT SYMBOL: [Character] Character::get_Frame *** */
int Character_GetFrame(CharacterInfo *chaa) {
  return chaa->frame;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_Frame *** */
void Character_SetFrame(CharacterInfo *chaa, int newval) {
  chaa->frame = newval;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_IdleView *** */
int Character_GetIdleView(CharacterInfo *chaa) {

  if (chaa->idleview < 1)
    return -1;

  return chaa->idleview + 1;
}

/* *** SCRIPT SYMBOL: [Character] Character::geti_InventoryQuantity *** */
int Character_GetIInventoryQuantity(CharacterInfo *chaa, int index) {
  if ((index < 1) || (index >= game.numinvitems))
    quitprintf("!Character.InventoryQuantity: invalid inventory index %d", index);

  return chaa->inv[index];
}

/* *** SCRIPT SYMBOL: [Character] Character::HasInventory^1 *** */
int Character_HasInventory(CharacterInfo *chaa, ScriptInvItem *invi)
{
  if (invi == NULL)
    quit("!Character.HasInventory: NULL inventory item supplied");

  return (chaa->inv[invi->id] > 0) ? 1 : 0;
}

/* *** SCRIPT SYMBOL: [Character] Character::seti_InventoryQuantity *** */
void Character_SetIInventoryQuantity(CharacterInfo *chaa, int index, int quant) {
  if ((index < 1) || (index >= game.numinvitems))
    quitprintf("!Character.InventoryQuantity: invalid inventory index %d", index);

  if ((quant < 0) || (quant > 32000))
    quitprintf("!Character.InventoryQuantity: invalid quantity %d", quant);

  chaa->inv[index] = quant;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_IgnoreLighting *** */
int Character_GetIgnoreLighting(CharacterInfo *chaa) {
  
  if (chaa->flags & CHF_NOLIGHTING)
    return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_IgnoreLighting *** */
void Character_SetIgnoreLighting(CharacterInfo *chaa, int yesorno) {
  
  chaa->flags &= ~CHF_NOLIGHTING;
  if (yesorno)
    chaa->flags |= CHF_NOLIGHTING;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_IgnoreScaling *** */
/* *** SCRIPT SYMBOL: [Character] Character::get_ManualScaling *** */
int Character_GetIgnoreScaling(CharacterInfo *chaa) {

  if (chaa->flags & CHF_MANUALSCALING)
    return 1;
  return 0;  
}

/* *** SCRIPT SYMBOL: [Character] Character::set_IgnoreScaling *** */
void Character_SetIgnoreScaling(CharacterInfo *chaa, int yesorno) {

  if (yesorno) {
    // when setting IgnoreScaling to 1, should reset zoom level
    // like it used to in pre-2.71
    charextra[chaa->index_id].zoom = 100;
  }
  Character_SetManualScaling(chaa, yesorno);
}

/* *** SCRIPT SYMBOL: [Character] Character::set_ManualScaling *** */
void Character_SetManualScaling(CharacterInfo *chaa, int yesorno) {

  chaa->flags &= ~CHF_MANUALSCALING;
  if (yesorno)
    chaa->flags |= CHF_MANUALSCALING;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_IgnoreWalkbehinds *** */
int Character_GetIgnoreWalkbehinds(CharacterInfo *chaa) {
  
  if (chaa->flags & CHF_NOWALKBEHINDS)
    return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_IgnoreWalkbehinds *** */
void Character_SetIgnoreWalkbehinds(CharacterInfo *chaa, int yesorno) {
  
  chaa->flags &= ~CHF_NOWALKBEHINDS;
  if (yesorno)
    chaa->flags |= CHF_NOWALKBEHINDS;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_MovementLinkedToAnimation *** */
int Character_GetMovementLinkedToAnimation(CharacterInfo *chaa) {
  
  if (chaa->flags & CHF_ANTIGLIDE)
    return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_MovementLinkedToAnimation *** */
void Character_SetMovementLinkedToAnimation(CharacterInfo *chaa, int yesorno) {
  
  chaa->flags &= ~CHF_ANTIGLIDE;
  if (yesorno)
    chaa->flags |= CHF_ANTIGLIDE;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_Loop *** */
int Character_GetLoop(CharacterInfo *chaa) {
  return chaa->loop;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_Loop *** */
void Character_SetLoop(CharacterInfo *chaa, int newval) {
  if ((newval < 0) || (newval >= views[chaa->view].numLoops))
    quit("!Character.Loop: invalid loop number for this view");

  chaa->loop = newval;

  if (chaa->frame >= views[chaa->view].loops[chaa->loop].numFrames)
    chaa->frame = 0;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_Moving *** */
int Character_GetMoving(CharacterInfo *chaa) {
  if (chaa->walking)
    return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_Name *** */
const char* Character_GetName(CharacterInfo *chaa) {
  return CreateNewScriptString(chaa->name);
}

/* *** SCRIPT SYMBOL: [Character] Character::set_Name *** */
void Character_SetName(CharacterInfo *chaa, const char *newName) {
  strncpy(chaa->name, newName, 40);
  chaa->name[39] = 0;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_NormalView *** */
int Character_GetNormalView(CharacterInfo *chaa) {
  return chaa->defview + 1;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_PreviousRoom *** */
int Character_GetPreviousRoom(CharacterInfo *chaa) {
  return chaa->prevroom;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_Room *** */
int Character_GetRoom(CharacterInfo *chaa) {
  return chaa->room;
}


/* *** SCRIPT SYMBOL: [Character] Character::get_ScaleMoveSpeed *** */
int Character_GetScaleMoveSpeed(CharacterInfo *chaa) {

  if (chaa->flags & CHF_SCALEMOVESPEED)
    return 1;
  return 0;  
}

/* *** SCRIPT SYMBOL: [Character] Character::set_ScaleMoveSpeed *** */
void Character_SetScaleMoveSpeed(CharacterInfo *chaa, int yesorno) {

  if ((yesorno < 0) || (yesorno > 1))
    quit("Character.ScaleMoveSpeed: value must be true or false (1 or 0)");

  chaa->flags &= ~CHF_SCALEMOVESPEED;
  if (yesorno)
    chaa->flags |= CHF_SCALEMOVESPEED;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_ScaleVolume *** */
int Character_GetScaleVolume(CharacterInfo *chaa) {

  if (chaa->flags & CHF_SCALEVOLUME)
    return 1;
  return 0;  
}

/* *** SCRIPT SYMBOL: [Character] Character::set_ScaleVolume *** */
void Character_SetScaleVolume(CharacterInfo *chaa, int yesorno) {

  if ((yesorno < 0) || (yesorno > 1))
    quit("Character.ScaleVolume: value must be true or false (1 or 0)");

  chaa->flags &= ~CHF_SCALEVOLUME;
  if (yesorno)
    chaa->flags |= CHF_SCALEVOLUME;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_Scaling *** */
int Character_GetScaling(CharacterInfo *chaa) {
  return charextra[chaa->index_id].zoom;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_Scaling *** */
void Character_SetScaling(CharacterInfo *chaa, int zoomlevel) {

  if ((chaa->flags & CHF_MANUALSCALING) == 0)
    quit("!Character.Scaling: cannot set property unless ManualScaling is enabled");
  if ((zoomlevel < 5) || (zoomlevel > 200))
    quit("!Character.Scaling: scaling level must be between 5 and 200%");

  charextra[chaa->index_id].zoom = zoomlevel;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_Solid *** */
int Character_GetSolid(CharacterInfo *chaa) {

  if (chaa->flags & CHF_NOBLOCKING)
    return 0;
  return 1;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_Solid *** */
void Character_SetSolid(CharacterInfo *chaa, int yesorno) {

  chaa->flags &= ~CHF_NOBLOCKING;
  if (!yesorno)
    chaa->flags |= CHF_NOBLOCKING;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_Speaking *** */
int Character_GetSpeaking(CharacterInfo *chaa) {
  if (get_character_currently_talking() == chaa->index_id)
    return 1;

  return 0;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_SpeechColor *** */
int Character_GetSpeechColor(CharacterInfo *chaa) {
  
  return chaa->talkcolor;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_SpeechColor *** */
void Character_SetSpeechColor(CharacterInfo *chaa, int ncol) {
  
  chaa->talkcolor = ncol;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_SpeechAnimationDelay *** */
int GetCharacterSpeechAnimationDelay(CharacterInfo *cha)
{
  if (game.options[OPT_OLDTALKANIMSPD])
    return play.talkanim_speed;
  else
    return cha->speech_anim_speed;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_SpeechAnimationDelay *** */
void Character_SetSpeechAnimationDelay(CharacterInfo *chaa, int newDelay) 
{
  if (game.options[OPT_OLDTALKANIMSPD])
    quit("!Character.SpeechAnimationDelay cannot be set when legacy speech animation speed is enabled");
  
  chaa->speech_anim_speed = newDelay;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_SpeechView *** */
int Character_GetSpeechView(CharacterInfo *chaa) {
  
  return chaa->talkview + 1;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_SpeechView *** */
void Character_SetSpeechView(CharacterInfo *chaa, int vii) {
  if (vii == -1) {
    chaa->talkview = -1;
    return;
  }

  if ((vii < 1) || (vii > game.numviews))
    quit("!SetCharacterSpeechView: invalid view number");
  
  chaa->talkview = vii - 1;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_ThinkView *** */
int Character_GetThinkView(CharacterInfo *chaa) {
  
  return chaa->thinkview + 1;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_ThinkView *** */
void Character_SetThinkView(CharacterInfo *chaa, int vii) {
  if (((vii < 2) || (vii > game.numviews)) && (vii != -1))
    quit("!SetCharacterThinkView: invalid view number");
  
  chaa->thinkview = vii - 1;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_Transparency *** */
int Character_GetTransparency(CharacterInfo *chaa) {
  
  if (chaa->transparency == 0)
    return 0;
  if (chaa->transparency == 255)
    return 100;

  return 100 - ((chaa->transparency * 10) / 25);
}

/* *** SCRIPT SYMBOL: [Character] Character::set_Transparency *** */
void Character_SetTransparency(CharacterInfo *chaa, int trans) {
  
  if ((trans < 0) || (trans > 100))
    quit("!SetCharTransparent: transparency value must be between 0 and 100");

  if (trans == 0)
    chaa->transparency=0;
  else if (trans == 100)
    chaa->transparency = 255;
  else
    chaa->transparency = ((100-trans) * 25) / 10;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_TurnBeforeWalking *** */
int Character_GetTurnBeforeWalking(CharacterInfo *chaa) {

  if (chaa->flags & CHF_NOTURNING)
    return 0;
  return 1;  
}

/* *** SCRIPT SYMBOL: [Character] Character::set_TurnBeforeWalking *** */
void Character_SetTurnBeforeWalking(CharacterInfo *chaa, int yesorno) {

  chaa->flags &= ~CHF_NOTURNING;
  if (!yesorno)
    chaa->flags |= CHF_NOTURNING;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_View *** */
int Character_GetView(CharacterInfo *chaa) {
  return chaa->view + 1;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_WalkSpeedX *** */
int Character_GetWalkSpeedX(CharacterInfo *chaa) {
  return chaa->walkspeed;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_WalkSpeedY *** */
int Character_GetWalkSpeedY(CharacterInfo *chaa) {
  if (chaa->walkspeed_y != UNIFORM_WALK_SPEED)
    return chaa->walkspeed_y;

  return chaa->walkspeed;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_X *** */
/* *** SCRIPT SYMBOL: [Character] Character::get_x *** */
int Character_GetX(CharacterInfo *chaa) {
  return chaa->x;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_X *** */
/* *** SCRIPT SYMBOL: [Character] Character::set_x *** */
void Character_SetX(CharacterInfo *chaa, int newval) {
  chaa->x = newval;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_Y *** */
/* *** SCRIPT SYMBOL: [Character] Character::get_y *** */
int Character_GetY(CharacterInfo *chaa) {
  return chaa->y;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_Y *** */
/* *** SCRIPT SYMBOL: [Character] Character::set_y *** */
void Character_SetY(CharacterInfo *chaa, int newval) {
  chaa->y = newval;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_Z *** */
/* *** SCRIPT SYMBOL: [Character] Character::get_z *** */
int Character_GetZ(CharacterInfo *chaa) {
  return chaa->z;
}

/* *** SCRIPT SYMBOL: [Character] Character::set_Z *** */
/* *** SCRIPT SYMBOL: [Character] Character::set_z *** */
void Character_SetZ(CharacterInfo *chaa, int newval) {
  chaa->z = newval;
}



/* *** SCRIPT SYMBOL: [Character] SetCharacterIdle *** */
void SetCharacterIdle(int who, int iview, int itime) {
  if (!is_valid_character(who))
    quit("!SetCharacterIdle: Invalid character specified");

  Character_SetIdleView(&game.chars[who], iview, itime);
}


int get_character_currently_talking() {
  if ((face_talking >= 0) && (facetalkrepeat))
    return facetalkchar->index_id;
  else if (char_speaking >= 0)
    return char_speaking;

  return -1;
}

/* *** SCRIPT SYMBOL: [Character] Character::get_SpeakingFrame *** */
int Character_GetSpeakingFrame(CharacterInfo *chaa) {

  if ((face_talking >= 0) && (facetalkrepeat))
  {
    if (facetalkchar->index_id == chaa->index_id)
    {
      return facetalkframe;
    }
  }
  else if (char_speaking >= 0)
  {
    if (char_speaking == chaa->index_id)
    {
      return chaa->frame;
    }
  }

  quit("!Character.SpeakingFrame: character is not currently speaking");
  return -1;
}



void _displayspeech(char*texx, int aschar, int xx, int yy, int widd, int isThought) {
  if (!is_valid_character(aschar))
    quit("!DisplaySpeech: invalid character");

  CharacterInfo *speakingChar = &game.chars[aschar];
  if ((speakingChar->view < 0) || (speakingChar->view >= game.numviews))
    quit("!DisplaySpeech: character has invalid view");

  if (is_text_overlay > 0)
    quit("!DisplaySpeech: speech was already displayed (nested DisplaySpeech, perhaps room script and global script conflict?)");

  EndSkippingUntilCharStops();

  said_speech_line = 1;

  int aa;
  if (play.bgspeech_stay_on_display == 0) {
    // remove any background speech
    for (aa=0;aa<numscreenover;aa++) {
      if (screenover[aa].timeout > 0) {
        remove_screen_overlay(screenover[aa].type);
        aa--;
      }
    }
  }
  said_text = 1;

  // the strings are pre-translated
  //texx = get_translation(texx);
  our_eip=150;

  int isPause = 1;
  // if the message is all .'s, don't display anything
  for (aa = 0; texx[aa] != 0; aa++) {
    if (texx[aa] != '.') {
      isPause = 0;
      break;
    }
  }

  play.messagetime = GetTextDisplayTime(texx);

  if (isPause) {
    if (update_music_at > 0)
      update_music_at += play.messagetime;
    do_main_cycle(UNTIL_INTISNEG,(int)&play.messagetime);
    return;
  }

  int textcol = speakingChar->talkcolor;
  
  // if it's 0, it won't be recognised as speech
  if (textcol == 0)
    textcol = 16;

  int allowShrink = 0;
  int bwidth = widd;
  if (bwidth < 0)
    bwidth = scrnwid/2 + scrnwid/4;

  our_eip=151;

  int useview = speakingChar->talkview;
  if (isThought) {
    useview = speakingChar->thinkview;
    // view 0 is not valid for think views
    if (useview == 0)
      useview = -1;
    // speech bubble can shrink to fit
    allowShrink = 1;
    if (speakingChar->room != displayed_room) {
      // not in room, centre it
      xx = -1;
      yy = -1;
    }
  }

  if (useview >= game.numviews)
    quitprintf("!Character.Say: attempted to use view %d for animation, but it does not exist", useview + 1);

  int tdxp = xx,tdyp = yy;
  int oldview=-1, oldloop = -1;
  int ovr_type = 0;

  text_lips_offset = 0;
  text_lips_text = texx;

  block closeupface=NULL;
  if (texx[0]=='&') {
    // auto-speech
    int igr=atoi(&texx[1]);
    while ((texx[0]!=' ') & (texx[0]!=0)) texx++;
    if (texx[0]==' ') texx++;
    if (igr <= 0)
      quit("DisplaySpeech: auto-voice symbol '&' not followed by valid integer");

    text_lips_text = texx;

    if (play_speech(aschar,igr)) {
      if (play.want_speech == 2)
        texx = "  ";  // speech only, no text.
    }
  }
  if (game.options[OPT_SPEECHTYPE] == 3)
    remove_screen_overlay(OVER_COMPLETE);
  our_eip=1500;

  if (game.options[OPT_SPEECHTYPE] == 0)
    allowShrink = 1;

  if (speakingChar->idleleft < 0)  {
    // if idle anim in progress for the character, stop it
    ReleaseCharacterView(aschar);
//    speakingChar->idleleft = speakingChar->idletime;
  }

  bool overlayPositionFixed = false;
  int charFrameWas = 0;
  int viewWasLocked = 0;
  if (speakingChar->flags & CHF_FIXVIEW)
    viewWasLocked = 1;

  /*if ((speakingChar->room == displayed_room) ||
      ((useview >= 0) && (game.options[OPT_SPEECHTYPE] > 0)) ) {*/

  if (speakingChar->room == displayed_room) {
    // If the character is in this room, go for it - otherwise
    // run the "else" clause which  does text in the middle of
    // the screen.
    our_eip=1501;
    if (tdxp < 0)
      tdxp = multiply_up_coordinate(speakingChar->x) - offsetx;
    if (tdxp < 2)
      tdxp=2;

    if (speakingChar->walking)
      StopMoving(aschar);

    // save the frame we need to go back to
    // if they were moving, this will be 0 (because we just called
    // StopMoving); otherwise, it might be a specific animation 
    // frame which we should return to
    if (viewWasLocked)
      charFrameWas = speakingChar->frame;

    // if the current loop doesn't exist in talking view, use loop 0
    if (speakingChar->loop >= views[speakingChar->view].numLoops)
      speakingChar->loop = 0;

    if ((speakingChar->view < 0) || 
        (speakingChar->loop >= views[speakingChar->view].numLoops) ||
        (views[speakingChar->view].loops[speakingChar->loop].numFrames < 1))
    {
      quitprintf("Unable to display speech because the character %s has an invalid view frame (View %d, loop %d, frame %d)", speakingChar->scrname, speakingChar->view + 1, speakingChar->loop, speakingChar->frame);
    }

    our_eip=1504;

    if (tdyp < 0) 
    {
      int sppic = views[speakingChar->view].loops[speakingChar->loop].frames[0].pic;
      tdyp = multiply_up_coordinate(speakingChar->get_effective_y()) - offsety - get_fixed_pixel_size(5);
      if (charextra[aschar].height < 1)
        tdyp -= spriteheight[sppic];
      else
        tdyp -= charextra[aschar].height;
      // if it's a thought, lift it a bit further up
      if (isThought)  
        tdyp -= get_fixed_pixel_size(10);
    }

    our_eip=1505;
    if (tdyp < 5)
      tdyp=5;
      
    tdxp=-tdxp;  // tell it to centre it
    our_eip=152;

    if ((useview >= 0) && (game.options[OPT_SPEECHTYPE] > 0)) {
      // Sierra-style close-up portrait

      if (play.swap_portrait_lastchar != aschar) {
        // if the portraits are set to Alternate, OR they are
        // set to Left but swap_portrait has been set to 1 (the old
        // method for enabling it), then swap them round
        if ((game.options[OPT_PORTRAITSIDE] == PORTRAIT_ALTERNATE) ||
            ((game.options[OPT_PORTRAITSIDE] == 0) &&
             (play.swap_portrait_side > 0))) {

          if (play.swap_portrait_side == 2)
            play.swap_portrait_side = 1;
          else
            play.swap_portrait_side = 2;
        }

        if (game.options[OPT_PORTRAITSIDE] == PORTRAIT_XPOSITION) {
          // Portrait side based on character X-positions
          if (play.swap_portrait_lastchar < 0) {
            // no previous character been spoken to
            // therefore, find another character in this room
            // that it could be
            for (int ce = 0; ce < game.numcharacters; ce++) {
              if ((game.chars[ce].room == speakingChar->room) &&
                  (game.chars[ce].on == 1) &&
                  (ce != aschar)) {
                play.swap_portrait_lastchar = ce;
                break;
              }
            }
          }

          if (play.swap_portrait_lastchar >= 0) {
            // if this character is right of the one before, put the
            // portrait on the right
            if (speakingChar->x > game.chars[play.swap_portrait_lastchar].x)
              play.swap_portrait_side = -1;
            else
              play.swap_portrait_side = 0;
          }
        }

        play.swap_portrait_lastchar = aschar;
      }

      // Determine whether to display the portrait on the left or right
      int portrait_on_right = 0;

      if (game.options[OPT_SPEECHTYPE] == 3) 
        { }  // always on left with QFG-style speech
      else if ((play.swap_portrait_side == 1) ||
          (play.swap_portrait_side == -1) ||
          (game.options[OPT_PORTRAITSIDE] == PORTRAIT_RIGHT))
        portrait_on_right = 1;


      int bigx=0,bigy=0,kk;
      ViewStruct*viptr=&views[useview];
      for (kk = 0; kk < viptr->loops[0].numFrames; kk++) 
      {
        int tw = spritewidth[viptr->loops[0].frames[kk].pic];
        if (tw > bigx) bigx=tw;
        tw = spriteheight[viptr->loops[0].frames[kk].pic];
        if (tw > bigy) bigy=tw;
      }

      // if they accidentally used a large full-screen image as the sierra-style
      // talk view, correct it
      if ((game.options[OPT_SPEECHTYPE] != 3) && (bigx > scrnwid - get_fixed_pixel_size(50)))
        bigx = scrnwid - get_fixed_pixel_size(50);

      if (widd > 0)
        bwidth = widd - bigx;

      our_eip=153;
      int draw_yp = 0, ovr_yp = get_fixed_pixel_size(20);
      if (game.options[OPT_SPEECHTYPE] == 3) {
        // QFG4-style whole screen picture
        closeupface = create_bitmap_ex(bitmap_color_depth(spriteset[viptr->loops[0].frames[0].pic]), scrnwid, scrnhit);
        clear_to_color(closeupface, 0);
        draw_yp = scrnhit/2 - spriteheight[viptr->loops[0].frames[0].pic]/2;
        bigx = scrnwid/2 - get_fixed_pixel_size(20);
        ovr_type = OVER_COMPLETE;
        ovr_yp = 0;
        tdyp = -1;  // center vertically
      }
      else {
        // KQ6-style close-up face picture
        if (yy < 0)
          ovr_yp = adjust_y_for_guis (ovr_yp);
        else
          ovr_yp = yy;

        closeupface = create_bitmap_ex(bitmap_color_depth(spriteset[viptr->loops[0].frames[0].pic]),bigx+1,bigy+1);
        clear_to_color(closeupface,bitmap_mask_color(closeupface));
        ovr_type = OVER_PICTURE;

        if (yy < 0)
          tdyp = ovr_yp + get_textwindow_top_border_height(play.speech_textwindow_gui);
      }
      //draw_sprite(closeupface,spriteset[viptr->frames[0][0].pic],0,draw_yp);
      DrawViewFrame(closeupface, &viptr->loops[0].frames[0], 0, draw_yp);

      int overlay_x = get_fixed_pixel_size(10);

      if (xx < 0) {
        tdxp = get_fixed_pixel_size(16) + bigx + get_textwindow_border_width(play.speech_textwindow_gui) / 2;

        int maxWidth = (scrnwid - tdxp) - get_fixed_pixel_size(5) - 
             get_textwindow_border_width (play.speech_textwindow_gui) / 2;

        if (bwidth > maxWidth)
          bwidth = maxWidth;
      }
      else {
        tdxp = xx + bigx + get_fixed_pixel_size(8);
        overlay_x = xx;
      }

      // allow the text box to be shrunk to fit the text
      allowShrink = 1;

      // if the portrait's on the right, swap it round
      if (portrait_on_right) {
        if ((xx < 0) || (widd < 0)) {
          overlay_x = (scrnwid - bigx) - get_fixed_pixel_size(5);
          tdxp = get_fixed_pixel_size(9);
        }
        else {
          overlay_x = (xx + widd - bigx) - get_fixed_pixel_size(5);
          tdxp = xx;
        }
        tdxp += get_textwindow_border_width(play.speech_textwindow_gui) / 2;
        allowShrink = 2;
      }
      if (game.options[OPT_SPEECHTYPE] == 3)
        overlay_x = 0;
      face_talking=add_screen_overlay(overlay_x,ovr_yp,ovr_type,closeupface);
      facetalkframe = 0;
      facetalkwait = viptr->loops[0].frames[0].speed + GetCharacterSpeechAnimationDelay(speakingChar);
      facetalkloop = 0;
      facetalkview = useview;
      facetalkrepeat = (isThought) ? 0 : 1;
      facetalkBlinkLoop = 0;
      facetalkAllowBlink = 1;
      if ((isThought) && (speakingChar->flags & CHF_NOBLINKANDTHINK))
        facetalkAllowBlink = 0;
      facetalkchar = &game.chars[aschar];
      if (facetalkchar->blinktimer < 0)
        facetalkchar->blinktimer = facetalkchar->blinkinterval;
      textcol=-textcol;
      overlayPositionFixed = true;
    }
    else if (useview >= 0) {
      // Lucasarts-style speech
      our_eip=154;

      oldview = speakingChar->view;
      oldloop = speakingChar->loop;
      speakingChar->animating = 1 | (GetCharacterSpeechAnimationDelay(speakingChar) << 8);
      // only repeat if speech, not thought
      if (!isThought)
        speakingChar->animating |= CHANIM_REPEAT;

      speakingChar->view = useview;
      speakingChar->frame=0;
      speakingChar->flags|=CHF_FIXVIEW;

      if (speakingChar->loop >= views[speakingChar->view].numLoops)
      {
        // current character loop is outside the normal talking directions
        speakingChar->loop = 0;
      }

      facetalkBlinkLoop = speakingChar->loop;

      if ((speakingChar->loop >= views[speakingChar->view].numLoops) ||
          (views[speakingChar->view].loops[speakingChar->loop].numFrames < 1))
      {
        quitprintf("!Unable to display speech because the character %s has an invalid speech view (View %d, loop %d, frame %d)", speakingChar->scrname, speakingChar->view + 1, speakingChar->loop, speakingChar->frame);
      }

      // set up the speed of the first frame
      speakingChar->wait = GetCharacterSpeechAnimationDelay(speakingChar) + 
                           views[speakingChar->view].loops[speakingChar->loop].frames[0].speed;

      if (widd < 0) {
        bwidth = scrnwid/2 + scrnwid/6;
        // If they are close to the screen edge, make the text narrower
        int relx = multiply_up_coordinate(speakingChar->x) - offsetx;
        if ((relx < scrnwid / 4) || (relx > scrnwid - (scrnwid / 4)))
          bwidth -= scrnwid / 5;
      }
/*   this causes the text to bob up and down as they talk
      tdxp = OVR_AUTOPLACE;
      tdyp = aschar;*/
      if (!isThought)  // set up the lip sync if not thinking
        char_speaking = aschar;

    }
  }
  else
    allowShrink = 1;

  // it wants the centred position, so make it so
  if ((xx >= 0) && (tdxp < 0))
    tdxp -= widd / 2;

  // if they used DisplaySpeechAt, then use the supplied width
  if ((widd > 0) && (isThought == 0))
    allowShrink = 0;

  our_eip=155;
  _display_at(tdxp,tdyp,bwidth,texx,0,textcol, isThought, allowShrink, overlayPositionFixed);
  our_eip=156;
  if ((play.in_conversation > 0) && (game.options[OPT_SPEECHTYPE] == 3))
    closeupface = NULL;
  if (closeupface!=NULL)
    remove_screen_overlay(ovr_type);
  screen_is_dirty = 1;
  face_talking = -1;
  facetalkchar = NULL;
  our_eip=157;
  if (oldview>=0) {
    speakingChar->flags &= ~CHF_FIXVIEW;
    if (viewWasLocked)
      speakingChar->flags |= CHF_FIXVIEW;
    speakingChar->view=oldview;
    speakingChar->loop = oldloop;
    speakingChar->animating=0;
    speakingChar->frame = charFrameWas;
    speakingChar->wait=0;
    speakingChar->idleleft = speakingChar->idletime;
    // restart the idle animation straight away
    charextra[aschar].process_idle_this_time = 1;
  }
  char_speaking = -1;
  stop_speech();
}


void DisplaySpeech(char*texx, int aschar) {
  _displayspeech (texx, aschar, -1, -1, -1, 0);
}

// **** THIS IS UNDOCUMENTED BECAUSE IT DOESN'T WORK PROPERLY
// **** AT 640x400 AND DOESN'T USE THE RIGHT SPEECH STYLE
/* *** SCRIPT SYMBOL: [Character] DisplaySpeechAt *** */
void DisplaySpeechAt (int xx, int yy, int wii, int aschar, char*spch) {
  multiply_up_coordinates(&xx, &yy);
  wii = multiply_up_coordinate(wii);
  _displayspeech (get_translation(spch), aschar, xx, yy, wii, 0);
}



void _DisplaySpeechCore(int chid, char *displbuf) {
  if (displbuf[0] == 0) {
    // no text, just update the current character who's speaking
    // this allows the portrait side to be switched with an empty
    // speech line
    play.swap_portrait_lastchar = chid;
    return;
  }

  // adjust timing of text (so that DisplaySpeech("%s", str) pauses
  // for the length of the string not 2 frames)
  if ((int)strlen(displbuf) > source_text_length + 3)
    source_text_length = strlen(displbuf);

  DisplaySpeech(displbuf, chid);
}

/* *** SCRIPT SYMBOL: [Character] DisplaySpeech *** */
void __sc_displayspeech(int chid,char*texx, ...) {
  if ((chid<0) || (chid>=game.numcharacters))
    quit("!DisplaySpeech: invalid character specified");

  char displbuf[STD_BUFFER_SIZE];
  va_list ap;
  va_start(ap,texx);
  my_sprintf(displbuf, get_translation(texx), ap);
  va_end(ap);

  _DisplaySpeechCore(chid, displbuf);

}


void _DisplayThoughtCore(int chid, const char *displbuf) {
  // adjust timing of text (so that DisplayThought("%s", str) pauses
  // for the length of the string not 2 frames)
  if ((int)strlen(displbuf) > source_text_length + 3)
    source_text_length = strlen(displbuf);

  int xpp = -1, ypp = -1, width = -1;

  if ((game.options[OPT_SPEECHTYPE] == 0) || (game.chars[chid].thinkview <= 0)) {
    // lucasarts-style, so we want a speech bubble actually above
    // their head (or if they have no think anim in Sierra-style)
    width = multiply_up_coordinate(play.speech_bubble_width);
    xpp = (multiply_up_coordinate(game.chars[chid].x) - offsetx) - width / 2;
    if (xpp < 0)
      xpp = 0;
    // -1 will automatically put it above the char's head
    ypp = -1;
  }

  _displayspeech ((char*)displbuf, chid, xpp, ypp, width, 1);
}

/* *** SCRIPT SYMBOL: [Character] DisplayThought *** */
void DisplayThought(int chid, const char*texx, ...) {
  if ((chid < 0) || (chid >= game.numcharacters))
    quit("!DisplayThought: invalid character specified");

  char displbuf[STD_BUFFER_SIZE];
  va_list ap;
  va_start(ap,texx);
  my_sprintf(displbuf,get_translation(texx),ap);
  va_end(ap);

  _DisplayThoughtCore(chid, displbuf);
}


/* *** SCRIPT SYMBOL: [Character] SetCharacterBaseline *** */
void SetCharacterBaseline (int obn, int basel) {
  if (!is_valid_character(obn)) quit("!SetCharacterBaseline: invalid object number specified");
  
  Character_SetBaseline(&game.chars[obn], basel);
}

// pass trans=0 for fully solid, trans=100 for fully transparent
/* *** SCRIPT SYMBOL: [Character] SetCharacterTransparency *** */
void SetCharacterTransparency(int obn,int trans) {
  if (!is_valid_character(obn))
    quit("!SetCharTransparent: invalid character number specified");
  
  Character_SetTransparency(&game.chars[obn], trans);
}

/* *** SCRIPT SYMBOL: [Character] AnimateCharacter *** */
void scAnimateCharacter (int chh, int loopn, int sppd, int rept) {
  if (!is_valid_character(chh))
    quit("AnimateCharacter: invalid character");

  animate_character(&game.chars[chh], loopn, sppd, rept);
}

/* *** SCRIPT SYMBOL: [Character] AnimateCharacterEx *** */
void AnimateCharacterEx(int chh, int loopn, int sppd, int rept, int direction, int blocking) {
  if ((direction < 0) || (direction > 1))
    quit("!AnimateCharacterEx: invalid direction");
  if (!is_valid_character(chh))
    quit("AnimateCharacter: invalid character");

  if (direction)
    direction = BACKWARDS;
  else
    direction = FORWARDS;

  if (blocking)
    blocking = BLOCKING;
  else
    blocking = IN_BACKGROUND;

  Character_Animate(&game.chars[chh], loopn, sppd, rept, blocking, direction);

}

void animate_character(CharacterInfo *chap, int loopn,int sppd,int rept, int noidleoverride, int direction) {

  if ((chap->view < 0) || (chap->view > game.numviews)) {
    quitprintf("!AnimateCharacter: you need to set the view number first\n"
      "(trying to animate '%s' using loop %d. View is currently %d).",chap->name,loopn,chap->view+1);
  }
  DEBUG_CONSOLE("%s: Start anim view %d loop %d, spd %d, repeat %d", chap->scrname, chap->view+1, loopn, sppd, rept);
  if ((chap->idleleft < 0) && (noidleoverride == 0)) {
    // if idle view in progress for the character (and this is not the
    // "start idle animation" animate_character call), stop the idle anim
    Character_UnlockView(chap);
    chap->idleleft=chap->idletime;
  }
  if ((loopn < 0) || (loopn >= views[chap->view].numLoops))
    quit("!AnimateCharacter: invalid loop number specified");
  Character_StopMoving(chap);
  chap->animating=1;
  if (rept) chap->animating |= CHANIM_REPEAT;
  if (direction) chap->animating |= CHANIM_BACKWARDS;

  chap->animating|=((sppd << 8) & 0xff00);
  chap->loop=loopn;
  
  if (direction) {
    chap->frame = views[chap->view].loops[loopn].numFrames - 1;
  }
  else
    chap->frame=0;

  chap->wait = sppd + views[chap->view].loops[loopn].frames[chap->frame].speed;
  CheckViewFrameForCharacter(chap);
}

block GetCharacterImage(int charid, int *isFlipped) 
{
  if (!gfxDriver->HasAcceleratedStretchAndFlip())
  {
    if (actsps[charid + MAX_INIT_SPR] != NULL) 
    {
      // the actsps image is pre-flipped, so no longer register the image as such
      if (isFlipped)
        *isFlipped = 0;
      return actsps[charid + MAX_INIT_SPR];
    }
  }
  CharacterInfo*chin=&game.chars[charid];
  int sppic = views[chin->view].loops[chin->loop].frames[chin->frame].pic;
  return spriteset[sppic];
}


/* *** SCRIPT SYMBOL: [Character] AreCharObjColliding *** */
int AreCharObjColliding(int charid,int objid) {
  if (!is_valid_character(charid))
    quit("!AreCharObjColliding: invalid character");
  if (!is_valid_object(objid))
    quit("!AreCharObjColliding: invalid object number");

  return Character_IsCollidingWithObject(&game.chars[charid], &scrObj[objid]);
}

/* *** SCRIPT SYMBOL: [Character] AreCharactersColliding *** */
int AreCharactersColliding(int cchar1,int cchar2) {
  if (!is_valid_character(cchar1))
    quit("!AreCharactersColliding: invalid char1");
  if (!is_valid_character(cchar2))
    quit("!AreCharactersColliding: invalid char2");

  return Character_IsCollidingWithChar(&game.chars[cchar1], &game.chars[cchar2]);
}


/* *** SCRIPT SYMBOL: [Character] AddInventory *** */
void add_inventory(int inum) {
  if ((inum < 0) || (inum >= MAX_INV))
    quit("!AddInventory: invalid inventory number");

  Character_AddInventory(playerchar, &scrInv[inum], SCR_NO_VALUE);

  play.obsolete_inv_numorder = charextra[game.playercharacter].invorder_count;
}

/* *** SCRIPT SYMBOL: [Character] LoseInventory *** */
void lose_inventory(int inum) {
  if ((inum < 0) || (inum >= MAX_INV))
    quit("!LoseInventory: invalid inventory number");

  Character_LoseInventory(playerchar, &scrInv[inum]);

  play.obsolete_inv_numorder = charextra[game.playercharacter].invorder_count;
}

/* *** SCRIPT SYMBOL: [Character] AddInventoryToCharacter *** */
void AddInventoryToCharacter(int charid, int inum) {
  if (!is_valid_character(charid))
    quit("!AddInventoryToCharacter: invalid character specified");
  if ((inum < 1) || (inum >= game.numinvitems))
    quit("!AddInventory: invalid inv item specified");

  Character_AddInventory(&game.chars[charid], &scrInv[inum], SCR_NO_VALUE);
}

/* *** SCRIPT SYMBOL: [Character] LoseInventoryFromCharacter *** */
void LoseInventoryFromCharacter(int charid, int inum) {
  if (!is_valid_character(charid))
    quit("!LoseInventoryFromCharacter: invalid character specified");
  if ((inum < 1) || (inum >= game.numinvitems))
    quit("!AddInventory: invalid inv item specified");
  
  Character_LoseInventory(&game.chars[charid], &scrInv[inum]);
}


/* *** SCRIPT SYMBOL: [Character] GetCharacterAt *** */
int GetCharacterAt (int xx, int yy) {
  xx += divide_down_coordinate(offsetx);
  yy += divide_down_coordinate(offsety);
  return is_pos_on_character(xx,yy);
}

/* *** SCRIPT SYMBOL: [Character] Character::GetAtScreenXY^2 *** */
CharacterInfo *GetCharacterAtLocation(int xx, int yy) {
  int hsnum = GetCharacterAt(xx, yy);
  if (hsnum < 0)
    return NULL;
  return &game.chars[hsnum];
}

/* *** SCRIPT SYMBOL: [Character] RunCharacterInteraction *** */
void RunCharacterInteraction (int cc, int mood) {
  if (!is_valid_character(cc))
    quit("!RunCharacterInteraction: invalid character");

  int passon=-1,cdata=-1;
  if (mood==MODE_LOOK) passon=0;
  else if (mood==MODE_HAND) passon=1;
  else if (mood==MODE_TALK) passon=2;
  else if (mood==MODE_USE) { passon=3;
    cdata=playerchar->activeinv;
    play.usedinv=cdata;
    }
  else if (mood==MODE_PICKUP) passon = 5;
  else if (mood==MODE_CUSTOM1) passon = 6;
  else if (mood==MODE_CUSTOM2) passon = 7;

  evblockbasename="character%d"; evblocknum=cc;
  if (game.charScripts != NULL) 
  {
    if (passon>=0)
      run_interaction_script(game.charScripts[cc], passon, 4, (passon == 3));
    run_interaction_script(game.charScripts[cc], 4);  // any click on char
  }
  else 
  {
    if (passon>=0)
      run_interaction_event(game.intrChar[cc],passon, 4, (passon == 3));
    run_interaction_event(game.intrChar[cc],4);  // any click on char
  }
}

/* *** SCRIPT SYMBOL: [Character] Character::RunInteraction^1 *** */
void Character_RunInteraction(CharacterInfo *chaa, int mood) {

  RunCharacterInteraction(chaa->index_id, mood);
}


int check_click_on_character(int xx,int yy,int mood) {
  int lowestwas=is_pos_on_character(xx,yy);
  if (lowestwas>=0) {
    RunCharacterInteraction (lowestwas, mood);
    return 1;
  }
  return 0;
}



/* *** SCRIPT SYMBOL: [Character] GetCharacterProperty *** */
int GetCharacterProperty (int cha, const char *property) {
  if (!is_valid_character(cha))
    quit("!GetCharacterProperty: invalid character");
  return get_int_property (&game.charProps[cha], property);
}
/* *** SCRIPT SYMBOL: [Character] Character::GetProperty^1 *** */
int Character_GetProperty(CharacterInfo *chaa, const char *property) {

  return get_int_property(&game.charProps[chaa->index_id], property);

}

/* *** SCRIPT SYMBOL: [Character] GetCharacterPropertyText *** */
void GetCharacterPropertyText (int item, const char *property, char *bufer) {
  get_text_property (&game.charProps[item], property, bufer);
}
/* *** SCRIPT SYMBOL: [Character] Character::GetPropertyText^2 *** */
void Character_GetPropertyText(CharacterInfo *chaa, const char *property, char *bufer) {
  get_text_property(&game.charProps[chaa->index_id], property, bufer);
}
/* *** SCRIPT SYMBOL: [Character] Character::GetTextProperty^1 *** */
const char* Character_GetTextProperty(CharacterInfo *chaa, const char *property) {
  return get_text_property_dynamic_string(&game.charProps[chaa->index_id], property);
}


/* *** SCRIPT SYMBOL: [Character] SetPlayerCharacter *** */
void SetPlayerCharacter(int newchar) {
  if (!is_valid_character(newchar))
    quit("!SetPlayerCharacter: Invalid character specified");

  Character_SetAsPlayer(&game.chars[newchar]);
}

/* *** SCRIPT SYMBOL: [Character] FollowCharacterEx *** */
void FollowCharacterEx(int who, int tofollow, int distaway, int eagerness) {
  if (!is_valid_character(who))
    quit("!FollowCharacter: Invalid character specified");
  CharacterInfo *chtofollow;
  if (tofollow == -1)
    chtofollow = NULL;
  else if (!is_valid_character(tofollow))
    quit("!FollowCharacterEx: invalid character to follow");
  else
    chtofollow = &game.chars[tofollow];

  Character_FollowCharacter(&game.chars[who], chtofollow, distaway, eagerness);
}

/* *** SCRIPT SYMBOL: [Character] FollowCharacter *** */
void FollowCharacter(int who, int tofollow) {
  FollowCharacterEx(who,tofollow,10,97);
  }

/* *** SCRIPT SYMBOL: [Character] SetCharacterIgnoreLight *** */
void SetCharacterIgnoreLight (int who, int yesorno) {
  if (!is_valid_character(who))
    quit("!SetCharacterIgnoreLight: Invalid character specified");

  Character_SetIgnoreLighting(&game.chars[who], yesorno);
}

/* *** SCRIPT SYMBOL: [Character] SetCharacterProperty *** */
void SetCharacterProperty (int who, int flag, int yesorno) {
  if (!is_valid_character(who))
    quit("!SetCharacterProperty: Invalid character specified");

  Character_SetOption(&game.chars[who], flag, yesorno);
}


/* *** SCRIPT SYMBOL: [Character] NewRoom *** */
void NewRoom(int nrnum) {
  if (nrnum < 0)
    quitprintf("!NewRoom: room change requested to invalid room number %d.", nrnum);

  if (displayed_room < 0) {
    // called from game_start; change the room where the game will start
    playerchar->room = nrnum;
    return;
  }

  
  DEBUG_CONSOLE("Room change requested to room %d", nrnum);
  EndSkippingUntilCharStops();

  can_run_delayed_command();

  if (play.stop_dialog_at_end != DIALOG_NONE) {
    if (play.stop_dialog_at_end == DIALOG_RUNNING)
      play.stop_dialog_at_end = DIALOG_NEWROOM + nrnum;
    else
      quit("!NewRoom: two NewRoom/RunDialog/StopDialog requests within dialog");
    return;
  }

  if (in_leaves_screen >= 0) {
    // NewRoom called from the Player Leaves Screen event -- just
    // change which room it will go to
    in_leaves_screen = nrnum;
  }
  else if (in_enters_screen) {
    setevent(EV_NEWROOM,nrnum);
    return;
  }
  else if (in_inv_screen) {
    inv_screen_newroom = nrnum;
    return;
  }
  else if ((inside_script==0) & (in_graph_script==0)) {
    new_room(nrnum,playerchar);
    return;
  }
  else if (inside_script) {
    curscript->queue_action(ePSANewRoom, nrnum, "NewRoom");
    // we might be within a MoveCharacterBlocking -- the room
    // change should abort it
    if ((playerchar->walking > 0) && (playerchar->walking < TURNING_AROUND)) {
      // nasty hack - make sure it doesn't move the character
      // to a walkable area
      mls[playerchar->walking].direct = 1;
      StopMoving(game.playercharacter);
    }
  }
  else if (in_graph_script)
    gs_to_newroom = nrnum;
}

/* *** SCRIPT SYMBOL: [Character] NewRoomEx *** */
void NewRoomEx(int nrnum,int newx,int newy) {

  Character_ChangeRoom(playerchar, nrnum, newx, newy);

}

/* *** SCRIPT SYMBOL: [Character] NewRoomNPC *** */
void NewRoomNPC(int charid, int nrnum, int newx, int newy) {
  if (!is_valid_character(charid))
    quit("!NewRoomNPC: invalid character");
  if (charid == game.playercharacter)
    quit("!NewRoomNPC: use NewRoomEx with the player character");

  Character_ChangeRoom(&game.chars[charid], nrnum, newx, newy);
}


/* *** SCRIPT SYMBOL: [Character] MoveCharacter *** */
void MoveCharacter(int cc,int xx,int yy) {
  walk_character(cc,xx,yy,0, true);
}
/* *** SCRIPT SYMBOL: [Character] MoveCharacterDirect *** */
void MoveCharacterDirect(int cc,int xx, int yy) {
  walk_character(cc,xx,yy,1, true);
}
/* *** SCRIPT SYMBOL: [Character] MoveCharacterStraight *** */
void MoveCharacterStraight(int cc,int xx, int yy) {
  if (!is_valid_character(cc))
    quit("!MoveCharacterStraight: invalid character specified");
  
  Character_WalkStraight(&game.chars[cc], xx, yy, IN_BACKGROUND);
}

// Append to character path
/* *** SCRIPT SYMBOL: [Character] MoveCharacterPath *** */
void MoveCharacterPath (int chac, int tox, int toy) {
  if (!is_valid_character(chac))
    quit("!MoveCharacterPath: invalid character specified");

  Character_AddWaypoint(&game.chars[chac], tox, toy);
}

/* *** SCRIPT SYMBOL: [Character] SetCharacterSpeedEx *** */
void SetCharacterSpeedEx(int chaa, int xspeed, int yspeed) {
  if (!is_valid_character(chaa))
    quit("!SetCharacterSpeedEx: invalid character");

  Character_SetSpeed(&game.chars[chaa], xspeed, yspeed);

}

/* *** SCRIPT SYMBOL: [Character] SetCharacterSpeed *** */
void SetCharacterSpeed(int chaa,int nspeed) {
  SetCharacterSpeedEx(chaa, nspeed, nspeed);
}

/* *** SCRIPT SYMBOL: [Character] SetTalkingColor *** */
void SetTalkingColor(int chaa,int ncol) {
  if (!is_valid_character(chaa)) quit("!SetTalkingColor: invalid character");
  
  Character_SetSpeechColor(&game.chars[chaa], ncol);
}

/* *** SCRIPT SYMBOL: [Character] SetCharacterSpeechView *** */
void SetCharacterSpeechView (int chaa, int vii) {
  if (!is_valid_character(chaa))
    quit("!SetCharacterSpeechView: invalid character specified");
  
  Character_SetSpeechView(&game.chars[chaa], vii);
}

/* *** SCRIPT SYMBOL: [Character] SetCharacterBlinkView *** */
void SetCharacterBlinkView (int chaa, int vii, int intrv) {
  if (!is_valid_character(chaa))
    quit("!SetCharacterBlinkView: invalid character specified");

  Character_SetBlinkView(&game.chars[chaa], vii);
  Character_SetBlinkInterval(&game.chars[chaa], intrv);
}

/* *** SCRIPT SYMBOL: [Character] SetCharacterView *** */
void SetCharacterView(int chaa,int vii) {
  if (!is_valid_character(chaa))
    quit("!SetCharacterView: invalid character specified");
  
  Character_LockView(&game.chars[chaa], vii);
}

/* *** SCRIPT SYMBOL: [Character] SetCharacterFrame *** */
void SetCharacterFrame(int chaa, int view, int loop, int frame) {

  Character_LockViewFrame(&game.chars[chaa], view, loop, frame);
}

// similar to SetCharView, but aligns the frame to make it line up
/* *** SCRIPT SYMBOL: [Character] SetCharacterViewEx *** */
void SetCharacterViewEx (int chaa, int vii, int loop, int align) {
  
  Character_LockViewAligned(&game.chars[chaa], vii, loop, align);
}

/* *** SCRIPT SYMBOL: [Character] SetCharacterViewOffset *** */
void SetCharacterViewOffset (int chaa, int vii, int xoffs, int yoffs) {

  Character_LockViewOffset(&game.chars[chaa], vii, xoffs, yoffs);
}


/* *** SCRIPT SYMBOL: [Character] ChangeCharacterView *** */
void ChangeCharacterView(int chaa,int vii) {
  if (!is_valid_character(chaa))
    quit("!ChangeCharacterView: invalid character specified");
  
  Character_ChangeView(&game.chars[chaa], vii);
}

/* *** SCRIPT SYMBOL: [Character] SetCharacterClickable *** */
void SetCharacterClickable (int cha, int clik) {
  if (!is_valid_character(cha))
    quit("!SetCharacterClickable: Invalid character specified");
  // make the character clicklabe (reset "No interaction" bit)
  game.chars[cha].flags&=~CHF_NOINTERACT;
  // if they don't want it clickable, set the relevant bit
  if (clik == 0)
    game.chars[cha].flags|=CHF_NOINTERACT;
  }

/* *** SCRIPT SYMBOL: [Character] SetCharacterIgnoreWalkbehinds *** */
void SetCharacterIgnoreWalkbehinds (int cha, int clik) {
  if (!is_valid_character(cha))
    quit("!SetCharacterIgnoreWalkbehinds: Invalid character specified");

  Character_SetIgnoreWalkbehinds(&game.chars[cha], clik);
}

/* *** SCRIPT SYMBOL: [Game] MoveCharacterToObject *** */
void MoveCharacterToObject(int chaa,int obbj) {
  // invalid object, do nothing
  // this allows MoveCharacterToObject(EGO, GetObjectAt(...));
  if (!is_valid_object(obbj))
    return;

  walk_character(chaa,objs[obbj].x+5,objs[obbj].y+6,0, true);
  do_main_cycle(UNTIL_MOVEEND,(int)&game.chars[chaa].walking);
}



/* *** SCRIPT SYMBOL: [Character] MoveCharacterBlocking *** */
void MoveCharacterBlocking(int chaa,int xx,int yy,int direct) {
  if (!is_valid_character (chaa))
    quit("!MoveCharacterBlocking: invalid character");

  // check if they try to move the player when Hide Player Char is
  // ticked -- otherwise this will hang the game
  if (game.chars[chaa].on != 1)
    quit("!MoveCharacterBlocking: character is turned off (is Hide Player Character selected?) and cannot be moved");

  if (direct)
    MoveCharacterDirect(chaa,xx,yy);
  else
    MoveCharacter(chaa,xx,yy);
  do_main_cycle(UNTIL_MOVEEND,(int)&game.chars[chaa].walking);
  }




void register_character_script_functions() {
    scAdd_External_Symbol("Character::AddInventory^2",(void *)Character_AddInventory);
  scAdd_External_Symbol("Character::AddWaypoint^2",(void *)Character_AddWaypoint);
  scAdd_External_Symbol("Character::Animate^5",(void *)Character_Animate);
  scAdd_External_Symbol("Character::ChangeRoom^3",(void *)Character_ChangeRoom);
  scAdd_External_Symbol("Character::ChangeRoomAutoPosition^2",(void *)Character_ChangeRoomAutoPosition);
  scAdd_External_Symbol("Character::ChangeView^1",(void *)Character_ChangeView);
  scAdd_External_Symbol("Character::FaceCharacter^2",(void *)Character_FaceCharacter);
  scAdd_External_Symbol("Character::FaceLocation^3",(void *)Character_FaceLocation);
  scAdd_External_Symbol("Character::FaceObject^2",(void *)Character_FaceObject);
  scAdd_External_Symbol("Character::FollowCharacter^3",(void *)Character_FollowCharacter);
  scAdd_External_Symbol("Character::GetProperty^1",(void *)Character_GetProperty);
  scAdd_External_Symbol("Character::GetPropertyText^2",(void *)Character_GetPropertyText);
  scAdd_External_Symbol("Character::GetTextProperty^1",(void *)Character_GetTextProperty);
  scAdd_External_Symbol("Character::HasInventory^1",(void *)Character_HasInventory);
  scAdd_External_Symbol("Character::IsCollidingWithChar^1",(void *)Character_IsCollidingWithChar);
  scAdd_External_Symbol("Character::IsCollidingWithObject^1",(void *)Character_IsCollidingWithObject);
  scAdd_External_Symbol("Character::LockView^1",(void *)Character_LockView);
  scAdd_External_Symbol("Character::LockViewAligned^3",(void *)Character_LockViewAligned);
  scAdd_External_Symbol("Character::LockViewFrame^3",(void *)Character_LockViewFrame);
  scAdd_External_Symbol("Character::LockViewOffset^3",(void *)Character_LockViewOffset);
  scAdd_External_Symbol("Character::LoseInventory^1",(void *)Character_LoseInventory);
  scAdd_External_Symbol("Character::Move^4",(void *)Character_Move);
  scAdd_External_Symbol("Character::PlaceOnWalkableArea^0",(void *)Character_PlaceOnWalkableArea);
  scAdd_External_Symbol("Character::RemoveTint^0",(void *)Character_RemoveTint);
  scAdd_External_Symbol("Character::RunInteraction^1",(void *)Character_RunInteraction);
  scAdd_External_Symbol("Character::Say^101",(void *)Character_Say);
  scAdd_External_Symbol("Character::SayAt^4",(void *)Character_SayAt);
  scAdd_External_Symbol("Character::SayBackground^1",(void *)Character_SayBackground);
  scAdd_External_Symbol("Character::SetAsPlayer^0",(void *)Character_SetAsPlayer);
  scAdd_External_Symbol("Character::SetIdleView^2",(void *)Character_SetIdleView);
  //scAdd_External_Symbol("Character::SetOption^2",(void *)Character_SetOption);
  scAdd_External_Symbol("Character::SetWalkSpeed^2",(void *)Character_SetSpeed);
  scAdd_External_Symbol("Character::StopMoving^0",(void *)Character_StopMoving);
  scAdd_External_Symbol("Character::Think^101",(void *)Character_Think);
  scAdd_External_Symbol("Character::Tint^5",(void *)Character_Tint);
  scAdd_External_Symbol("Character::UnlockView^0",(void *)Character_UnlockView);
  scAdd_External_Symbol("Character::Walk^4",(void *)Character_Walk);
  scAdd_External_Symbol("Character::WalkStraight^3",(void *)Character_WalkStraight);

  // static
  scAdd_External_Symbol("Character::GetAtScreenXY^2", (void *)GetCharacterAtLocation);

  scAdd_External_Symbol("Character::get_ActiveInventory",(void *)Character_GetActiveInventory);
  scAdd_External_Symbol("Character::set_ActiveInventory",(void *)Character_SetActiveInventory);
  scAdd_External_Symbol("Character::get_Animating", (void *)Character_GetAnimating);
  scAdd_External_Symbol("Character::get_AnimationSpeed", (void *)Character_GetAnimationSpeed);
  scAdd_External_Symbol("Character::set_AnimationSpeed", (void *)Character_SetAnimationSpeed);
  scAdd_External_Symbol("Character::get_Baseline",(void *)Character_GetBaseline);
  scAdd_External_Symbol("Character::set_Baseline",(void *)Character_SetBaseline);
  scAdd_External_Symbol("Character::get_BlinkInterval",(void *)Character_GetBlinkInterval);
  scAdd_External_Symbol("Character::set_BlinkInterval",(void *)Character_SetBlinkInterval);
  scAdd_External_Symbol("Character::get_BlinkView",(void *)Character_GetBlinkView);
  scAdd_External_Symbol("Character::set_BlinkView",(void *)Character_SetBlinkView);
  scAdd_External_Symbol("Character::get_BlinkWhileThinking",(void *)Character_GetBlinkWhileThinking);
  scAdd_External_Symbol("Character::set_BlinkWhileThinking",(void *)Character_SetBlinkWhileThinking);
  scAdd_External_Symbol("Character::get_BlockingHeight",(void *)Character_GetBlockingHeight);
  scAdd_External_Symbol("Character::set_BlockingHeight",(void *)Character_SetBlockingHeight);
  scAdd_External_Symbol("Character::get_BlockingWidth",(void *)Character_GetBlockingWidth);
  scAdd_External_Symbol("Character::set_BlockingWidth",(void *)Character_SetBlockingWidth);
  scAdd_External_Symbol("Character::get_Clickable",(void *)Character_GetClickable);
  scAdd_External_Symbol("Character::set_Clickable",(void *)Character_SetClickable);
  scAdd_External_Symbol("Character::get_DiagonalLoops", (void *)Character_GetDiagonalWalking);
  scAdd_External_Symbol("Character::set_DiagonalLoops", (void *)Character_SetDiagonalWalking);
  scAdd_External_Symbol("Character::get_Frame", (void *)Character_GetFrame);
  scAdd_External_Symbol("Character::set_Frame", (void *)Character_SetFrame);
  scAdd_External_Symbol("Character::get_HasExplicitTint", (void *)Character_GetHasExplicitTint);
  scAdd_External_Symbol("Character::get_ID", (void *)Character_GetID);
  scAdd_External_Symbol("Character::get_IdleView", (void *)Character_GetIdleView);
  scAdd_External_Symbol("Character::geti_InventoryQuantity", (void *)Character_GetIInventoryQuantity);
  scAdd_External_Symbol("Character::seti_InventoryQuantity", (void *)Character_SetIInventoryQuantity);
  scAdd_External_Symbol("Character::get_IgnoreLighting",(void *)Character_GetIgnoreLighting);
  scAdd_External_Symbol("Character::set_IgnoreLighting",(void *)Character_SetIgnoreLighting);
  scAdd_External_Symbol("Character::get_IgnoreScaling", (void *)Character_GetIgnoreScaling);
  scAdd_External_Symbol("Character::set_IgnoreScaling", (void *)Character_SetIgnoreScaling);
  scAdd_External_Symbol("Character::get_IgnoreWalkbehinds",(void *)Character_GetIgnoreWalkbehinds);
  scAdd_External_Symbol("Character::set_IgnoreWalkbehinds",(void *)Character_SetIgnoreWalkbehinds);
  scAdd_External_Symbol("Character::get_Loop", (void *)Character_GetLoop);
  scAdd_External_Symbol("Character::set_Loop", (void *)Character_SetLoop);
  scAdd_External_Symbol("Character::get_ManualScaling", (void *)Character_GetIgnoreScaling);
  scAdd_External_Symbol("Character::set_ManualScaling", (void *)Character_SetManualScaling);
  scAdd_External_Symbol("Character::get_MovementLinkedToAnimation",(void *)Character_GetMovementLinkedToAnimation);
  scAdd_External_Symbol("Character::set_MovementLinkedToAnimation",(void *)Character_SetMovementLinkedToAnimation);
  scAdd_External_Symbol("Character::get_Moving", (void *)Character_GetMoving);
  scAdd_External_Symbol("Character::get_Name", (void *)Character_GetName);
  scAdd_External_Symbol("Character::set_Name", (void *)Character_SetName);
  scAdd_External_Symbol("Character::get_NormalView",(void *)Character_GetNormalView);
  scAdd_External_Symbol("Character::get_PreviousRoom",(void *)Character_GetPreviousRoom);
  scAdd_External_Symbol("Character::get_Room",(void *)Character_GetRoom);
  scAdd_External_Symbol("Character::get_ScaleMoveSpeed", (void *)Character_GetScaleMoveSpeed);
  scAdd_External_Symbol("Character::set_ScaleMoveSpeed", (void *)Character_SetScaleMoveSpeed);
  scAdd_External_Symbol("Character::get_ScaleVolume", (void *)Character_GetScaleVolume);
  scAdd_External_Symbol("Character::set_ScaleVolume", (void *)Character_SetScaleVolume);
  scAdd_External_Symbol("Character::get_Scaling", (void *)Character_GetScaling);
  scAdd_External_Symbol("Character::set_Scaling", (void *)Character_SetScaling);
  scAdd_External_Symbol("Character::get_Solid", (void *)Character_GetSolid);
  scAdd_External_Symbol("Character::set_Solid", (void *)Character_SetSolid);
  scAdd_External_Symbol("Character::get_Speaking", (void *)Character_GetSpeaking);
  scAdd_External_Symbol("Character::get_SpeakingFrame", (void *)Character_GetSpeakingFrame);
  scAdd_External_Symbol("Character::get_SpeechAnimationDelay",(void *)GetCharacterSpeechAnimationDelay);
  scAdd_External_Symbol("Character::set_SpeechAnimationDelay",(void *)Character_SetSpeechAnimationDelay);
  scAdd_External_Symbol("Character::get_SpeechColor",(void *)Character_GetSpeechColor);
  scAdd_External_Symbol("Character::set_SpeechColor",(void *)Character_SetSpeechColor);
  scAdd_External_Symbol("Character::get_SpeechView",(void *)Character_GetSpeechView);
  scAdd_External_Symbol("Character::set_SpeechView",(void *)Character_SetSpeechView);
  scAdd_External_Symbol("Character::get_ThinkView",(void *)Character_GetThinkView);
  scAdd_External_Symbol("Character::set_ThinkView",(void *)Character_SetThinkView);
  scAdd_External_Symbol("Character::get_Transparency",(void *)Character_GetTransparency);
  scAdd_External_Symbol("Character::set_Transparency",(void *)Character_SetTransparency);
  scAdd_External_Symbol("Character::get_TurnBeforeWalking", (void *)Character_GetTurnBeforeWalking);
  scAdd_External_Symbol("Character::set_TurnBeforeWalking", (void *)Character_SetTurnBeforeWalking);
  scAdd_External_Symbol("Character::get_View", (void *)Character_GetView);
  scAdd_External_Symbol("Character::get_WalkSpeedX", (void *)Character_GetWalkSpeedX);
  scAdd_External_Symbol("Character::get_WalkSpeedY", (void *)Character_GetWalkSpeedY);
  scAdd_External_Symbol("Character::get_X", (void *)Character_GetX);
  scAdd_External_Symbol("Character::set_X", (void *)Character_SetX);
  scAdd_External_Symbol("Character::get_x", (void *)Character_GetX);
  scAdd_External_Symbol("Character::set_x", (void *)Character_SetX);
  scAdd_External_Symbol("Character::get_Y", (void *)Character_GetY);
  scAdd_External_Symbol("Character::set_Y", (void *)Character_SetY);
  scAdd_External_Symbol("Character::get_y", (void *)Character_GetY);
  scAdd_External_Symbol("Character::set_y", (void *)Character_SetY);
  scAdd_External_Symbol("Character::get_Z", (void *)Character_GetZ);
  scAdd_External_Symbol("Character::set_Z", (void *)Character_SetZ);
  scAdd_External_Symbol("Character::get_z", (void *)Character_GetZ);
  scAdd_External_Symbol("Character::set_z", (void *)Character_SetZ);
  scAdd_External_Symbol("FaceLocation",(void *)FaceLocation);
  scAdd_External_Symbol("ReleaseCharacterView",(void *)ReleaseCharacterView);
  scAdd_External_Symbol("StopMoving",(void *)StopMoving);
  scAdd_External_Symbol("AddInventory",(void *)add_inventory);
  scAdd_External_Symbol("AddInventoryToCharacter",(void *)AddInventoryToCharacter);
  scAdd_External_Symbol("AnimateCharacter",(void *)scAnimateCharacter);
  scAdd_External_Symbol("AnimateCharacterEx",(void *)AnimateCharacterEx);
  scAdd_External_Symbol("AreCharactersColliding",(void *)AreCharactersColliding);
  scAdd_External_Symbol("AreCharObjColliding",(void *)AreCharObjColliding);
  scAdd_External_Symbol("ChangeCharacterView",(void *)ChangeCharacterView);
  scAdd_External_Symbol("DisplaySpeech",(void *)__sc_displayspeech);
  scAdd_External_Symbol("DisplaySpeechAt", (void *)DisplaySpeechAt);
  scAdd_External_Symbol("DisplayThought",(void *)DisplayThought);
  scAdd_External_Symbol("FollowCharacter",(void *)FollowCharacter);
  scAdd_External_Symbol("FollowCharacterEx",(void *)FollowCharacterEx);
  scAdd_External_Symbol("GetCharacterAt",(void *)GetCharacterAt);
  scAdd_External_Symbol("GetCharacterProperty",(void *)GetCharacterProperty);
  scAdd_External_Symbol("GetCharacterPropertyText",(void *)GetCharacterPropertyText);
  scAdd_External_Symbol("LoseInventory",(void *)lose_inventory);
  scAdd_External_Symbol("LoseInventoryFromCharacter",(void *)LoseInventoryFromCharacter);
  scAdd_External_Symbol("MoveCharacter",(void *)MoveCharacter);
  scAdd_External_Symbol("MoveCharacterBlocking",(void *)MoveCharacterBlocking);
  scAdd_External_Symbol("MoveCharacterDirect",(void *)MoveCharacterDirect);
  scAdd_External_Symbol("MoveCharacterPath",(void *)MoveCharacterPath);
  scAdd_External_Symbol("MoveCharacterStraight",(void *)MoveCharacterStraight);
  scAdd_External_Symbol("MoveCharacterToObject",(void *)MoveCharacterToObject);
  scAdd_External_Symbol("NewRoom",(void *)NewRoom);
  scAdd_External_Symbol("NewRoomEx",(void *)NewRoomEx);
  scAdd_External_Symbol("NewRoomNPC",(void *)NewRoomNPC);
  scAdd_External_Symbol("SetCharacterBaseline",(void *)SetCharacterBaseline);
  scAdd_External_Symbol("SetCharacterClickable",(void *)SetCharacterClickable);
  scAdd_External_Symbol("SetCharacterFrame",(void *)SetCharacterFrame);
  scAdd_External_Symbol("SetCharacterIdle",(void *)SetCharacterIdle);
  scAdd_External_Symbol("SetCharacterIgnoreLight",(void *)SetCharacterIgnoreLight);
  scAdd_External_Symbol("SetCharacterIgnoreWalkbehinds",(void *)SetCharacterIgnoreWalkbehinds);
  scAdd_External_Symbol("SetCharacterProperty",(void *)SetCharacterProperty);
  scAdd_External_Symbol("SetCharacterBlinkView",(void *)SetCharacterBlinkView);
  scAdd_External_Symbol("SetCharacterSpeechView",(void *)SetCharacterSpeechView);
  scAdd_External_Symbol("SetCharacterSpeed",(void *)SetCharacterSpeed);
  scAdd_External_Symbol("SetCharacterSpeedEx",(void *)SetCharacterSpeedEx);
  scAdd_External_Symbol("SetCharacterTransparency",(void *)SetCharacterTransparency);
  scAdd_External_Symbol("SetCharacterView",(void *)SetCharacterView);
  scAdd_External_Symbol("SetCharacterViewEx",(void *)SetCharacterViewEx);
  scAdd_External_Symbol("SetCharacterViewOffset",(void *)SetCharacterViewOffset);
  scAdd_External_Symbol("RunCharacterInteraction",(void *)RunCharacterInteraction);
  scAdd_External_Symbol("SetPlayerCharacter",(void *)SetPlayerCharacter);
  scAdd_External_Symbol("SetTalkingColor",(void *)SetTalkingColor);
  scAdd_External_Symbol("FaceCharacter",(void *)FaceCharacter);
  scAdd_External_Symbol("MoveToWalkableArea", (void *)MoveToWalkableArea);
}