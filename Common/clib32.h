
#ifndef CLIB32_H
#define CLIB32_H

enum {PR_DATAFIRST=1, PR_FILEFIRST=2};

extern char lib_file_name[];

extern "C"
{
	// open file locally or from lib
    extern FILE *clibfopen(char *, char *);
	// get offset of file in lib
    extern long cliboffset(char *);
	// get size of file in lib
    extern long clibfilesize(char *);
	// file size of last opened file.
    extern long last_opened_size;
	// set the library to be used.
    extern int  csetlib(char*,char*);
	// order of file openign, file, or lib
    extern int  cfopenpriority;

	//extern const char *clibgetoriginalfilename();
	//extern const char *clibGetFileName(int index);
	//extern int clibGetNumFiles();
	
	// for pack_fopen:
	
	// get offset of file in lib
    extern long cliboffset(char *);
	// search and get official file name+path
    extern char* clibgetdatafile(char*);
}

#ifdef CLIB32_REDEFINE_FOPEN
#define fopen clibfopen
#endif

#endif