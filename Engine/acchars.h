#ifndef _ACCHARS_H_HEADER
#define _ACCHARS_H_HEADER

extern void DisplaySpeech(char*texx, int aschar);
extern int check_click_on_character(int xx,int yy,int mood);
extern void add_inventory(int inum) ;
extern void MoveCharacterBlocking(int chaa,int xx,int yy,int direct) ;
extern void MoveCharacter(int cc,int xx,int yy) ;
extern void lose_inventory(int inum);
extern void NewRoomEx(int nrnum,int newx,int newy);
extern void FollowCharacter(int who, int tofollow) ;
extern void SetCharacterIdle(int who, int iview, int itime);
extern void ChangeCharacterView(int chaa,int vii);

extern void register_character_script_functions();

#endif