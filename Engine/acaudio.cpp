/*
  Adventure Game Studio source code Copyright 1999-2011 Chris Jones.
  All rights reserved.

  The AGS Editor Source Code is provided under the Artistic License 2.0
  http://www.opensource.org/licenses/artistic-license-2.0.php

  You MAY NOT compile your own builds of the engine without making it EXPLICITLY
  CLEAR that the code has been altered from the Standard Version.

*/
#include "allegro_wrapper.h"

#include "acsound.h"
#include <math.h>
#include "ac.h"
#include "ac_context.h"
#include "wgt2allg.h"
#include "acroom.h"
#include "acruntim.h"
#include "clib32.h"
#include "ac_multimedia.h"
#include "ac_file.h"
#include "dynobj/cc_audio_channel.h"
#include "dynobj/cc_audio_clip.h"
#include "cscomp.h"

ScriptAudioChannel scrAudioChannel[MAX_SOUND_CHANNELS + 1];
CCAudioChannel ccDynamicAudio;
CCAudioClip ccDynamicAudioClip;
char acaudio_buffer[256];
int reserved_channel_count = 0;

// forward defs:
SOUNDCLIP *load_music_from_disk(int mnum, bool doRepeat);
void play_new_music(int mnum, SOUNDCLIP *music);

void acaudio_update_mp3() {
  while (switching_away_from_game) 
    { }
  for (int i = 0; i <= MAX_SOUND_CHANNELS; i++) { 
    if ((channels[i] != NULL) && (channels[i]->done == 0)) 
      channels[i]->poll();
  }
}


bool AmbientSound::IsPlaying () {
  if (channel <= 0)
    return false;
  return (channels[channel] != NULL) ? true : false;
}


void calculate_reserved_channel_count()
{
  int reservedChannels = 0;
  for (int i = 0; i < game.audioClipTypeCount; i++)
  {
    reservedChannels += game.audioClipTypes[i].reservedChannels;
  }
  reserved_channel_count = reservedChannels;
}

void register_audio_script_objects()
{
  int ee;
  for (ee = 0; ee <= MAX_SOUND_CHANNELS; ee++) 
  {
    scrAudioChannel[ee].id = ee;
    ccRegisterManagedObject(&scrAudioChannel[ee], &ccDynamicAudio);
  }

  for (ee = 0; ee < game.audioClipCount; ee++)
  {
    game.audioClips[ee].id = ee;
    ccRegisterManagedObject(&game.audioClips[ee], &ccDynamicAudioClip);
    scAdd_External_Symbol(game.audioClips[ee].scriptName, &game.audioClips[ee]);
  }

  calculate_reserved_channel_count();
}

/* *** SCRIPT SYMBOL: [AudioChannel] AudioChannel::get_ID *** */
int AudioChannel_GetID(ScriptAudioChannel *channel)
{
  return channel->id;
}

/* *** SCRIPT SYMBOL: [AudioChannel] AudioChannel::get_IsPlaying *** */
int AudioChannel_GetIsPlaying(ScriptAudioChannel *channel)
{
  if (play.fast_forward)
  {
    return 0;
  }

  if ((channels[channel->id] != NULL) &&
      (channels[channel->id]->done == 0))
  {
    return 1;
  }
  return 0;
}

/* *** SCRIPT SYMBOL: [AudioChannel] AudioChannel::get_Panning *** */
int AudioChannel_GetPanning(ScriptAudioChannel *channel)
{
  if ((channels[channel->id] != NULL) &&
      (channels[channel->id]->done == 0))
  {
    return channels[channel->id]->panningAsPercentage;
  }
  return 0;
}

/* *** SCRIPT SYMBOL: [AudioChannel] AudioChannel::set_Panning *** */
void AudioChannel_SetPanning(ScriptAudioChannel *channel, int newPanning)
{
  if ((newPanning < -100) || (newPanning > 100))
    quitprintf("!AudioChannel.Panning: panning value must be between -100 and 100 (passed=%d)", newPanning);
  
  if ((channels[channel->id] != NULL) &&
      (channels[channel->id]->done == 0))
  {
    channels[channel->id]->set_panning(((newPanning + 100) * 255) / 200);
    channels[channel->id]->panningAsPercentage = newPanning;
  }
}

/* *** SCRIPT SYMBOL: [AudioChannel] AudioChannel::get_PlayingClip *** */
ScriptAudioClip* AudioChannel_GetPlayingClip(ScriptAudioChannel *channel)
{
  if ((channels[channel->id] != NULL) &&
      (channels[channel->id]->done == 0))
  {
    return (ScriptAudioClip*)channels[channel->id]->sourceClip;
  }
  return NULL;
}

/* *** SCRIPT SYMBOL: [AudioChannel] AudioChannel::get_Position *** */
int AudioChannel_GetPosition(ScriptAudioChannel *channel)
{
  if ((channels[channel->id] != NULL) &&
      (channels[channel->id]->done == 0))
  {
    if (play.fast_forward)
      return 999999999;

    return channels[channel->id]->get_pos();
  }
  return 0;
}

/* *** SCRIPT SYMBOL: [AudioChannel] AudioChannel::get_PositionMs *** */
int AudioChannel_GetPositionMs(ScriptAudioChannel *channel)
{
  if ((channels[channel->id] != NULL) &&
      (channels[channel->id]->done == 0))
  {
    if (play.fast_forward)
      return 999999999;

    return channels[channel->id]->get_pos_ms();
  }
  return 0;
}

/* *** SCRIPT SYMBOL: [AudioChannel] AudioChannel::get_LengthMs *** */
int AudioChannel_GetLengthMs(ScriptAudioChannel *channel)
{
  if ((channels[channel->id] != NULL) &&
      (channels[channel->id]->done == 0))
  {
    return channels[channel->id]->get_length_ms();
  }
  return 0;
}

/* *** SCRIPT SYMBOL: [AudioChannel] AudioChannel::get_Volume *** */
int AudioChannel_GetVolume(ScriptAudioChannel *channel)
{
  if ((channels[channel->id] != NULL) &&
      (channels[channel->id]->done == 0))
  {
    return channels[channel->id]->volAsPercentage;
  }
  return 0;
}

/* *** SCRIPT SYMBOL: [AudioChannel] AudioChannel::set_Volume *** */
int AudioChannel_SetVolume(ScriptAudioChannel *channel, int newVolume)
{
  if ((newVolume < 0) || (newVolume > 100))
    quitprintf("!AudioChannel.Volume: new value out of range (supplied: %d, range: 0..100)", newVolume);

  if ((channels[channel->id] != NULL) &&
      (channels[channel->id]->done == 0))
  {
    channels[channel->id]->set_volume((newVolume * 255) / 100);
    channels[channel->id]->volAsPercentage = newVolume;
  }
  return 0;
}

void update_clip_default_volume(ScriptAudioClip *audioClip)
{
  if (play.default_audio_type_volumes[audioClip->type] >= 0) 
  {
    audioClip->defaultVolume = play.default_audio_type_volumes[audioClip->type];
  }
}

void start_fading_in_new_track_if_applicable(int fadeInChannel, ScriptAudioClip *newSound)
{
  int crossfadeSpeed = game.audioClipTypes[newSound->type].crossfadeSpeed;
  if (crossfadeSpeed > 0)
  {
    update_clip_default_volume(newSound);
    play.crossfade_in_volume_per_step = crossfadeSpeed;
    play.crossfade_final_volume_in = newSound->defaultVolume;
    play.crossfading_in_channel = fadeInChannel;
  }
}

