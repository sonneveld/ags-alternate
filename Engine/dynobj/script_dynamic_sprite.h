#ifndef _SCRIPT_DYNAMIC_SPRITE_H_HEADER
#define _SCRIPT_DYNAMIC_SPRITE_H_HEADER

#include "dynobj/ags_cc_dynamic_object.h" 


struct ScriptDynamicSprite : AGSCCDynamicObject {
  int slot;

  virtual int Dispose(const char *address, bool force);
  virtual const char *GetType();
  virtual int Serialize(const char *address, char *buffer, int bufsize);
  virtual void Unserialize(int index, const char *serializedData, int dataSize);

  ScriptDynamicSprite(int slot);
  ScriptDynamicSprite();
};

#endif
