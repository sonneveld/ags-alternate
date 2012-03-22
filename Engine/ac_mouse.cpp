#include "ac_mouse.h"

#include "ac.h"
#include "ac_context.h"
#include "acgui.h"
#include "ali3d.h"
#include "sprcache.h"
#include "acgfx.h"
#include "bmp.h"

void update_script_mouse_coords() {
  global_mouse_state.scmouse.x = divide_down_coordinate(mousex);
  global_mouse_state.scmouse.y = divide_down_coordinate(mousey);
}

/* *** SCRIPT SYMBOL: [Mouse] HideMouseCursor *** */
static void HideMouseCursor () {
  play.mouse_cursor_hidden = 1;
}

/* *** SCRIPT SYMBOL: [Mouse] ShowMouseCursor *** */
static void ShowMouseCursor () {
  play.mouse_cursor_hidden = 0;
}

// The Mouse:: functions are static so the script doesn't pass
// in an object parameter
/* *** SCRIPT SYMBOL: [Mouse] Mouse::set_Visible *** */
static void Mouse_SetVisible(int isOn) {
  if (isOn)
    ShowMouseCursor();
  else
    HideMouseCursor();
}

/* *** SCRIPT SYMBOL: [Mouse] Mouse::get_Visible *** */
static int Mouse_GetVisible() {
  if (play.mouse_cursor_hidden)
    return 0;
  return 1;
}

#define MOUSE_MAX_Y divide_down_coordinate(vesa_yres)
/* *** SCRIPT SYMBOL: [Mouse] Mouse::SetBounds^4 *** */
/* *** SCRIPT SYMBOL: [Mouse] SetMouseBounds *** */
void SetMouseBounds (int x1, int y1, int x2, int y2) {
  if ((x1 == 0) && (y1 == 0) && (x2 == 0) && (y2 == 0)) {
    x2 = BASEWIDTH-1;
    y2 = MOUSE_MAX_Y - 1;
  }
  if (x2 == BASEWIDTH) x2 = BASEWIDTH-1;
  if (y2 == MOUSE_MAX_Y) y2 = MOUSE_MAX_Y - 1;
  if ((x1 > x2) || (y1 > y2) || (x1 < 0) || (x2 >= BASEWIDTH) ||
      (y1 < 0) || (y2 >= MOUSE_MAX_Y))
    quit("!SetMouseBounds: invalid co-ordinates, must be within (0,0) - (320,200)");
  DEBUG_CONSOLE("Mouse bounds constrained to (%d,%d)-(%d,%d)", x1, y1, x2, y2);
  multiply_up_coordinates(&x1, &y1);
  multiply_up_coordinates_round_up(&x2, &y2);
 
  play.mboundx1 = x1;
  play.mboundx2 = x2;
  play.mboundy1 = y1;
  play.mboundy2 = y2;
  filter->SetMouseLimit(x1,y1,x2,y2);
}




static void update_cached_mouse_cursor() 
{
  if (global_mouse_state.mouseCursor != NULL)
    gfxDriver->DestroyDDB(global_mouse_state.mouseCursor);
  global_mouse_state.mouseCursor = gfxDriver->CreateDDBFromBitmap(mousecurs[0], alpha_blend_cursor != 0);
}

void set_new_cursor_graphic (int spriteslot) {
  mousecurs[0] = spriteset[spriteslot];

  if ((spriteslot < 1) || (mousecurs[0] == NULL))
  {
    if (global_mouse_state.blank_mouse_cursor == NULL)
    {
      global_mouse_state.blank_mouse_cursor = create_bitmap_ex(final_col_dep, 1, 1);
      clear_to_color(global_mouse_state.blank_mouse_cursor, bitmap_mask_color(global_mouse_state.blank_mouse_cursor));
    }
    mousecurs[0] = global_mouse_state.blank_mouse_cursor;
  }

  if (game.spriteflags[spriteslot] & SPF_ALPHACHANNEL)
    alpha_blend_cursor = 1;
  else
    alpha_blend_cursor = 0;

  update_cached_mouse_cursor();
}


static void putpixel_compensate (block onto, int xx,int yy, int col) {
  if ((bitmap_color_depth(onto) == 32) && (col != 0)) {
    // ensure the alpha channel is preserved if it has one
    int alphaval = geta32(getpixel(onto, xx, yy));
    col = makeacol32(getr32(col), getg32(col), getb32(col), alphaval);
  }
  rectfill(onto, xx, yy, xx + get_fixed_pixel_size(1) - 1, yy + get_fixed_pixel_size(1) - 1, col);
}

