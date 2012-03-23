#ifndef _AC_GAME_H_HEADER
#define _AC_GAME_H_HEADER

extern void restore_game_dialog();
extern void restart_game();
extern void save_game_dialog();
extern void UnPauseGame();
extern void PauseGame();
extern int IsGamePaused();
extern void close_translation ();
extern void GiveScore(int amnt) ;
extern void scrWait(int nloops);
extern void DisableInterface();
extern void EnableInterface() ;
extern int RunAGSGame (char *newgame, unsigned int mode, int data);
extern void QuitGame(int dialog);
extern bool init_translation (const char *lang);
extern void SetRestartPoint();
extern int Game_SetSaveGameDirectory(const char *newFolder);
extern void sc_inputbox(const char*,char*);

extern void register_game_script_functions();

#endif
