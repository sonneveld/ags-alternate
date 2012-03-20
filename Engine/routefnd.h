#ifndef _ROUTEFND_H_HEADER
#define _ROUTEFND_H_HEADER

#define WGT2ALLEGRO_NOFUNCTIONS
#include "wgt2allg.h"
#undef WGT2ALLEGRO_NOFUNCTIONS

extern int route_script_link();
extern int find_route(short , short , short , short , block , int , int, int);
extern void print_welcome_text(char*,char*);

#endif