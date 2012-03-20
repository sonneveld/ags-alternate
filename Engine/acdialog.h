#ifndef _ACDIALOG_H_HEADER
#define _ACDIALOG_H_HEADER

extern int  loadgamedialog();
extern int  savegamedialog();
extern int  quitdialog();
extern int  cbuttfont;
extern int  acdialog_font;
extern int  enternumberwindow(char*);
extern void enterstringwindow(char*,char*);
extern int  roomSelectorWindow(int currentRoom, int numRooms, int*roomNumbers, char**roomNames);

extern char buffer2[260];

#endif