void move_track_to_crossfade_channel(int currentChannel, int crossfadeSpeed, int fadeInChannel, ScriptAudioClip *newSound)
{
  stop_and_destroy_channel(SPECIAL_CROSSFADE_CHANNEL);
  channels[SPECIAL_CROSSFADE_CHANNEL] = channels[currentChannel];
  channels[currentChannel] = NULL;

  play.crossfading_out_channel = SPECIAL_CROSSFADE_CHANNEL;
  play.crossfade_step = 0;
  play.crossfade_initial_volume_out = channels[SPECIAL_CROSSFADE_CHANNEL]->volAsPercentage;
  play.crossfade_out_volume_per_step = crossfadeSpeed;

  play.crossfading_in_channel = fadeInChannel;
  if (newSound != NULL)
  {
    start_fading_in_new_track_if_applicable(fadeInChannel, newSound);
  }
}

void stop_or_fade_out_channel(int fadeOutChannel, int fadeInChannel = -1, ScriptAudioClip *newSound = NULL)
{
  ScriptAudioClip *sourceClip = AudioChannel_GetPlayingClip(&scrAudioChannel[fadeOutChannel]);
  if ((sourceClip != NULL) && (game.audioClipTypes[sourceClip->type].crossfadeSpeed > 0))
  {
    move_track_to_crossfade_channel(fadeOutChannel, game.audioClipTypes[sourceClip->type].crossfadeSpeed, fadeInChannel, newSound);
  }
  else
  {
    stop_and_destroy_channel(fadeOutChannel);
  }
}

/* *** SCRIPT SYMBOL: [AudioChannel] AudioChannel::Stop^0 *** */
void AudioChannel_Stop(ScriptAudioChannel *channel)
{
  stop_or_fade_out_channel(channel->id, -1, NULL);
}

/* *** SCRIPT SYMBOL: [AudioChannel] AudioChannel::Seek^1 *** */
void AudioChannel_Seek(ScriptAudioChannel *channel, int newPosition)
{
  if (newPosition < 0)
    quitprintf("!AudioChannel.Seek: invalid seek position %d", newPosition);

  if ((channels[channel->id] != NULL) &&
      (channels[channel->id]->done == 0))
  {
    channels[channel->id]->seek(newPosition);
  }
}

/* *** SCRIPT SYMBOL: [AudioChannel] AudioChannel::SetRoomLocation^2 *** */
void AudioChannel_SetRoomLocation(ScriptAudioChannel *channel, int xPos, int yPos)
{
  if ((channels[channel->id] != NULL) &&
      (channels[channel->id]->done == 0))
  {
    int maxDist = ((xPos > thisroom.width / 2) ? xPos : (thisroom.width - xPos)) - AMBIENCE_FULL_DIST;
    channels[channel->id]->xSource = (xPos > 0) ? xPos : -1;
    channels[channel->id]->ySource = yPos;
    channels[channel->id]->maximumPossibleDistanceAway = maxDist;
    if (xPos > 0)
    {
      update_directional_sound_vol();
    }
    else
    {
      channels[channel->id]->directionalVolModifier = 0;
      channels[channel->id]->set_volume(channels[channel->id]->vol);
    }
  }
}

/* *** SCRIPT SYMBOL: [AudioClip] AudioClip::get_FileType *** */
int AudioClip_GetFileType(ScriptAudioClip *clip)
{
  return game.audioClips[clip->id].fileType;
}

/* *** SCRIPT SYMBOL: [AudioClip] AudioClip::get_Type *** */
int AudioClip_GetType(ScriptAudioClip *clip)
{
  return game.audioClips[clip->id].type;
}

const char* get_audio_clip_file_name(ScriptAudioClip *clip)
{
  if (game.audioClips[clip->id].bundlingType == AUCL_BUNDLE_EXE)
  {
    strcpy(acaudio_buffer, game.audioClips[clip->id].fileName);
    FILE *iii = clibfopen(acaudio_buffer, "rb");
    if (iii != NULL)
    {
      fclose(iii);
      return &acaudio_buffer[0];
    }
  }
  else
  {
    sprintf(acaudio_buffer, "~audio.vox~%s", game.audioClips[clip->id].fileName);
    PACKFILE *iii = alw_pack_fopen(acaudio_buffer, "rb");
    if (iii != NULL)
    {
      alw_pack_fclose(iii);
      return &acaudio_buffer[0];
    }
  }
  sprintf(acaudio_buffer, "AudioCache\\%s", game.audioClips[clip->id].fileName);
  if (alw_exists(acaudio_buffer))
  {
    return &acaudio_buffer[0];
  }
  return NULL;
}

/* *** SCRIPT SYMBOL: [AudioClip] AudioClip::get_IsAvailable *** */
int AudioClip_GetIsAvailable(ScriptAudioClip *clip)
{
  if (get_audio_clip_file_name(clip) != NULL)
    return 1;

  return 0;
}

int find_free_audio_channel(ScriptAudioClip *clip, int priority, bool interruptEqualPriority)
{
  int lowestPrioritySoFar = 9999999;
  int lowestPriorityID = -1;
  int channelToUse = -1;

  if (!interruptEqualPriority)
    priority--;

  int startAtChannel = reserved_channel_count;
  int endBeforeChannel = MAX_SOUND_CHANNELS;

  if (game.audioClipTypes[clip->type].reservedChannels > 0)
  {
    startAtChannel = 0;
    for (int i = 0; i < clip->type; i++)
    {
      startAtChannel += game.audioClipTypes[i].reservedChannels;
    }
    endBeforeChannel = startAtChannel + game.audioClipTypes[clip->type].reservedChannels;
  }

  for (int i = startAtChannel; i < endBeforeChannel; i++)
  {
    if ((channels[i] == NULL) || (channels[i]->done))
    {
      channelToUse = i;
      stop_and_destroy_channel(i);
      break;
    }
    if ((channels[i]->priority < lowestPrioritySoFar) &&
        (channels[i]->soundType == clip->type))
    {
      lowestPrioritySoFar = channels[i]->priority;
      lowestPriorityID = i;
    }
  }

  if ((channelToUse < 0) && (lowestPriorityID >= 0) &&
      (lowestPrioritySoFar <= priority))
  {
    stop_or_fade_out_channel(lowestPriorityID, lowestPriorityID, clip);
    channelToUse = lowestPriorityID;
  }
  else if ((channelToUse >= 0) && (play.crossfading_in_channel < 1))
  {
    start_fading_in_new_track_if_applicable(channelToUse, clip);
  }
  return channelToUse;
}

SOUNDCLIP *load_sound_clip(ScriptAudioClip *audioClip, bool repeat)
{
  const char *clipFileName = get_audio_clip_file_name(audioClip);
  if ((clipFileName == NULL) || (usetup.digicard == DIGI_NONE))
  {
    return NULL;
  }

  update_clip_default_volume(audioClip);

  SOUNDCLIP *soundClip = NULL;
  switch (audioClip->fileType)
  {
  case eAudioFileOGG:
    soundClip = my_load_static_ogg(clipFileName, audioClip->defaultVolume, repeat);
    break;
  case eAudioFileMP3:
    soundClip = my_load_static_mp3(clipFileName, audioClip->defaultVolume, repeat);
    break;
  case eAudioFileWAV:
  case eAudioFileVOC:
    soundClip = my_load_wave(clipFileName, audioClip->defaultVolume, repeat);
    break;
  case eAudioFileMIDI:
    soundClip = my_load_midi(clipFileName, repeat);
    break;
  case eAudioFileMOD:
    soundClip = my_load_mod(clipFileName, repeat);
    break;
  default:
    quitprintf("AudioClip.Play: invalid audio file type encountered: %d", audioClip->fileType);
  }
  if (soundClip != NULL)
  {
    soundClip->volAsPercentage = audioClip->defaultVolume;
    soundClip->originalVolAsPercentage = soundClip->volAsPercentage;
    soundClip->set_volume((audioClip->defaultVolume * 255) / 100);
    soundClip->soundType = audioClip->type;
    soundClip->sourceClip = audioClip;
  }
  return soundClip;
}

