#ifndef _AC_ROOM_H_HEADER
#define _AC_ROOM_H_HEADER

#define OVERLAPPING_OBJECT 1000

extern int AreThingsOverlapping(int thing1, int thing2);
extern int GetPlayerCharacter();
extern int GetScalingAt (int x, int y);
extern void on_background_frame_change();

extern void register_room_script_functions();

#endif