#ifndef _SCRIPT_DATE_TIME_H_HEADER
#define _SCRIPT_DATE_TIME_H_HEADER

#include "dynobj/ags_cc_dynamic_object.h"  // for AGSCCDynamicObject

struct ScriptDateTime : AGSCCDynamicObject {
  int year, month, day;
  int hour, minute, second;
  int rawUnixTime;

  virtual int Dispose(const char *address, bool force);
  virtual const char *GetType();
  virtual int Serialize(const char *address, char *buffer, int bufsize);
  virtual void Unserialize(int index, const char *serializedData, int dataSize);

  ScriptDateTime();
};

#endif
