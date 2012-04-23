/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      File I/O.
 *
 *      By Shawn Hargreaves.
 *
 *      _pack_fdopen() and related modifications by Annie Testes.
 *
 *      Evert Glebbeek added the support for relative filenames:
 *      make_absolute_filename(), make_relative_filename() and
 *      is_relative_filename().
 *
 *      Peter Wang added support for packfile vtables.
 *
 *      See readme.txt for copyright information.
 *
 * ****** CJ CHANGES IN THIS FILE: Renamed pack_fopen to __old_pack_fopen
 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "allegro.h"

//typedef ALW_PACKFILE PACKFILE;
#define PACKFILE ALW_PACKFILE
#define AL_CONST const
#define ASSERT(x) assert(x)
#define _AL_MALLOC(x) malloc(x)
#define _AL_FREE(x) free(x)
#define MIN(x,y)     (((x) < (y)) ? (x) : (y))

/* some OSes have no concept of "group" and "other" */
#ifndef S_IRGRP
#define S_IRGRP   0
#define S_IWGRP   0
#endif
#ifndef S_IROTH
#define S_IROTH   0
#define S_IWOTH   0
#endif

#define OPEN_PERMS   (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)


int _packfile_filesize = 0;
int _packfile_datasize = 0;

int _packfile_type = 0;

static int file_encoding = U_ASCII;



#if 0 
/***************************************************
 ****************** Path handling ******************
 ***************************************************/


/* fix_filename_case:
 *  Converts filename to upper case.
 */
char *fix_filename_case(char *filename)
{
  ASSERT(filename);
  
  if (!ALLEGRO_LFN)
    ustrupr(filename);
  
  return filename;
}



/* fix_filename_slashes:
 *  Converts '/' or '\' to system specific path separator.
 */
char *fix_filename_slashes(char *filename)
{
  int pos, c;
  ASSERT(filename);
  
  for (pos=0; ugetc(filename+pos); pos+=uwidth(filename+pos)) {
    c = ugetc(filename+pos);
    if ((c == '/') || (c == '\\'))
      usetat(filename+pos, 0, OTHER_PATH_SEPARATOR);
  }
  
  return filename;
}



/* append_filename:
 *  Append filename to path, adding separator if necessary.
 */
char *append_filename(char *dest, AL_CONST char *path, AL_CONST char *filename, int size)
{
  char tmp[1024];
  int pos, c;
  ASSERT(dest);
  ASSERT(path);
  ASSERT(filename);
  ASSERT(size >= 0);
  
  ustrzcpy(tmp, sizeof(tmp), path);
  pos = ustrlen(tmp);
  
  if ((pos > 0) && (uoffset(tmp, pos) < ((int)sizeof(tmp) - ucwidth(OTHER_PATH_SEPARATOR) - ucwidth(0)))) {
    c = ugetat(tmp, pos-1);
    
    if ((c != '/') && (c != OTHER_PATH_SEPARATOR) && (c != DEVICE_SEPARATOR)) {
      pos = uoffset(tmp, pos);
      pos += usetc(tmp+pos, OTHER_PATH_SEPARATOR);
      usetc(tmp+pos, 0);
    }
  }
  
  ustrzcat(tmp, sizeof(tmp), filename);
  
  ustrzcpy(dest, size, tmp);
  
  return dest;
}



/* get_filename:
 *  When passed a completely specified file path, this returns a pointer
 *  to the filename portion. Both '\' and '/' are recognized as directory
 *  separators.
 */
char *get_filename(AL_CONST char *path)
{
  int c;
  const char *ptr, *ret;
  ASSERT(path);
  
  ptr = path;
  ret = ptr;
  for (;;) {
    c = ugetxc(&ptr);
    if (!c) break;
    if ((c == '/') || (c == OTHER_PATH_SEPARATOR) || (c == DEVICE_SEPARATOR))
      ret = (char*)ptr;
  }
  return (char*)ret;
}



/* get_extension:
 *  When passed a complete filename (with or without path information)
 *  this returns a pointer to the file extension.
 */
