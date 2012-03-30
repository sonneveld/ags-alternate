#include "ac_system.h"

#include "allegro_wrapper.h"

#include "ac.h"
#include "ac_context.h"
#include "acsound.h"
#include "acgfx.h"
#include "ali3d.h"
#include "ac_string.h"

/* *** SCRIPT SYMBOL: [System] System::get_ColorDepth *** */
int System_GetColorDepth() {
  return final_col_dep;
}

/* *** SCRIPT SYMBOL: [System] System::get_OperatingSystem *** */
int System_GetOS() {
  return scsystem.os;
}

/* *** SCRIPT SYMBOL: [System] System::get_ScreenWidth *** */
int System_GetScreenWidth() {
  return final_scrn_wid;
}

/* *** SCRIPT SYMBOL: [System] System::get_ScreenHeight *** */
int System_GetScreenHeight() {
  return final_scrn_hit;
}

/* *** SCRIPT SYMBOL: [System] System::get_ViewportHeight *** */
int System_GetViewportHeight() {
  return divide_down_coordinate(scrnhit);
}

/* *** SCRIPT SYMBOL: [System] System::get_ViewportWidth *** */
int System_GetViewportWidth() {
  return divide_down_coordinate(scrnwid);
}

/* *** SCRIPT SYMBOL: [System] System::get_Version *** */
/* *** SCRIPT SYMBOL: [SystemInfo] SystemInfo::get_Version *** */
const char *System_GetVersion() {
  return CreateNewScriptString(ACI_VERSION_TEXT);
}

/* *** SCRIPT SYMBOL: [System] System::get_HardwareAcceleration *** */
int System_GetHardwareAcceleration() 
{
  return gfxDriver->HasAcceleratedStretchAndFlip() ? 1 : 0;
}

/* *** SCRIPT SYMBOL: [System] System::get_NumLock *** */
int System_GetNumLock()
{
  return (alw_key_shifts & KB_NUMLOCK_FLAG) ? 1 : 0;
}

/* *** SCRIPT SYMBOL: [System] System::get_CapsLock *** */
int System_GetCapsLock()
{
  return (alw_key_shifts & KB_CAPSLOCK_FLAG) ? 1 : 0;
}

/* *** SCRIPT SYMBOL: [System] System::get_ScrollLock *** */
int System_GetScrollLock()
{
  return (alw_key_shifts & KB_SCROLOCK_FLAG) ? 1 : 0;
}

/* *** SCRIPT SYMBOL: [System] System::set_NumLock *** */
void System_SetNumLock(int newValue)
{
  // doesn't work ... maybe allegro doesn't implement this on windows
  int ledState = alw_key_shifts & (KB_SCROLOCK_FLAG | KB_CAPSLOCK_FLAG);
  if (newValue)
  {
    ledState |= KB_NUMLOCK_FLAG;
  }
  alw_set_leds(ledState);
}

/* *** SCRIPT SYMBOL: [System] System::get_VSync *** */
int System_GetVsync() {
  return scsystem.vsync;
}

/* *** SCRIPT SYMBOL: [System] System::set_VSync *** */
void System_SetVsync(int newValue) {
  scsystem.vsync = newValue;
}

/* *** SCRIPT SYMBOL: [System] System::get_Windowed *** */
int System_GetWindowed() {
  if (usetup.windowed)
    return 1;
  return 0;
}


/* *** SCRIPT SYMBOL: [System] System::get_SupportsGammaControl *** */
int System_GetSupportsGammaControl() {
  return gfxDriver->SupportsGammaControl();
}

/* *** SCRIPT SYMBOL: [System] System::get_Gamma *** */
int System_GetGamma() {
  return play.gamma_adjustment;
}

/* *** SCRIPT SYMBOL: [System] System::set_Gamma *** */
void System_SetGamma(int newValue) {
  if ((newValue < 0) || (newValue > 200))
    quitprintf("!System.Gamma: value must be between 0-200 (not %d)", newValue);

  if (play.gamma_adjustment != newValue) {
    DEBUG_CONSOLE("Gamma control set to %d", newValue);
    play.gamma_adjustment = newValue;

    if (gfxDriver->SupportsGammaControl())
      gfxDriver->SetGamma(newValue);
  }
}


