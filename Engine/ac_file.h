#ifndef _AC_FILE_H_HEADER
#define _AC_FILE_H_HEADER

#include <stdint.h>
#include <stdio.h>

#ifdef WINDOWS_VERSION
// from WinNT.h
typedef wchar_t WCHAR;
typedef const WCHAR *LPCWSTR;
extern void change_to_directory_of_file(LPCWSTR fileName);
#else
extern void change_to_directory_of_file(const char *fileName);
#endif

// maybe not best place to put this?
// archive attributes to search for - al_findfirst breaks with 0
#define FA_SEARCH -1

extern uint32_t ac_fd_sizebytes(int fd);
extern uint32_t ac_fstream_sizebytes(FILE *f);

extern void get_current_dir_path(char* buffer, const char *fileName);
extern bool validate_user_file_path(const char *fnmm, char *output, bool currentDirOnly);

extern void register_file_script_functions();

#endif
