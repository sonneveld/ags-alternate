#include "dynobj/cc_audio_channel.h"

#include "acruntim.h"
#include "acsound.h"
#include "cscomp.h"

const char *CCAudioChannel::GetType() {
  return "AudioChannel";
}

int CCAudioChannel::Serialize(const char *address, char *buffer, int bufsize) {
  ScriptAudioChannel *ach = (ScriptAudioChannel*)address;
  StartSerialize(buffer);
  SerializeInt(ach->id);
  return EndSerialize();
}

void CCAudioChannel::Unserialize(int index, const char *serializedData, int dataSize) {
  StartUnserialize(serializedData, dataSize);
  int id = UnserializeInt();
  ccRegisterUnserializedObject(index, &scrAudioChannel[id], this);
}