/* *** SCRIPT SYMBOL: [AudioClip] AudioClip::Stop^0 *** */
void AudioClip_Stop(ScriptAudioClip *clip)
{
  for (int i = 0; i < MAX_SOUND_CHANNELS; i++)
  {
    if ((channels[i] != NULL) && (!channels[i]->done) && (channels[i]->sourceClip == clip))
    {
      AudioChannel_Stop(&scrAudioChannel[i]);
    }
  }
}

void recache_queued_clips_after_loading_save_game()
{
  for (int i = 0; i < play.new_music_queue_size; i++)
  {
    play.new_music_queue[i].cachedClip = NULL;
  }
}

void audio_update_polled_stuff()
{
  play.crossfade_step++;

  if (play.crossfading_out_channel > 0)
  {
    if (channels[play.crossfading_out_channel] == NULL)
      quitprintf("Crossfade out channel is %d but channel has gone", play.crossfading_out_channel);

    int newVolume = channels[play.crossfading_out_channel]->volAsPercentage - play.crossfade_out_volume_per_step;
    if (newVolume > 0)
    {
      AudioChannel_SetVolume(&scrAudioChannel[play.crossfading_out_channel], newVolume);
    }
    else
    {
      stop_and_destroy_channel(play.crossfading_out_channel);
      play.crossfading_out_channel = 0;
    }
  }

  if (play.crossfading_in_channel > 0)
  {
    int newVolume = channels[play.crossfading_in_channel]->volAsPercentage + play.crossfade_in_volume_per_step;
    if (newVolume > play.crossfade_final_volume_in)
    {
      newVolume = play.crossfade_final_volume_in;
    }

    AudioChannel_SetVolume(&scrAudioChannel[play.crossfading_in_channel], newVolume);

    if (newVolume >= play.crossfade_final_volume_in)
    {
      play.crossfading_in_channel = 0;
    }
  }

  if (play.new_music_queue_size > 0)
  {
    for (int i = 0; i < play.new_music_queue_size; i++)
    {
      ScriptAudioClip *clip = &game.audioClips[play.new_music_queue[i].audioClipIndex];
      int channel = find_free_audio_channel(clip, clip->defaultPriority, false);
      if (channel >= 0)
      {
        QueuedAudioItem itemToPlay = play.new_music_queue[i];

        play.new_music_queue_size--;
        for (int j = i; j < play.new_music_queue_size; j++)
        {
          play.new_music_queue[j] = play.new_music_queue[j + 1];
        }

        play_audio_clip_on_channel(channel, clip, itemToPlay.priority, itemToPlay.repeat, 0, itemToPlay.cachedClip);
        i--;
      }
    }
  }
}

void queue_audio_clip_to_play(ScriptAudioClip *clip, int priority, int repeat)
{
  if (play.new_music_queue_size >= MAX_QUEUED_MUSIC) {
    DEBUG_CONSOLE("Too many queued music, cannot add %s", clip->scriptName);
    return;
  }

  SOUNDCLIP *cachedClip = load_sound_clip(clip, (repeat != 0));
  if (cachedClip != NULL) 
  {
    play.new_music_queue[play.new_music_queue_size].audioClipIndex = clip->id;
    play.new_music_queue[play.new_music_queue_size].priority = priority;
    play.new_music_queue[play.new_music_queue_size].repeat = (repeat != 0);
    play.new_music_queue[play.new_music_queue_size].cachedClip = cachedClip;
    play.new_music_queue_size++;
  }
  update_polled_stuff(false);
}

ScriptAudioChannel* play_audio_clip_on_channel(int channel, ScriptAudioClip *clip, int priority, int repeat, int fromOffset, SOUNDCLIP *soundfx)
{
  if (soundfx == NULL)
  {
    soundfx = load_sound_clip(clip, (repeat) ? true : false);
  }
  if (soundfx == NULL)
  {
    DEBUG_CONSOLE("AudioClip.Play: unable to load sound file");
    if (play.crossfading_in_channel == channel)
    {
      play.crossfading_in_channel = 0;
    }
    return NULL;
  }
  soundfx->priority = priority;

  if (play.crossfading_in_channel == channel)
  {
    soundfx->set_volume(0);
    soundfx->volAsPercentage = 0;
  }

  if (play.fast_forward) 
  {
    soundfx->set_volume(0);
    soundfx->volAsPercentage = 0;

    if (game.audioClipTypes[clip->type].reservedChannels != 1)
      soundfx->originalVolAsPercentage = 0;
  }

  if (soundfx->play_from(fromOffset) == 0)
  {
    DEBUG_CONSOLE("AudioClip.Play: failed to play sound file");
    return NULL;
  }

  last_sound_played[channel] = -1;
  channels[channel] = soundfx;
  return &scrAudioChannel[channel];
}

void remove_clips_of_type_from_queue(int audioType) 
{
  int aa;
  for (aa = 0; aa < play.new_music_queue_size; aa++)
  {
    ScriptAudioClip *clip = &game.audioClips[play.new_music_queue[aa].audioClipIndex];
    if (clip->type == audioType)
    {
      play.new_music_queue_size--;
      for (int bb = aa; bb < play.new_music_queue_size; bb++)
        play.new_music_queue[bb] = play.new_music_queue[bb + 1];
      aa--;
    }
  }
}

ScriptAudioChannel* play_audio_clip(ScriptAudioClip *clip, int priority, int repeat, int fromOffset, bool queueIfNoChannel)
{
  if (!queueIfNoChannel)
    remove_clips_of_type_from_queue(clip->type);

  if (priority == SCR_NO_VALUE)
    priority = clip->defaultPriority;
  if (repeat == SCR_NO_VALUE)
    repeat = clip->defaultRepeat;

  int channel = find_free_audio_channel(clip, priority, !queueIfNoChannel);
  if (channel < 0)
  {
    if (queueIfNoChannel)
      queue_audio_clip_to_play(clip, priority, repeat);
    else
      DEBUG_CONSOLE("AudioClip.Play: no channels available to interrupt PRI:%d TYPE:%d", priority, clip->type);

    return NULL;
  }

  return play_audio_clip_on_channel(channel, clip, priority, repeat, fromOffset);
}

/* *** SCRIPT SYMBOL: [AudioClip] AudioClip::Play^2 *** */
ScriptAudioChannel* AudioClip_Play(ScriptAudioClip *clip, int priority, int repeat)
{
  return play_audio_clip(clip, priority, repeat, 0, false);
}

/* *** SCRIPT SYMBOL: [AudioClip] AudioClip::PlayFrom^3 *** */
ScriptAudioChannel* AudioClip_PlayFrom(ScriptAudioClip *clip, int position, int priority, int repeat)
{
  return play_audio_clip(clip, priority, repeat, position, false);
}

/* *** SCRIPT SYMBOL: [AudioClip] AudioClip::PlayQueued^2 *** */
ScriptAudioChannel* AudioClip_PlayQueued(ScriptAudioClip *clip, int priority, int repeat)
{
  return play_audio_clip(clip, priority, repeat, 0, true);
}

