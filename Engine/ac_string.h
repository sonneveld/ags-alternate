#ifndef _AC_STRING_H_HEADER
#define _AC_STRING_H_HEADER

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

extern const char *CreateNewScriptString(const char *fromText, bool reAllocate=true);

extern int MAXSTRLEN;
extern void check_strlen(char*ptt);

extern void my_sprintf(char *buffer, const char *fmt, va_list ap);

extern void register_string_script_functions();

#endif