/* *** SCRIPT SYMBOL: [System] SetMusicMasterVolume *** */
void SetMusicMasterVolume(int newvol) {
  if ((newvol<0) | (newvol>100))
    quit("!SetMusicMasterVolume: invalid volume - must be from 0-100");
  play.music_master_volume=newvol+60;
  update_music_volume();
  }


/* *** SCRIPT SYMBOL: [System] SetDigitalMasterVolume *** */
void SetDigitalMasterVolume (int newvol) {
  if ((newvol<0) | (newvol>100))
    quit("!SetDigitalMasterVolume: invalid volume - must be from 0-100");
  play.digital_master_volume = newvol;
  alw_set_volume ((newvol * 255) / 100, -1);
}

/* *** SCRIPT SYMBOL: [System] System::get_Volume *** */
int System_GetVolume() 
{
  return play.digital_master_volume;
}

/* *** SCRIPT SYMBOL: [System] System::set_Volume *** */
void System_SetVolume(int newvol) 
{
  if ((newvol < 0) || (newvol > 100))
    quit("!System.Volume: invalid volume - must be from 0-100");

  if (newvol == play.digital_master_volume)
    return;

  play.digital_master_volume = newvol;
  alw_set_volume((newvol * 255) / 100, (newvol * 255) / 100);

  // allegro's set_volume can lose the volumes of all the channels
  // if it was previously set low; so restore them
  for (int i = 0; i <= MAX_SOUND_CHANNELS; i++) 
  {
    if ((channels[i] != NULL) && (channels[i]->done == 0)) 
    {
      channels[i]->set_volume(channels[i]->vol);
    }
  }
}




void register_system_script_functions() {

  scAdd_External_Symbol("System::get_CapsLock", (void *)System_GetCapsLock);
  scAdd_External_Symbol("System::get_ColorDepth", (void *)System_GetColorDepth);
  scAdd_External_Symbol("System::get_Gamma", (void *)System_GetGamma);
  scAdd_External_Symbol("System::set_Gamma", (void *)System_SetGamma);
  scAdd_External_Symbol("System::get_HardwareAcceleration", (void *)System_GetHardwareAcceleration);
  scAdd_External_Symbol("System::get_NumLock", (void *)System_GetNumLock);
  scAdd_External_Symbol("System::set_NumLock", (void *)System_SetNumLock);
  scAdd_External_Symbol("System::get_OperatingSystem", (void *)System_GetOS);
  scAdd_External_Symbol("System::get_ScreenHeight", (void *)System_GetScreenHeight);
  scAdd_External_Symbol("System::get_ScreenWidth", (void *)System_GetScreenWidth);
  scAdd_External_Symbol("System::get_ScrollLock", (void *)System_GetScrollLock);
  scAdd_External_Symbol("System::get_SupportsGammaControl", (void *)System_GetSupportsGammaControl);
  scAdd_External_Symbol("System::get_Version", (void *)System_GetVersion);
  scAdd_External_Symbol("SystemInfo::get_Version", (void *)System_GetVersion);
  scAdd_External_Symbol("System::get_ViewportHeight", (void *)System_GetViewportHeight);
  scAdd_External_Symbol("System::get_ViewportWidth", (void *)System_GetViewportWidth);
  scAdd_External_Symbol("System::get_Volume",(void *)System_GetVolume);
  scAdd_External_Symbol("System::set_Volume",(void *)System_SetVolume);
  scAdd_External_Symbol("System::get_VSync", (void *)System_GetVsync);
  scAdd_External_Symbol("System::set_VSync", (void *)System_SetVsync);
  scAdd_External_Symbol("System::get_Windowed", (void *)System_GetWindowed);
  scAdd_External_Symbol("SetDigitalMasterVolume",(void *)SetDigitalMasterVolume);
  scAdd_External_Symbol("SetMusicMasterVolume",(void *)SetMusicMasterVolume);

}

