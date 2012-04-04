#include "ac_gui_list_box.h"

#include "sdlwrap/allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "acgui.h"
#include "ac_file.h"
#include "ac_string.h"
extern int GetSaveSlotDescription(int slnum,char*desbuf); // from ac_game eventually



// *** LIST BOX FUNCTIONS

/* *** SCRIPT SYMBOL: [ListBox] ListBox::AddItem^1 *** */
int ListBox_AddItem(GUIListBox *lbb, const char *text) {
  if (lbb->AddItem(text) < 0)
    return 0;

  guis_need_update = 1;
  return 1;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::InsertItemAt^2 *** */
int ListBox_InsertItemAt(GUIListBox *lbb, int index, const char *text) {
  if (lbb->InsertItem(index, text) < 0)
    return 0;

  guis_need_update = 1;
  return 1;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::Clear^0 *** */
void ListBox_Clear(GUIListBox *listbox) {
  listbox->Clear();
  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::FillDirList^1 *** */
void ListBox_FillDirList(GUIListBox *listbox, const char *filemask) {
  char searchPath[MAX_PATH];
  validate_user_file_path(filemask, searchPath, false);

  listbox->Clear();
  alw_al_ffblk dfb;
  int	dun = alw_al_findfirst(searchPath, &dfb, FA_SEARCH);
  while (!dun) {
    listbox->AddItem(dfb.name);
    dun = alw_al_findnext(&dfb);
  }
  alw_al_findclose(&dfb);
  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::geti_SaveGameSlots *** */
int ListBox_GetSaveGameSlots(GUIListBox *listbox, int index) {
  if ((index < 0) || (index >= listbox->numItems))
    quit("!ListBox.SaveGameSlot: index out of range");

  return listbox->saveGameIndex[index];
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::FillSaveGameList^0 *** */
int ListBox_FillSaveGameList(GUIListBox *listbox) {
  listbox->Clear();

  int numsaves=0;
  int bufix=0;
  alw_al_ffblk ffb; 
  long filedates[MAXSAVEGAMES];
  char buff[200];

  char searchPath[260];
  sprintf(searchPath, "%s""agssave.*", saveGameDirectory);

  int don = alw_al_findfirst(searchPath, &ffb, FA_SEARCH);
  while (!don) {
    bufix=0;
    if (numsaves >= MAXSAVEGAMES)
      break;
    // only list games .000 to .099 (to allow higher slots for other perposes)
    if (strstr(ffb.name,".0")==NULL) {
      don = alw_al_findnext(&ffb);
      continue;
    }
    const char *numberExtension = strstr(ffb.name, ".0") + 1;
    int saveGameSlot = atoi(numberExtension);
    GetSaveSlotDescription(saveGameSlot, buff);
    listbox->AddItem(buff);
    listbox->saveGameIndex[numsaves] = saveGameSlot;
    filedates[numsaves]=(long int)ffb.time;
    numsaves++;
    don = alw_al_findnext(&ffb);
  }
  alw_al_findclose(&ffb);

  int nn;
  for (nn=0;nn<numsaves-1;nn++) {
    for (int kk=0;kk<numsaves-1;kk++) {  // Date order the games

      if (filedates[kk] < filedates[kk+1]) {   // swap them round
        char*tempptr = listbox->items[kk];
        listbox->items[kk] = listbox->items[kk+1];
        listbox->items[kk+1] = tempptr;
        int numtem = listbox->saveGameIndex[kk];
        listbox->saveGameIndex[kk] = listbox->saveGameIndex[kk+1];
        listbox->saveGameIndex[kk+1] = numtem;
        long numted=filedates[kk]; filedates[kk]=filedates[kk+1];
        filedates[kk+1]=numted;
      }
    }
  }

  // update the global savegameindex[] array for backward compatibilty
  for (nn = 0; nn < numsaves; nn++) {
    play.filenumbers[nn] = listbox->saveGameIndex[nn];
  }

  guis_need_update = 1;
  listbox->exflags |= GLF_SGINDEXVALID;

  if (numsaves >= MAXSAVEGAMES)
    return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::GetItemAtLocation^2 *** */
int ListBox_GetItemAtLocation(GUIListBox *listbox, int x, int y) {

  if (guis[listbox->guin].on < 1)
    return -1;

  multiply_up_coordinates(&x, &y);
  x = (x - listbox->x) - guis[listbox->guin].x;
  y = (y - listbox->y) - guis[listbox->guin].y;

  if ((x < 0) || (y < 0) || (x >= listbox->wid) || (y >= listbox->hit))
    return -1;
  
  return listbox->GetIndexFromCoordinates(x, y);
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::GetItemText^2 *** */
char *ListBox_GetItemText(GUIListBox *listbox, int index, char *buffer) {
  if ((index < 0) || (index >= listbox->numItems))
    quit("!ListBoxGetItemText: invalid item specified");
  strncpy(buffer, listbox->items[index],198);
  buffer[199] = 0;
  return buffer;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::geti_Items *** */
const char* ListBox_GetItems(GUIListBox *listbox, int index) {
  if ((index < 0) || (index >= listbox->numItems))
    quit("!ListBox.Items: invalid index specified");

  return CreateNewScriptString(listbox->items[index]);
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::SetItemText^2 *** */
/* *** SCRIPT SYMBOL: [ListBox] ListBox::seti_Items *** */
void ListBox_SetItemText(GUIListBox *listbox, int index, const char *newtext) {
  if ((index < 0) || (index >= listbox->numItems))
    quit("!ListBoxSetItemText: invalid item specified");

  if (strcmp(listbox->items[index], newtext)) {
    listbox->SetItemText(index, newtext);
    guis_need_update = 1;
  }
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::RemoveItem^1 *** */
void ListBox_RemoveItem(GUIListBox *listbox, int itemIndex) {
  
  if ((itemIndex < 0) || (itemIndex >= listbox->numItems))
    quit("!ListBoxRemove: invalid listindex specified");

  listbox->RemoveItem(itemIndex);
  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::get_ItemCount *** */
int ListBox_GetItemCount(GUIListBox *listbox) {
  return listbox->numItems;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::get_Font *** */
int ListBox_GetFont(GUIListBox *listbox) {
  return listbox->font;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::set_Font *** */
void ListBox_SetFont(GUIListBox *listbox, int newfont) {

  if ((newfont < 0) || (newfont >= game.numfonts))
    quit("!ListBox.Font: invalid font number.");

  if (newfont != listbox->font) {
    listbox->ChangeFont(newfont);
    guis_need_update = 1;
  }

}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::get_HideBorder *** */
int ListBox_GetHideBorder(GUIListBox *listbox) {
  return (listbox->exflags & GLF_NOBORDER) ? 1 : 0;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::set_HideBorder *** */
void ListBox_SetHideBorder(GUIListBox *listbox, int newValue) {
  listbox->exflags &= ~GLF_NOBORDER;
  if (newValue)
    listbox->exflags |= GLF_NOBORDER;
  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::get_HideScrollArrows *** */
int ListBox_GetHideScrollArrows(GUIListBox *listbox) {
  return (listbox->exflags & GLF_NOARROWS) ? 1 : 0;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::set_HideScrollArrows *** */
void ListBox_SetHideScrollArrows(GUIListBox *listbox, int newValue) {
  listbox->exflags &= ~GLF_NOARROWS;
  if (newValue)
    listbox->exflags |= GLF_NOARROWS;
  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::get_SelectedIndex *** */
int ListBox_GetSelectedIndex(GUIListBox *listbox) {
  if ((listbox->selected < 0) || (listbox->selected >= listbox->numItems))
    return -1;
  return listbox->selected;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::set_SelectedIndex *** */
void ListBox_SetSelectedIndex(GUIListBox *guisl, int newsel) {

  if (newsel >= guisl->numItems)
    newsel = -1;

  if (guisl->selected != newsel) {
    guisl->selected = newsel;
    if (newsel >= 0) {
      if (newsel < guisl->topItem)
        guisl->topItem = newsel;
      if (newsel >= guisl->topItem + guisl->num_items_fit)
        guisl->topItem = (newsel - guisl->num_items_fit) + 1;
    }
    guis_need_update = 1;
  }

}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::get_TopItem *** */
int ListBox_GetTopItem(GUIListBox *listbox) {
  return listbox->topItem;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::set_TopItem *** */
void ListBox_SetTopItem(GUIListBox *guisl, int item) {
  if ((guisl->numItems == 0) && (item == 0))
    ;  // allow resetting an empty box to the top
  else if ((item >= guisl->numItems) || (item < 0))
    quit("!ListBoxSetTopItem: tried to set top to beyond top or bottom of list");

  guisl->topItem = item;
  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::get_RowCount *** */
int ListBox_GetRowCount(GUIListBox *listbox) {
  return listbox->num_items_fit;
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::ScrollDown^0 *** */
void ListBox_ScrollDown(GUIListBox *listbox) {
  if (listbox->topItem + listbox->num_items_fit < listbox->numItems) {
    listbox->topItem++;
    guis_need_update = 1;
  }
}

/* *** SCRIPT SYMBOL: [ListBox] ListBox::ScrollUp^0 *** */
void ListBox_ScrollUp(GUIListBox *listbox) {
  if (listbox->topItem > 0) {
    listbox->topItem--;
    guis_need_update = 1;
  }
}


GUIListBox* is_valid_listbox (int guin, int objn) {
  if ((guin<0) | (guin>=game.numgui)) quit("!ListBox: invalid GUI number");
  if ((objn<0) | (objn>=guis[guin].numobjs)) quit("!ListBox: invalid object number");
  if (guis[guin].get_control_type(objn)!=GOBJ_LISTBOX)
    quit("!ListBox: specified control is not a list box");
  guis_need_update = 1;
  return (GUIListBox*)guis[guin].objs[objn];
}

/* *** SCRIPT SYMBOL: [ListBox] ListBoxClear *** */
void ListBoxClear(int guin, int objn) {
  GUIListBox*guisl=is_valid_listbox(guin,objn);
  ListBox_Clear(guisl);
}
/* *** SCRIPT SYMBOL: [ListBox] ListBoxAdd *** */
void ListBoxAdd(int guin, int objn, const char*newitem) {
  GUIListBox*guisl=is_valid_listbox(guin,objn);
  ListBox_AddItem(guisl, newitem);
}
/* *** SCRIPT SYMBOL: [ListBox] ListBoxRemove *** */
void ListBoxRemove(int guin, int objn, int itemIndex) {
  GUIListBox*guisl = is_valid_listbox(guin,objn);
  ListBox_RemoveItem(guisl, itemIndex);
}
/* *** SCRIPT SYMBOL: [ListBox] ListBoxGetSelected *** */
int ListBoxGetSelected(int guin, int objn) {
  GUIListBox*guisl=is_valid_listbox(guin,objn);
  return ListBox_GetSelectedIndex(guisl);
}
/* *** SCRIPT SYMBOL: [ListBox] ListBoxGetNumItems *** */
int ListBoxGetNumItems(int guin, int objn) {
  GUIListBox*guisl=is_valid_listbox(guin,objn);
  return ListBox_GetItemCount(guisl);
}
/* *** SCRIPT SYMBOL: [ListBox] ListBoxGetItemText *** */
char* ListBoxGetItemText(int guin, int objn, int item, char*buffer) {
  VALIDATE_STRING(buffer);
  GUIListBox*guisl=is_valid_listbox(guin,objn);
  return ListBox_GetItemText(guisl, item, buffer);
}
/* *** SCRIPT SYMBOL: [ListBox] ListBoxSetSelected *** */
void ListBoxSetSelected(int guin, int objn, int newsel) {
  GUIListBox*guisl=is_valid_listbox(guin,objn);
  ListBox_SetSelectedIndex(guisl, newsel);
}
/* *** SCRIPT SYMBOL: [ListBox] ListBoxSetTopItem *** */
void ListBoxSetTopItem (int guin, int objn, int item) {
  GUIListBox*guisl = is_valid_listbox(guin,objn);
  ListBox_SetTopItem(guisl, item);
}

/* *** SCRIPT SYMBOL: [ListBox] ListBoxSaveGameList *** */
int ListBoxSaveGameList (int guin, int objn) {
  GUIListBox*guisl=is_valid_listbox(guin,objn);
  return ListBox_FillSaveGameList(guisl);
}

/* *** SCRIPT SYMBOL: [ListBox] ListBoxDirList *** */
void ListBoxDirList (int guin, int objn, const char*filemask) {
  GUIListBox *guisl = is_valid_listbox(guin,objn);
  ListBox_FillDirList(guisl, filemask);
}






void register_list_box_script_functions() {
  scAdd_External_Symbol("ListBox::AddItem^1", (void *)ListBox_AddItem);
  scAdd_External_Symbol("ListBox::Clear^0", (void *)ListBox_Clear);
  scAdd_External_Symbol("ListBox::FillDirList^1", (void *)ListBox_FillDirList);
  scAdd_External_Symbol("ListBox::FillSaveGameList^0", (void *)ListBox_FillSaveGameList);
  scAdd_External_Symbol("ListBox::GetItemAtLocation^2", (void *)ListBox_GetItemAtLocation);
  scAdd_External_Symbol("ListBox::GetItemText^2", (void *)ListBox_GetItemText);
  scAdd_External_Symbol("ListBox::InsertItemAt^2", (void *)ListBox_InsertItemAt);
  scAdd_External_Symbol("ListBox::RemoveItem^1", (void *)ListBox_RemoveItem);
  scAdd_External_Symbol("ListBox::ScrollDown^0", (void *)ListBox_ScrollDown);
  scAdd_External_Symbol("ListBox::ScrollUp^0", (void *)ListBox_ScrollUp);
  scAdd_External_Symbol("ListBox::SetItemText^2", (void *)ListBox_SetItemText);
  scAdd_External_Symbol("ListBox::get_Font", (void *)ListBox_GetFont);
  scAdd_External_Symbol("ListBox::set_Font", (void *)ListBox_SetFont);
  scAdd_External_Symbol("ListBox::get_HideBorder", (void *)ListBox_GetHideBorder);
  scAdd_External_Symbol("ListBox::set_HideBorder", (void *)ListBox_SetHideBorder);
  scAdd_External_Symbol("ListBox::get_HideScrollArrows", (void *)ListBox_GetHideScrollArrows);
  scAdd_External_Symbol("ListBox::set_HideScrollArrows", (void *)ListBox_SetHideScrollArrows);
  scAdd_External_Symbol("ListBox::get_ItemCount", (void *)ListBox_GetItemCount);
  scAdd_External_Symbol("ListBox::geti_Items", (void *)ListBox_GetItems);
  scAdd_External_Symbol("ListBox::seti_Items", (void *)ListBox_SetItemText);
  scAdd_External_Symbol("ListBox::get_RowCount", (void *)ListBox_GetRowCount);
  scAdd_External_Symbol("ListBox::geti_SaveGameSlots", (void *)ListBox_GetSaveGameSlots);
  scAdd_External_Symbol("ListBox::get_SelectedIndex", (void *)ListBox_GetSelectedIndex);
  scAdd_External_Symbol("ListBox::set_SelectedIndex", (void *)ListBox_SetSelectedIndex);
  scAdd_External_Symbol("ListBox::get_TopItem", (void *)ListBox_GetTopItem);
  scAdd_External_Symbol("ListBox::set_TopItem", (void *)ListBox_SetTopItem);
  scAdd_External_Symbol("ListBoxAdd", (void *)ListBoxAdd);
  scAdd_External_Symbol("ListBoxClear", (void *)ListBoxClear);
  scAdd_External_Symbol("ListBoxDirList", (void *)ListBoxDirList);
  scAdd_External_Symbol("ListBoxGetItemText", (void *)ListBoxGetItemText);
  scAdd_External_Symbol("ListBoxGetNumItems", (void *)ListBoxGetNumItems);
  scAdd_External_Symbol("ListBoxGetSelected", (void *)ListBoxGetSelected);
  scAdd_External_Symbol("ListBoxRemove", (void *)ListBoxRemove);
  scAdd_External_Symbol("ListBoxSaveGameList", (void *)ListBoxSaveGameList);
  scAdd_External_Symbol("ListBoxSetSelected", (void *)ListBoxSetSelected);
  scAdd_External_Symbol("ListBoxSetTopItem", (void *)ListBoxSetTopItem);
}

