#include "dynobj/sc_file.h"

extern FILE* FileOpen(const char*fnmm, const char* mode);
extern void FileClose(FILE*hha);

// object-based File routines

const char *fopenModes[] = {NULL, "rb", "wb", "ab"};

int sc_File::Dispose(const char *address, bool force) {
  Close();
  delete this;
  return 1;
}

const char *sc_File::GetType() {
  return "File";
}

int sc_File::Serialize(const char *address, char *buffer, int bufsize) {
  // we cannot serialize an open file, so it will get closed
  return 0;
}

sc_File::sc_File() {
  handle = NULL;
}


int sc_File::OpenFile(const char *filename, int mode) {
handle = FileOpen(filename, fopenModes[mode]);
if (handle == NULL)
  return 0;
return 1;
}

void sc_File::Close() {
  if (handle) {
    FileClose(handle);
    handle = NULL;
  }
}
