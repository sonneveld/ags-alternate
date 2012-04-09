//
//  strimpl.cpp
//  EngineMac
//
//  Created by Nick Sonneveld on 8/04/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//


#include <ctype.h>

char *ac_strlwr(char * str) {
    char *p=str;
    while (*p) {
        *p = tolower(*p);
        p++;
    }
    return p;
}
              
char *ac_strupr(char * str) {
    char *p=str;
    while (*p) {
        *p = toupper(*p);
        p++;
    }
    return p;
}

