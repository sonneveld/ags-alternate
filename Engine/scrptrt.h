#ifndef _SCRPTRT_H_HEADER
#define _SCRPTRT_H_HEADER

extern void load_script_configuration(FILE *);
extern void save_script_configuration(FILE *);
extern void load_graphical_scripts(FILE *, roomstruct *);
extern void save_graphical_scripts(FILE *, roomstruct *);
extern char*scripttempn;

#endif