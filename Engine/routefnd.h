#ifndef _ROUTEFND_H_HEADER
#define _ROUTEFND_H_HEADER

// forward declarations:
#include "sdlwrap/allegro.h"        // for BITMAP
typedef ALW_BITMAP *block;      // wgt2allh.h

extern int route_script_link();
extern int find_route(short , short , short , short , block , int , int, int);
extern void print_welcome_text(char*,char*);

#endif