// mouse cursor functions:
// set_mouse_cursor: changes visual appearance to specified cursor
/* *** SCRIPT SYMBOL: [Mouse] Mouse::UseModeGraphic^1 *** */
/* *** SCRIPT SYMBOL: [Mouse] SetMouseCursor *** */
void set_mouse_cursor(int newcurs) {
  int hotspotx = game.mcurs[newcurs].hotx, hotspoty = game.mcurs[newcurs].hoty;

  set_new_cursor_graphic(game.mcurs[newcurs].pic);
  if (global_mouse_state.dotted_mouse_cursor) {
    wfreeblock (global_mouse_state.dotted_mouse_cursor);
    global_mouse_state.dotted_mouse_cursor = NULL;
  }

  if ((newcurs == MODE_USE) && (game.mcurs[newcurs].pic > 0) &&
      ((game.hotdot > 0) || (game.invhotdotsprite > 0)) ) {
    // If necessary, create a copy of the cursor and put the hotspot
    // dot onto it
    global_mouse_state.dotted_mouse_cursor = create_bitmap_ex (bitmap_color_depth(mousecurs[0]), BMP_W(mousecurs[0]),BMP_H(mousecurs[0]));
    blit (mousecurs[0], global_mouse_state.dotted_mouse_cursor, 0, 0, 0, 0, BMP_W(mousecurs[0]), BMP_H(mousecurs[0]));

    if (game.invhotdotsprite > 0) {
      block abufWas = abuf;
      abuf = global_mouse_state.dotted_mouse_cursor;

      draw_sprite_support_alpha(
        hotspotx - spritewidth[game.invhotdotsprite] / 2,
        hotspoty - spriteheight[game.invhotdotsprite] / 2,
        spriteset[game.invhotdotsprite],
        game.invhotdotsprite);

      abuf = abufWas;
    }
    else {
      putpixel_compensate (global_mouse_state.dotted_mouse_cursor, hotspotx, hotspoty,
        (bitmap_color_depth(global_mouse_state.dotted_mouse_cursor) > 8) ? get_col8_lookup (game.hotdot) : game.hotdot);

      if (game.hotdotouter > 0) {
        int outercol = game.hotdotouter;
        if (bitmap_color_depth (global_mouse_state.dotted_mouse_cursor) > 8)
          outercol = get_col8_lookup(game.hotdotouter);

        putpixel_compensate (global_mouse_state.dotted_mouse_cursor, hotspotx + get_fixed_pixel_size(1), hotspoty, outercol);
        putpixel_compensate (global_mouse_state.dotted_mouse_cursor, hotspotx, hotspoty + get_fixed_pixel_size(1), outercol);
        putpixel_compensate (global_mouse_state.dotted_mouse_cursor, hotspotx - get_fixed_pixel_size(1), hotspoty, outercol);
        putpixel_compensate (global_mouse_state.dotted_mouse_cursor, hotspotx, hotspoty - get_fixed_pixel_size(1), outercol);
      }
    }
    mousecurs[0] = global_mouse_state.dotted_mouse_cursor;
    update_cached_mouse_cursor();
  }
  msethotspot(hotspotx, hotspoty);
  if (newcurs != cur_cursor)
  {
    cur_cursor = newcurs;
    global_mouse_state.mouse_frame=0;
    global_mouse_state.mouse_delay=0;
  }
}


// set_default_cursor: resets visual appearance to current mode (walk, look, etc)
/* *** SCRIPT SYMBOL: [Mouse] Mouse::UseDefaultGraphic^0 *** */
/* *** SCRIPT SYMBOL: [Mouse] SetDefaultCursor *** */
void set_default_cursor() {
  set_mouse_cursor(cur_mode);
  }

// permanently change cursor graphic
/* *** SCRIPT SYMBOL: [Mouse] Mouse::ChangeModeGraphic^2 *** */
/* *** SCRIPT SYMBOL: [Mouse] ChangeCursorGraphic *** */
static void ChangeCursorGraphic (int curs, int newslot) {
  if ((curs < 0) || (curs >= game.numcursors))
    quit("!ChangeCursorGraphic: invalid mouse cursor");

  if ((curs == MODE_USE) && (game.options[OPT_FIXEDINVCURSOR] == 0))
    debug_log("Mouse.ChangeModeGraphic should not be used on the Inventory cursor when the cursor is linked to the active inventory item");

  game.mcurs[curs].pic = newslot;
  spriteset.precache (newslot);
  if (curs == cur_mode)
    set_mouse_cursor (curs);
}

