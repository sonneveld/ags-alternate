#include "ac_script_gui.h"

#include "allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "acgui.h"
#include "ali3d.h"
#include "acgfx.h"
extern void PauseGame();  // from ac_game eventually.
extern void UnPauseGame();



/* *** SCRIPT SYMBOL: [GUI] GetGUIAt *** */
int GetGUIAt (int xx,int yy) {
  multiply_up_coordinates(&xx, &yy);
  
  int aa, ll;
  for (ll = game.numgui - 1; ll >= 0; ll--) {
    aa = play.gui_draw_order[ll];
    if (guis[aa].on<1) continue;
    if (guis[aa].flags & GUIF_NOCLICK) continue;
    if ((xx>=guis[aa].x) & (yy>=guis[aa].y) &
      (xx<=guis[aa].x+guis[aa].wid) & (yy<=guis[aa].y+guis[aa].hit))
      return aa;
  }
  return -1;
}

/* *** SCRIPT SYMBOL: [GUI] GUI::GetAtScreenXY^2 *** */
ScriptGUI *GetGUIAtLocation(int xx, int yy) {
  int guiid = GetGUIAt(xx, yy);
  if (guiid < 0)
    return NULL;
  return &scrGui[guiid];
}



/* *** SCRIPT SYMBOL: [GUI] IsGUIOn *** */
int IsGUIOn (int guinum) {
  if ((guinum < 0) || (guinum >= game.numgui))
    quit("!IsGUIOn: invalid GUI number specified");
  return (guis[guinum].on >= 1) ? 1 : 0;
}

// This is an internal script function, and is undocumented.
// It is used by the editor's automatic macro generation.
/* *** SCRIPT SYMBOL: [GUI] FindGUIID *** */
int FindGUIID (const char* GUIName) {
  for (int ii = 0; ii < game.numgui; ii++) {
    if (strcmp(guis[ii].name, GUIName) == 0)
      return ii;
    if ((guis[ii].name[0] == 'g') && (ac_stricmp(&guis[ii].name[1], GUIName) == 0))
      return ii;
  }
  quit("FindGUIID: No matching GUI found: GUI may have been deleted");
  return -1;
}


/* *** SCRIPT SYMBOL: [GUI] InterfaceOn *** */
void InterfaceOn(int ifn) {
  if ((ifn<0) | (ifn>=game.numgui))
    quit("!GUIOn: invalid GUI specified");

  EndSkippingUntilCharStops();

  if (guis[ifn].on == 1) {
    DEBUG_CONSOLE("GUIOn(%d) ignored (already on)", ifn);
    return;
  }
  guis_need_update = 1;
  guis[ifn].on=1;
  DEBUG_CONSOLE("GUI %d turned on", ifn);
  // modal interface
  if (guis[ifn].popup==POPUP_SCRIPT) PauseGame();
  else if (guis[ifn].popup==POPUP_MOUSEY) guis[ifn].on=0;
  // clear the cached mouse position
  guis[ifn].control_positions_changed();
  guis[ifn].poll();
}

/* *** SCRIPT SYMBOL: [GUI] InterfaceOff *** */
void InterfaceOff(int ifn) {
  if ((ifn<0) | (ifn>=game.numgui)) quit("!GUIOff: invalid GUI specified");
  if ((guis[ifn].on==0) && (guis[ifn].popup!=POPUP_MOUSEY)) {
    DEBUG_CONSOLE("GUIOff(%d) ignored (already off)", ifn);
    return;
  }
  DEBUG_CONSOLE("GUI %d turned off", ifn);
  guis[ifn].on=0;
  if (guis[ifn].mouseover>=0) {
    // Make sure that the overpic is turned off when the GUI goes off
    guis[ifn].objs[guis[ifn].mouseover]->MouseLeave();
    guis[ifn].mouseover = -1;
  }
  guis[ifn].control_positions_changed();
  guis_need_update = 1;
  // modal interface
  if (guis[ifn].popup==POPUP_SCRIPT) UnPauseGame();
  else if (guis[ifn].popup==POPUP_MOUSEY) guis[ifn].on=-1;
}

/* *** SCRIPT SYMBOL: [GUI] GUI::set_Visible *** */
void GUI_SetVisible(ScriptGUI *tehgui, int isvisible) {
  if (isvisible)
    InterfaceOn(tehgui->id);
  else
    InterfaceOff(tehgui->id);
}

/* *** SCRIPT SYMBOL: [GUI] GUI::get_Visible *** */
int GUI_GetVisible(ScriptGUI *tehgui) {
  // GUI_GetVisible is slightly different from IsGUIOn, because
  // with a mouse ypos gui it returns 1 if the GUI is enabled,
  // whereas IsGUIOn actually checks if it is displayed
  if (tehgui->gui->on != 0)
    return 1;
  return 0;
}