void play_audio_clip_by_index(int audioClipIndex)
{
  if ((audioClipIndex >= 0) && (audioClipIndex < game.audioClipCount))
    AudioClip_Play(&game.audioClips[audioClipIndex], SCR_NO_VALUE, SCR_NO_VALUE);
}

/* *** SCRIPT SYMBOL: [System] System::get_AudioChannelCount *** */
int System_GetAudioChannelCount()
{
  return MAX_SOUND_CHANNELS;
}

/* *** SCRIPT SYMBOL: [System] System::geti_AudioChannels *** */
ScriptAudioChannel* System_GetAudioChannels(int index)
{
  if ((index < 0) || (index >= MAX_SOUND_CHANNELS))
    quit("!System.AudioChannels: invalid sound channel index");

  return &scrAudioChannel[index];
}

/* *** SCRIPT SYMBOL: [Game] Game::StopAudio^1 *** */
void Game_StopAudio(int audioType)
{
  if (((audioType < 0) || (audioType >= game.audioClipTypeCount)) && (audioType != SCR_NO_VALUE))
    quitprintf("!Game.StopAudio: invalid audio type %d", audioType);
  int aa;

  for (aa = 0; aa < MAX_SOUND_CHANNELS; aa++)
  {
    if (audioType == SCR_NO_VALUE)
    {
      stop_or_fade_out_channel(aa);
    }
    else
    {
      ScriptAudioClip *clip = AudioChannel_GetPlayingClip(&scrAudioChannel[aa]);
      if ((clip != NULL) && (clip->type == audioType))
        stop_or_fade_out_channel(aa);
    }
  }

  remove_clips_of_type_from_queue(audioType);
}

/* *** SCRIPT SYMBOL: [Game] Game::IsAudioPlaying^1 *** */
int Game_IsAudioPlaying(int audioType)
{
  if (((audioType < 0) || (audioType >= game.audioClipTypeCount)) && (audioType != SCR_NO_VALUE))
    quitprintf("!Game.IsAudioPlaying: invalid audio type %d", audioType);

  if (play.fast_forward)
    return 0;

  for (int aa = 0; aa < MAX_SOUND_CHANNELS; aa++)
  {
    ScriptAudioClip *clip = AudioChannel_GetPlayingClip(&scrAudioChannel[aa]);
    if (clip != NULL) 
    {
      if ((clip->type == audioType) || (audioType == SCR_NO_VALUE))
      {
        return 1;
      }
    }
  }
  return 0;
}

/* *** SCRIPT SYMBOL: [Game] Game::SetAudioTypeSpeechVolumeDrop^2 *** */
void Game_SetAudioTypeSpeechVolumeDrop(int audioType, int volumeDrop) 
{
  if ((audioType < 0) || (audioType >= game.audioClipTypeCount))
    quit("!Game.SetAudioTypeVolume: invalid audio type");

  game.audioClipTypes[audioType].volume_reduction_while_speech_playing = volumeDrop;
}

/* *** SCRIPT SYMBOL: [Game] Game::SetAudioTypeVolume^3 *** */
void Game_SetAudioTypeVolume(int audioType, int volume, int changeType)
{
  if ((volume < 0) || (volume > 100))
    quitprintf("!Game.SetAudioTypeVolume: volume %d is not between 0..100", volume);
  if ((audioType < 0) || (audioType >= game.audioClipTypeCount))
    quit("!Game.SetAudioTypeVolume: invalid audio type");
  int aa;

  if ((changeType == VOL_CHANGEEXISTING) ||
      (changeType == VOL_BOTH))
  {
    for (aa = 0; aa < MAX_SOUND_CHANNELS; aa++)
    {
      ScriptAudioClip *clip = AudioChannel_GetPlayingClip(&scrAudioChannel[aa]);
      if ((clip != NULL) && (clip->type == audioType))
      {
        channels[aa]->set_volume((volume * 255) / 100);
        channels[aa]->volAsPercentage = volume;
      }
    }
  }

  if ((changeType == VOL_SETFUTUREDEFAULT) ||
      (changeType == VOL_BOTH))
  {
    play.default_audio_type_volumes[audioType] = volume;
  }

}


bool unserialize_audio_script_object(int index, const char *objectType, const char *serializedData, int dataSize)
{
  if (strcmp(objectType, "AudioChannel") == 0)
  {
    ccDynamicAudio.Unserialize(index, serializedData, dataSize);
  }
  else if (strcmp(objectType, "AudioClip") == 0)
  {
    ccDynamicAudioClip.Unserialize(index, serializedData, dataSize);
  }
  else
  {
    return false;
  }
  return true;
}


// ***** BACKWARDS COMPATIBILITY WITH OLD AUDIO SYSTEM ***** //

ScriptAudioClip* get_audio_clip_for_old_style_number(bool isMusic, int indexNumber)
{
  char audioClipName[200];
  if (isMusic)
    sprintf(audioClipName, "aMusic%d", indexNumber);
  else
    sprintf(audioClipName, "aSound%d", indexNumber);

  for (int bb = 0; bb < game.audioClipCount; bb++)
  {
    if (stricmp(game.audioClips[bb].scriptName, audioClipName) == 0)
    {
      return &game.audioClips[bb];
    }
  }

  return NULL;
}

SOUNDCLIP *load_sound_clip_from_old_style_number(bool isMusic, int indexNumber, bool repeat)
{
  ScriptAudioClip* audioClip = get_audio_clip_for_old_style_number(isMusic, indexNumber);

  if (audioClip != NULL)
  {
      return load_sound_clip(audioClip, repeat);
  }

  return NULL;
}



void force_audiostream_include() {
  // This should never happen, but the call is here to make it
  // link the audiostream libraries
  alw_stop_audio_stream(NULL);
}


AmbientSound ambient[MAX_SOUND_CHANNELS + 1];  // + 1 just for safety on array iterations

int get_volume_adjusted_for_distance(int volume, int sndX, int sndY, int sndMaxDist)
{
  int distx = playerchar->x - sndX;
  int disty = playerchar->y - sndY;
  // it uses Allegro's "fix" sqrt without the ::
  int dist = (int)::sqrt((double)(distx*distx + disty*disty));

  // if they're quite close, full volume
  int wantvol = volume;

  if (dist >= AMBIENCE_FULL_DIST)
  {
    // get the relative volume
    wantvol = ((dist - AMBIENCE_FULL_DIST) * volume) / sndMaxDist;
    // closer is louder
    wantvol = volume - wantvol;
  }

  return wantvol;
}

void update_directional_sound_vol()
{
  for (int chan = 1; chan < MAX_SOUND_CHANNELS; chan++) 
  {
    if ((channels[chan] != NULL) && (channels[chan]->done == 0) &&
        (channels[chan]->xSource >= 0)) 
    {
      channels[chan]->directionalVolModifier = 
        get_volume_adjusted_for_distance(channels[chan]->vol, 
                channels[chan]->xSource,
                channels[chan]->ySource,
                channels[chan]->maximumPossibleDistanceAway) -
        channels[chan]->vol;

      channels[chan]->set_volume(channels[chan]->vol);
    }
  }
}

