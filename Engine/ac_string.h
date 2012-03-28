#ifndef _AC_STRING_H_HEADER
#define _AC_STRING_H_HEADER

#include <stdarg.h>

extern const char *CreateNewScriptString(const char *fromText, bool reAllocate=true);

extern int MAXSTRLEN;
extern void check_strlen(char*ptt);

extern void my_sprintf(char *buffer, const char *fmt, va_list ap);

extern void register_string_script_functions();

#endif