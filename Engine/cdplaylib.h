#ifndef _CDPLAYLIB_H_HEADER
#define _CDPLAYLIB_H_HEADER

// CD Player functions
// flags returned with cd_getstatus
#define CDS_DRIVEOPEN    0x0001  // tray is open
#define CDS_DRIVELOCKED  0x0002  // tray locked shut by software
#define CDS_AUDIOSUPPORT 0x0010  // supports audio CDs
#define CDS_DRIVEEMPTY   0x0800  // no CD in drive
// function definitions
extern int  cd_installed();
extern int  cd_getversion();
extern int  cd_getdriveletters(char*);
extern void cd_driverinit(int);
extern void cd_driverclose(int);
extern long cd_getstatus(int);
extern void cd_playtrack(int,int);
extern void cd_stopmusic(int);
extern void cd_resumemusic(int);
extern void cd_eject(int);
extern void cd_uneject(int);
extern int  cd_getlasttrack(int);
extern int  cd_isplayingaudio(int);

#endif