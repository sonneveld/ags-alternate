#ifndef _SC_FILE_H_HEADER
#define _SC_FILE_H_HEADER

#include <stdio.h>  // for FILE

#include "dynobj/cc_dynamic_object.h"

// object-based File routine -- struct definition
#define scFileRead   1
#define scFileWrite  2
#define scFileAppend 3
extern const char *fopenModes[];

struct sc_File : ICCDynamicObject {
  FILE *handle;

  virtual int Dispose(const char *address, bool force);
  virtual const char *GetType() ;

  virtual int Serialize(const char *address, char *buffer, int bufsize) ;

  int OpenFile(const char *filename, int mode);
  void Close();

  sc_File() ;
};



#endif
