#ifndef _AC_FILE_H_HEADER
#define _AC_FILE_H_HEADER

// from WinNT.h
typedef wchar_t WCHAR;
typedef const WCHAR *LPCWSTR;

// maybe not best place to put this?
// archive attributes to search for - al_findfirst breaks with 0
#define FA_SEARCH -1

extern void get_current_dir_path(char* buffer, const char *fileName);
extern bool validate_user_file_path(const char *fnmm, char *output, bool currentDirOnly);
extern void change_to_directory_of_file(LPCWSTR fileName);

extern void register_file_script_functions();

#endif