void update_ambient_sound_vol () {

  for (int chan = 1; chan < MAX_SOUND_CHANNELS; chan++) {

    AmbientSound *thisSound = &ambient[chan];

    if (thisSound->channel == 0)
      continue;

    int sourceVolume = thisSound->vol;

    if ((channels[SCHAN_SPEECH] != NULL) && (channels[SCHAN_SPEECH]->done == 0)) {
      // Negative value means set exactly; positive means drop that amount
      if (play.speech_music_drop < 0)
        sourceVolume = -play.speech_music_drop;
      else
        sourceVolume -= play.speech_music_drop;

      if (sourceVolume < 0)
        sourceVolume = 0;
      if (sourceVolume > 255)
        sourceVolume = 255;
    }

    // Adjust ambient volume so it maxes out at overall sound volume
    int ambientvol = (sourceVolume * play.sound_volume) / 255;

    int wantvol;

    if ((thisSound->x == 0) && (thisSound->y == 0)) {
      wantvol = ambientvol;
    }
    else {
      wantvol = get_volume_adjusted_for_distance(ambientvol, thisSound->x, thisSound->y, thisSound->maxdist);
    }

    if (channels[thisSound->channel] == NULL)
      quit("Internal error: the ambient sound channel is enabled, but it has been destroyed");

    channels[thisSound->channel]->set_volume(wantvol);
  }
}

void stop_and_destroy_channel_ex(int chid, bool resetLegacyMusicSettings) {
  if ((chid < 0) || (chid > MAX_SOUND_CHANNELS))
    quit("!StopChannel: invalid channel ID");

  if (channels[chid] != NULL) {
    channels[chid]->destroy();
    delete channels[chid];
    channels[chid] = NULL;
  }

  if (play.crossfading_in_channel == chid)
    play.crossfading_in_channel = 0;
  if (play.crossfading_out_channel == chid)
    play.crossfading_out_channel = 0;
  
  // destroyed an ambient sound channel
  if (ambient[chid].channel > 0)
    ambient[chid].channel = 0;

  if ((chid == SCHAN_MUSIC) && (resetLegacyMusicSettings))
  {
    play.cur_music_number = -1;
    current_music_type = 0;
  }
}

/* *** SCRIPT SYMBOL: [AudioChannel] StopChannel *** */
void stop_and_destroy_channel (int chid) 
{
	stop_and_destroy_channel_ex(chid, true);
}

/* *** SCRIPT SYMBOL: [AudioClip] PlayMusic *** */
void PlayMusicResetQueue(int newmus) {
  play.music_queue_size = 0;
  newmusic(newmus);
}

/* *** SCRIPT SYMBOL: [AudioChannel] StopAmbientSound *** */
void StopAmbientSound (int channel) {
  if ((channel < 0) || (channel >= MAX_SOUND_CHANNELS))
    quit("!StopAmbientSound: invalid channel");

  if (ambient[channel].channel == 0)
    return;

  stop_and_destroy_channel(channel);
  ambient[channel].channel = 0;
}

SOUNDCLIP *load_sound_from_path(int soundNumber, int volume, bool repeat) 
{
  SOUNDCLIP *soundfx = load_sound_clip_from_old_style_number(false, soundNumber, repeat);

  if (soundfx != NULL) {
    if (soundfx->play() == 0)
      soundfx = NULL;
  }

  return soundfx;
}

/* *** SCRIPT SYMBOL: [AudioClip] PlayAmbientSound *** */
void PlayAmbientSound (int channel, int sndnum, int vol, int x, int y) {
  // the channel parameter is to allow multiple ambient sounds in future
  if ((channel < 1) || (channel == SCHAN_SPEECH) || (channel >= MAX_SOUND_CHANNELS))
    quit("!PlayAmbientSound: invalid channel number");
  if ((vol < 1) || (vol > 255))
    quit("!PlayAmbientSound: volume must be 1 to 255");

  if (usetup.digicard == DIGI_NONE)
    return;

  // only play the sound if it's not already playing
  if ((ambient[channel].channel < 1) || (channels[ambient[channel].channel] == NULL) ||
      (channels[ambient[channel].channel]->done == 1) ||
      (ambient[channel].num != sndnum)) {

    StopAmbientSound(channel);
    // in case a normal non-ambient sound was playing, stop it too
    stop_and_destroy_channel(channel);

    SOUNDCLIP *asound = load_sound_from_path(sndnum, vol, true);

    if (asound == NULL) {
      debug_log ("Cannot load ambient sound %d", sndnum);
      DEBUG_CONSOLE("FAILED to load ambient sound %d", sndnum);
      return;
    }

    DEBUG_CONSOLE("Playing ambient sound %d on channel %d", sndnum, channel);
    ambient[channel].channel = channel;
    channels[channel] = asound;
    channels[channel]->priority = 15;  // ambient sound higher priority than normal sfx
  }
  // calculate the maximum distance away the player can be, using X
  // only (since X centred is still more-or-less total Y)
  ambient[channel].maxdist = ((x > thisroom.width / 2) ? x : (thisroom.width - x)) - AMBIENCE_FULL_DIST;
  ambient[channel].num = sndnum;
  ambient[channel].x = x;
  ambient[channel].y = y;
  ambient[channel].vol = vol;
  update_ambient_sound_vol();
}

/*
#include "almp3_old.h"
ALLEGRO_MP3 *mp3ptr;
int mp3vol=128;

void amp_setvolume(int newvol) { mp3vol=newvol; }
int load_amp(char*namm,int loop) {
  mp3ptr = new ALLEGRO_MP3(namm);
  if (mp3ptr == NULL) return 0;
  if (mp3ptr->get_error_code() != 0) {
    delete mp3ptr;
    return 0;
    }
  mp3ptr->play(mp3vol, 8192);
  return 1;
  }
void install_amp() { }
void unload_amp() {
  mp3ptr->stop();
  delete mp3ptr;
  }
int amp_decode() {
  mp3ptr->poll();
  if (mp3ptr->is_finished()) {
    if (play.music_repeat)
      mp3ptr->play(mp3vol, 8192);
    else return -1;
    }
  return 0;
  }
*/
//#endif



/* *** SCRIPT SYMBOL: [AudioChannel] IsChannelPlaying *** */
int IsChannelPlaying(int chan) {
  if (play.fast_forward)
    return 0;

  if ((chan < 0) || (chan >= MAX_SOUND_CHANNELS))
    quit("!IsChannelPlaying: invalid sound channel");

  if ((channels[chan] != NULL) && (channels[chan]->done == 0))
    return 1;

  return 0;
}


/* *** SCRIPT SYMBOL: [AudioChannel] SeekMIDIPosition *** */
void SeekMIDIPosition (int position) {
  if (play.silent_midi)
    alw_midi_seek (position);
  if (current_music_type == MUS_MIDI) {
    alw_midi_seek(position);
    DEBUG_CONSOLE("Seek MIDI position to %d", position);
  }
}

/* *** SCRIPT SYMBOL: [AudioChannel] GetMIDIPosition *** */
int GetMIDIPosition () {
  if (play.silent_midi)
    return alw_midi_pos;
  if (current_music_type != MUS_MIDI)
    return -1;
  if (play.fast_forward)
    return 99999;

  return alw_midi_pos;
}




void clear_music_cache() {

  if (cachedQueuedMusic != NULL) {
    cachedQueuedMusic->destroy();
    delete cachedQueuedMusic;
    cachedQueuedMusic = NULL;
  }

}

/* *** SCRIPT SYMBOL: [AudioClip] PlayMusicQueued *** */
int PlayMusicQueued(int musnum) {

  // Just get the queue size
  if (musnum < 0)
    return play.music_queue_size;

  if ((IsMusicPlaying() == 0) && (play.music_queue_size == 0)) {
    newmusic(musnum);
    return 0;
  }

  if (play.music_queue_size >= MAX_QUEUED_MUSIC) {
    DEBUG_CONSOLE("Too many queued music, cannot add %d", musnum);
    return 0;
  }

  if ((play.music_queue_size > 0) && 
      (play.music_queue[play.music_queue_size - 1] >= QUEUED_MUSIC_REPEAT)) {
    quit("!PlayMusicQueued: cannot queue music after a repeating tune has been queued");
  }

  if (play.music_repeat) {
    DEBUG_CONSOLE("Queuing music %d to loop", musnum);
    musnum += QUEUED_MUSIC_REPEAT;
  }
  else {
    DEBUG_CONSOLE("Queuing music %d", musnum);
  }

  play.music_queue[play.music_queue_size] = musnum;
  play.music_queue_size++;

  if (play.music_queue_size == 1) {

    clear_music_cache();

    cachedQueuedMusic = load_music_from_disk(musnum, (play.music_repeat > 0));
  }

  return play.music_queue_size;
}

