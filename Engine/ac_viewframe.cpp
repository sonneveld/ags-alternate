#include "ac_viewframe.h"

#include "allegro_wrapper.h"

#include "ac.h"
#include "ac_context.h"
#include "dynobj/script_view_frame.h"


/* *** SCRIPT SYMBOL: [ViewFrame] SetFrameSound *** */
void SetFrameSound (int vii, int loop, int frame, int sound) {
  if ((vii < 1) || (vii > game.numviews))
    quit("!SetFrameSound: invalid view number");
  vii--;

  if (loop >= views[vii].numLoops)
    quit("!SetFrameSound: invalid loop number");

  if (frame >= views[vii].loops[loop].numFrames)
    quit("!SetFrameSound: invalid frame number");

  if (sound < 1)
  {
    views[vii].loops[loop].frames[frame].sound = -1;
  }
  else
  {
    ScriptAudioClip* clip = get_audio_clip_for_old_style_number(false, sound);
    if (clip == NULL)
      quitprintf("!SetFrameSound: audio clip aSound%d not found", sound);

    views[vii].loops[loop].frames[frame].sound = clip->id;
  }
}

// the specified frame has just appeared, see if we need
// to play a sound or whatever
void CheckViewFrame (int view, int loop, int frame) {
  if (views[view].loops[loop].frames[frame].sound >= 0) {
    // play this sound (eg. footstep)
    play_audio_clip_by_index(views[view].loops[loop].frames[frame].sound);
  }
}

void CheckViewFrameForCharacter(CharacterInfo *chi) {

  int soundVolumeWas = play.sound_volume;

  if (chi->flags & CHF_SCALEVOLUME) {
    // adjust the sound volume using the character's zoom level
    int zoom_level = charextra[chi->index_id].zoom;
    if (zoom_level == 0)
      zoom_level = 100;

    play.sound_volume = (play.sound_volume * zoom_level) / 100;

    if (play.sound_volume < 0)
      play.sound_volume = 0;
    if (play.sound_volume > 255)
      play.sound_volume = 255;
  }

  CheckViewFrame(chi->view, chi->loop, chi->frame);

  play.sound_volume = soundVolumeWas;
}



/* *** SCRIPT SYMBOL: [ViewFrame] ViewFrame::get_Flipped *** */
int ViewFrame_GetFlipped(ScriptViewFrame *svf) {
  if (views[svf->view].loops[svf->loop].frames[svf->frame].flags & VFLG_FLIPSPRITE)
    return 1;
  return 0;
}

/* *** SCRIPT SYMBOL: [ViewFrame] ViewFrame::get_Graphic *** */
int ViewFrame_GetGraphic(ScriptViewFrame *svf) {
  return views[svf->view].loops[svf->loop].frames[svf->frame].pic;
}

/* *** SCRIPT SYMBOL: [ViewFrame] ViewFrame::set_Graphic *** */
void ViewFrame_SetGraphic(ScriptViewFrame *svf, int newPic) {
  views[svf->view].loops[svf->loop].frames[svf->frame].pic = newPic;
}

/* *** SCRIPT SYMBOL: [ViewFrame] ViewFrame::get_LinkedAudio *** */
ScriptAudioClip* ViewFrame_GetLinkedAudio(ScriptViewFrame *svf) 
{
  int soundIndex = views[svf->view].loops[svf->loop].frames[svf->frame].sound;
  if (soundIndex < 0)
    return NULL;

  return &game.audioClips[soundIndex];
}

/* *** SCRIPT SYMBOL: [ViewFrame] ViewFrame::set_LinkedAudio *** */
void ViewFrame_SetLinkedAudio(ScriptViewFrame *svf, ScriptAudioClip* clip) 
{
  int newSoundIndex = -1;
  if (clip != NULL)
    newSoundIndex = clip->id;

  views[svf->view].loops[svf->loop].frames[svf->frame].sound = newSoundIndex;
}

/* *** SCRIPT SYMBOL: [ViewFrame] ViewFrame::get_Sound *** */
int ViewFrame_GetSound(ScriptViewFrame *svf) {
  // convert audio clip to old-style sound number
  int soundIndex = views[svf->view].loops[svf->loop].frames[svf->frame].sound;
  if (soundIndex >= 0)
  {
    if (sscanf(game.audioClips[soundIndex].scriptName, "aSound%d", &soundIndex) == 1)
      return soundIndex;
  }
  return 0;
}

/* *** SCRIPT SYMBOL: [ViewFrame] ViewFrame::set_Sound *** */
void ViewFrame_SetSound(ScriptViewFrame *svf, int newSound) 
{
  if (newSound < 1)
  {
    views[svf->view].loops[svf->loop].frames[svf->frame].sound = -1;
  }
  else
  {
    // convert sound number to audio clip
    ScriptAudioClip* clip = get_audio_clip_for_old_style_number(false, newSound);
    if (clip == NULL)
      quitprintf("!SetFrameSound: audio clip aSound%d not found", newSound);

    views[svf->view].loops[svf->loop].frames[svf->frame].sound = clip->id;
  }
}

/* *** SCRIPT SYMBOL: [ViewFrame] ViewFrame::get_Speed *** */
int ViewFrame_GetSpeed(ScriptViewFrame *svf) {
  return views[svf->view].loops[svf->loop].frames[svf->frame].speed;
}

/* *** SCRIPT SYMBOL: [ViewFrame] ViewFrame::get_View *** */
int ViewFrame_GetView(ScriptViewFrame *svf) {
  return svf->view + 1;
}

/* *** SCRIPT SYMBOL: [ViewFrame] ViewFrame::get_Loop *** */
int ViewFrame_GetLoop(ScriptViewFrame *svf) {
  return svf->loop;
}

/* *** SCRIPT SYMBOL: [ViewFrame] ViewFrame::get_Frame *** */
int ViewFrame_GetFrame(ScriptViewFrame *svf) {
  return svf->frame;
}




/* PASTE:
*/
void register_view_frame_script_functions() {
  scAdd_External_Symbol("ViewFrame::get_Flipped", (void *)ViewFrame_GetFlipped);
  scAdd_External_Symbol("ViewFrame::get_Frame", (void *)ViewFrame_GetFrame);
  scAdd_External_Symbol("ViewFrame::get_Graphic", (void *)ViewFrame_GetGraphic);
  scAdd_External_Symbol("ViewFrame::set_Graphic", (void *)ViewFrame_SetGraphic);
  scAdd_External_Symbol("ViewFrame::get_LinkedAudio", (void *)ViewFrame_GetLinkedAudio);
  scAdd_External_Symbol("ViewFrame::set_LinkedAudio", (void *)ViewFrame_SetLinkedAudio);
  scAdd_External_Symbol("ViewFrame::get_Loop", (void *)ViewFrame_GetLoop);
  scAdd_External_Symbol("ViewFrame::get_Sound", (void *)ViewFrame_GetSound);
  scAdd_External_Symbol("ViewFrame::set_Sound", (void *)ViewFrame_SetSound);
  scAdd_External_Symbol("ViewFrame::get_Speed", (void *)ViewFrame_GetSpeed);
  scAdd_External_Symbol("ViewFrame::get_View", (void *)ViewFrame_GetView);
  scAdd_External_Symbol("SetFrameSound",(void *)SetFrameSound);
}

