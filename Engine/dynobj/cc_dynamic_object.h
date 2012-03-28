#ifndef _CC_DYNAMIC_OBJECT_H_HEADER
#define _CC_DYNAMIC_OBJECT_H_HEADER

// OBJECT-BASED SCRIPTING RUNTIME FUNCTIONS
// interface 
struct ICCDynamicObject {
  // when a ref count reaches 0, this is called with the address
  // of the object. Return 1 to remove the object from memory, 0 to
  // leave it
  virtual int Dispose(const char *address, bool force) = 0;

  // return the type name of the object
  virtual const char *GetType() = 0;

  // serialize the object into BUFFER (which is BUFSIZE bytes)
  // return number of bytes used
  virtual int Serialize(const char *address, char *buffer, int bufsize) = 0;
};

#endif