/* *** SCRIPT SYMBOL: [Mouse] Mouse::GetModeGraphic^1 *** */
static int Mouse_GetModeGraphic(int curs) {
  if ((curs < 0) || (curs >= game.numcursors))
    quit("!Mouse.GetModeGraphic: invalid mouse cursor");

  return game.mcurs[curs].pic;
}

/* *** SCRIPT SYMBOL: [Mouse] Mouse::ChangeModeHotspot^3 *** */
/* *** SCRIPT SYMBOL: [Mouse] ChangeCursorHotspot *** */
static void ChangeCursorHotspot (int curs, int x, int y) {
  if ((curs < 0) || (curs >= game.numcursors))
    quit("!ChangeCursorHotspot: invalid mouse cursor");
  game.mcurs[curs].hotx = multiply_up_coordinate(x);
  game.mcurs[curs].hoty = multiply_up_coordinate(y);
  if (curs == cur_cursor)
    set_mouse_cursor (cur_cursor);
}

/* *** SCRIPT SYMBOL: [Mouse] Mouse::ChangeModeView^2 *** */
static void Mouse_ChangeModeView(int curs, int newview) {
  if ((curs < 0) || (curs >= game.numcursors))
    quit("!Mouse.ChangeModeView: invalid mouse cursor");

  newview--;

  game.mcurs[curs].view = newview;

  if (newview >= 0)
  {
    precache_view(newview);
  }

  if (curs == cur_cursor)
    global_mouse_state.mouse_delay = 0;  // force update
}


static int find_next_enabled_cursor(int startwith) {
  if (startwith >= game.numcursors)
    startwith = 0;
  int testing=startwith;
  do {
    if ((game.mcurs[testing].flags & MCF_DISABLED)==0) {
      // inventory cursor, and they have an active item
      if (testing == MODE_USE) 
      {
        if (playerchar->activeinv > 0)
          break;
      }
      // standard cursor that's not disabled, go with it
      else if (game.mcurs[testing].flags & MCF_STANDARD)
        break;
    }

    testing++;
    if (testing >= game.numcursors) testing=0;
  } while (testing!=startwith);

  if (testing!=startwith)
    set_cursor_mode(testing);

  return testing;
}

/* *** SCRIPT SYMBOL: [Mouse] Mouse::SelectNextMode^0 *** */
/* *** SCRIPT SYMBOL: [Mouse] SetNextCursorMode *** */
void SetNextCursor () {
  set_cursor_mode (find_next_enabled_cursor(cur_mode + 1));
}

// set_cursor_mode: changes mode and appearance
/* *** SCRIPT SYMBOL: [Mouse] Mouse::set_Mode *** */
/* *** SCRIPT SYMBOL: [Mouse] SetCursorMode *** */
void set_cursor_mode(int newmode) {
  if ((newmode < 0) || (newmode >= game.numcursors))
    quit("!SetCursorMode: invalid cursor mode specified");

  guis_need_update = 1;
  if (game.mcurs[newmode].flags & MCF_DISABLED) {
    find_next_enabled_cursor(newmode);
    return; }
  if (newmode == MODE_USE) {
    if (playerchar->activeinv == -1) {
      find_next_enabled_cursor(0);
      return;
      }
    update_inv_cursor(playerchar->activeinv);
    }
  cur_mode=newmode;
  set_default_cursor();

  DEBUG_CONSOLE("Cursor mode set to %d", newmode);
}


/* *** SCRIPT SYMBOL: [Mouse] Mouse::EnableMode^1 *** */
/* *** SCRIPT SYMBOL: [Mouse] EnableCursorMode *** */
void enable_cursor_mode(int modd) {
  game.mcurs[modd].flags&=~MCF_DISABLED;
  // now search the interfaces for related buttons to re-enable
  int uu,ww;

  for (uu=0;uu<game.numgui;uu++) {
    for (ww=0;ww<guis[uu].numobjs;ww++) {
      if ((guis[uu].objrefptr[ww] >> 16)!=GOBJ_BUTTON) continue;
      GUIButton*gbpt=(GUIButton*)guis[uu].objs[ww];
      if (gbpt->leftclick!=IBACT_SETMODE) continue;
      if (gbpt->lclickdata!=modd) continue;
      gbpt->Enable();
      }
    }
  guis_need_update = 1;
  }

