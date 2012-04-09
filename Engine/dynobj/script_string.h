#ifndef _SCRIPT_STRING_H_HEADER
#define _SCRIPT_STRING_H_HEADER


#include "ags_cc_dynamic_object.h"  // AGSCCDynamicObject
#include "cscomp.h"     // ICCStringClass

struct ScriptString : AGSCCDynamicObject, ICCStringClass {
  char *text;

  virtual int Dispose(const char *address, bool force);
  virtual const char *GetType();
  virtual int Serialize(const char *address, char *buffer, int bufsize);
  virtual void Unserialize(int index, const char *serializedData, int dataSize);

  virtual void* CreateString(const char *fromText);

  ScriptString();
  ScriptString(const char *fromText);
};

#endif