char *get_extension(AL_CONST char *filename)
{
  int pos, c;
  ASSERT(filename);
  
  pos = ustrlen(filename);
  
  while (pos>0) {
    c = ugetat(filename, pos-1);
    if ((c == '.') || (c == '/') || (c == OTHER_PATH_SEPARATOR) || (c == DEVICE_SEPARATOR))
      break;
    pos--;
  }
  
  if ((pos>0) && (ugetat(filename, pos-1) == '.'))
    return (char *)filename + uoffset(filename, pos);
  
  return (char *)filename + ustrsize(filename);
}



/* put_backslash:
 *  If the last character of the filename is not a \, /, or #, or a device
 *  separator (eg. : under DOS), this routine will concatenate a \ or / on
 *  to it (depending on platform).
 */
void put_backslash(char *filename)
{
  int c;
  ASSERT(filename);
  
  if (ugetc(filename)) {
    c = ugetat(filename, -1);
    
    if ((c == '/') || (c == OTHER_PATH_SEPARATOR) || (c == DEVICE_SEPARATOR) || (c == '#'))
      return;
  }
  
  filename += ustrsize(filename);
  filename += usetc(filename, OTHER_PATH_SEPARATOR);
  usetc(filename, 0);
}



/***************************************************
 ******************* Filesystem ********************
 ***************************************************/


/* set_file_encoding:
 *  Sets the encoding to use for filenames. By default, UTF8 is assumed.
 */
void set_file_encoding(int encoding)
{
  file_encoding = encoding;
}



/* get_file_encoding:
 *  Returns the encoding currently assumed for filenames.
 */
int get_file_encoding(void)
{
  return file_encoding ;
}



/* file_exists:
 *  Checks whether a file matching the given name and attributes exists,
 *  returning non zero if it does. The file attribute may contain any of
 *  the FA_* constants from dir.h. If aret is not null, it will be set 
 *  to the attributes of the matching file. If an error occurs the system 
 *  error code will be stored in errno.
 */
int file_exists(AL_CONST char *filename, int attrib, int *aret)
{
  struct al_ffblk info;
  ASSERT(filename);
  
  if (!_al_file_isok(filename))
    return FALSE;
  
  if (al_findfirst(filename, &info, attrib) != 0) {
    /* no entry is not an error for file_exists() */
    if (*allegro_errno == ENOENT)
      *allegro_errno = 0;
    
    return FALSE;
  }
  
  al_findclose(&info);
  
  if (aret)
    *aret = info.attrib;
  
  return TRUE;
}



/* file_size_ex:
 *  Returns the size of a file, in bytes.
 *  If the file does not exist or an error occurs, it will return zero
 *  and store the system error code in errno.
 */
uint64_t file_size_ex(AL_CONST char *filename)
{
  ASSERT(filename);
  
  if (!_al_file_isok(filename))
    return 0;
  
  return _al_file_size_ex(filename);
}

#endif



/***************************************************
 ******************** Packfiles ********************
 ***************************************************/


static int normal_fclose(void *_f);
static int normal_getc(void *_f);
//static int normal_ungetc(int ch, void *_f);
//static int normal_putc(int c, void *_f);
static long normal_fread(void *p, long n, void *_f);
//static long normal_fwrite(AL_CONST void *p, long n, void *_f);
static int normal_fseek(void *_f, int offset);
//static int normal_feof(void *_f);
//static int normal_ferror(void *_f);

static int normal_refill_buffer(PACKFILE *f);
//static int normal_flush_buffer(PACKFILE *f, int last);




/* create_packfile:
 *  Helper function for creating a PACKFILE structure.
 */
static PACKFILE *create_packfile(int is_normal_packfile)
{
  PACKFILE *f;
  
  
  f = (PACKFILE*)_AL_MALLOC(sizeof(PACKFILE));
  
  if (f == NULL) {
    errno = ENOMEM;
    return NULL;
  }
  
  
  f->userdata = f;
  f->is_normal_packfile = TRUE;
  
  f->normal.buf_pos = f->normal.buf;
  f->normal.flags = 0;
  f->normal.buf_size = 0;
  f->normal.filename = NULL;
  f->normal.parent = NULL;
  f->normal.todo = 0;
  
  
  return f;
}



/* free_packfile:
 *  Helper function for freeing the PACKFILE struct.
 */
static void free_packfile(PACKFILE *f)
{
  if (f) {
    /* These are no longer the responsibility of this function, but
     * these assertions help catch instances of old code which still
     * rely on the old behaviour.
     */
    if (f->is_normal_packfile) {
      
    }
    
    _AL_FREE(f);
  }
}



