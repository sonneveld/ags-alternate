#ifndef _AC_DIALOG_H_HEADER
#define _AC_DIALOG_H_HEADER

#define CHOSE_TEXTPARSER -3053
#define SAYCHOSEN_USEFLAG 1
#define SAYCHOSEN_YES 2
#define SAYCHOSEN_NO  3 

extern void RunDialog(int tum);
extern void SetDialogOption(int dlg,int opt,int onoroff);

extern void register_dialog_script_functions();

#endif