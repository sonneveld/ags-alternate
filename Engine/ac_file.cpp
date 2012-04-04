#include "ac_file.h"

#include "sdlwrap/allegro.h"

#include "ac.h"
#include "ac_context.h"
#include "ac_string.h"
#include "clib32.h"
#include "misc.h"
#include "dynobj/sc_file.h"
#include "cscomp.h"

#ifdef WINDOWS_VERSION
#include <shlwapi.h>
#elif defined(LINUX_VERSION) || defined(MAC_VERSION)
#define HWND long
#define _getcwd getcwd

long int filelength(int fhandle)
{
	struct stat statbuf;
	fstat(fhandle, &statbuf);
	return statbuf.st_size;
}
#else   // it's DOS (DJGPP)
#include "sys/exceptn.h"
#define _getcwd getcwd

int sys_getch() {
  return getch();
}
#endif  // WINDOWS_VERSION

#define MAX_OPEN_SCRIPT_FILES 10
static FILE*valid_handles[MAX_OPEN_SCRIPT_FILES+1];
static int num_open_script_files = 0;

void get_current_dir_path(char* buffer, const char *fileName)
{
  if (use_compiled_folder_as_current_dir)
  {
    sprintf(buffer, "Compiled\\%s", fileName);
  }
  else
  {
    strcpy(buffer, fileName);
  }
}

static int check_valid_file_handle(FILE*hann, char*msg) {
  int aa;
  if (hann != NULL) {
    for (aa=0; aa < num_open_script_files; aa++) {
      if (hann == valid_handles[aa])
        return aa;
    }
  }
  char exmsg[100];
  sprintf(exmsg,"!%s: invalid file handle; file not previously opened or has been closed",msg);
  quit(exmsg);
  return -1;
  }

bool validate_user_file_path(const char *fnmm, char *output, bool currentDirOnly)
{
  if (strncmp(fnmm, "$SAVEGAMEDIR$", 13) == 0) 
  {
    fnmm += 14;
    sprintf(output, "%s%s", saveGameDirectory, fnmm);
  }
  else if (strncmp(fnmm, "$APPDATADIR$", 12) == 0) 
  {
    fnmm += 13;
    const char *appDataDir = platform->GetAllUsersDataDirectory();
    if (appDataDir == NULL) appDataDir = ".";
    if (game.saveGameFolderName[0] != 0)
    {
      sprintf(output, "%s/%s", appDataDir, game.saveGameFolderName);
      alw_fix_filename_slashes(output);
      mkdir(output);
    }
    else 
    {
      strcpy(output, appDataDir);
    }
    alw_put_backslash(output);
    strcat(output, fnmm);
  }
  else
  {
    get_current_dir_path(output, fnmm);
  }

  // don't allow access to files outside current dir
  if (!currentDirOnly) { }
  else if ((strchr (fnmm, '/') != NULL) || (strchr(fnmm, '\\') != NULL) ||
    (strstr(fnmm, "..") != NULL) || (strchr(fnmm, ':') != NULL)) {
    debug_log("Attempt to access file '%s' denied (not current directory)", fnmm);
    return false;
  }

  return true;
}

/* *** SCRIPT SYMBOL: [File] FileOpen *** */
FILE* FileOpen(const char*fnmm, const char* mode) {
  int useindx = 0;
  char fileToOpen[MAX_PATH];

  if (!validate_user_file_path(fnmm, fileToOpen, strcmp(mode, "rb") != 0))
    return NULL;

  // find a free file handle to use
  for (useindx = 0; useindx < num_open_script_files; useindx++) 
  {
    if (valid_handles[useindx] == NULL)
      break;
  }

  valid_handles[useindx] = fopen(fileToOpen, mode);

  if (valid_handles[useindx] == NULL)
    return NULL;

  if (useindx >= num_open_script_files) 
  {
    if (num_open_script_files >= MAX_OPEN_SCRIPT_FILES)
      quit("!FileOpen: tried to open more than 10 files simultaneously - close some first");
    num_open_script_files++;
  }
  return valid_handles[useindx];
}

/* *** SCRIPT SYMBOL: [File] FileClose *** */
void FileClose(FILE*hha) {
  valid_handles[check_valid_file_handle(hha,"FileClose")] = NULL;
  fclose(hha);
  }
/* *** SCRIPT SYMBOL: [File] FileWrite *** */
static void FileWrite(FILE*haa, const char *towrite) {
  check_valid_file_handle(haa,"FileWrite");
  putw(strlen(towrite)+1,haa);
  fwrite(towrite,strlen(towrite)+1,1,haa);
  }