/* _pack_fdopen:
 *  Converts the given file descriptor into a PACKFILE. The mode can have
 *  the same values as for pack_fopen() and must be compatible with the
 *  mode of the file descriptor. Unlike the libc fdopen(), pack_fdopen()
 *  is unable to convert an already partially read or written file (i.e.
 *  the file offset must be 0).
 *  On success, it returns a pointer to a file structure, and on error it
 *  returns NULL and stores an error code in errno. An attempt to read
 *  a normal file in packed mode will cause errno to be set to EDOM.
 */
PACKFILE *_pack_fdopen(int fd, AL_CONST char *mode)
{
  PACKFILE *f, *f2;
  long header = FALSE;
  int c;
  
  if ((f = create_packfile(TRUE)) == NULL)
    return NULL;
  
  ASSERT(f->is_normal_packfile);
  
  while ((c = *(mode++)) != 0) {
    switch (c) {
      case 'r': case 'R': /*f->normal.flags &= ~PACKFILE_FLAG_WRITE;*/ break;
      case 'w': case 'W': return NULL; /*f->normal.flags |= PACKFILE_FLAG_WRITE; */ break;
      case 'p': case 'P': return NULL; /*f->normal.flags |= PACKFILE_FLAG_PACK; */break;
      case '!': return NULL; /*f->normal.flags &= ~PACKFILE_FLAG_PACK; header = TRUE;*/ break;
    }
  }
  
  /* read a 'real' file */
  f->normal.todo = lseek(fd, 0, SEEK_END);  /* size of the file */
  if (f->normal.todo < 0) {
    //*allegro_errno = errno;
    free_packfile(f);
    return NULL;
  }
  
  lseek(fd, 0, SEEK_SET);
  
  f->normal.hndl = fd;
  
  return f;
}

#define uconvert_tofilename(s, buf)      uconvert(s, U_CURRENT, buf, get_file_encoding(), sizeof(buf))

#ifndef WINDOWS_VERISON
#define O_BINARY 0
#endif

/* pack_fopen:
 *  Opens a file according to mode, which may contain any of the flags:
 *  'r': open file for reading.
 *  'w': open file for writing, overwriting any existing data.
 *  'p': open file in 'packed' mode. Data will be compressed as it is
 *       written to the file, and automatically uncompressed during read
 *       operations. Files created in this mode will produce garbage if
 *       they are read without this flag being set.
 *  '!': open file for writing in normal, unpacked mode, but add the value
 *       F_NOPACK_MAGIC to the start of the file, so that it can be opened
 *       in packed mode and Allegro will automatically detect that the
 *       data does not need to be decompressed.
 *
 *  Instead of these flags, one of the constants F_READ, F_WRITE,
 *  F_READ_PACKED, F_WRITE_PACKED or F_WRITE_NOPACK may be used as the second 
 *  argument to fopen().
 *
 *  On success, fopen() returns a pointer to a file structure, and on error
 *  it returns NULL and stores an error code in errno. An attempt to read a 
 *  normal file in packed mode will cause errno to be set to EDOM.
 */
PACKFILE *__old_pack_fopen(AL_CONST char *filename, AL_CONST char *mode)
{
  char tmp[1024];
  int fd;
  ASSERT(filename);
  
  _packfile_type = 0;
  
  // is it ok to read from a floppy.. i guess yes?
  //if (!_al_file_isok(filename))
  //  return NULL;
  
  //fd = open(uconvert_tofilename(filename, tmp), O_RDONLY | O_BINARY, OPEN_PERMS);
  fd = open(filename, O_RDONLY | O_BINARY, OPEN_PERMS);
  
  if (fd < 0) {
    //*allegro_errno = errno;
    return NULL;
  }
  
  return _pack_fdopen(fd, mode);
}


/* pack_fclose:
 *  Closes a file after it has been read or written.
 *  Returns zero on success. On error it returns an error code which is
 *  also stored in errno. This function can fail only when writing to
 *  files: if the file was opened in read mode it will always succeed.
 */
int pack_fclose(PACKFILE *f)
{
  int ret;
  
  if (!f)
    return 0;
  
  ret = normal_fclose(f->userdata);
  //if (ret != 0)
  //  *allegro_errno = errno;
  
  free_packfile(f);
  
  return ret;
}

