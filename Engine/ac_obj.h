#ifndef _AC_SCR_OBJ_H_HEADER
#define _AC_SCR_OBJ_H_HEADER

// forward declarations:
#include "sdlwrap/allegro.h"    // for BITMAP
typedef ALW_BITMAP *block;  // wgt2allh.h
struct MoveList;        // acroom.h

extern int check_click_on_object(int xx,int yy,int mood);
extern int GetObjectAt(int xx,int yy) ;
extern void MoveObject(int objj,int xx,int yy,int spp);
extern void ObjectOff(int obn);
extern void ObjectOn(int obn);
extern void AnimateObject(int,int,int,int);
extern void convert_move_path_to_high_res(MoveList *ml);

extern void get_local_tint(int xpp, int ypp, int nolight,
                    int *tint_amnt, int *tint_r, int *tint_g,
                    int *tint_b, int *tint_lit,
                    int *light_lev);
extern void apply_tint_or_light(int actspsindex, int light_level,
                         int tint_amount, int tint_red, int tint_green,
                         int tint_blue, int tint_light, int coldept,
                         block blitFrom);
extern int scale_and_flip_sprite(int useindx, int coldept, int zoom_level,
                          int sppic, int newwidth, int newheight,
                          int isMirrored);
extern void scale_sprite_size(int sppic, int zoom_level, int *newwidth, int *newheight);
extern void prepare_objects_for_drawing();
extern int get_area_scaling (int onarea, int xx, int yy);
extern void SetObjectView(int,int);
extern void MergeObject(int obn);

extern void register_object_script_functions();

#endif