#ifndef _SCRIPT_OVERLAY_H_HEADER
#define _SCRIPT_OVERLAY_H_HEADER

#include "ags_cc_dynamic_object.h" 

struct ScriptOverlay : AGSCCDynamicObject {
  int overlayId;
  int borderWidth;
  int borderHeight;
  int isBackgroundSpeech;

  virtual int Dispose(const char *address, bool force);
  virtual const char *GetType();
  virtual int Serialize(const char *address, char *buffer, int bufsize);
  virtual void Unserialize(int index, const char *serializedData, int dataSize);
  void Remove();
  ScriptOverlay();
};


#endif