/* pack_fseek:
 *  Like the stdio fseek() function, but only supports forward seeks 
 *  relative to the current file position.
 */
int pack_fseek(PACKFILE *f, int offset)
{
  ASSERT(f);
  ASSERT(offset >= 0);
  
  return normal_fseek(f->userdata, offset);
}



/* pack_getc:
 *  Returns the next character from the stream f, or EOF if the end of the
 *  file has been reached.
 */
int pack_getc(PACKFILE *f)
{
  ASSERT(f);
  
  return normal_getc(f->userdata);
}



/* pack_igetw:
 *  Reads a 16 bit word from a file, using intel byte ordering.
 */
int pack_igetw(PACKFILE *f)
{
  int b1, b2;
  ASSERT(f);
  
  if ((b1 = pack_getc(f)) != EOF)
    if ((b2 = pack_getc(f)) != EOF)
      return ((b2 << 8) | b1);
  
  return EOF;
}



/* pack_igetl:
 *  Reads a 32 bit long from a file, using intel byte ordering.
 */
long pack_igetl(PACKFILE *f)
{
  int b1, b2, b3, b4;
  ASSERT(f);
  
  if ((b1 = pack_getc(f)) != EOF)
    if ((b2 = pack_getc(f)) != EOF)
      if ((b3 = pack_getc(f)) != EOF)
        if ((b4 = pack_getc(f)) != EOF)
          return (((long)b4 << 24) | ((long)b3 << 16) |
                  ((long)b2 << 8) | (long)b1);
  
  return EOF;
}



/* pack_mgetw:
 *  Reads a 16 bit int from a file, using motorola byte-ordering.
 */
int pack_mgetw(PACKFILE *f)
{
  int b1, b2;
  ASSERT(f);
  
  if ((b1 = pack_getc(f)) != EOF)
    if ((b2 = pack_getc(f)) != EOF)
      return ((b1 << 8) | b2);
  
  return EOF;
}



/* pack_mgetl:
 *  Reads a 32 bit long from a file, using motorola byte-ordering.
 */
long pack_mgetl(PACKFILE *f)
{
  int b1, b2, b3, b4;
  ASSERT(f);
  
  if ((b1 = pack_getc(f)) != EOF)
    if ((b2 = pack_getc(f)) != EOF)
      if ((b3 = pack_getc(f)) != EOF)
        if ((b4 = pack_getc(f)) != EOF)
          return (((long)b1 << 24) | ((long)b2 << 16) |
                  ((long)b3 << 8) | (long)b4);
  
  return EOF;
}




/* pack_fread:
 *  Reads n bytes from f and stores them at memory location p. Returns the 
 *  number of items read, which will be less than n if EOF is reached or an 
 *  error occurs. Error codes are stored in errno.
 */
long pack_fread(void *p, long n, PACKFILE *f)
{
  ASSERT(f);
  ASSERT(p);
  ASSERT(n >= 0);
  
  return normal_fread(p, n, f->userdata);
}


/***************************************************
 ************ "Normal" packfile vtable *************
 ***************************************************
 
 Ideally this would be the only section which knows about the details
 of "normal" packfiles. However, this ideal is still being violated in
 many places (partly due to the API, and partly because it would be
 quite a lot of work to move the _al_normal_packfile_details field
 into the userdata field of the PACKFILE structure.
 */



int pack_fclose(PACKFILE *f);


static int normal_fclose(void *_f)
{
  PACKFILE *f = (PACKFILE*)_f;
  int ret;
  
  if (f->normal.parent) {
    PACKFILE *parent = (PACKFILE*) f->normal.parent;
    ret = pack_fclose(parent);
  }
  else {
    ret = close(f->normal.hndl);
    //if (ret != 0)
    //  *allegro_errno = errno;
  }
  
  return ret;
}



/* normal_no_more_input:
 *  Return non-zero if the number of bytes remaining in the file (todo) is
 *  less than or equal to zero.
 *
 *  However, there is a special case.  If we are reading from a LZSS
 *  compressed file, the latest call to lzss_read() may have suspended while
 *  writing out a sequence of bytes due to the output buffer being too small.
 *  In that case the `todo' count would be decremented (possibly to zero),
 *  but the output isn't yet completely written out.
 */
