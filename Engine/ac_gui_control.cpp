#include "ac_gui_control.h"

#include "allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "acgui.h"
#include "ac_script_gui.h"



/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::GetAtScreenXY^2 *** */
GUIObject *GetGUIControlAtLocation(int xx, int yy) {
  int guinum = GetGUIAt(xx, yy);
  if (guinum == -1)
    return NULL;

  multiply_up_coordinates(&xx, &yy);

  int oldmousex = mousex, oldmousey = mousey;
  mousex = xx - guis[guinum].x;
  mousey = yy - guis[guinum].y;
  int toret = guis[guinum].find_object_under_mouse(0, false);
  mousex = oldmousex;
  mousey = oldmousey;
  if (toret < 0)
    return NULL;

  return guis[guinum].objs[toret];
}

/* *** SCRIPT SYMBOL: [GUIControl] GetGUIObjectAt *** */
int GetGUIObjectAt (int xx, int yy) {
  GUIObject *toret = GetGUIControlAtLocation(xx, yy);
  if (toret == NULL)
    return -1;

  return toret->objn;
}




/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_Visible *** */
int GUIControl_GetVisible(GUIObject *guio) {
  return guio->IsVisible();
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::set_Visible *** */
void GUIControl_SetVisible(GUIObject *guio, int visible) 
{
  if (visible != guio->IsVisible()) 
  {
    if (visible)
      guio->Show();
    else
      guio->Hide();

    guis[guio->guin].control_positions_changed();
    guis_need_update = 1;
  }
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_Clickable *** */
int GUIControl_GetClickable(GUIObject *guio) {
  if (guio->IsClickable())
    return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::set_Clickable *** */
void GUIControl_SetClickable(GUIObject *guio, int enabled) {
  if (enabled)
    guio->SetClickable(true);
  else
    guio->SetClickable(false);

  guis[guio->guin].control_positions_changed();
  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_Enabled *** */
int GUIControl_GetEnabled(GUIObject *guio) {
  if (guio->IsDisabled())
    return 0;
  return 1;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::set_Enabled *** */
void GUIControl_SetEnabled(GUIObject *guio, int enabled) {
  if (enabled)
    guio->Enable();
  else
    guio->Disable();

  guis[guio->guin].control_positions_changed();
  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [GUIControl] SetGUIObjectEnabled *** */
void SetGUIObjectEnabled(int guin, int objn, int enabled) {
  if ((guin<0) || (guin>=game.numgui))
    quit("!SetGUIObjectEnabled: invalid GUI number");
  if ((objn<0) || (objn>=guis[guin].numobjs))
    quit("!SetGUIObjectEnabled: invalid object number");

  GUIControl_SetEnabled(guis[guin].objs[objn], enabled);
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_ID *** */
int GUIControl_GetID(GUIObject *guio) {
  return guio->objn;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_OwningGUI *** */
ScriptGUI* GUIControl_GetOwningGUI(GUIObject *guio) {
  return &scrGui[guio->guin];
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_AsButton *** */
GUIButton* GUIControl_GetAsButton(GUIObject *guio) {
  if (guis[guio->guin].get_control_type(guio->objn) != GOBJ_BUTTON)
    return NULL;

  return (GUIButton*)guio;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_AsInvWindow *** */
GUIInv* GUIControl_GetAsInvWindow(GUIObject *guio) {
  if (guis[guio->guin].get_control_type(guio->objn) != GOBJ_INVENTORY)
    return NULL;

  return (GUIInv*)guio;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_AsLabel *** */
GUILabel* GUIControl_GetAsLabel(GUIObject *guio) {
  if (guis[guio->guin].get_control_type(guio->objn) != GOBJ_LABEL)
    return NULL;

  return (GUILabel*)guio;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_AsListBox *** */
GUIListBox* GUIControl_GetAsListBox(GUIObject *guio) {
  if (guis[guio->guin].get_control_type(guio->objn) != GOBJ_LISTBOX)
    return NULL;

  return (GUIListBox*)guio;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_AsSlider *** */
GUISlider* GUIControl_GetAsSlider(GUIObject *guio) {
  if (guis[guio->guin].get_control_type(guio->objn) != GOBJ_SLIDER)
    return NULL;

  return (GUISlider*)guio;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_AsTextBox *** */
GUITextBox* GUIControl_GetAsTextBox(GUIObject *guio) {
  if (guis[guio->guin].get_control_type(guio->objn) != GOBJ_TEXTBOX)
    return NULL;

  return (GUITextBox*)guio;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_X *** */
int GUIControl_GetX(GUIObject *guio) {
  return divide_down_coordinate(guio->x);
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::set_X *** */
void GUIControl_SetX(GUIObject *guio, int xx) {
  guio->x = multiply_up_coordinate(xx);
  guis[guio->guin].control_positions_changed();
  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_Y *** */
int GUIControl_GetY(GUIObject *guio) {
  return divide_down_coordinate(guio->y);
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::set_Y *** */
void GUIControl_SetY(GUIObject *guio, int yy) {
  guio->y = multiply_up_coordinate(yy);
  guis[guio->guin].control_positions_changed();
  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::SetPosition^2 *** */
void GUIControl_SetPosition(GUIObject *guio, int xx, int yy) {
  GUIControl_SetX(guio, xx);
  GUIControl_SetY(guio, yy);
}

/* *** SCRIPT SYMBOL: [GUIControl] SetGUIObjectPosition *** */
void SetGUIObjectPosition(int guin, int objn, int xx, int yy) {
  if ((guin<0) || (guin>=game.numgui))
    quit("!SetGUIObjectPosition: invalid GUI number");
  if ((objn<0) || (objn>=guis[guin].numobjs))
    quit("!SetGUIObjectPosition: invalid object number");

  GUIControl_SetPosition(guis[guin].objs[objn], xx, yy);
}


/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_Width *** */
int GUIControl_GetWidth(GUIObject *guio) {
  return divide_down_coordinate(guio->wid);
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::set_Width *** */
void GUIControl_SetWidth(GUIObject *guio, int newwid) {
  guio->wid = multiply_up_coordinate(newwid);
  guio->Resized();
  guis[guio->guin].control_positions_changed();
  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::get_Height *** */
int GUIControl_GetHeight(GUIObject *guio) {
  return divide_down_coordinate(guio->hit);
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::set_Height *** */
void GUIControl_SetHeight(GUIObject *guio, int newhit) {
  guio->hit = multiply_up_coordinate(newhit);
  guio->Resized();
  guis[guio->guin].control_positions_changed();
  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::SetSize^2 *** */
void GUIControl_SetSize(GUIObject *guio, int newwid, int newhit) {
  if ((newwid < 2) || (newhit < 2))
    quit("!SetGUIObjectSize: new size is too small (must be at least 2x2)");

  DEBUG_CONSOLE("SetGUIObject %d,%d size %d,%d", guio->guin, guio->objn, newwid, newhit);
  GUIControl_SetWidth(guio, newwid);
  GUIControl_SetHeight(guio, newhit);
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::SendToBack^0 *** */
void GUIControl_SendToBack(GUIObject *guio) {
  if (guis[guio->guin].send_to_back(guio->objn))
    guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [GUIControl] GUIControl::BringToFront^0 *** */
void GUIControl_BringToFront(GUIObject *guio) {
  if (guis[guio->guin].bring_to_front(guio->objn))
    guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [GUIControl] SetGUIObjectSize *** */
void SetGUIObjectSize(int ifn, int objn, int newwid, int newhit) {
  if ((ifn<0) || (ifn>=game.numgui))
    quit("!SetGUIObjectSize: invalid GUI number");

  if ((objn<0) || (objn >= guis[ifn].numobjs))
    quit("!SetGUIObjectSize: invalid object number");

  GUIControl_SetSize(guis[ifn].objs[objn], newwid, newhit);
}




void register_gui_control_script_functions() {
  scAdd_External_Symbol("GUIControl::BringToFront^0", (void *)GUIControl_BringToFront);
  scAdd_External_Symbol("GUIControl::GetAtScreenXY^2", (void *)GetGUIControlAtLocation);
  scAdd_External_Symbol("GUIControl::SendToBack^0", (void *)GUIControl_SendToBack);
  scAdd_External_Symbol("GUIControl::SetPosition^2", (void *)GUIControl_SetPosition);
  scAdd_External_Symbol("GUIControl::SetSize^2", (void *)GUIControl_SetSize);
  scAdd_External_Symbol("GUIControl::get_AsButton", (void *)GUIControl_GetAsButton);
  scAdd_External_Symbol("GUIControl::get_AsInvWindow", (void *)GUIControl_GetAsInvWindow);
  scAdd_External_Symbol("GUIControl::get_AsLabel", (void *)GUIControl_GetAsLabel);
  scAdd_External_Symbol("GUIControl::get_AsListBox", (void *)GUIControl_GetAsListBox);
  scAdd_External_Symbol("GUIControl::get_AsSlider", (void *)GUIControl_GetAsSlider);
  scAdd_External_Symbol("GUIControl::get_AsTextBox", (void *)GUIControl_GetAsTextBox);
  scAdd_External_Symbol("GUIControl::get_Clickable", (void *)GUIControl_GetClickable);
  scAdd_External_Symbol("GUIControl::set_Clickable", (void *)GUIControl_SetClickable);
  scAdd_External_Symbol("GUIControl::get_Enabled", (void *)GUIControl_GetEnabled);
  scAdd_External_Symbol("GUIControl::set_Enabled", (void *)GUIControl_SetEnabled);
  scAdd_External_Symbol("GUIControl::get_Height", (void *)GUIControl_GetHeight);
  scAdd_External_Symbol("GUIControl::set_Height", (void *)GUIControl_SetHeight);
  scAdd_External_Symbol("GUIControl::get_ID", (void *)GUIControl_GetID);
  scAdd_External_Symbol("GUIControl::get_OwningGUI", (void *)GUIControl_GetOwningGUI);
  scAdd_External_Symbol("GUIControl::get_Visible", (void *)GUIControl_GetVisible);
  scAdd_External_Symbol("GUIControl::set_Visible", (void *)GUIControl_SetVisible);
  scAdd_External_Symbol("GUIControl::get_Width", (void *)GUIControl_GetWidth);
  scAdd_External_Symbol("GUIControl::set_Width", (void *)GUIControl_SetWidth);
  scAdd_External_Symbol("GUIControl::get_X", (void *)GUIControl_GetX);
  scAdd_External_Symbol("GUIControl::set_X", (void *)GUIControl_SetX);
  scAdd_External_Symbol("GUIControl::get_Y", (void *)GUIControl_GetY);
  scAdd_External_Symbol("GUIControl::set_Y", (void *)GUIControl_SetY);
  scAdd_External_Symbol("GetGUIObjectAt", (void *)GetGUIObjectAt);
  scAdd_External_Symbol("SetGUIObjectEnabled",(void *)SetGUIObjectEnabled);
  scAdd_External_Symbol("SetGUIObjectPosition",(void *)SetGUIObjectPosition);
  scAdd_External_Symbol("SetGUIObjectSize",(void *)SetGUIObjectSize);
}

