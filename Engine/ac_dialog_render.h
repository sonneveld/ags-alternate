#ifndef _AC_DIALOG_RENDER_H_HEADER
#define _AC_DIALOG_RENDER_H_HEADER

#include "ac_dynobj.h"

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
  virtual const char *GetType() {
    return "DialogOptionsRendering";
  }

  // serialize the object into BUFFER (which is BUFSIZE bytes)
  // return number of bytes used
  virtual int Serialize(const char *address, char *buffer, int bufsize) {
    return 0;
  }

  virtual void Unserialize(int index, const char *serializedData, int dataSize) {
    ccRegisterUnserializedObject(index, this, this);
  }

  void Reset()
  {
    x = 0;
    y = 0;
    width = 0;
    height = 0;
    parserTextboxX = 0;
    parserTextboxY = 0;
    parserTextboxWidth = 0;
    dialogID = 0;
    surfaceToRenderTo = NULL;
    surfaceAccessed = false;
    activeOptionID = -1;
  }

  ScriptDialogOptionsRendering()
  {
    Reset();
  }
};

extern void register_dialog_options_rendering_info_script_functions();

#endif