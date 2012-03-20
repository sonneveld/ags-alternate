#ifndef _AC_DATETIME_H_HEADER
#define _AC_DATETIME_H_HEADER

#include "ac_dynobj.h"

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

extern void register_datetime_script_functions() ;

#endif