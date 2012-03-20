/* 
  Script Editor run-time engine component (c) 1998 Chris Jones
  script chunk format:
  00h  1 dword  version - should be 2
  04h  1 dword  sizeof(scriptblock)
  08h  1 dword  number of ScriptBlocks
  0Ch  n STRUCTs ScriptBlocks

  Adventure Game Studio source code Copyright 1999-2011 Chris Jones.
  All rights reserved.

  The AGS Editor Source Code is provided under the Artistic License 2.0
  http://www.opensource.org/licenses/artistic-license-2.0.php

  You MAY NOT compile your own builds of the engine without making it EXPLICITLY
  CLEAR that the code has been altered from the Standard Version.
*/
#include <stdio.h>
#include <stdlib.h>
#include "agsio.h"
#include "routefnd.h"

#include "wgt2allg.h"
#include "acroom.h"
#include "bigend.h"

static char *scripteditruntimecopr = "Script Editor v1.2 run-time component. (c) 1998 Chris Jones";

#define SCRIPT_CONFIG_VERSION 1

void save_script_configuration(FILE * config_file)
{
  quit("ScriptEdit: run-time version can't save");
}

void load_script_configuration(FILE * config_file)
{
    int script_ver;
    if(fread_le_s32(&script_ver, config_file))
        quit("ScriptEdit: file read error");
    if (script_ver != SCRIPT_CONFIG_VERSION)
        quit("ScriptEdit: invalid config version");

    int numvarnames;
    if(fread_le_s32(&numvarnames, config_file))
        quit("ScriptEdit: file read error");
    for (int aa = 0; aa < numvarnames; aa++) {
        int lenoft = getc(config_file);
        fseek(config_file, lenoft, SEEK_CUR);
    }
}

void save_graphical_scripts(FILE * script_file, roomstruct * room)
{
  quit("ScriptEdit: run-time version can't save");
}

char *scripttempn = "~acsc%d.tmp";

void load_graphical_scripts(FILE * script_file, roomstruct * room)
{
    if (route_script_link()) {
        quit("STOP IT.");
        exit(767);
        abort();
    }

    while (1) {
        int ct;
        if(fread_le_s32(&ct, script_file))
            quit("ScriptEdit: file read error");

        if ((ct == -1) || (feof(script_file) != 0))
            break;

        int lee;
        if(fread_le_s32(&lee, script_file))
            quit("ScriptEdit: file read error");

        char thisscn[20];
        sprintf(thisscn, scripttempn, ct);
        FILE *te = fopen(thisscn, "wb");

        char *scnf = (char *)malloc(lee);
        // MACPORT FIX: swap size and nmemb
        fread(scnf, sizeof(char), lee, script_file);
        fwrite(scnf, sizeof(char), lee, te);
        fclose(te);

        free(scnf);
    }
}
