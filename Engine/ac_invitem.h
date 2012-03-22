#ifndef _AC_INVITEM_H_HEADER
#define _AC_INVITEM_H_HEADER

struct GUIInv; //acgui.h

struct ScriptInvItem {
  int id;
  int reserved;
};

extern void RunInventoryInteraction (int iit, int modd);
extern int GetInvAt (int xxx, int yyy) ;
extern int offset_over_inv(GUIInv *inv);
extern void run_event_block_inv(int invNum, int aaa);

extern void register_inventory_item_script_functions();

#endif
