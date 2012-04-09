#ifndef _CC_CHARACTER_H_HEADER
#define _CC_CHARACTER_H_HEADER

#include "ags_cc_dynamic_object.h"  // AGSCCDynamicObject

struct CCCharacter : AGSCCDynamicObject {

  // return the type name of the object
  virtual const char *GetType();
  // serialize the object into BUFFER (which is BUFSIZE bytes)
  // return number of bytes used
  virtual int Serialize(const char *address, char *buffer, int bufsize) ;

  virtual void Unserialize(int index, const char *serializedData, int dataSize);

};

#endif
