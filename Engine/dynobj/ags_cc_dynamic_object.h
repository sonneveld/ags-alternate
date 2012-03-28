#ifndef _AGS_CC_DYNAMIC_OBJECT_H_HEADER
#define _AGS_CC_DYNAMIC_OBJECT_H_HEADER

#include "dynobj/cc_dynamic_object.h" // for ICCDynamicObject

struct AGSCCDynamicObject : ICCDynamicObject {
public:
  // default implementation
  virtual int Dispose(const char *address, bool force);
  virtual void Unserialize(int index, const char *serializedData, int dataSize) = 0;

protected:
  int bytesSoFar;
  int totalBytes;
  char *serbuffer;

  void StartSerialize(char *sbuffer);
  void SerializeInt(int val);
  int  EndSerialize();
  void StartUnserialize(const char *sbuffer, int pTotalBytes);
  int  UnserializeInt();
};

#endif
