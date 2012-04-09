#ifndef _SCRIPT_DIALOG_OPTIONS_RENDERING_H_HEADER
#define _SCRIPT_DIALOG_OPTIONS_RENDERING_H_HEADER

#include "ags_cc_dynamic_object.h"

struct ScriptDrawingSurface;  // acruntim.h

struct ScriptDialogOptionsRendering : AGSCCDynamicObject {
  int x, y, width, height;
  int parserTextboxX, parserTextboxY;
  int parserTextboxWidth;
  int dialogID;
  int activeOptionID;
  ScriptDrawingSurface *surfaceToRenderTo;
  bool surfaceAccessed;

  // return the type name of the object
  virtual const char *GetType();

  // serialize the object into BUFFER (which is BUFSIZE bytes)
  // return number of bytes used
  virtual int Serialize(const char *address, char *buffer, int bufsize) ;

  virtual void Unserialize(int index, const char *serializedData, int dataSize);
  void Reset();
  ScriptDialogOptionsRendering();
};

#endif
