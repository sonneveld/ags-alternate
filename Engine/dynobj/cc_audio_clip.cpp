#include "dynobj/cc_audio_clip.h"

#include "allegro_wrapper.h"

#include "acsound.h"
#include "acruntim.h"
#include "cscomp.h"


const char *CCAudioClip::GetType() {
  return "AudioClip";
}

int CCAudioClip::Serialize(const char *address, char *buffer, int bufsize) {
  ScriptAudioClip *ach = (ScriptAudioClip*)address;
  StartSerialize(buffer);
  SerializeInt(ach->id);
  return EndSerialize();
}

void CCAudioClip::Unserialize(int index, const char *serializedData, int dataSize) {
  StartUnserialize(serializedData, dataSize);
  int id = UnserializeInt();
  ccRegisterUnserializedObject(index, &game.audioClips[id], this);
}