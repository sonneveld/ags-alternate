#include "ac_gui_inv_window.h"

#include "allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "acgui.h"
#include "sprcache.h"
#include "bmp.h"
#include "ac_mouse.h"
#include "ac_exescr.h"
#include "ac_input.h"


extern void DisplayMessage(int msnum); // ac_test.h

// *** INV WINDOW FUNCTIONS

/* *** SCRIPT SYMBOL: [InvWindow] InvWindow::set_CharacterToUse *** */
void InvWindow_SetCharacterToUse(GUIInv *guii, CharacterInfo *chaa) {
  if (chaa == NULL)
    guii->charId = -1;
  else
    guii->charId = chaa->index_id;
  // reset to top of list
  guii->topIndex = 0;

  guis_need_update = 1;
}

/* *** SCRIPT SYMBOL: [InvWindow] InvWindow::get_CharacterToUse *** */
CharacterInfo* InvWindow_GetCharacterToUse(GUIInv *guii) {
  if (guii->charId < 0)
    return NULL;

  return &game.chars[guii->charId];
}

/* *** SCRIPT SYMBOL: [InvWindow] InvWindow::set_ItemWidth *** */
void InvWindow_SetItemWidth(GUIInv *guii, int newwidth) {
  guii->itemWidth = newwidth;
  guii->Resized();
}

/* *** SCRIPT SYMBOL: [InvWindow] InvWindow::get_ItemWidth *** */
int InvWindow_GetItemWidth(GUIInv *guii) {
  return guii->itemWidth;
}

/* *** SCRIPT SYMBOL: [InvWindow] InvWindow::set_ItemHeight *** */
void InvWindow_SetItemHeight(GUIInv *guii, int newhit) {
  guii->itemHeight = newhit;
  guii->Resized();
}

/* *** SCRIPT SYMBOL: [InvWindow] InvWindow::get_ItemHeight *** */
int InvWindow_GetItemHeight(GUIInv *guii) {
  return guii->itemHeight;
}

/* *** SCRIPT SYMBOL: [InvWindow] InvWindow::set_TopItem *** */
void InvWindow_SetTopItem(GUIInv *guii, int topitem) {
  if (guii->topIndex != topitem) {
    guii->topIndex = topitem;
    guis_need_update = 1;
  }
}

/* *** SCRIPT SYMBOL: [InvWindow] InvWindow::get_TopItem *** */
int InvWindow_GetTopItem(GUIInv *guii) {
  return guii->topIndex;
}

/* *** SCRIPT SYMBOL: [InvWindow] InvWindow::get_ItemsPerRow *** */
int InvWindow_GetItemsPerRow(GUIInv *guii) {
  return guii->itemsPerLine;
}

/* *** SCRIPT SYMBOL: [InvWindow] InvWindow::get_ItemCount *** */
int InvWindow_GetItemCount(GUIInv *guii) {
  return charextra[guii->CharToDisplay()].invorder_count;
}

/* *** SCRIPT SYMBOL: [InvWindow] InvWindow::get_RowCount *** */
int InvWindow_GetRowCount(GUIInv *guii) {
  return guii->numLines;
}

/* *** SCRIPT SYMBOL: [InvWindow] InvWindow::ScrollDown^0 *** */
void InvWindow_ScrollDown(GUIInv *guii) {
  if ((charextra[guii->CharToDisplay()].invorder_count) >
      (guii->topIndex + (guii->itemsPerLine * guii->numLines))) { 
    guii->topIndex += guii->itemsPerLine;
    guis_need_update = 1;
  }
}

/* *** SCRIPT SYMBOL: [InvWindow] InvWindow::ScrollUp^0 *** */
void InvWindow_ScrollUp(GUIInv *guii) {
  if (guii->topIndex > 0) {
    guii->topIndex -= guii->itemsPerLine;
    if (guii->topIndex < 0)
      guii->topIndex = 0;

    guis_need_update = 1;
  }
}

/* *** SCRIPT SYMBOL: [InvWindow] InvWindow::geti_ItemAtIndex *** */
ScriptInvItem* InvWindow_GetItemAtIndex(GUIInv *guii, int index) {
  if ((index < 0) || (index >= charextra[guii->CharToDisplay()].invorder_count))
    return NULL;
  return &scrInv[charextra[guii->CharToDisplay()].invorder[index]];
}



#define ICONSPERLINE 4