/* *** SCRIPT SYMBOL: [File] FileWriteRawLine *** */
static void FileWriteRawLine(FILE*haa, const char*towrite) {
  check_valid_file_handle(haa,"FileWriteRawLine");
  fwrite(towrite,strlen(towrite),1,haa);
  fputc (13, haa);
  fputc (10, haa);
  }
/* *** SCRIPT SYMBOL: [File] FileRead *** */
static void FileRead(FILE*haa,char*toread) {
  VALIDATE_STRING(toread);
  check_valid_file_handle(haa,"FileRead");
  if (feof(haa)) {
    toread[0] = 0;
    return;
  }
  int lle=getw(haa);
  if ((lle>=200) | (lle<1)) quit("!FileRead: file was not written by FileWrite");
  fread(toread,lle,1,haa);
  }
/* *** SCRIPT SYMBOL: [File] FileIsEOF *** */
static int FileIsEOF (FILE *haa) {
  check_valid_file_handle(haa,"FileIsEOF");
  if (feof(haa))
    return 1;
  if (ferror (haa))
    return 1;
  if (ftell (haa) >= filelength (fileno(haa)))
    return 1;
  return 0;
}
/* *** SCRIPT SYMBOL: [File] FileIsError *** */
static int FileIsError(FILE *haa) {
  check_valid_file_handle(haa,"FileIsError");
  if (ferror(haa))
    return 1;
  return 0;
}
/* *** SCRIPT SYMBOL: [File] FileWriteInt *** */
static void FileWriteInt(FILE*haa,int into) {
  check_valid_file_handle(haa,"FileWriteInt");
  fputc('I',haa);
  putw(into,haa);
  }
/* *** SCRIPT SYMBOL: [File] FileReadInt *** */
static int FileReadInt(FILE*haa) {
  check_valid_file_handle(haa,"FileReadInt");
  if (feof(haa))
    return -1;
  if (fgetc(haa)!='I')
    quit("!FileReadInt: File read back in wrong order");
  return getw(haa);
  }
/* *** SCRIPT SYMBOL: [File] FileReadRawChar *** */
static char FileReadRawChar(FILE*haa) {
  check_valid_file_handle(haa,"FileReadRawChar");
  if (feof(haa))
    return -1;
  return fgetc(haa);
  }
/* *** SCRIPT SYMBOL: [File] FileReadRawInt *** */
static int FileReadRawInt(FILE*haa) {
  check_valid_file_handle(haa,"FileReadRawInt");
  if (feof(haa))
    return -1;
  return getw(haa);
}
/* *** SCRIPT SYMBOL: [File] FileWriteRawChar *** */
static void FileWriteRawChar(FILE *haa, int chartoWrite) {
  check_valid_file_handle(haa,"FileWriteRawChar");
  if ((chartoWrite < 0) || (chartoWrite > 255))
    quit("!FileWriteRawChar: can only write values 0-255");

  fputc(chartoWrite, haa);
}


/* *** SCRIPT SYMBOL: [File] File::Exists^1 *** */
static int File_Exists(const char *fnmm) {
  char fileToCheck[MAX_PATH];

  if (!validate_user_file_path(fnmm, fileToCheck, false))
    return 0;

  FILE *iii = fopen(fileToCheck, "rb");
  if (iii == NULL)
    return 0;

  fclose(iii);
  return 1;
}

/* *** SCRIPT SYMBOL: [File] File::Delete^1 *** */
static int File_Delete(const char *fnmm) {

  char fileToDelete[MAX_PATH];

  if (!validate_user_file_path(fnmm, fileToDelete, true))
    return 0;

  unlink(fileToDelete);

  return 1;
}

/* *** SCRIPT SYMBOL: [File] File::Open^2 *** */
static void *sc_OpenFile(const char *fnmm, int mode) {
  if ((mode < scFileRead) || (mode > scFileAppend))
    quit("!OpenFile: invalid file mode");

  sc_File *scf = new sc_File();
  if (scf->OpenFile(fnmm, mode) == 0) {
    delete scf;
    return 0;
  }
  ccRegisterManagedObject(scf, scf);
  return scf;
}

/* *** SCRIPT SYMBOL: [File] File::Close^0 *** */
static void File_Close(sc_File *fil) {
  fil->Close();
}

/* *** SCRIPT SYMBOL: [File] File::WriteString^1 *** */
static void File_WriteString(sc_File *fil, const char *towrite) {
  FileWrite(fil->handle, towrite);
}

/* *** SCRIPT SYMBOL: [File] File::WriteInt^1 *** */
static void File_WriteInt(sc_File *fil, int towrite) {
  FileWriteInt(fil->handle, towrite);
}

