#ifndef _AC_DYNOBJ_H_HEADER
#define _AC_DYNOBJ_H_HEADER

#include "cscomp.h" // for ICCDynamicObject

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