struct DisplayInvItem {
  int num;
  int sprnum;
  };
int __actual_invscreen() {
  
  int BUTTONAREAHEIGHT = get_fixed_pixel_size(30);
  int cmode=CURS_ARROW, toret = -1;
  int top_item = 0, num_visible_items = 0;
  int MAX_ITEMAREA_HEIGHT = ((scrnhit - BUTTONAREAHEIGHT) - get_fixed_pixel_size(20));
  in_inv_screen++;
  inv_screen_newroom = -1;

start_actinv:
  wsetscreen(virtual_screen);
  
  DisplayInvItem dii[MAX_INV];
  int numitems=0,ww,widest=0,highest=0;
  if (charextra[game.playercharacter].invorder_count < 0)
    update_invorder();
  if (charextra[game.playercharacter].invorder_count == 0) {
    DisplayMessage(996);
    in_inv_screen--;
    return -1;
  }

  if (inv_screen_newroom >= 0) {
    in_inv_screen--;
    NewRoom(inv_screen_newroom);
    return -1;
  }

  for (ww = 0; ww < charextra[game.playercharacter].invorder_count; ww++) {
    if (game.invinfo[charextra[game.playercharacter].invorder[ww]].name[0]!=0) {
      dii[numitems].num = charextra[game.playercharacter].invorder[ww];
      dii[numitems].sprnum = game.invinfo[charextra[game.playercharacter].invorder[ww]].pic;
      int snn=dii[numitems].sprnum;
      if (spritewidth[snn] > widest) widest=spritewidth[snn];
      if (spriteheight[snn] > highest) highest=spriteheight[snn];
      numitems++;
      }
    }
  if (numitems != charextra[game.playercharacter].invorder_count)
    quit("inconsistent inventory calculations");

  widest += get_fixed_pixel_size(4);
  highest += get_fixed_pixel_size(4);
  num_visible_items = (MAX_ITEMAREA_HEIGHT / highest) * ICONSPERLINE;

  int windowhit = highest * (numitems/ICONSPERLINE) + get_fixed_pixel_size(4);
  if ((numitems%ICONSPERLINE) !=0) windowhit+=highest;
  if (windowhit > MAX_ITEMAREA_HEIGHT) {
    windowhit = (MAX_ITEMAREA_HEIGHT / highest) * highest + get_fixed_pixel_size(4);
  }
  windowhit += BUTTONAREAHEIGHT;

  int windowwid = widest*ICONSPERLINE + get_fixed_pixel_size(4);
  if (windowwid < get_fixed_pixel_size(105)) windowwid = get_fixed_pixel_size(105);
  int windowxp=scrnwid/2-windowwid/2;
  int windowyp=scrnhit/2-windowhit/2;
  int buttonyp=windowyp+windowhit-BUTTONAREAHEIGHT;
  wsetcolor(play.sierra_inv_color);
  wbar(windowxp,windowyp,windowxp+windowwid,windowyp+windowhit);
  wsetcolor(0); 
  int bartop = windowyp + get_fixed_pixel_size(2);
  int barxp = windowxp + get_fixed_pixel_size(2);
  wbar(barxp,bartop, windowxp + windowwid - get_fixed_pixel_size(2),buttonyp-1);
  for (ww = top_item; ww < numitems; ww++) {
    if (ww >= top_item + num_visible_items)
      break;
    block spof=spriteset[dii[ww].sprnum];
    wputblock(barxp+1+((ww-top_item)%4)*widest+widest/2-wgetblockwidth(spof)/2,
      bartop+1+((ww-top_item)/4)*highest+highest/2-wgetblockheight(spof)/2,spof,1);
    }
  if ((spriteset[2041] == NULL) || (spriteset[2042] == NULL) || (spriteset[2043] == NULL))
    quit("!InventoryScreen: one or more of the inventory screen graphics have been deleted");
  #define BUTTONWID spritewidth[2042]
  // Draw select, look and OK buttons
  wputblock(windowxp+2, buttonyp + get_fixed_pixel_size(2), spriteset[2041], 1);
  wputblock(windowxp+3+BUTTONWID, buttonyp + get_fixed_pixel_size(2), spriteset[2042], 1);
  wputblock(windowxp+4+BUTTONWID*2, buttonyp + get_fixed_pixel_size(2), spriteset[2043], 1);

  // Draw Up and Down buttons if required
  const int ARROWBUTTONWID = 11;
  block arrowblock = alw_create_bitmap (ARROWBUTTONWID, ARROWBUTTONWID);
  alw_clear_to_color(arrowblock, alw_bitmap_mask_color(arrowblock));
  int usecol;
  __my_setcolor(&usecol, 0);
  if (play.sierra_inv_color == 0)
    __my_setcolor(&usecol, 14);

  alw_line(arrowblock,ARROWBUTTONWID/2, 2, ARROWBUTTONWID-2, 9, usecol);
  alw_line(arrowblock,ARROWBUTTONWID/2, 2, 2, 9, usecol);
  alw_line(arrowblock, 2, 9, ARROWBUTTONWID-2, 9, usecol);
  alw_floodfill(arrowblock, ARROWBUTTONWID/2, 4, usecol);

  if (top_item > 0)
    wputblock(windowxp+windowwid-ARROWBUTTONWID, buttonyp + get_fixed_pixel_size(2), arrowblock, 1);
  if (top_item + num_visible_items < numitems)
    alw_draw_sprite_v_flip (abuf, arrowblock, windowxp+windowwid-ARROWBUTTONWID, buttonyp + get_fixed_pixel_size(4) + ARROWBUTTONWID);
  wfreeblock(arrowblock);

  ac_domouse(1);
  set_mouse_cursor(cmode);
  int wasonitem=-1;
  while (!ac_kbhit()) {
    timerloop = 0;
    NEXT_ITERATION();
    ac_domouse(0);
    update_polled_stuff_and_crossfade();
    write_screen();

    int isonitem=((mousey-bartop)/highest)*ICONSPERLINE+(mousex-barxp)/widest;
    if (mousey<=bartop) isonitem=-1;
    else if (isonitem >= 0) isonitem += top_item;
    if ((isonitem<0) | (isonitem>=numitems) | (isonitem >= top_item + num_visible_items))
      isonitem=-1;

    int mclick = ac_mgetbutton();
    if (mclick == LEFT) {
      if ((mousey<windowyp) | (mousey>windowyp+windowhit) | (mousex<windowxp) | (mousex>windowxp+windowwid))
        continue;
      if (mousey<buttonyp) {
        int clickedon=isonitem;
        if (clickedon<0) continue;
        evblocknum=dii[clickedon].num;
        play.used_inv_on = dii[clickedon].num;

        if (cmode==MODE_LOOK) {
          ac_domouse(2);
          run_event_block_inv(dii[clickedon].num, 0); 
          // in case the script did anything to the screen, redraw it
          mainloop();
          
          goto start_actinv;
          continue;
        }
        else if (cmode==MODE_USE) {
          // use objects on each other
          play.usedinv=toret;

          // set the activeinv so the script can check it
          int activeinvwas = playerchar->activeinv;
          playerchar->activeinv = toret;

          ac_domouse(2);
          run_event_block_inv(dii[clickedon].num, 3);

          // if the script didn't change it, then put it back
          if (playerchar->activeinv == toret)
            playerchar->activeinv = activeinvwas;

          // in case the script did anything to the screen, redraw it
          mainloop();
          
          // They used the active item and lost it
          if (playerchar->inv[toret] < 1) {
            cmode = CURS_ARROW;
            set_mouse_cursor(cmode);
            toret = -1;
          }
 
          goto start_actinv;
//          continue;
          }
        toret=dii[clickedon].num;
//        int plusng=play.using; play.using=toret;
        update_inv_cursor(toret);
        set_mouse_cursor(MODE_USE);
        cmode=MODE_USE;
//        play.using=plusng;
//        break;
        continue;
        }
      else {
        if (mousex >= windowxp+windowwid-ARROWBUTTONWID) {
          if (mousey < buttonyp + get_fixed_pixel_size(2) + ARROWBUTTONWID) {
            if (top_item > 0) {
              top_item -= ICONSPERLINE;
              ac_domouse(2);
              goto start_actinv;
              }
            }
          else if ((mousey < buttonyp + get_fixed_pixel_size(4) + ARROWBUTTONWID*2) && (top_item + num_visible_items < numitems)) {
            top_item += ICONSPERLINE;
            ac_domouse(2);
            goto start_actinv;
            }
          continue;
          }

        int buton=(mousex-windowxp)-2;
        if (buton<0) continue;
        buton/=BUTTONWID;
        if (buton>=3) continue;
        if (buton==0) { toret=-1; cmode=MODE_LOOK; }
        else if (buton==1) { cmode=CURS_ARROW; toret=-1; }
        else break;
        set_mouse_cursor(cmode);
        }
      }
    else if (mclick == RIGHT) {
      if (cmode == CURS_ARROW)
        cmode = MODE_LOOK;
      else
        cmode = CURS_ARROW;
      toret = -1;
      set_mouse_cursor(cmode);
    }
    else if (isonitem!=wasonitem) { ac_domouse(2);
      int rectxp=barxp+1+(wasonitem%4)*widest;
      int rectyp=bartop+1+((wasonitem - top_item)/4)*highest;
      if (wasonitem>=0) {
        wsetcolor(0);
        wrectangle(rectxp,rectyp,rectxp+widest-1,rectyp+highest-1);
        }
      if (isonitem>=0) { wsetcolor(14);//opts.invrectcol);
        rectxp=barxp+1+(isonitem%4)*widest;
        rectyp=bartop+1+((isonitem - top_item)/4)*highest;
        wrectangle(rectxp,rectyp,rectxp+widest-1,rectyp+highest-1);
        }
      ac_domouse(1);
      }
    wasonitem=isonitem;
    while (timerloop == 0) {
      update_polled_stuff();
      platform->YieldCPU();
    }
  }
  while (ac_kbhit()) ac_getch();
  set_default_cursor();
  ac_domouse(2);
  construct_virtual_screen(true);
  in_inv_screen--;
  return toret;
  }