/* *** SCRIPT SYMBOL: [File] File::WriteRawChar^1 *** */
static void File_WriteRawChar(sc_File *fil, int towrite) {
  FileWriteRawChar(fil->handle, towrite);
}

/* *** SCRIPT SYMBOL: [File] File::WriteRawLine^1 *** */
static void File_WriteRawLine(sc_File *fil, const char *towrite) {
  FileWriteRawLine(fil->handle, towrite);
}

/* *** SCRIPT SYMBOL: [File] File::ReadRawLine^1 *** */
static void File_ReadRawLine(sc_File *fil, char* buffer) {
  check_valid_file_handle(fil->handle, "File.ReadRawLine");
  check_strlen(buffer);
  int i = 0;
  while (i < MAXSTRLEN - 1) {
    buffer[i] = fgetc(fil->handle);
    if (buffer[i] == 13) {
      // CR -- skip LF and abort
      fgetc(fil->handle);
      break;
    }
    if (buffer[i] == 10)  // LF only -- abort
      break;
    if (feof(fil->handle))  // EOF -- abort
      break;
    i++;
  }
  buffer[i] = 0;
}

/* *** SCRIPT SYMBOL: [File] File::ReadRawLineBack^0 *** */
static const char* File_ReadRawLineBack(sc_File *fil) {
  char readbuffer[MAX_MAXSTRLEN + 1];
  File_ReadRawLine(fil, readbuffer);
  return CreateNewScriptString(readbuffer);
}

/* *** SCRIPT SYMBOL: [File] File::ReadString^1 *** */
static void File_ReadString(sc_File *fil, char *toread) {
  FileRead(fil->handle, toread);
}

/* *** SCRIPT SYMBOL: [File] File::ReadStringBack^0 *** */
static const char* File_ReadStringBack(sc_File *fil) {
  check_valid_file_handle(fil->handle, "File.ReadStringBack");
  if (feof(fil->handle)) {
    return CreateNewScriptString("");
  }

  int lle = getw(fil->handle);
  if ((lle >= 20000) || (lle < 1))
    quit("!File.ReadStringBack: file was not written by WriteString");

  char *retVal = (char*)malloc(lle);
  fread(retVal, lle, 1, fil->handle);

  return CreateNewScriptString(retVal, false);
}

/* *** SCRIPT SYMBOL: [File] File::ReadInt^0 *** */
static int File_ReadInt(sc_File *fil) {
  return FileReadInt(fil->handle);
}

/* *** SCRIPT SYMBOL: [File] File::ReadRawChar^0 *** */
static int File_ReadRawChar(sc_File *fil) {
  return FileReadRawChar(fil->handle);
}

/* *** SCRIPT SYMBOL: [File] File::ReadRawInt^0 *** */
static int File_ReadRawInt(sc_File *fil) {
  return FileReadRawInt(fil->handle);
}

/* *** SCRIPT SYMBOL: [File] File::get_EOF *** */
static int File_GetEOF(sc_File *fil) {
  if (fil->handle == NULL)
    return 1;
  return FileIsEOF(fil->handle);
}

/* *** SCRIPT SYMBOL: [File] File::get_Error *** */
static int File_GetError(sc_File *fil) {
  if (fil->handle == NULL)
    return 1;
  return FileIsError(fil->handle);
}


void change_to_directory_of_file(LPCWSTR fileName)
{
  WCHAR wcbuffer[MAX_PATH];
  StrCpyW(wcbuffer, fileName);

#if defined(LINUX_VERSION) || defined(MAC_VERSION)
    if (strrchr(wcbuffer, '/') != NULL) {
      strrchr(wcbuffer, '/')[0] = 0;
      chdir(wcbuffer);
    }
#else
  LPWSTR backSlashAt = StrRChrW(wcbuffer, NULL, L'\\');
  if (backSlashAt != NULL) {
      wcbuffer[wcslen(wcbuffer) - wcslen(backSlashAt)] = L'\0';
      SetCurrentDirectoryW(wcbuffer);
    }
#endif
}



// ============================================================================
// FILE - FOPEN OVERRIDE
// ============================================================================

