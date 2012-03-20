#ifndef _ACWAVI3D_H_HEADER
#define _ACWAVI3D_H_HEADER

extern int dxmedia_play_video_3d(const char*filename, IDirect3DDevice9 *device, bool useAVISound, int canskip, int stretch);
extern void dxmedia_shutdown_3d();

#endif