int invscreen() {
  int selt=__actual_invscreen();
  if (selt<0) return -1;
  playerchar->activeinv=selt;
  guis_need_update = 1;
  set_cursor_mode(MODE_USE);
  return selt;
  }

/* *** SCRIPT SYMBOL: [Game] InventoryScreen *** */
void sc_invscreen() {
  curscript->queue_action(ePSAInvScreen, 0, "InventoryScreen");
}




/* *** SCRIPT SYMBOL: [InvWindow] SetInvDimensions *** */
void SetInvDimensions(int ww,int hh) {
  play.inv_item_wid = ww;
  play.inv_item_hit = hh;
  play.inv_numdisp = 0;
  // backwards compatibility
  for (int i = 0; i < numguiinv; i++) {
    guiinv[i].itemWidth = ww;
    guiinv[i].itemHeight = hh;
    guiinv[i].Resized();
  }
  guis_need_update = 1;
}



void register_inv_window_script_functions() {
  scAdd_External_Symbol("InvWindow::ScrollDown^0", (void *)InvWindow_ScrollDown);
  scAdd_External_Symbol("InvWindow::ScrollUp^0", (void *)InvWindow_ScrollUp);
  scAdd_External_Symbol("InvWindow::get_CharacterToUse", (void *)InvWindow_GetCharacterToUse);
  scAdd_External_Symbol("InvWindow::set_CharacterToUse", (void *)InvWindow_SetCharacterToUse);
  scAdd_External_Symbol("InvWindow::geti_ItemAtIndex", (void *)InvWindow_GetItemAtIndex);
  scAdd_External_Symbol("InvWindow::get_ItemCount", (void *)InvWindow_GetItemCount);
  scAdd_External_Symbol("InvWindow::get_ItemHeight", (void *)InvWindow_GetItemHeight);
  scAdd_External_Symbol("InvWindow::set_ItemHeight", (void *)InvWindow_SetItemHeight);
  scAdd_External_Symbol("InvWindow::get_ItemWidth", (void *)InvWindow_GetItemWidth);
  scAdd_External_Symbol("InvWindow::set_ItemWidth", (void *)InvWindow_SetItemWidth);
  scAdd_External_Symbol("InvWindow::get_ItemsPerRow", (void *)InvWindow_GetItemsPerRow);
  scAdd_External_Symbol("InvWindow::get_RowCount", (void *)InvWindow_GetRowCount);
  scAdd_External_Symbol("InvWindow::get_TopItem", (void *)InvWindow_GetTopItem);
  scAdd_External_Symbol("InvWindow::set_TopItem", (void *)InvWindow_SetTopItem);
  scAdd_External_Symbol("InventoryScreen",(void *)sc_invscreen);
  scAdd_External_Symbol("SetInvDimensions",(void *)SetInvDimensions);
}