// override packfile functions to allow it to load from our
// custom CLIB datafiles
extern "C" {
ALW_PACKFILE*_my_temppack;
extern ALW_PACKFILE *__old_pack_fopen(char *,char *);

#if ALW_ALLEGRO_DATE > 19991010
ALW_PACKFILE *pack_fopen(const char *filnam1, const char *modd1) {
#else
ALW_PACKFILE *pack_fopen(char *filnam1, char *modd1) {
#endif
      
    char msg[2000];
    sprintf(msg, "pack_fopen: %s - %s\n", filnam1, modd1);
    write_log_debug(msg);
  char  *filnam = (char *)filnam1;
  char  *modd = (char *)modd1;
  int   needsetback = 0;

  if (filnam[0] == '~') {
    // ~ signals load from specific data file, not the main default one
    char gfname[80];
    int ii = 0;
    
    filnam++;
    while (filnam[0]!='~') {
      gfname[ii] = filnam[0];
      filnam++;
      ii++;
    }
    filnam++;
    // MACPORT FIX 9/6/5: changed from NULL TO '\0'
    gfname[ii] = '\0';
/*    char useloc[250];
#ifdef LINUX_VERSION
    sprintf(useloc,"%s/%s",usetup.data_files_dir,gfname);
#else
    sprintf(useloc,"%s\\%s",usetup.data_files_dir,gfname);
#endif
    csetlib(useloc,"");*/
    
    char *libname = ci_find_file(usetup.data_files_dir, gfname);
    if (csetlib(libname,""))
    {
      // Hack for running in Debugger
      free(libname);
      libname = ci_find_file("Compiled", gfname);
      csetlib(libname,"");
    }
    free(libname);
    
    needsetback = 1;
  }

  // if the file exists, override the internal file
  FILE *testf = fopen(filnam, "rb");
  if (testf != NULL)
    fclose(testf);

  if ((cliboffset(filnam)<1) || (testf != NULL)) {
    if (needsetback) csetlib(game_file_name,"");
    return __old_pack_fopen(filnam, modd);
  } 
  else {
    _my_temppack=__old_pack_fopen(clibgetdatafile(filnam), modd);
    if (_my_temppack == NULL)
      quitprintf("pack_fopen: unable to change datafile: not found: %s", clibgetdatafile(filnam));

    alw_pack_fseek(_my_temppack,cliboffset(filnam));
    
#if ALW_ALLEGRO_DATE < 20050101
    _my_temppack->todo=clibfilesize(filnam);
#else
    _my_temppack->normal.todo = clibfilesize(filnam);
#endif

    if (needsetback)
      csetlib(game_file_name,"");
    return _my_temppack;
  }
}

} // end extern "C"

// end packfile functions



void register_file_script_functions() {
  scAdd_External_Symbol("File::Delete^1",(void *)File_Delete);
  scAdd_External_Symbol("File::Exists^1",(void *)File_Exists);
  scAdd_External_Symbol("File::Open^2",(void *)sc_OpenFile);
  scAdd_External_Symbol("File::Close^0", (void *)File_Close);
  scAdd_External_Symbol("File::ReadInt^0", (void *)File_ReadInt);
  scAdd_External_Symbol("File::ReadRawChar^0", (void *)File_ReadRawChar);
  scAdd_External_Symbol("File::ReadRawInt^0", (void *)File_ReadRawInt);
  scAdd_External_Symbol("File::ReadRawLine^1", (void *)File_ReadRawLine);
  scAdd_External_Symbol("File::ReadRawLineBack^0", (void *)File_ReadRawLineBack);
  scAdd_External_Symbol("File::ReadString^1", (void *)File_ReadString);
  scAdd_External_Symbol("File::ReadStringBack^0", (void *)File_ReadStringBack);
  scAdd_External_Symbol("File::WriteInt^1", (void *)File_WriteInt);
  scAdd_External_Symbol("File::WriteRawChar^1", (void *)File_WriteRawChar);
  scAdd_External_Symbol("File::WriteRawLine^1", (void *)File_WriteRawLine);
  scAdd_External_Symbol("File::WriteString^1", (void *)File_WriteString);
  scAdd_External_Symbol("File::get_EOF", (void *)File_GetEOF);
  scAdd_External_Symbol("File::get_Error", (void *)File_GetError);
  scAdd_External_Symbol("FileClose",(void *)FileClose);
  scAdd_External_Symbol("FileIsEOF",(void *)FileIsEOF);
  scAdd_External_Symbol("FileIsError",(void *)FileIsError);
  scAdd_External_Symbol("FileOpen",(void *)FileOpen);
  scAdd_External_Symbol("FileRead",(void *)FileRead);
  scAdd_External_Symbol("FileReadInt",(void *)FileReadInt);
  scAdd_External_Symbol("FileReadRawChar",(void *)FileReadRawChar);
  scAdd_External_Symbol("FileReadRawInt",(void *)FileReadRawInt);
  scAdd_External_Symbol("FileWrite",(void *)FileWrite);
  scAdd_External_Symbol("FileWriteInt",(void *)FileWriteInt);
  scAdd_External_Symbol("FileWriteRawChar",(void *)FileWriteRawChar);
  scAdd_External_Symbol("FileWriteRawLine", (void *)FileWriteRawLine);
}

