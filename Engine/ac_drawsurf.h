#ifndef _AC_DRAWSURF_H_HEADER
#define _AC_DRAWSURF_H_HEADER

struct ScriptDrawingSurface;

extern void DrawingSurface_Release(ScriptDrawingSurface* sds);

extern void register_drawing_surface_script_functions();

#endif
