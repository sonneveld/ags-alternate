#ifndef _AC_OVERLAY_H_HEADER
#define _AC_OVERLAY_H_HEADER

// forward declarations:
#include "sdlwrap/allegro.h"        // for BITMAP
typedef ALW_BITMAP *block;      // wgt2allg.h
struct ScriptOverlay;       // acruntim.h

extern void remove_screen_overlay_index(int cc);
extern void remove_screen_overlay(int type);
extern int add_screen_overlay(int x,int y,int type,block piccy, bool alphaChannel = false);
extern int Overlay_GetValid(ScriptOverlay *scover) ;

extern void register_overlay_script_functions();

#endif
