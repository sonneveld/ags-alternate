#ifndef MAC_VERSION
#error This file should only be included on the Mac build
#endif

// ********* MacOS PLACEHOLDER DRIVER *********

#include "acplatfm.h"
#include <unistd.h>
#include "mac_alert.h"


bool PlayMovie(char const *name, int skipType) {
    return true;
}


struct AGSMac : AGS32BitOSDriver {

  virtual int  CDPlayerCommand(int cmdd, int datt);
  virtual void Delay(int millis);
  virtual void DisplayAlert(const char*, ...);
  virtual unsigned long GetDiskFreeSpaceMB();
  virtual const char* GetNoMouseErrorString();
  virtual eScriptSystemOSID GetSystemOSID();
  virtual int  InitializeCDPlayer();
  virtual void PlayVideo(const char* name, int skip, int flags);
  virtual void PostAllegroExit();
  virtual int  RunSetup();
  virtual void SetGameWindowIcon();
  virtual void ShutdownCDPlayer();
  virtual void WriteConsole(const char*, ...);
  virtual void ReplaceSpecialPaths(const char *sourcePath, char *destPath);
};


void AGSMac::ReplaceSpecialPaths(const char *sourcePath, char *destPath) {}

int AGSMac::CDPlayerCommand(int cmdd, int datt) {
  return cd_player_control(cmdd, datt);
}

void AGSMac::DisplayAlert(const char *text, ...) {
  char displbuf[2501];  // 1 more than win version
  va_list ap;
  va_start(ap, text);
  vsnprintf(displbuf, 2501, text, ap);
  va_end(ap);
  show_alert(displbuf, "Adventure Game Studio");
}

void AGSMac::Delay(int millis) {
  alw_rest(millis);
}

unsigned long AGSMac::GetDiskFreeSpaceMB() {
  // placeholder
  return 100;
}

const char* AGSMac::GetNoMouseErrorString() {
  return "This game requires a mouse. You need to configure and setup your mouse to play this game.\n";
}

eScriptSystemOSID AGSMac::GetSystemOSID() {
  // Gemini Rue checks this value before deciding to play ambient sounds!  
  // Right now we fake it by pretending to be win
  return eOS_Win;
  // return eOS_Mac;
}

int AGSMac::InitializeCDPlayer() {
  return cd_player_init();
}

void AGSMac::PlayVideo(const char *name, int skip, int flags) {
  if (!PlayMovie(name, skip))
  {
    char useloc[512];
    sprintf(useloc, "%s/%s", alw_get_filename(usetup.data_files_dir), name);
    PlayMovie(useloc, skip);
  }
}

void AGSMac::PostAllegroExit() {
  // do nothing
}

int AGSMac::RunSetup() {
  return 0;
}

void AGSMac::SetGameWindowIcon() {
  // do nothing
}

void AGSMac::WriteConsole(const char *text, ...) {
  char displbuf[2000];
  va_list ap;
  va_start(ap, text);
  vsprintf(displbuf, text, ap);
  va_end(ap);
  printf("%s", displbuf);
}

void AGSMac::ShutdownCDPlayer() {
  cd_exit();
}

AGSPlatformDriver* AGSPlatformDriver::GetDriver() {
  if (instance == NULL)
    instance = new AGSMac();
  return instance;
}