/* *** SCRIPT SYMBOL: [GUI] GUI::get_X *** */
int GUI_GetX(ScriptGUI *tehgui) {
  return divide_down_coordinate(tehgui->gui->x);
}

/* *** SCRIPT SYMBOL: [GUI] GUI::set_X *** */
void GUI_SetX(ScriptGUI *tehgui, int xx) {
  if (xx >= thisroom.width)
    quit("!GUI.X: co-ordinates specified are out of range.");

  tehgui->gui->x = multiply_up_coordinate(xx);
}

/* *** SCRIPT SYMBOL: [GUI] GUI::get_Y *** */
int GUI_GetY(ScriptGUI *tehgui) {
  return divide_down_coordinate(tehgui->gui->y);
}

/* *** SCRIPT SYMBOL: [GUI] GUI::set_Y *** */
void GUI_SetY(ScriptGUI *tehgui, int yy) {
  if (yy >= thisroom.height)
    quit("!GUI.Y: co-ordinates specified are out of range.");

  tehgui->gui->y = multiply_up_coordinate(yy);
}

/* *** SCRIPT SYMBOL: [GUI] GUI::SetPosition^2 *** */
void GUI_SetPosition(ScriptGUI *tehgui, int xx, int yy) {
  GUI_SetX(tehgui, xx);
  GUI_SetY(tehgui, yy);
}

/* *** SCRIPT SYMBOL: [GUI] SetGUIPosition *** */
void SetGUIPosition(int ifn,int xx,int yy) {
  if ((ifn<0) || (ifn>=game.numgui))
    quit("!SetGUIPosition: invalid GUI number");
  
  GUI_SetPosition(&scrGui[ifn], xx, yy);
}

void recreate_guibg_image(GUIMain *tehgui)
{
  int ifn = tehgui->guiId;
  alw_destroy_bitmap(guibg[ifn]);
  guibg[ifn] = alw_create_bitmap_ex (final_col_dep, tehgui->wid, tehgui->hit);
  if (guibg[ifn] == NULL)
    quit("SetGUISize: internal error: unable to reallocate gui cache");
  guibg[ifn] = gfxDriver->ConvertBitmapToSupportedColourDepth(guibg[ifn]);

  if (guibgbmp[ifn] != NULL)
  {
    gfxDriver->DestroyDDB(guibgbmp[ifn]);
    guibgbmp[ifn] = NULL;
  }
}


