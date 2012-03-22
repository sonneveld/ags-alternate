#include "ac_multimedia.h"

#include "ac.h"
#include "ac_context.h"
#include "acsound.h"
#include "bmp.h"
#include "acgfx.h"
#include "ali3d.h"
#include "clib32.h"


/* *** SCRIPT SYMBOL: [Multimedia] IsSoundPlaying *** */
int IsSoundPlaying() {
  if (play.fast_forward)
    return 0;

  // find if there's a sound playing
  for (int i = SCHAN_NORMAL; i < numSoundChannels; i++) {
    if ((channels[i] != NULL) && (channels[i]->done == 0))
      return 1;
  }

  return 0;
}



/* *** SCRIPT SYMBOL: [Multimedia] IsMusicPlaying *** */
int IsMusicPlaying() {
  // in case they have a "while (IsMusicPlaying())" loop
  if ((play.fast_forward) && (play.skip_until_char_stops < 0))
    return 0;

  if (usetup.midicard == MIDI_NONE)
    return 0;

  if (current_music_type != 0) {
    if (channels[SCHAN_MUSIC] == NULL)
      current_music_type = 0;
    else if (channels[SCHAN_MUSIC]->done == 0)
      return 1;
    else if ((crossFading > 0) && (channels[crossFading] != NULL))
      return 1;
    return 0;
  }

  return 0;
}



/* *** SCRIPT SYMBOL: [Multimedia] SetSpeechVolume *** */
void SetSpeechVolume(int newvol) {
  if ((newvol<0) | (newvol>255))
    quit("!SetSpeechVolume: invalid volume - must be from 0-255");

  if (channels[SCHAN_SPEECH])
    channels[SCHAN_SPEECH]->set_volume (newvol);

  play.speech_volume = newvol;
  }



// 0 = text only
// 1 = voice & text
// 2 = voice only
/* *** SCRIPT SYMBOL: [Multimedia] SetVoiceMode *** */
void SetVoiceMode (int newmod) {
  if ((newmod < 0) | (newmod > 2))
    quit("!SetVoiceMode: invalid mode number (must be 0,1,2)");
  // If speech is turned off, store the mode anyway in case the
  // user adds the VOX file later
  if (play.want_speech < 0)
    play.want_speech = (-newmod) - 1;
  else
    play.want_speech = newmod;
}

/* *** SCRIPT SYMBOL: [Multimedia] IsVoxAvailable *** */
int IsVoxAvailable() {
  if (play.want_speech < 0)
    return 0;
  return 1;
}


// FLIC player start
block fli_buffer;
short fliwidth,fliheight;
int canabort=0, stretch_flc = 1;
block hicol_buf=NULL;
IDriverDependantBitmap *fli_ddb;
BITMAP *fli_target;
int fliTargetWidth, fliTargetHeight;
int check_if_user_input_should_cancel_video()
{
  NEXT_ITERATION();
  if (ac_kbhit()) {
    if ((ac_getch()==27) && (canabort==1))
      return 1;
    if (canabort >= 2)
      return 1;  // skip on any key
  }
  if (canabort == 3) {  // skip on mouse click
    if (ac_mgetbutton()!=NONE) return 1;
  }
  return 0;
}

