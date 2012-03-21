#ifndef _MOUSEW32_H_HEADER
#define _MOUSEW32_H_HEADER

#define DOMOUSE_NOCURSOR 5

enum {NONE=-1, LEFT=0, RIGHT=1, MIDDLE=2};

extern char currentcursor;
extern int mousex;
extern int mousey;
extern int hotx;
extern int hoty;
extern int disable_mgetgraphpos;
extern char ignore_bounds;
extern block mousecurs[];

extern void msetcallback(IMouseGetPosCallback *gpCallback);
extern void mgraphconfine(int x1, int y1, int x2, int y2);
extern void mgetgraphpos();
extern void msetcursorlimit(int x1, int y1, int x2, int y2);
extern void domouse(int str);
extern int mgetbutton();
extern int misbuttondown(int buno);
extern void msetgraphpos(int xa, int ya);
extern void msethotspot(int xx, int yy);
extern int minstalled();

#endif