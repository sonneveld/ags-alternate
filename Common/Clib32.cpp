/* CLIB32 - DJGPP implemention of the CLIB reader.
  (c) 1998-99 Chris Jones
  
  22/12/02 - Shawn's Linux changes approved and integrated - CJ

  v1.2 (Apr'01)  added support for new multi-file CLIB version 10 files
  v1.1 (Jul'99)  added support for appended-to-exe data files

  This is UNPUBLISHED PROPRIETARY SOURCE CODE;
  the contents of this file may not be disclosed to third parties,
  copied or duplicated in any form, in whole or in part, without
  prior express permission from Chris Jones.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdint.h"

#include "clib32.h"

#define NATIVESTATIC

#ifdef _MANAGED
// ensure this doesn't get compiled to .NET IL
#pragma unmanaged
// don't allow these fields to be in the assembly manifest
#undef NATIVESTATIC
#define NATIVESTATIC static
#endif


#if !defined(LINUX_VERSION) && !defined(MAC_VERSION)
#include <io.h>
#else
#include "djcompat.h"
#include "allegro.h"
#endif
#include "misc.h"

#ifdef MAC_VERSION
#include "macport.h"
#endif

#include "bigend.h"

//#define CLIB_IS_INSTALLED
//static char clib32copyright[] = "CLIB32 v1.21 (c) 1995,1996,1998,2001,2007 Chris Jones";
char lib_file_name[255] = " ";
static char base_path[255] = ".";
static char original_base_filename[255];
static char clbuff[20];
static const int RAND_SEED_SALT = 9338638;  // must update editor agsnative.cpp if this changes
#define MAX_FILES 10000
#define MAXMULTIFILES 25

struct MultiFileLib
{
  char data_filenames[MAXMULTIFILES][20];
  int num_data_files;
  char filenames[MAX_FILES][25];
  long offset[MAX_FILES];
  long length[MAX_FILES];
  char file_datafile[MAX_FILES];        // number of datafile
  int num_files;
};

struct MultiFileLibNew
{
  char data_filenames[MAXMULTIFILES][50];
  int num_data_files;
  char filenames[MAX_FILES][100];
  long offset[MAX_FILES];
  long length[MAX_FILES];
  char file_datafile[MAX_FILES];        // number of datafile
  int num_files;
};

static MultiFileLibNew mflib;
static NATIVESTATIC char *clibendfilesig = "CLIB\x1\x2\x3\x4SIGE";
static NATIVESTATIC char *clibpasswencstring = "My\x1\xde\x4Jibzle";
static NATIVESTATIC int _last_rand;

static void init_pseudo_rand_gen(int seed)
{
  _last_rand = seed;
}

static int get_pseudo_rand()
{
  return( ((_last_rand = _last_rand * 214013L
      + 2531011L) >> 16) & 0x7fff );
}

static void clib_decrypt_text(char *toenc)
{
  int adx = 0;

  while (1) {
    toenc[0] -= clibpasswencstring[adx];
    if (toenc[0] == 0)
      break;

    adx++;
    toenc++;

    if (adx > 10)
      adx = 0;
  }
}

static void fgetnulltermstring(char *sss, FILE *ddd, int bufsize) {
  int b = -1;
  do {
    if (b < bufsize - 1)
      b++;
    sss[b] = fgetc(ddd);
    if (feof(ddd))
      return;
  } while (sss[b] != 0);
}

extern "C"
{
  long last_opened_size;

  static void fread_data_enc(void *data, int dataSize, int dataCount, FILE *ooo)
  {
    fread(data, dataSize, dataCount, ooo);
    unsigned char *dataChar = (unsigned char*)data;
    for (int i = 0; i < dataSize * dataCount; i++)
    {
      dataChar[i] -= get_pseudo_rand();
    }
  }

  static void fgetstring_enc(char *sss, FILE *ooo, int maxLength) 
  {
    int i = 0;
    while ((i == 0) || (sss[i - 1] != 0))
    {
      sss[i] = fgetc(ooo) - get_pseudo_rand();

      if (i < maxLength - 1)
        i++;
    }
  }

  static int getw_enc(FILE *ooo)
  {
    int numberRead;
    fread_data_enc(&numberRead, 4, 1, ooo);
    return numberRead;
  }



    // read little endian 32 bit from file
    uint32_t fget_le32(FILE* fp)
    {
        uint8_t buf[4];
        size_t count = fread(buf, 4, 1, fp);
        if (count != 1)
            abort();

        uint32_t result;
        result  = buf[0];
        result |= buf[1] <<  8;
        result |= buf[2] << 16;
        result |= buf[3] << 24;
        return result;
    }


    // read little endian 16 bit from file
    uint16_t fget_le16(FILE* fp)
    {
        uint8_t buf[2];
        size_t count = fread(buf, 2, 1, fp);
        if (count != 1)
            abort();

        uint16_t result;
        result  = buf[0];
        result |= buf[1] <<  8;
        return result;
    }

  static int read_new_new_enc_format_clib(MultiFileLibNew * mfl, FILE * wout, int libver)
  {
    int randSeed = fget_le32(wout);

    init_pseudo_rand_gen(randSeed + RAND_SEED_SALT);
    mfl->num_data_files = getw_enc(wout);
    for (int aa = 0; aa < mfl->num_data_files; aa++)
    {
      fgetstring_enc(mfl->data_filenames[aa], wout, 50);
    }
    mfl->num_files = getw_enc(wout);

    if (mfl->num_files > MAX_FILES)
      return -1;

    for (int aa = 0; aa < mfl->num_files; aa++)
    {
      fgetstring_enc(mfl->filenames[aa], wout, 100);
    }
    fread_data_enc(&mfl->offset[0], sizeof(long), mfl->num_files, wout);
    fread_data_enc(&mfl->length[0], sizeof(long), mfl->num_files, wout);
    fread_data_enc(&mfl->file_datafile[0], 1, mfl->num_files, wout);
    return 0;
  }

  static int read_new_new_format_clib(MultiFileLibNew * mfl, FILE * wout, int libver)
  {
    int aa;
    mfl->num_data_files = fget_le32(wout);
    for (aa = 0; aa < mfl->num_data_files; aa++)
    {
      fgetnulltermstring(mfl->data_filenames[aa], wout, 50);
    }
    mfl->num_files = fget_le32(wout);

    if (mfl->num_files > MAX_FILES)
      return -1;

    for (aa = 0; aa < mfl->num_files; aa++)
    {
      short nameLength;
      fread(&nameLength, 2, 1, wout);
      nameLength /= 5;
      fread(mfl->filenames[aa], nameLength, 1, wout);
      clib_decrypt_text(mfl->filenames[aa]);
    }
    fread(&mfl->offset[0], sizeof(long), mfl->num_files, wout);
    fread(&mfl->length[0], sizeof(long), mfl->num_files, wout);
    fread(&mfl->file_datafile[0], 1, mfl->num_files, wout);
    return 0;
  }

  static int read_new_format_clib(MultiFileLib * mfl, FILE * wout, int libver)
  {
    mfl->num_data_files = fget_le32(wout);
    fread(&mfl->data_filenames[0][0], 20, mfl->num_data_files, wout);
    mfl->num_files = fget_le32(wout);

    if (mfl->num_files > MAX_FILES)
      return -1;

    fread(&mfl->filenames[0][0], 25, mfl->num_files, wout);
    fread(&mfl->offset[0], sizeof(long), mfl->num_files, wout);
    fread(&mfl->length[0], sizeof(long), mfl->num_files, wout);
    fread(&mfl->file_datafile[0], 1, mfl->num_files, wout);

    if (libver >= 11)
    {
      int aa;
      for (aa = 0; aa < mfl->num_files; aa++)
          clib_decrypt_text(mfl->filenames[aa]);
    }
    return 0;
  }

  // given a library path (an exe) and its password (normally empty string), 
  // setup the clib.
  int csetlib(char *libpath, char *passw)
  {
    // clear original_base_filename
    strcpy(original_base_filename, "");

    // if given a null libpath, set lib_filename to " "
    if (libpath == NULL) {
      strcpy(lib_file_name, " ");
      return 0;
    }

    strcpy(base_path, ".");

    FILE *lib_file = ci_fopen(libpath, "rb");
    if (lib_file == NULL)
      return -1;

    long absoffs = 0;
    fread(&clbuff[0], 5, 1, lib_file);

    // if we don't have CLIB header search for data at the
    // end of file to show where actual start is.
    if (strncmp(clbuff, "CLIB", 4) != 0) {
      fseek(lib_file, -12, SEEK_END);
      fread(&clbuff[0], 12, 1, lib_file);

      if (strncmp(clbuff, clibendfilesig, 12) != 0)
        return -2;

      fseek(lib_file, -16, SEEK_END);  // it's an appended-to-end-of-exe thing
      fread(&absoffs, 4, 1, lib_file);
      fseek(lib_file, absoffs + 5, SEEK_SET);
    }

    int lib_version = fgetc(lib_file);
    if ((lib_version != 6) && (lib_version != 10) &&
        (lib_version != 11) && (lib_version != 15) &&
        (lib_version != 20) && (lib_version != 21))
      return -3;  // unsupported version

    // if libpath was "c:\games\something\data.exe"
    // then libpath = "data.exe"
    // base_path = "c:\games\something"

    char *libpathwas_prev = libpath;
    // remove slashes so that the lib name fits in the buffer
    while ((strchr(libpath, '\\') != NULL) || (strchr(libpath, '/') != NULL))
      libpath++;

    if (libpath != libpathwas_prev) {
      // store complete path
      strcpy(base_path, libpathwas_prev);
      base_path[libpath - libpathwas_prev] = 0;
      if ((base_path[strlen(base_path) - 1] == '\\') || (base_path[strlen(base_path) - 1] == '/'))
        base_path[strlen(base_path) - 1] = 0;
    }

    if (lib_version >= 10) {
      if (fgetc(lib_file) != 0)
        return -4;  // not first datafile in chain


      // 21 -----
      if (lib_version >= 21)
      {
        if (read_new_new_enc_format_clib(&mflib, lib_file, lib_version))
          return -5;
      }
      // 20 ---------
      else if (lib_version == 20)
      {
        if (read_new_new_format_clib(&mflib, lib_file, lib_version))
          return -5;
      }
      else 
      {
          // 10, 11, 15 ---------
        MultiFileLib mflibOld;
        if (read_new_format_clib(&mflibOld, lib_file, lib_version))
          return -5;
        // convert to newer format
        mflib.num_files = mflibOld.num_files;
        mflib.num_data_files = mflibOld.num_data_files;
        memcpy(&mflib.offset[0], &mflibOld.offset[0], sizeof(long) * mflib.num_files);
        memcpy(&mflib.length[0], &mflibOld.length[0], sizeof(long) * mflib.num_files);
        memcpy(&mflib.file_datafile[0], &mflibOld.file_datafile[0], sizeof(char) * mflib.num_files);
        for (int aa = 0; aa < mflib.num_data_files; aa++)
          strcpy(mflib.data_filenames[aa], mflibOld.data_filenames[aa]);
        for (int aa = 0; aa < mflib.num_files; aa++)
          strcpy(mflib.filenames[aa], mflibOld.filenames[aa]);
      }

       // 10, 11, 15, 20, 21
      fclose(lib_file);
      strcpy(lib_file_name, libpath);

      // make a backup of the original file name
      strcpy(original_base_filename, mflib.data_filenames[0]);
      strlwr(original_base_filename);

      strcpy(mflib.data_filenames[0], libpath);
      for (int aa = 0; aa < mflib.num_files; aa++) {
        // correct offsetes for EXE file
        if (mflib.file_datafile[aa] == 0)
          mflib.offset[aa] += absoffs;
      }
      return 0;
    }

    // 6 ----------
    int passwmodifier = fgetc(lib_file);
    fgetc(lib_file); // unused byte
    mflib.num_data_files = 1;
    strcpy(mflib.data_filenames[0], libpath);

    short tempshort;
    fread(&tempshort, 2, 1, lib_file);
    mflib.num_files = tempshort;

    if (mflib.num_files > MAX_FILES)
      return -4;

    fread(clbuff, 13, 1, lib_file);  // skip password dooberry
    for (int aa = 0; aa < mflib.num_files; aa++) {
      fread(&mflib.filenames[aa][0], 13, 1, lib_file);
      for (int cc = 0; cc < (int)strlen(mflib.filenames[aa]); cc++)
        mflib.filenames[aa][cc] -= passwmodifier;
    }
    fread(&mflib.length[0], 4, mflib.num_files, lib_file);
    fseek(lib_file, 2 * mflib.num_files, SEEK_CUR);  // skip flags & ratio

    mflib.offset[0] = ftell(lib_file);
    strcpy(lib_file_name, libpath);
    fclose(lib_file);

    for (int aa = 1; aa < mflib.num_files; aa++) {
      mflib.offset[aa] = mflib.offset[aa - 1] + mflib.length[aa - 1];
      mflib.file_datafile[aa] = 0;
    }
    mflib.file_datafile[0] = 0;
    return 0;
  }

#if 0
  static int clibGetNumFiles()
  {
    if (lib_file_name[0] == ' ')
      return 0;
    return mflib.num_files;
  }

  static const char *clibGetFileName(int index)
  {
    if (lib_file_name[0] == ' ')
      return NULL;

    if ((index < 0) || (index >= mflib.num_files))
      return NULL;

    return &mflib.filenames[index][0];
  }
#endif

  static int clibfindindex(char *fill)
  {
    if (lib_file_name[0] == ' ')
      return -1;

    int bb;
    for (bb = 0; bb < mflib.num_files; bb++) {
      if (stricmp(mflib.filenames[bb], fill) == 0)
        return bb;
    }
    return -1;
  }

  long clibfilesize(char *fill)
  {
    int idxx = clibfindindex(fill);
    if (idxx >= 0)
      return mflib.length[idxx];
    return -1;
  }

  long cliboffset(char *fill)
  {
    int idxx = clibfindindex(fill);
    if (idxx >= 0)
      return mflib.offset[idxx];
    return -1;
  }

#if 0
  static const char *clibgetoriginalfilename() {
    return original_base_filename;
  }
#endif

  static char actfilename[250];
  // search and find a datafile name
  char *clibgetdatafile(char *fill)
  {
    int idxx = clibfindindex(fill);
    if (idxx >= 0) {
#if defined(LINUX_VERSION) || defined(MAC_VERSION) 
      sprintf(actfilename, "%s/%s", base_path, mflib.data_filenames[mflib.file_datafile[idxx]]);
#else
      sprintf(actfilename, "%s\\%s", base_path, mflib.data_filenames[mflib.file_datafile[idxx]]);
#endif
      return &actfilename[0];
    }
    return NULL;
  }

  static FILE *tfil;
  // open file from clib
  FILE *clibopenfile(char *filly, char *readmode)
  {
    int bb;
    for (bb = 0; bb < mflib.num_files; bb++) {
      if (stricmp(mflib.filenames[bb], filly) == 0) {
        char actfilename[250];
        sprintf(actfilename, "%s\\%s", base_path, mflib.data_filenames[mflib.file_datafile[bb]]);
        tfil = ci_fopen(actfilename, readmode);
        if (tfil == NULL)
          return NULL;
        fseek(tfil, mflib.offset[bb], SEEK_SET);
        return tfil;
      }
    }
    return ci_fopen(filly, readmode);
  }


  // public: set to datafirst or filefirst to configure the operation of clibfopen
  int cfopenpriority = PR_DATAFIRST;

  // given a filename and mode, attempt to open file either as a normal file
  // or through a clib file.
  FILE *clibfopen(char *filnamm, char *fmt)
  {
    last_opened_size = -1;
    if (cfopenpriority == PR_FILEFIRST) {
      // check for file, otherwise use datafile
      if (fmt[0] != 'r') {
        tfil = ci_fopen(filnamm, fmt);
      } else {
        tfil = ci_fopen(filnamm, fmt);

        if ((tfil == NULL) && (lib_file_name[0] != ' ')) {
          tfil = clibopenfile(filnamm, fmt);
          last_opened_size = clibfilesize(filnamm);
        }
      }

    } else {
      // check datafile first, then scan directory
      if ((cliboffset(filnamm) < 1) | (fmt[0] != 'r'))
        tfil = ci_fopen(filnamm, fmt);
      else {
        tfil = clibopenfile(filnamm, fmt);
        last_opened_size = clibfilesize(filnamm);
      }

    }

    if ((last_opened_size < 0) && (tfil != NULL))
#ifdef MAC_VERSION
      last_opened_size = flength(tfil);
#else
      last_opened_size = filelength(fileno(tfil));
#endif

    return tfil;
  }
} // extern "C"