void play_next_queued() {
  // check if there's a queued one to play
  if (play.music_queue_size > 0) {

    int tuneToPlay = play.music_queue[0];

    if (tuneToPlay >= QUEUED_MUSIC_REPEAT) {
      // Loop it!
      play.music_repeat++;
      play_new_music(tuneToPlay - QUEUED_MUSIC_REPEAT, cachedQueuedMusic);
      play.music_repeat--;
    }
    else {
      // Don't loop it!
      int repeatWas = play.music_repeat;
      play.music_repeat = 0;
      play_new_music(tuneToPlay, cachedQueuedMusic);
      play.music_repeat = repeatWas;
    }

    // don't free the memory, as it has been transferred onto the
    // main music channel
    cachedQueuedMusic = NULL;

    play.music_queue_size--;
    for (int i = 0; i < play.music_queue_size; i++)
      play.music_queue[i] = play.music_queue[i + 1];

    if (play.music_queue_size > 0)
      cachedQueuedMusic = load_music_from_disk(play.music_queue[0], 0);
  }

}

int calculate_max_volume() {
  // quieter so that sounds can be heard better
  int newvol=play.music_master_volume + ((int)thisroom.options[ST_VOLUME]) * 30;
  if (newvol>255) newvol=255;
  if (newvol<0) newvol=0;

  if (play.fast_forward)
    newvol = 0;

  return newvol;
}

void update_polled_stuff()
{
  update_polled_stuff(true);
}

// add/remove the volume drop to the audio channels while speech is playing
void apply_volume_drop_modifier(bool applyModifier)
{
  for (int i = 0; i < MAX_SOUND_CHANNELS; i++) 
  {
    if ((channels[i] != NULL) && (channels[i]->done == 0) && (channels[i]->sourceClip != NULL))
    {
      if (applyModifier)
      {
        int audioType = ((ScriptAudioClip*)channels[i]->sourceClip)->type;
        channels[i]->volModifier = -(game.audioClipTypes[audioType].volume_reduction_while_speech_playing * 255 / 100);
      }
      else
        channels[i]->volModifier = 0;

      channels[i]->set_volume(channels[i]->vol);
    }
  }
}

void update_polled_stuff(bool checkForDebugMessages) {
  acaudio_update_mp3();

  if (want_exit) {
    want_exit = 0;
    quit("||exit!");
  }
  if (mvolcounter > update_music_at) {
    update_music_volume();
    apply_volume_drop_modifier(false);
    update_music_at = 0;
    mvolcounter = 0;
    update_ambient_sound_vol();
  }

  if ((editor_debugging_initialized) && (checkForDebugMessages))
    check_for_messages_from_editor();
}

// Update the music, and advance the crossfade on a step
// (this should only be called once per game loop)
void update_polled_stuff_and_crossfade () {
  update_polled_stuff ();

  audio_update_polled_stuff();

  if (crossFading) {
    crossFadeStep++;
    update_music_volume();
  }

  // Check if the current music has finished playing
  if ((play.cur_music_number >= 0) && (play.fast_forward == 0)) {
    if (IsMusicPlaying() == 0) {
      // The current music has finished
      play.cur_music_number = -1;
      play_next_queued();
    }
    else if ((game.options[OPT_CROSSFADEMUSIC] > 0) &&
             (play.music_queue_size > 0) && (!crossFading)) {
      // want to crossfade, and new tune in the queue
      int curpos = channels[SCHAN_MUSIC]->get_pos_ms();
      int muslen = channels[SCHAN_MUSIC]->get_length_ms();
      if ((curpos > 0) && (muslen > 0)) {
        // we want to crossfade, and we know how far through
        // the tune we are
        int takesSteps = calculate_max_volume() / game.options[OPT_CROSSFADEMUSIC];
        int takesMs = (takesSteps * 1000) / frames_per_second;
        if (curpos >= muslen - takesMs)
          play_next_queued();
      }
    }
  }

}




/* *** SCRIPT SYMBOL: [AudioClip] IsMusicVoxAvailable *** */
int IsMusicVoxAvailable () {
  return play.seperate_music_lib;
}



// returns -1 on failure, channel number on success
/* *** SCRIPT SYMBOL: [AudioClip] PlaySoundEx *** */
int PlaySoundEx(int val1, int channel) {

  if (debug_flags & DBG_NOSFX)
    return -1;

  // if no sound, ignore it
  if (usetup.digicard == DIGI_NONE)
    return -1;

  if ((channel < SCHAN_NORMAL) || (channel >= MAX_SOUND_CHANNELS))
    quit("!PlaySoundEx: invalid channel specified, must be 3-7");

  // if an ambient sound is playing on this channel, abort it
  StopAmbientSound(channel);

  if (val1 < 0) {
    stop_and_destroy_channel (channel);
    return -1;
  }
  // if skipping a cutscene, don't try and play the sound
  if (play.fast_forward)
    return -1;
  
  // that sound is already in memory, play it
  if ((last_sound_played[channel] == val1) && (channels[channel] != NULL)) {
    DEBUG_CONSOLE("Playing sound %d on channel %d; cached", val1, channel);
    channels[channel]->restart();
    channels[channel]->set_volume (play.sound_volume);
    return channel;
  }
  // free the old sound
  stop_and_destroy_channel (channel);
  DEBUG_CONSOLE("Playing sound %d on channel %d", val1, channel);

  last_sound_played[channel] = val1;

  SOUNDCLIP *soundfx = load_sound_from_path(val1, play.sound_volume, 0);

  if (soundfx == NULL) {
    debug_log("Sound sample load failure: cannot load sound %d", val1);
    DEBUG_CONSOLE("FAILED to load sound %d", val1);
    return -1;
  }

  channels[channel] = soundfx;
  channels[channel]->priority = 10;
  channels[channel]->set_volume (play.sound_volume);
  return channel;
}

/* *** SCRIPT SYMBOL: [Game] Game::StopSound^1 *** */
void StopAllSounds(int evenAmbient) {
  // backwards-compatible hack -- stop Type 3 (default Sound Type)
  Game_StopAudio(3);

  if (evenAmbient)
    Game_StopAudio(1);
}

// the sound will only be played if there is a free channel or
// it has a priority >= an existing sound to override
int play_sound_priority (int val1, int priority) {
  int lowest_pri = 9999, lowest_pri_id = -1;

  // find a free channel to play it on
  for (int i = SCHAN_NORMAL; i < numSoundChannels; i++) {
    if (val1 < 0) {
      // Playing sound -1 means iterate through and stop all sound
      if ((channels[i] != NULL) && (channels[i]->done == 0))
        stop_and_destroy_channel (i);
    }
    else if ((channels[i] == NULL) || (channels[i]->done != 0)) {
      if (PlaySoundEx(val1, i) >= 0)
        channels[i]->priority = priority;
      return i;
    }
    else if (channels[i]->priority < lowest_pri) {
      lowest_pri = channels[i]->priority;
      lowest_pri_id = i;
    }
      
  }
  if (val1 < 0)
    return -1;

  // no free channels, see if we have a high enough priority
  // to override one
  if (priority >= lowest_pri) {
    if (PlaySoundEx(val1, lowest_pri_id) >= 0) {
      channels[lowest_pri_id]->priority = priority;
      return lowest_pri_id;
    }
  }

  return -1;
}