/* *** SCRIPT SYMBOL: [Mouse] Mouse::DisableMode^1 *** */
/* *** SCRIPT SYMBOL: [Mouse] DisableCursorMode *** */
void disable_cursor_mode(int modd) {
  game.mcurs[modd].flags|=MCF_DISABLED;
  // now search the interfaces for related buttons to kill
  int uu,ww;

  for (uu=0;uu<game.numgui;uu++) {
    for (ww=0;ww<guis[uu].numobjs;ww++) {
      if ((guis[uu].objrefptr[ww] >> 16)!=GOBJ_BUTTON) continue;
      GUIButton*gbpt=(GUIButton*)guis[uu].objs[ww];
      if (gbpt->leftclick!=IBACT_SETMODE) continue;
      if (gbpt->lclickdata!=modd) continue;
      gbpt->Disable();
      }
    }
  if (cur_mode==modd) find_next_enabled_cursor(modd);
  guis_need_update = 1;
  }


/* *** SCRIPT SYMBOL: [Mouse] Mouse::IsButtonDown^1 *** */
/* *** SCRIPT SYMBOL: [Mouse] IsButtonDown *** */
static int IsButtonDown(int which) {
  if ((which < 1) || (which > 3))
    quit("!IsButtonDown: only works with eMouseLeft, eMouseRight, eMouseMiddle");
  if (ac_misbuttondown(which-1))
    return 1;
  return 0;
}


/* *** SCRIPT SYMBOL: [Mouse] Mouse::Update^0 *** */
/* *** SCRIPT SYMBOL: [Mouse] RefreshMouse *** */
void RefreshMouse() {
  ac_domouse(DOMOUSE_NOCURSOR);
  global_mouse_state.scmouse.x = divide_down_coordinate(mousex);
  global_mouse_state.scmouse.y = divide_down_coordinate(mousey);
}

/* *** SCRIPT SYMBOL: [Mouse] Mouse::SetPosition^2 *** */
/* *** SCRIPT SYMBOL: [Mouse] SetMousePosition *** */
static void SetMousePosition (int newx, int newy) {
  if (newx < 0)
    newx = 0;
  if (newy < 0)
    newy = 0;
  if (newx >= BASEWIDTH)
    newx = BASEWIDTH - 1;
  if (newy >= GetMaxScreenHeight())
    newy = GetMaxScreenHeight() - 1;

  multiply_up_coordinates(&newx, &newy);
  filter->SetMousePosition(newx, newy);
  RefreshMouse();
}

/* *** SCRIPT SYMBOL: [Mouse] Mouse::get_Mode *** */
/* *** SCRIPT SYMBOL: [Mouse] GetCursorMode *** */
int GetCursorMode() {
  return cur_mode;
}

static int GetMouseCursor() {
  return cur_cursor;
}


/* *** SCRIPT SYMBOL: [Mouse] Mouse::SaveCursorUntilItLeaves^0 *** */
/* *** SCRIPT SYMBOL: [Mouse] SaveCursorForLocationChange *** */
static void SaveCursorForLocationChange() {
  // update the current location name
  char tempo[100];
  GetLocationName(divide_down_coordinate(mousex), divide_down_coordinate(mousey), tempo);

  if (play.get_loc_name_save_cursor != play.get_loc_name_last_time) {
    play.get_loc_name_save_cursor = play.get_loc_name_last_time;
    play.restore_cursor_mode_to = GetCursorMode();
    play.restore_cursor_image_to = GetMouseCursor();
    DEBUG_CONSOLE("Saving mouse: mode %d cursor %d", play.restore_cursor_mode_to, play.restore_cursor_image_to);
  }
}


void update_animating_cursor() {
  // update animating mouse cursor
  if (game.mcurs[cur_cursor].view>=0) {
    ac_domouse (DOMOUSE_NOCURSOR);
    // only on mousemove, and it's not moving
    if (((game.mcurs[cur_cursor].flags & MCF_ANIMMOVE)!=0) &&
      (mousex==global_mouse_state.lastmx) && (mousey==global_mouse_state.lastmy)) ;
    // only on hotspot, and it's not on one
    else if (((game.mcurs[cur_cursor].flags & MCF_HOTSPOT)!=0) &&
        (GetLocationType(divide_down_coordinate(mousex), divide_down_coordinate(mousey)) == 0))
      set_new_cursor_graphic(game.mcurs[cur_cursor].pic);
    else if (global_mouse_state.mouse_delay>0) global_mouse_state.mouse_delay--;
    else {
      int viewnum=game.mcurs[cur_cursor].view;
      int loopnum=0;
      if (loopnum >= views[viewnum].numLoops)
        quitprintf("An animating mouse cursor is using view %d which has no loops", viewnum + 1);
      if (views[viewnum].loops[loopnum].numFrames < 1)
        quitprintf("An animating mouse cursor is using view %d which has no frames in loop %d", viewnum + 1, loopnum);

      global_mouse_state.mouse_frame++;
      if (global_mouse_state.mouse_frame >= views[viewnum].loops[loopnum].numFrames)
        global_mouse_state.mouse_frame=0;
      set_new_cursor_graphic(views[viewnum].loops[loopnum].frames[global_mouse_state.mouse_frame].pic);
      global_mouse_state.mouse_delay = views[viewnum].loops[loopnum].frames[global_mouse_state.mouse_frame].speed + 5;
      CheckViewFrame (viewnum, loopnum, global_mouse_state.mouse_frame);
    }
    global_mouse_state.lastmx=mousex; global_mouse_state.lastmy=mousey;
  }
}

