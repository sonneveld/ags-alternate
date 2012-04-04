#ifndef _AC_MOUSE_H_HEADER
#define _AC_MOUSE_H_HEADER

// forward declarations:
#include "sdlwrap/allegro.h"        // for BITMAP
typedef ALW_BITMAP *block;      // wgt2allh.h
class IDriverDependantBitmap; // ali3d.h

// The text script's "mouse" struct
struct ScriptMouse {
  int x,y;
};

struct GlobalMouseState {

    GlobalMouseState() 
    {
        
        blank_mouse_cursor = NULL;
        dotted_mouse_cursor = NULL;
        mouse_frame=0;
        mouse_delay=0;
        lastmx = -1;
        lastmy = -1;

    }

    IDriverDependantBitmap *mouseCursor;  // bitmap of current mouse cursor
    block blank_mouse_cursor;             // default blank mouse
    block dotted_mouse_cursor;            // dotted mouse cursor
    int mouse_frame;    // current frame of animated mouse
    int mouse_delay;    // countdown delay until next frame update
    int lastmx;     // last mouse position
    int lastmy;
    ScriptMouse scmouse;  // x,y of mouse.
};


extern void SetMouseBounds (int x1, int y1, int x2, int y2);
extern void set_new_cursor_graphic (int spriteslot);
extern void RefreshMouse();
extern int GetCursorMode();
extern void enable_cursor_mode(int modd);
extern void disable_cursor_mode(int modd);
extern void set_cursor_mode(int newmode);
extern void SetNextCursor ();
extern void update_script_mouse_coords();
extern void update_animating_cursor() ;
extern void update_and_draw_mouse_on_screen();
extern void set_mouse_cursor(int newcurs);
extern void set_default_cursor();

extern void register_mouse_script_functions();

#endif