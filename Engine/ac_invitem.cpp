#include "ac_invitem.h"

#include "ac.h"
#include "ac_context.h"
#include "ac_mouse.h"
#include "ac_string.h"
#include "acgui.h"
extern int GetGUIAt (int xx,int yy);


void set_inv_item_cursorpic(int invItemId, int piccy) 
{
  game.invinfo[invItemId].cursorPic = piccy;

  if ((cur_cursor == MODE_USE) && (playerchar->activeinv == invItemId)) 
  {
    update_inv_cursor(invItemId);
    set_mouse_cursor(cur_cursor);
  }
}

/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::set_CursorGraphic *** */
void InventoryItem_SetCursorGraphic(ScriptInvItem *iitem, int newSprite) 
{
  set_inv_item_cursorpic(iitem->id, newSprite);
}

/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::get_CursorGraphic *** */
int InventoryItem_GetCursorGraphic(ScriptInvItem *iitem) 
{
  return game.invinfo[iitem->id].cursorPic;
}

/* *** SCRIPT SYMBOL: [InventoryItem] SetInvItemPic *** */
void set_inv_item_pic(int invi, int piccy) {
  if ((invi < 1) || (invi > game.numinvitems))
    quit("!SetInvItemPic: invalid inventory item specified");

  if (game.invinfo[invi].pic == piccy)
    return;

  if (game.invinfo[invi].pic == game.invinfo[invi].cursorPic)
  {
    // Backwards compatibility -- there didn't used to be a cursorPic,
    // so if they're the same update both.
    set_inv_item_cursorpic(invi, piccy);
  }

  game.invinfo[invi].pic = piccy;
  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::set_Graphic *** */
void InventoryItem_SetGraphic(ScriptInvItem *iitem, int piccy) {
  set_inv_item_pic(iitem->id, piccy);
}

/* *** SCRIPT SYMBOL: [InventoryItem] SetInvItemName *** */
void SetInvItemName(int invi, const char *newName) {
  if ((invi < 1) || (invi > game.numinvitems))
    quit("!SetInvName: invalid inventory item specified");

  // set the new name, making sure it doesn't overflow the buffer
  strncpy(game.invinfo[invi].name, newName, 25);
  game.invinfo[invi].name[24] = 0;

  // might need to redraw the GUI if it has the inv item name on it
  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::SetName^1 *** */
/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::set_Name *** */
void InventoryItem_SetName(ScriptInvItem *scii, const char *newname) {
  SetInvItemName(scii->id, newname);
}

/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::get_ID *** */
int InventoryItem_GetID(ScriptInvItem *scii) {
  return scii->id;
}



int offset_over_inv(GUIInv *inv) {

  int mover = mouse_ifacebut_xoffs / multiply_up_coordinate(inv->itemWidth);
  // if it's off the edge of the visible items, ignore
  if (mover >= inv->itemsPerLine)
    return -1;
  mover += (mouse_ifacebut_yoffs / multiply_up_coordinate(inv->itemHeight)) * inv->itemsPerLine;
  if (mover >= inv->itemsPerLine * inv->numLines)
    return -1;

  mover += inv->topIndex;
  if ((mover < 0) || (mover >= charextra[inv->CharToDisplay()].invorder_count))
    return -1;

  return charextra[inv->CharToDisplay()].invorder[mover];
}

void run_event_block_inv(int invNum, int aaa) {
  evblockbasename="inventory%d";
  if (game.invScripts != NULL)
  {
    run_interaction_script(game.invScripts[invNum], aaa);
  }
  else 
  {
    run_interaction_event(game.intrInv[invNum], aaa);
  }

}

/* *** SCRIPT SYMBOL: [Character] SetActiveInventory *** */
void SetActiveInventory(int iit) {

  ScriptInvItem *tosend = NULL;
  if ((iit > 0) && (iit < game.numinvitems))
    tosend = &scrInv[iit];
  else if (iit != -1)
    quitprintf("!SetActiveInventory: invalid inventory number %d", iit);

  Character_SetActiveInventory(playerchar, tosend);
}

/* *** SCRIPT SYMBOL: [InventoryItem] RunInventoryInteraction *** */
void RunInventoryInteraction (int iit, int modd) {
  if ((iit < 0) || (iit >= game.numinvitems))
    quit("!RunInventoryInteraction: invalid inventory number");

  evblocknum = iit;
  if (modd == MODE_LOOK)
    run_event_block_inv(iit, 0);
  else if (modd == MODE_HAND)
    run_event_block_inv(iit, 1);
  else if (modd == MODE_USE) {
    play.usedinv = playerchar->activeinv;
    run_event_block_inv(iit, 3);
  }
  else if (modd == MODE_TALK)
    run_event_block_inv(iit, 2);
  else // other click on invnetory
    run_event_block_inv(iit, 4);
}

/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::RunInteraction^1 *** */
void InventoryItem_RunInteraction(ScriptInvItem *iitem, int mood) {
  RunInventoryInteraction(iitem->id, mood);
}



/* *** SCRIPT SYMBOL: [InventoryItem] GetInvProperty *** */
int GetInvProperty (int item, const char *property) {
  return get_int_property (&game.invProps[item], property);
}
/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::GetProperty^1 *** */
int InventoryItem_GetProperty(ScriptInvItem *scii, const char *property) {
  return get_int_property (&game.invProps[scii->id], property);
}



/* *** SCRIPT SYMBOL: [InventoryItem] GetInvPropertyText *** */
void GetInvPropertyText (int item, const char *property, char *bufer) {
  get_text_property (&game.invProps[item], property, bufer);
}
/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::GetPropertyText^2 *** */
void InventoryItem_GetPropertyText(ScriptInvItem *scii, const char *property, char *bufer) {
  get_text_property(&game.invProps[scii->id], property, bufer);
}
/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::GetTextProperty^1 *** */
const char* InventoryItem_GetTextProperty(ScriptInvItem *scii, const char *property) {
  return get_text_property_dynamic_string(&game.invProps[scii->id], property);
}

/* *** SCRIPT SYMBOL: [InventoryItem] IsInventoryInteractionAvailable *** */
int IsInventoryInteractionAvailable (int item, int mood) {
  if ((item < 0) || (item >= MAX_INV))
    quit("!IsInventoryInteractionAvailable: invalid inventory number");

  play.check_interaction_only = 1;

  RunInventoryInteraction(item, mood);

  int ciwas = play.check_interaction_only;
  play.check_interaction_only = 0;

  if (ciwas == 2)
    return 1;

  return 0;
}

/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::IsInteractionAvailable^1 *** */
int InventoryItem_CheckInteractionAvailable(ScriptInvItem *iitem, int mood) {
  return IsInventoryInteractionAvailable(iitem->id, mood);
}



/* *** SCRIPT SYMBOL: [InventoryItem] GetInvAt *** */
int GetInvAt (int xxx, int yyy) {
  int ongui = GetGUIAt (xxx, yyy);
  if (ongui >= 0) {
    int mxwas = mousex, mywas = mousey;
    mousex = multiply_up_coordinate(xxx) - guis[ongui].x;
    mousey = multiply_up_coordinate(yyy) - guis[ongui].y;
    int onobj = guis[ongui].find_object_under_mouse();
    if (onobj>=0) {
      mouse_ifacebut_xoffs = mousex-(guis[ongui].objs[onobj]->x);
      mouse_ifacebut_yoffs = mousey-(guis[ongui].objs[onobj]->y);
    }
    mousex = mxwas;
    mousey = mywas;
    if ((onobj>=0) && ((guis[ongui].objrefptr[onobj] >> 16)==GOBJ_INVENTORY))
      return offset_over_inv((GUIInv*)guis[ongui].objs[onobj]);
  }
  return -1;
}

/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::GetAtScreenXY^2 *** */
ScriptInvItem *GetInvAtLocation(int xx, int yy) {
  int hsnum = GetInvAt(xx, yy);
  if (hsnum <= 0)
    return NULL;
  return &scrInv[hsnum];
}


/* *** SCRIPT SYMBOL: [InventoryItem] GetInvName *** */
void GetInvName(int indx,char*buff) {
  VALIDATE_STRING(buff);
  if ((indx<0) | (indx>=game.numinvitems)) quit("!GetInvName: invalid inventory item specified");
  strcpy(buff,get_translation(game.invinfo[indx].name));
}

/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::GetName^1 *** */
void InventoryItem_GetName(ScriptInvItem *iitem, char *buff) {
  GetInvName(iitem->id, buff);
}

/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::get_Name *** */
const char* InventoryItem_GetName_New(ScriptInvItem *invitem) {
  return CreateNewScriptString(get_translation(game.invinfo[invitem->id].name));
}

/* *** SCRIPT SYMBOL: [InventoryItem] GetInvGraphic *** */
int GetInvGraphic(int indx) {
  if ((indx<0) | (indx>=game.numinvitems)) quit("!GetInvGraphic: invalid inventory item specified");

  return game.invinfo[indx].pic;
}

/* *** SCRIPT SYMBOL: [InventoryItem] InventoryItem::get_Graphic *** */
int InventoryItem_GetGraphic(ScriptInvItem *iitem) {
  return game.invinfo[iitem->id].pic;
}





void register_inventory_item_script_functions() {
  scAdd_External_Symbol("InventoryItem::GetAtScreenXY^2", (void *)GetInvAtLocation);
  scAdd_External_Symbol("InventoryItem::IsInteractionAvailable^1", (void *)InventoryItem_CheckInteractionAvailable);
  scAdd_External_Symbol("InventoryItem::GetName^1", (void *)InventoryItem_GetName);
  scAdd_External_Symbol("InventoryItem::GetProperty^1", (void *)InventoryItem_GetProperty);
  scAdd_External_Symbol("InventoryItem::GetPropertyText^2", (void *)InventoryItem_GetPropertyText);
  scAdd_External_Symbol("InventoryItem::GetTextProperty^1",(void *)InventoryItem_GetTextProperty);
  scAdd_External_Symbol("InventoryItem::RunInteraction^1", (void *)InventoryItem_RunInteraction);
  scAdd_External_Symbol("InventoryItem::SetName^1", (void *)InventoryItem_SetName);
  scAdd_External_Symbol("InventoryItem::get_CursorGraphic", (void *)InventoryItem_GetCursorGraphic);
  scAdd_External_Symbol("InventoryItem::set_CursorGraphic", (void *)InventoryItem_SetCursorGraphic);
  scAdd_External_Symbol("InventoryItem::get_Graphic", (void *)InventoryItem_GetGraphic);
  scAdd_External_Symbol("InventoryItem::set_Graphic", (void *)InventoryItem_SetGraphic);
  scAdd_External_Symbol("InventoryItem::get_ID", (void *)InventoryItem_GetID);
  scAdd_External_Symbol("InventoryItem::get_Name", (void *)InventoryItem_GetName_New);
  scAdd_External_Symbol("InventoryItem::set_Name", (void *)InventoryItem_SetName);
  scAdd_External_Symbol("GetInvAt",(void *)GetInvAt);
  scAdd_External_Symbol("GetInvGraphic",(void *)GetInvGraphic);
  scAdd_External_Symbol("GetInvName",(void *)GetInvName);
  scAdd_External_Symbol("GetInvProperty",(void *)GetInvProperty);
  scAdd_External_Symbol("GetInvPropertyText",(void *)GetInvPropertyText);
  scAdd_External_Symbol("IsInventoryInteractionAvailable", (void *)IsInventoryInteractionAvailable);
  scAdd_External_Symbol("RunInventoryInteraction", (void *)RunInventoryInteraction);
  scAdd_External_Symbol("SetInvItemName",(void *)SetInvItemName);
  scAdd_External_Symbol("SetInvItemPic",(void *)set_inv_item_pic);
  scAdd_External_Symbol("SetActiveInventory",(void *)SetActiveInventory);
}

