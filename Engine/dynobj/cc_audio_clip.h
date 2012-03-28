#ifndef _CC_AUDIO_CLIP_H_HEADER
#define _CC_AUDIO_CLIP_H_HEADER

#include "dynobj/ags_cc_dynamic_object.h" 


struct CCAudioClip : AGSCCDynamicObject {
  virtual const char *GetType();
  virtual int Serialize(const char *address, char *buffer, int bufsize);
  virtual void Unserialize(int index, const char *serializedData, int dataSize);
};

#endif