static int normal_no_more_input(PACKFILE *f)
{
  return (f->normal.todo <= 0);
}



static int normal_getc(void *_f)
{
  PACKFILE *f = (PACKFILE*)_f;
  
  f->normal.buf_size--;
  if (f->normal.buf_size > 0)
    return *(f->normal.buf_pos++);
  
  if (f->normal.buf_size == 0) {
    if (normal_no_more_input(f))
      f->normal.flags |= PACKFILE_FLAG_EOF;
    return *(f->normal.buf_pos++);
  }
  
  return normal_refill_buffer(f);
}


static long normal_fread(void *p, long n, void *_f)
{
  PACKFILE *f = (PACKFILE*)_f;
  unsigned char *cp = (unsigned char *)p;
  long i;
  int c;
  
  for (i=0; i<n; i++) {
    if ((c = normal_getc(f)) == EOF)
      break;
    
    *(cp++) = c;
  }
  
  return i;
}




static int normal_fseek(void *_f, int offset)
{
  PACKFILE *f = (PACKFILE*)_f;
  int i;
  
  
  errno = 0;
  
  /* skip forward through the buffer */
  if (f->normal.buf_size > 0) {
    i = MIN(offset, f->normal.buf_size);
    f->normal.buf_size -= i;
    f->normal.buf_pos += i;
    offset -= i;
    if ((f->normal.buf_size <= 0) && normal_no_more_input(f))
      f->normal.flags |= PACKFILE_FLAG_EOF;
  }
  
  /* need to seek some more? */
  if (offset > 0) {
    i = MIN(offset, f->normal.todo);
    
    
    if (f->normal.parent) {
	    /* pass the seek request on to the parent file */
      PACKFILE *parent = (PACKFILE*)f->normal.parent;
	    pack_fseek(parent, i);
    }
    else {
	    /* do a real seek */
	    lseek(f->normal.hndl, i, SEEK_CUR);
    }
    f->normal.todo -= i;
    if (normal_no_more_input(f))
	    f->normal.flags |= PACKFILE_FLAG_EOF;
    
  }
  
  if (errno)
    return -1;
  else
    return 0;
}


/* normal_refill_buffer:
 *  Refills the read buffer. The file must have been opened in read mode,
 *  and the buffer must be empty.
 */
static int normal_refill_buffer(PACKFILE *f)
{
  int i, sz, done, offset;
  
  if (f->normal.flags & PACKFILE_FLAG_EOF)
    return EOF;
  
  if (normal_no_more_input(f)) {
    f->normal.flags |= PACKFILE_FLAG_EOF;
    return EOF;
  }
  
  if (f->normal.parent) {
    PACKFILE *parent = (PACKFILE*)f->normal.parent;
    f->normal.buf_size = pack_fread(f->normal.buf, MIN(F_BUF_SIZE, f->normal.todo), parent);
    
    if (parent->normal.flags & PACKFILE_FLAG_EOF)
      f->normal.todo = 0;
    if (parent->normal.flags & PACKFILE_FLAG_ERROR)
      goto Error;
  }
  else {
    f->normal.buf_size = MIN(F_BUF_SIZE, f->normal.todo);
    
    offset = lseek(f->normal.hndl, 0, SEEK_CUR);
    done = 0;
    
    errno = 0;
    sz = read(f->normal.hndl, f->normal.buf, f->normal.buf_size);
    
    while (sz+done < f->normal.buf_size) {
      if ((sz < 0) && ((errno != EINTR) && (errno != EAGAIN)))
        goto Error;
      
      if (sz > 0)
        done += sz;
      
      lseek(f->normal.hndl, offset+done, SEEK_SET);
      errno = 0;
      sz = read(f->normal.hndl, f->normal.buf+done, f->normal.buf_size-done);
    }
    
  }
  
  f->normal.todo -= f->normal.buf_size;
  f->normal.buf_pos = f->normal.buf;
  f->normal.buf_size--;
  if (f->normal.buf_size <= 0)
    if (normal_no_more_input(f))
      f->normal.flags |= PACKFILE_FLAG_EOF;
  
  if (f->normal.buf_size < 0)
    return EOF;
  else
    return *(f->normal.buf_pos++);
  
Error:
  errno = EFAULT;
  f->normal.flags |= PACKFILE_FLAG_ERROR;
  return EOF;
}

