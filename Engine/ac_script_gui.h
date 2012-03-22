#ifndef _AC_SCRIPT_GUI_H_HEADER
#define _AC_SCRIPT_GUI_H_HEADER

extern int GetGUIAt (int xx,int yy);
extern void InterfaceOn(int ifn);
extern void InterfaceOff(int ifn);
extern void update_gui_zorder();

extern void register_gui_script_functions();

#endif
