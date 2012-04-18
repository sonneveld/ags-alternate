//
//  auderr.h
//  EngineMac
//
//  Created by Nick Sonneveld on 18/04/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef EngineMac_auderr_h
#define EngineMac_auderr_h

#include <CoreServices/CoreServices.h>

extern void CheckError(OSStatus error, const char *operation);
extern void CheckALError (const char *operation);

#endif