/* *** SCRIPT SYMBOL: [AudioClip] PlaySound *** */
int play_sound(int val1) {
  return play_sound_priority(val1, 10);
}



/* *** SCRIPT SYMBOL: [AudioChannel] SeekMODPattern *** */
void SeekMODPattern(int patnum) {
  if (current_music_type == MUS_MOD) {
    channels[SCHAN_MUSIC]->seek (patnum);
    DEBUG_CONSOLE("Seek MOD/XM to pattern %d", patnum);
  }
}

/* *** SCRIPT SYMBOL: [Game] Game::GetMODPattern^0 *** */
int Game_GetMODPattern() {
  if (current_music_type == MUS_MOD) {
    return channels[SCHAN_MUSIC]->get_pos();
  }
  return -1;
}

/* *** SCRIPT SYMBOL: [AudioChannel] SeekMP3PosMillis *** */
void SeekMP3PosMillis (int posn) {
  if (current_music_type) {
    DEBUG_CONSOLE("Seek MP3/OGG to %d ms", posn);
    if (crossFading)
      channels[crossFading]->seek (posn);
    else
      channels[SCHAN_MUSIC]->seek (posn);
  }
}

/* *** SCRIPT SYMBOL: [AudioChannel] GetMP3PosMillis *** */
int GetMP3PosMillis () {
  // in case they have "while (GetMP3PosMillis() < 5000) "
  if (play.fast_forward)
    return 999999;

  if (current_music_type) {
    int result = channels[SCHAN_MUSIC]->get_pos_ms();
    if (result >= 0)
      return result;

    return channels[SCHAN_MUSIC]->get_pos ();
  }

  return 0;
}

void update_music_volume() {

  if ((current_music_type) || (crossFading < 0)) 
  {
    // targetVol is the maximum volume we're fading in to
    // newvol is the starting volume that we faded out from
    int targetVol = calculate_max_volume();
    int newvol;
    if (crossFading)
      newvol = crossFadeVolumeAtStart;
    else
      newvol = targetVol;

    // fading out old track, target volume is silence
    if (crossFading < 0)
      targetVol = 0;

    if (crossFading) {
      int curvol = crossFadeVolumePerStep * crossFadeStep;

      if ((curvol > targetVol) && (curvol > newvol)) {
        // it has fully faded to the new track
        newvol = targetVol;
        stop_and_destroy_channel_ex(SCHAN_MUSIC, false);
        if (crossFading > 0) {
          channels[SCHAN_MUSIC] = channels[crossFading];
          channels[crossFading] = NULL;
        }
        crossFading = 0;
      }
      else {
        if (crossFading > 0)
          channels[crossFading]->set_volume((curvol > targetVol) ? targetVol : curvol);

        newvol -= curvol;
        if (newvol < 0)
          newvol = 0;
      }
    }
    if (channels[SCHAN_MUSIC])
      channels[SCHAN_MUSIC]->set_volume (newvol);
  }
}

/* *** SCRIPT SYMBOL: [AudioChannel] SetMusicVolume *** */
void SetMusicVolume(int newvol) {
  if ((newvol < -3) || (newvol > 5))
    quit("!SetMusicVolume: invalid volume number. Must be from -3 to 5.");
  thisroom.options[ST_VOLUME]=newvol;
  update_music_volume();
  }


/* *** SCRIPT SYMBOL: [AudioChannel] SetChannelVolume *** */
void SetChannelVolume(int chan, int newvol) {
  if ((newvol<0) || (newvol>255))
    quit("!SetChannelVolume: invalid volume - must be from 0-255");
  if ((chan < 0) || (chan >= MAX_SOUND_CHANNELS))
    quit("!SetChannelVolume: invalid channel id");

  if ((channels[chan] != NULL) && (channels[chan]->done == 0)) {
    if (chan == ambient[chan].channel) {
      ambient[chan].vol = newvol;
      update_ambient_sound_vol();
    }
    else
      channels[chan]->set_volume (newvol);
  }
}


/* *** SCRIPT SYMBOL: [AudioChannel] GetCurrentMusic *** */
int GetCurrentMusic() {
  return play.cur_music_number;
  }

/* *** SCRIPT SYMBOL: [AudioClip] SetMusicRepeat *** */
void SetMusicRepeat(int loopflag) {
  play.music_repeat=loopflag;
}

// Ensures crossfader is stable after loading (or failing to load)
// new music
void post_new_music_check (int newchannel) {
  if ((crossFading > 0) && (channels[crossFading] == NULL)) {
    crossFading = 0;
    // Was fading out but then they played invalid music, continue
    // to fade out
    if (channels[SCHAN_MUSIC] != NULL)
      crossFading = -1;
  }

}

// Sets up the crossfading for playing the new music track,
// and returns the channel number to use
int prepare_for_new_music () {
  int useChannel = SCHAN_MUSIC;
  
  if ((game.options[OPT_CROSSFADEMUSIC] > 0)
      && (channels[SCHAN_MUSIC] != NULL)
      && (channels[SCHAN_MUSIC]->done == 0)
      && (current_music_type != MUS_MIDI)
      && (current_music_type != MUS_MOD)) {
      
    if (crossFading > 0) {
      // It's still crossfading to the previous track
      stop_and_destroy_channel_ex(SCHAN_MUSIC, false);
      channels[SCHAN_MUSIC] = channels[crossFading];
      channels[crossFading] = NULL;
      crossFading = 0;
      update_music_volume();
    }
    else if (crossFading < 0) {
      // an old track is still fading out, no new music yet
      // Do nothing, and keep the current crossfade step
    }
    else {
      // start crossfading
      crossFadeStep = 0;
      crossFadeVolumePerStep = game.options[OPT_CROSSFADEMUSIC];
      crossFadeVolumeAtStart = calculate_max_volume();
    }
    useChannel = SPECIAL_CROSSFADE_CHANNEL;
    crossFading = useChannel;
  }
  else {
    // crossfading is now turned off
    stopmusic();
    // ensure that any traces of old tunes fading are eliminated
    // (otherwise the new track will be faded out)
    crossFading = 0;
  }

  // Just make sure, because it will be overwritten in a sec
  if (channels[useChannel] != NULL)
    stop_and_destroy_channel (useChannel);

  return useChannel;
}

/* *** SCRIPT SYMBOL: [AudioClip] PlayMP3File *** */
void PlayMP3File (char *filename) {
  if (strlen(filename) >= PLAYMP3FILE_MAX_FILENAME_LEN)
    quit("!PlayMP3File: filename too long");

  DEBUG_CONSOLE("PlayMP3File %s", filename);

  char pathToFile[MAX_PATH];
  get_current_dir_path(pathToFile, filename);

  int useChan = prepare_for_new_music ();
  bool doLoop = (play.music_repeat > 0);
  
  if ((channels[useChan] = my_load_static_ogg(pathToFile, 150, doLoop)) != NULL) {
    channels[useChan]->play();
    current_music_type = MUS_OGG;
    play.cur_music_number = 1000;
    // save the filename (if it's not what we were supplied with)
    if (filename != &play.playmp3file_name[0])
      strcpy (play.playmp3file_name, filename);
  }
  else if ((channels[useChan] = my_load_static_mp3(pathToFile, 150, doLoop)) != NULL) {
    channels[useChan]->play();
    current_music_type = MUS_MP3;
    play.cur_music_number = 1000;
    // save the filename (if it's not what we were supplied with)
    if (filename != &play.playmp3file_name[0])
      strcpy (play.playmp3file_name, filename);
  }
  else
    debug_log ("PlayMP3File: file '%s' not found or cannot play", filename);

  post_new_music_check(useChan);

  update_music_volume();
}