#if defined(WINDOWS_VERSION) || defined(LINUX_VERSION) || defined(MAC_VERSION)
int __cdecl fli_callback() {
#else
int fli_callback(...) {
#endif
  block usebuf = fli_buffer;

  update_polled_stuff_and_crossfade ();

  if (game.color_depth > 1) {
    blit(fli_buffer,hicol_buf,0,0,0,0,fliwidth,fliheight);
    usebuf=hicol_buf;
  }
  if (stretch_flc == 0)
    blit(usebuf, fli_target, 0,0,scrnwid/2-fliwidth/2,scrnhit/2-fliheight/2,scrnwid,scrnhit);
  else 
    stretch_blit(usebuf, fli_target, 0,0,fliwidth,fliheight,0,0,scrnwid,scrnhit);

  gfxDriver->UpdateDDBFromBitmap(fli_ddb, fli_target, false);
  gfxDriver->DrawSprite(0, 0, fli_ddb);
  render_to_screen(fli_target, 0, 0);

  return check_if_user_input_should_cancel_video();
}

/* *** SCRIPT SYMBOL: [Multimedia] PlayFlic *** */
void play_flc_file(int numb,int playflags) {
  color oldpal[256];

  if (play.fast_forward)
    return;

  wreadpalette(0,255,oldpal);

  int clearScreenAtStart = 1;
  canabort = playflags % 10;
  playflags -= canabort;

  if (canabort == 2) // convert to PlayVideo-compatible setting
    canabort = 3;

  if (playflags % 100 == 0)
    stretch_flc = 1;
  else
    stretch_flc = 0;

  if (playflags / 100)
    clearScreenAtStart = 0;

  char flicnam[20]; sprintf(flicnam,"flic%d.flc",numb);
  FILE*iii=clibfopen(flicnam,"rb");
  if (iii==NULL) { sprintf(flicnam,"flic%d.fli",numb);
    iii=clibfopen(flicnam,"rb"); }
  if (iii==NULL) {
    debug_log("FLIC animation FLIC%d.FLC not found",numb);
    return;
    }
  fseek(iii,8,SEEK_CUR);
  fread(&fliwidth,2,1,iii);
  fread(&fliheight,2,1,iii);
  fclose(iii);
  if (game.color_depth > 1) {
    hicol_buf=create_bitmap_ex(final_col_dep,fliwidth,fliheight);
    clear(hicol_buf);
    }
  // override the stretch option if necessary
  if ((fliwidth==scrnwid) && (fliheight==scrnhit))
    stretch_flc = 0;
  else if ((fliwidth > scrnwid) || (fliheight > scrnhit))
    stretch_flc = 1;
  fli_buffer=create_bitmap_ex(8,fliwidth,fliheight); //640,400); //scrnwid,scrnhit);
  if (fli_buffer==NULL) quit("Not enough memory to play animation");
  clear(fli_buffer);

  if (clearScreenAtStart) {
    clear(screen);
    render_to_screen(screen, 0, 0);
  }

  fli_target = create_bitmap_ex(final_col_dep, BMP_W(screen), BMP_H(screen));
  fli_ddb = gfxDriver->CreateDDBFromBitmap(fli_target, false, true);

  if (play_fli(flicnam,fli_buffer,0,fli_callback)==FLI_ERROR)
    quit("FLI/FLC animation play error");

  wfreeblock(fli_buffer);
  clear(screen);
  wsetpalette(0,255,oldpal);
  render_to_screen(screen, 0, 0);

  destroy_bitmap(fli_target);
  gfxDriver->DestroyDDB(fli_ddb);
  fli_ddb = NULL;

  if (hicol_buf!=NULL) {
    wfreeblock(hicol_buf);
    hicol_buf=NULL; }
//  wsetscreen(screen); wputblock(0,0,backbuffer,0);
  while (ac_mgetbutton()!=NONE) ;
  invalidate_screen();
}
// FLIC player end

int theora_playing_callback(BITMAP *theoraBuffer)
{
  if (theoraBuffer == NULL)
  {
    // No video, only sound
    return check_if_user_input_should_cancel_video();
  }

  int drawAtX = 0, drawAtY = 0;
  if (fli_ddb == NULL)
  {
    fli_ddb = gfxDriver->CreateDDBFromBitmap(theoraBuffer, false, true);
  }
  if (stretch_flc) 
  {
    drawAtX = scrnwid / 2 - fliTargetWidth / 2;
    drawAtY = scrnhit / 2 - fliTargetHeight / 2;
    if (!gfxDriver->HasAcceleratedStretchAndFlip())
    {
      stretch_blit(theoraBuffer, fli_target, 0, 0, BMP_W(theoraBuffer), BMP_H(theoraBuffer), 
                   drawAtX, drawAtY, fliTargetWidth, fliTargetHeight);
      gfxDriver->UpdateDDBFromBitmap(fli_ddb, fli_target, false);
      drawAtX = 0;
      drawAtY = 0;
    }
    else
    {
      gfxDriver->UpdateDDBFromBitmap(fli_ddb, theoraBuffer, false);
      fli_ddb->SetStretch(fliTargetWidth, fliTargetHeight);
    }
  }
  else
  {
    gfxDriver->UpdateDDBFromBitmap(fli_ddb, theoraBuffer, false);
    drawAtX = scrnwid / 2 - BMP_W(theoraBuffer) / 2;
    drawAtY = scrnhit / 2 - BMP_H(theoraBuffer) / 2;
  }

  gfxDriver->DrawSprite(drawAtX, drawAtY, fli_ddb);
  render_to_screen(virtual_screen, 0, 0);
  update_polled_stuff_and_crossfade ();

  return check_if_user_input_should_cancel_video();
}

#ifdef ENABLE_THIS_LATER

APEG_STREAM* get_theora_size(const char *fileName, int *width, int *height)
{
  APEG_STREAM* oggVid = apeg_open_stream(fileName);
  if (oggVid != NULL)
  {
    apeg_get_video_size(oggVid, width, height);
  }
  else
  {
    *width = 0;
    *height = 0;
  }
  return oggVid;
}

void calculate_destination_size_maintain_aspect_ratio(int vidWidth, int vidHeight, int *targetWidth, int *targetHeight)
{
  float aspectRatioVideo = (float)vidWidth / (float)vidHeight;
  float aspectRatioScreen = (float)scrnwid / (float)scrnhit;

  if (aspectRatioVideo == aspectRatioScreen)
  {
	  *targetWidth = scrnwid;
	  *targetHeight = scrnhit;
  }
  else if (aspectRatioVideo > aspectRatioScreen)
  {
    *targetWidth = scrnwid;
    *targetHeight = (int)(((float)scrnwid / aspectRatioVideo) + 0.5f);
  }
  else
  {
    *targetHeight = scrnhit;
    *targetWidth = (float)scrnhit * aspectRatioVideo;
  }

}

void play_theora_video(const char *name, int skip, int flags)
{
  apeg_set_display_depth(bitmap_color_depth(screen));
  // we must disable length detection, otherwise it takes ages to start
  // playing if the file is large because it seeks through the whole thing
  apeg_disable_length_detection(TRUE);
  apeg_enable_framedrop(TRUE);
  update_polled_stuff();

  stretch_flc = (flags % 10);
  canabort = skip;
  apeg_ignore_audio((flags >= 10) ? 1 : 0);

  int videoWidth, videoHeight;
  APEG_STREAM *oggVid = get_theora_size(name, &videoWidth, &videoHeight);

  if (videoWidth == 0)
  {
    Display("Unable to load theora video '%s'", name);
    return;
  }

  if (flags < 10)
  {
    stop_all_sound_and_music();
  }

  fli_target = NULL;
  //fli_buffer = create_bitmap_ex(final_col_dep, videoWidth, videoHeight);
  calculate_destination_size_maintain_aspect_ratio(videoWidth, videoHeight, &fliTargetWidth, &fliTargetHeight);

  if ((fliTargetWidth == videoWidth) && (fliTargetHeight == videoHeight) && (stretch_flc))
  {
    // don't need to stretch after all
    stretch_flc = 0;
  }

  if ((stretch_flc) && (!gfxDriver->HasAcceleratedStretchAndFlip()))
  {
    fli_target = create_bitmap_ex(final_col_dep, scrnwid, scrnhit);
    clear(fli_target);
    fli_ddb = gfxDriver->CreateDDBFromBitmap(fli_target, false, true);
  }
  else
  {
    fli_ddb = NULL;
  }

  update_polled_stuff();

  clear(virtual_screen);

  if (apeg_play_apeg_stream(oggVid, NULL, 0, theora_playing_callback) == APEG_ERROR)
  {
    Display("Error playing theora video '%s'", name);
  }
  apeg_close_stream(oggVid);

  //destroy_bitmap(fli_buffer);
  if (fli_target != NULL)
    destroy_bitmap(fli_target);
  gfxDriver->DestroyDDB(fli_ddb);
  fli_ddb = NULL;
  invalidate_screen();
}

void pause_sound_if_necessary_and_play_video(const char *name, int skip, int flags)
{
  int musplaying = play.cur_music_number, i;
  int ambientWas[MAX_SOUND_CHANNELS];
  for (i = 1; i < MAX_SOUND_CHANNELS; i++)
    ambientWas[i] = ambient[i].channel;

  if ((strlen(name) > 3) && (stricmp(&name[strlen(name) - 3], "ogv") == 0))
  {
    play_theora_video(name, skip, flags);
  }
  else
  {
    char videoFilePath[MAX_PATH];
    get_current_dir_path(videoFilePath, name);

    platform->PlayVideo(videoFilePath, skip, flags);
  }

  if (flags < 10) 
  {
    update_music_volume();
    // restart the music
    if (musplaying >= 0)
      newmusic (musplaying);
    for (i = 1; i < MAX_SOUND_CHANNELS; i++) {
      if (ambientWas[i] > 0)
        PlayAmbientSound(ambientWas[i], ambient[i].num, ambient[i].vol, ambient[i].x, ambient[i].y);
    }
  }
}

/* *** SCRIPT SYMBOL: [Multimedia] PlayVideo *** */
void scrPlayVideo(const char* name, int skip, int flags) {
  EndSkippingUntilCharStops();

  if (play.fast_forward)
    return;
  if (debug_flags & DBG_NOVIDEO)
    return;

  if ((flags < 10) && (usetup.digicard == DIGI_NONE)) {
    // if game audio is disabled in Setup, then don't
    // play any sound on the video either
    flags += 10;
  }

  pause_sound_if_necessary_and_play_video(name, skip, flags);
}

#else
/* *** SCRIPT SYMBOL: [Multimedia] PlayVideo *** */
void scrPlayVideo(const char* name, int skip, int flags) {}
#endif



void stopmusic() {

  if (crossFading > 0) {
    // stop in the middle of a new track fading in
    // Abort the new track, and let the old one finish fading out
    stop_and_destroy_channel (crossFading);
    crossFading = -1;
  }
  else if (crossFading < 0) {
    // the music is already fading out
    if (game.options[OPT_CROSSFADEMUSIC] <= 0) {
      // If they have since disabled crossfading, stop the fadeout
      stop_and_destroy_channel(SCHAN_MUSIC);
      crossFading = 0;
      crossFadeStep = 0;
      update_music_volume();
    }
  }
  else if ((game.options[OPT_CROSSFADEMUSIC] > 0)
      && (channels[SCHAN_MUSIC] != NULL)
      && (channels[SCHAN_MUSIC]->done == 0)
      && (current_music_type != 0)
      && (current_music_type != MUS_MIDI)
      && (current_music_type != MUS_MOD)) {

    crossFading = -1;
    crossFadeStep = 0;
    crossFadeVolumePerStep = game.options[OPT_CROSSFADEMUSIC];
    crossFadeVolumeAtStart = calculate_max_volume();
  }
  else
    stop_and_destroy_channel (SCHAN_MUSIC);

  play.cur_music_number = -1;
  current_music_type = 0;
}

/* *** SCRIPT SYMBOL: [Multimedia] StopMusic *** */
void scr_StopMusic() {
  play.music_queue_size = 0;
  stopmusic();
}


/* *** SCRIPT SYMBOL: [Multimedia] SetSoundVolume *** */
void SetSoundVolume(int newvol) {
  if ((newvol<0) | (newvol>255))
    quit("!SetSoundVolume: invalid volume - must be from 0-255");
  play.sound_volume = newvol;
  Game_SetAudioTypeVolume(AUDIOTYPE_LEGACY_AMBIENT_SOUND, (newvol * 100) / 255, VOL_BOTH);
  Game_SetAudioTypeVolume(AUDIOTYPE_LEGACY_SOUND, (newvol * 100) / 255, VOL_BOTH);
  update_ambient_sound_vol ();
}

/* *** SCRIPT SYMBOL: [Multimedia] PlaySilentMIDI *** */
void PlaySilentMIDI (int mnum) {
  if (current_music_type == MUS_MIDI)
    quit("!PlaySilentMIDI: proper midi music is in progress");

  set_volume (-1, 0);
  play.silent_midi = mnum;
  play.silent_midi_channel = SCHAN_SPEECH;
  stop_and_destroy_channel(play.silent_midi_channel);
  channels[play.silent_midi_channel] = load_sound_clip_from_old_style_number(true, mnum, false);
  if (channels[play.silent_midi_channel] == NULL)
  {
    quitprintf("!PlaySilentMIDI: failed to load aMusic%d", mnum);
  }
  channels[play.silent_midi_channel]->play();
  channels[play.silent_midi_channel]->set_volume(0);
  channels[play.silent_midi_channel]->volAsPercentage = 0;
}


int init_cd_player() 
{
  use_cdplayer=0;
  return platform->InitializeCDPlayer();
}

/* *** SCRIPT SYMBOL: [Multimedia] CDAudio *** */
int cd_manager(int cmdd,int datt) 
{
  if (!triedToUseCdAudioCommand)
  {
    triedToUseCdAudioCommand = true;
    init_cd_player();
  }
  if (cmdd==0) return use_cdplayer;
  if (use_cdplayer==0) return 0;  // ignore other commands

  return platform->CDPlayerCommand(cmdd, datt);
}




/* PASTE:
  register_multimedia_script_functions();
*/
void register_multimedia_script_functions() {
  scAdd_External_Symbol("IsSoundPlaying",(void *)IsSoundPlaying);
  scAdd_External_Symbol("IsMusicPlaying",(void *)IsMusicPlaying);
  scAdd_External_Symbol("SetSpeechVolume",(void *)SetSpeechVolume);
  scAdd_External_Symbol("SetVoiceMode",(void *)SetVoiceMode);
  scAdd_External_Symbol("IsVoxAvailable",(void *)IsVoxAvailable);
  scAdd_External_Symbol("PlayFlic",(void *)play_flc_file);
  scAdd_External_Symbol("PlayVideo",(void *)scrPlayVideo);
  scAdd_External_Symbol("StopMusic", (void *)scr_StopMusic);
  scAdd_External_Symbol("SetSoundVolume",(void *)SetSoundVolume);
  scAdd_External_Symbol("PlaySilentMIDI",(void *)PlaySilentMIDI);
  scAdd_External_Symbol("CDAudio",(void *)cd_manager);


}

