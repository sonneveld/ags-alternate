#ifndef _AC_HOTSPOT_H_HEADER
#define _AC_HOTSPOT_H_HEADER

extern void RunHotspotInteraction (int hotspothere, int mood);
extern void DisableHotspot(int hsnum);
void EnableHotspot(int hsnum);
extern int get_hotspot_at(int xpp,int ypp);
extern int __GetLocationType(int xxx,int yyy, int allowHotspot0);

extern void register_hotspot_script_functions();

#endif