SOUNDCLIP *load_music_from_disk(int mnum, bool doRepeat) {

  if (mnum >= QUEUED_MUSIC_REPEAT) {
    mnum -= QUEUED_MUSIC_REPEAT;
    doRepeat = true;
  }

  SOUNDCLIP *loaded = load_sound_clip_from_old_style_number(true, mnum, doRepeat);

  if ((loaded == NULL) && (mnum > 0)) 
  {
    debug_log("Music %d not found",mnum);
    DEBUG_CONSOLE("FAILED to load music %d", mnum);
  }

  return loaded;
}


void play_new_music(int mnum, SOUNDCLIP *music) {
  if (debug_flags & DBG_NOMUSIC)
    return;
  if (usetup.midicard == MIDI_NONE)
    return;

  if ((play.cur_music_number == mnum) && (music == NULL)) {
    DEBUG_CONSOLE("PlayMusic %d but already playing", mnum);
    return;  // don't play the music if it's already playing
  }

  int useChannel = SCHAN_MUSIC;
  DEBUG_CONSOLE("Playing music %d", mnum);

  if (mnum<0) {
    stopmusic();
    return;
  }

  if (play.fast_forward) {
    // while skipping cutscene, don't change the music
    play.end_cutscene_music = mnum;
    return;
  }

  useChannel = prepare_for_new_music ();

  play.cur_music_number=mnum;
  current_music_type = 0;
  channels[useChannel] = NULL;

  play.current_music_repeating = play.music_repeat;
  // now that all the previous music is unloaded, load in the new one

  if (music != NULL) {
    channels[useChannel] = music;
    music = NULL;
  }
  else {
    channels[useChannel] = load_music_from_disk(mnum, (play.music_repeat > 0));
  }

  if (channels[useChannel] != NULL) {

    if (channels[useChannel]->play() == 0)
      channels[useChannel] = NULL;
    else
      current_music_type = channels[useChannel]->get_sound_type();
  }

  post_new_music_check(useChannel);

  update_music_volume();

}

void newmusic(int mnum) {
  play_new_music(mnum, NULL);
}



void register_audio_script_functions()
{
  scAdd_External_Symbol("AudioChannel::Seek^1",(void *)AudioChannel_Seek);
  scAdd_External_Symbol("AudioChannel::SetRoomLocation^2",(void *)AudioChannel_SetRoomLocation);
  scAdd_External_Symbol("AudioChannel::Stop^0",(void *)AudioChannel_Stop);
  scAdd_External_Symbol("AudioChannel::get_ID",(void *)AudioChannel_GetID);
  scAdd_External_Symbol("AudioChannel::get_IsPlaying",(void *)AudioChannel_GetIsPlaying);
  scAdd_External_Symbol("AudioChannel::get_LengthMs",(void *)AudioChannel_GetLengthMs);
  scAdd_External_Symbol("AudioChannel::get_Panning",(void *)AudioChannel_GetPanning);
  scAdd_External_Symbol("AudioChannel::set_Panning",(void *)AudioChannel_SetPanning);
  scAdd_External_Symbol("AudioChannel::get_PlayingClip",(void *)AudioChannel_GetPlayingClip);
  scAdd_External_Symbol("AudioChannel::get_Position",(void *)AudioChannel_GetPosition);
  scAdd_External_Symbol("AudioChannel::get_PositionMs",(void *)AudioChannel_GetPositionMs);
  scAdd_External_Symbol("AudioChannel::get_Volume",(void *)AudioChannel_GetVolume);
  scAdd_External_Symbol("AudioChannel::set_Volume",(void *)AudioChannel_SetVolume);

  scAdd_External_Symbol("AudioClip::Play^2",(void *)AudioClip_Play);
  scAdd_External_Symbol("AudioClip::PlayFrom^3",(void *)AudioClip_PlayFrom);
  scAdd_External_Symbol("AudioClip::PlayQueued^2",(void *)AudioClip_PlayQueued);
  scAdd_External_Symbol("AudioClip::Stop^0",(void *)AudioClip_Stop);
  scAdd_External_Symbol("AudioClip::get_FileType",(void *)AudioClip_GetFileType);
  scAdd_External_Symbol("AudioClip::get_IsAvailable",(void *)AudioClip_GetIsAvailable);
  scAdd_External_Symbol("AudioClip::get_Type",(void *)AudioClip_GetType);

  scAdd_External_Symbol("Game::IsAudioPlaying^1",(void *)Game_IsAudioPlaying);
  scAdd_External_Symbol("Game::SetAudioTypeSpeechVolumeDrop^2", (void*)Game_SetAudioTypeSpeechVolumeDrop);
  scAdd_External_Symbol("Game::SetAudioTypeVolume^3", (void*)Game_SetAudioTypeVolume);
  scAdd_External_Symbol("Game::StopAudio^1",(void *)Game_StopAudio);

  scAdd_External_Symbol("System::get_AudioChannelCount", (void*)System_GetAudioChannelCount);
  scAdd_External_Symbol("System::geti_AudioChannels", (void*)System_GetAudioChannels);
  scAdd_External_Symbol("Game::GetMODPattern^0",(void *)Game_GetMODPattern);
  scAdd_External_Symbol("Game::StopSound^1", (void *)StopAllSounds);
  scAdd_External_Symbol("GetCurrentMusic",(void *)GetCurrentMusic);
  scAdd_External_Symbol("GetMIDIPosition", (void *)GetMIDIPosition);
  scAdd_External_Symbol("GetMP3PosMillis", (void *)GetMP3PosMillis);
  scAdd_External_Symbol("IsMusicVoxAvailable",(void *)IsMusicVoxAvailable);
  scAdd_External_Symbol("PlayMP3File",(void *)PlayMP3File);
  scAdd_External_Symbol("PlayMusic",(void *)PlayMusicResetQueue);
  scAdd_External_Symbol("PlayMusicQueued",(void *)PlayMusicQueued);
  scAdd_External_Symbol("PlaySound",(void *)play_sound);
  scAdd_External_Symbol("PlaySoundEx",(void *)PlaySoundEx);
  scAdd_External_Symbol("SeekMIDIPosition", (void *)SeekMIDIPosition);
  scAdd_External_Symbol("SeekMODPattern",(void *)SeekMODPattern);
  scAdd_External_Symbol("SeekMP3PosMillis", (void *)SeekMP3PosMillis);
  scAdd_External_Symbol("SetChannelVolume",(void *)SetChannelVolume);
  scAdd_External_Symbol("SetMusicRepeat",(void *)SetMusicRepeat);
  scAdd_External_Symbol("SetMusicRepeat",(void *)SetMusicRepeat);
  scAdd_External_Symbol("SetMusicRepeat",(void *)SetMusicRepeat);
  scAdd_External_Symbol("SetMusicVolume",(void *)SetMusicVolume);
  scAdd_External_Symbol("StopAmbientSound",(void *)StopAmbientSound);
  scAdd_External_Symbol("IsChannelPlaying",(void *)IsChannelPlaying);
  scAdd_External_Symbol("PlayAmbientSound",(void *)PlayAmbientSound);
  scAdd_External_Symbol("StopChannel",(void *)stop_and_destroy_channel);
}
