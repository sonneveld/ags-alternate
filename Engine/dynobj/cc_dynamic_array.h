#ifndef _CC_DYNAMIC_ARRAY_H_HEADER
#define _CC_DYNAMIC_ARRAY_H_HEADER

#include "cc_dynamic_object.h"

// *** IMPL FOR DYNAMIC ARRAYS **

#define CC_DYNAMIC_ARRAY_TYPE_NAME "CCDynamicArray"
#define ARRAY_MANAGED_TYPE_FLAG    0x80000000

struct CCDynamicArray : ICCDynamicObject
{
  // return the type name of the object
  virtual const char *GetType();

  virtual int Dispose(const char *address, bool force);

  // serialize the object into BUFFER (which is BUFSIZE bytes)
  // return number of bytes used
  virtual int Serialize(const char *address, char *buffer, int bufsize);

  virtual void Unserialize(int index, const char *serializedData, int dataSize);

  long Create(int numElements, int elementSize, bool isManagedType);

};
#endif