void update_and_draw_mouse_on_screen() {
    
  ac_domouse(DOMOUSE_NOCURSOR);

  if (!play.mouse_cursor_hidden)
  {
    gfxDriver->DrawSprite(mousex - hotx, mousey - hoty, global_mouse_state.mouseCursor);
    invalidate_sprite(mousex - hotx, mousey - hoty, global_mouse_state.mouseCursor);
  }

  /*
  ac_domouse(1);
  // if the cursor is hidden, remove it again. However, it needs
  // to go on-off in order to update the stored mouse coordinates
  if (play.mouse_cursor_hidden)
    ac_domouse(2);*/

  //if (!play.mouse_cursor_hidden)
//    ac_domouse(2);

}


void register_mouse_script_functions() {
  scAdd_External_Symbol("Mouse::ChangeModeGraphic^2",(void *)ChangeCursorGraphic);
  scAdd_External_Symbol("Mouse::ChangeModeHotspot^3",(void *)ChangeCursorHotspot);
  scAdd_External_Symbol("Mouse::ChangeModeView^2",(void *)Mouse_ChangeModeView);
  scAdd_External_Symbol("Mouse::DisableMode^1",(void *)disable_cursor_mode);
  scAdd_External_Symbol("Mouse::EnableMode^1",(void *)enable_cursor_mode);
  scAdd_External_Symbol("Mouse::GetModeGraphic^1",(void *)Mouse_GetModeGraphic);
  scAdd_External_Symbol("Mouse::IsButtonDown^1",(void *)IsButtonDown);
  scAdd_External_Symbol("Mouse::SaveCursorUntilItLeaves^0",(void *)SaveCursorForLocationChange);
  scAdd_External_Symbol("Mouse::SelectNextMode^0", (void *)SetNextCursor);
  scAdd_External_Symbol("Mouse::SetBounds^4",(void *)SetMouseBounds);
  scAdd_External_Symbol("Mouse::SetPosition^2",(void *)SetMousePosition);
  scAdd_External_Symbol("Mouse::Update^0",(void *)RefreshMouse);
  scAdd_External_Symbol("Mouse::UseDefaultGraphic^0",(void *)set_default_cursor);
  scAdd_External_Symbol("Mouse::UseModeGraphic^1",(void *)set_mouse_cursor);
  scAdd_External_Symbol("Mouse::get_Mode",(void *)GetCursorMode);
  scAdd_External_Symbol("Mouse::set_Mode",(void *)set_cursor_mode);
  scAdd_External_Symbol("Mouse::get_Visible", (void *)Mouse_GetVisible);
  scAdd_External_Symbol("Mouse::set_Visible", (void *)Mouse_SetVisible);
  scAdd_External_Symbol("ChangeCursorGraphic",(void *)ChangeCursorGraphic);
  scAdd_External_Symbol("ChangeCursorHotspot",(void *)ChangeCursorHotspot);
  scAdd_External_Symbol("HideMouseCursor",(void *)HideMouseCursor);
  scAdd_External_Symbol("IsButtonDown",(void *)IsButtonDown);
  scAdd_External_Symbol("SaveCursorForLocationChange",(void *)SaveCursorForLocationChange);
  scAdd_External_Symbol("SetMouseBounds",(void *)SetMouseBounds);
  scAdd_External_Symbol("SetMouseCursor",(void *)set_mouse_cursor);
  scAdd_External_Symbol("SetMousePosition",(void *)SetMousePosition);
  scAdd_External_Symbol("ShowMouseCursor",(void *)ShowMouseCursor);
  scAdd_External_Symbol("RefreshMouse",(void *)RefreshMouse);
  scAdd_External_Symbol("mouse",&global_mouse_state.scmouse);
}