/* *** SCRIPT SYMBOL: [GUI] GUI::SetSize^2 *** */
void GUI_SetSize(ScriptGUI *sgui, int widd, int hitt) {
  if ((widd < 1) || (hitt < 1) || (widd > BASEWIDTH) || (hitt > GetMaxScreenHeight()))
    quitprintf("!SetGUISize: invalid dimensions (tried to set to %d x %d)", widd, hitt);

  GUIMain *tehgui = sgui->gui;
  multiply_up_coordinates(&widd, &hitt);

  if ((tehgui->wid == widd) && (tehgui->hit == hitt))
    return;
  
  tehgui->wid = widd;
  tehgui->hit = hitt;
  
  recreate_guibg_image(tehgui);

  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [GUI] GUI::get_Width *** */
int GUI_GetWidth(ScriptGUI *sgui) {
  return divide_down_coordinate(sgui->gui->wid);
}

/* *** SCRIPT SYMBOL: [GUI] GUI::get_Height *** */
int GUI_GetHeight(ScriptGUI *sgui) {
  return divide_down_coordinate(sgui->gui->hit);
}

/* *** SCRIPT SYMBOL: [GUI] GUI::set_Width *** */
void GUI_SetWidth(ScriptGUI *sgui, int newwid) {
  GUI_SetSize(sgui, newwid, GUI_GetHeight(sgui));
}

/* *** SCRIPT SYMBOL: [GUI] GUI::set_Height *** */
void GUI_SetHeight(ScriptGUI *sgui, int newhit) {
  GUI_SetSize(sgui, GUI_GetWidth(sgui), newhit);
}

/* *** SCRIPT SYMBOL: [GUI] SetGUISize *** */
void SetGUISize (int ifn, int widd, int hitt) {
  if ((ifn<0) || (ifn>=game.numgui))
    quit("!SetGUISize: invalid GUI number");

  GUI_SetSize(&scrGui[ifn], widd, hitt);
}

void update_gui_zorder() {
  int numdone = 0, b;
  
  // for each GUI
  for (int a = 0; a < game.numgui; a++) {
    // find the right place in the draw order array
    int insertAt = numdone;
    for (b = 0; b < numdone; b++) {
      if (guis[a].zorder < guis[play.gui_draw_order[b]].zorder) {
        insertAt = b;
        break;
      }
    }
    // insert the new item
    for (b = numdone - 1; b >= insertAt; b--)
      play.gui_draw_order[b + 1] = play.gui_draw_order[b];
    play.gui_draw_order[insertAt] = a;
    numdone++;
  }

}

/* *** SCRIPT SYMBOL: [GUI] GUI::set_ZOrder *** */
void GUI_SetZOrder(ScriptGUI *tehgui, int z) {
  tehgui->gui->zorder = z;
  update_gui_zorder();
}

/* *** SCRIPT SYMBOL: [GUI] GUI::get_ZOrder *** */
int GUI_GetZOrder(ScriptGUI *tehgui) {
  return tehgui->gui->zorder;
}

/* *** SCRIPT SYMBOL: [GUI] SetGUIZOrder *** */
void SetGUIZOrder(int guin, int z) {
  if ((guin<0) || (guin>=game.numgui))
    quit("!SetGUIZOrder: invalid GUI number");

  GUI_SetZOrder(&scrGui[guin], z);
}

/* *** SCRIPT SYMBOL: [GUI] GUI::set_Clickable *** */
void GUI_SetClickable(ScriptGUI *tehgui, int clickable) {
  tehgui->gui->flags &= ~GUIF_NOCLICK;
  if (clickable == 0)
    tehgui->gui->flags |= GUIF_NOCLICK;
}

/* *** SCRIPT SYMBOL: [GUI] GUI::get_Clickable *** */
int GUI_GetClickable(ScriptGUI *tehgui) {
  if (tehgui->gui->flags & GUIF_NOCLICK)
    return 0;
  return 1;
}

/* *** SCRIPT SYMBOL: [GUI] SetGUIClickable *** */
void SetGUIClickable(int guin, int clickable) {
  if ((guin<0) || (guin>=game.numgui))
    quit("!SetGUIClickable: invalid GUI number");

  GUI_SetClickable(&scrGui[guin], clickable);
}

/* *** SCRIPT SYMBOL: [GUI] GUI::get_ID *** */
int GUI_GetID(ScriptGUI *tehgui) {
  return tehgui->id;
}

/* *** SCRIPT SYMBOL: [GUI] GUI::geti_Controls *** */
GUIObject* GUI_GetiControls(ScriptGUI *tehgui, int idx) {
  if ((idx < 0) || (idx >= tehgui->gui->numobjs))
    return NULL;
  return tehgui->gui->objs[idx];
}

/* *** SCRIPT SYMBOL: [GUI] GUI::get_ControlCount *** */
int GUI_GetControlCount(ScriptGUI *tehgui) {
  return tehgui->gui->numobjs;
}

/* *** SCRIPT SYMBOL: [GUI] GUI::set_Transparency *** */
void GUI_SetTransparency(ScriptGUI *tehgui, int trans) {
  if ((trans < 0) | (trans > 100))
    quit("!SetGUITransparency: transparency value must be between 0 and 100");

  tehgui->gui->SetTransparencyAsPercentage(trans);
}

/* *** SCRIPT SYMBOL: [GUI] GUI::get_Transparency *** */
int GUI_GetTransparency(ScriptGUI *tehgui) {
  if (tehgui->gui->transparency == 0)
    return 0;
  if (tehgui->gui->transparency == 255)
    return 100;

  return 100 - ((tehgui->gui->transparency * 10) / 25);
}

// pass trans=0 for fully solid, trans=100 for fully transparent
/* *** SCRIPT SYMBOL: [GUI] SetGUITransparency *** */
void SetGUITransparency(int ifn, int trans) {
  if ((ifn < 0) | (ifn >= game.numgui))
    quit("!SetGUITransparency: invalid GUI number");

  GUI_SetTransparency(&scrGui[ifn], trans);
}

/* *** SCRIPT SYMBOL: [GUI] GUI::Centre^0 *** */
void GUI_Centre(ScriptGUI *sgui) {
  GUIMain *tehgui = sgui->gui;
  tehgui->x = scrnwid / 2 - tehgui->wid / 2;
  tehgui->y = scrnhit / 2 - tehgui->hit / 2;
}

/* *** SCRIPT SYMBOL: [GUI] CentreGUI *** */
void CentreGUI (int ifn) {
  if ((ifn<0) | (ifn>=game.numgui))
    quit("!CentreGUI: invalid GUI number");

  GUI_Centre(&scrGui[ifn]);
}


/* *** SCRIPT SYMBOL: [GUI] GUI::set_BackgroundGraphic *** */
void GUI_SetBackgroundGraphic(ScriptGUI *tehgui, int slotn) {
  if (tehgui->gui->bgpic != slotn) {
    tehgui->gui->bgpic = slotn;
    guis_need_update = 1;
  }
}

/* *** SCRIPT SYMBOL: [GUI] GUI::get_BackgroundGraphic *** */
int GUI_GetBackgroundGraphic(ScriptGUI *tehgui) {
  if (tehgui->gui->bgpic < 1)
    return 0;
  return tehgui->gui->bgpic;
}

/* *** SCRIPT SYMBOL: [GUI] SetGUIBackgroundPic *** */
void SetGUIBackgroundPic (int guin, int slotn) {
  if ((guin<0) | (guin>=game.numgui))
    quit("!SetGUIBackgroundPic: invalid GUI number");

  GUI_SetBackgroundGraphic(&scrGui[guin], slotn);
}





void register_gui_script_functions() {
  scAdd_External_Symbol("GUI::Centre^0", (void *)GUI_Centre);
  scAdd_External_Symbol("GUI::GetAtScreenXY^2", (void *)GetGUIAtLocation);
  scAdd_External_Symbol("GUI::SetPosition^2", (void *)GUI_SetPosition);
  scAdd_External_Symbol("GUI::SetSize^2", (void *)GUI_SetSize);
  scAdd_External_Symbol("GUI::get_BackgroundGraphic", (void *)GUI_GetBackgroundGraphic);
  scAdd_External_Symbol("GUI::set_BackgroundGraphic", (void *)GUI_SetBackgroundGraphic);
  scAdd_External_Symbol("GUI::get_Clickable", (void *)GUI_GetClickable);
  scAdd_External_Symbol("GUI::set_Clickable", (void *)GUI_SetClickable);
  scAdd_External_Symbol("GUI::get_ControlCount", (void *)GUI_GetControlCount);
  scAdd_External_Symbol("GUI::geti_Controls", (void *)GUI_GetiControls);
  scAdd_External_Symbol("GUI::get_Height", (void *)GUI_GetHeight);
  scAdd_External_Symbol("GUI::set_Height", (void *)GUI_SetHeight);
  scAdd_External_Symbol("GUI::get_ID", (void *)GUI_GetID);
  scAdd_External_Symbol("GUI::get_Transparency", (void *)GUI_GetTransparency);
  scAdd_External_Symbol("GUI::set_Transparency", (void *)GUI_SetTransparency);
  scAdd_External_Symbol("GUI::get_Visible", (void *)GUI_GetVisible);
  scAdd_External_Symbol("GUI::set_Visible", (void *)GUI_SetVisible);
  scAdd_External_Symbol("GUI::get_Width", (void *)GUI_GetWidth);
  scAdd_External_Symbol("GUI::set_Width", (void *)GUI_SetWidth);
  scAdd_External_Symbol("GUI::get_X", (void *)GUI_GetX);
  scAdd_External_Symbol("GUI::set_X", (void *)GUI_SetX);
  scAdd_External_Symbol("GUI::get_Y", (void *)GUI_GetY);
  scAdd_External_Symbol("GUI::set_Y", (void *)GUI_SetY);
  scAdd_External_Symbol("GUI::get_ZOrder", (void *)GUI_GetZOrder);
  scAdd_External_Symbol("GUI::set_ZOrder", (void *)GUI_SetZOrder);
  scAdd_External_Symbol("CentreGUI",(void *)CentreGUI);
  scAdd_External_Symbol("FindGUIID",(void *)FindGUIID);
  scAdd_External_Symbol("GetGUIAt", (void *)GetGUIAt);
  scAdd_External_Symbol("InterfaceOff",(void *)InterfaceOff);
  scAdd_External_Symbol("InterfaceOn",(void *)InterfaceOn);
  scAdd_External_Symbol("IsGUIOn", (void *)IsGUIOn);
  scAdd_External_Symbol("SetGUIBackgroundPic", (void *)SetGUIBackgroundPic);
  scAdd_External_Symbol("SetGUIClickable", (void *)SetGUIClickable);
  scAdd_External_Symbol("SetGUIPosition",(void *)SetGUIPosition);
  scAdd_External_Symbol("SetGUISize",(void *)SetGUISize);
  scAdd_External_Symbol("SetGUITransparency", (void *)SetGUITransparency);
  scAdd_External_Symbol("SetGUIZOrder", (void *)SetGUIZOrder);
}

