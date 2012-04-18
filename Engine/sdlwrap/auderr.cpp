//
//  auderr.cpp
//  EngineMac
//
//  Created by Nick Sonneveld on 18/04/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "auderr.h"

#include <iostream>
#import <OpenAL/al.h>
#import <OpenAL/alc.h>

// generic error handler - if err is nonzero, prints error message and exits program.
void CheckError(OSStatus error, const char *operation)
{
	if (error == noErr) return;
	
	char str[20];
	// see if it appears to be a 4-char-code
	*(UInt32 *)(str + 1) = CFSwapInt32HostToBig(error);
	if (isprint(str[1]) && isprint(str[2]) && isprint(str[3]) && isprint(str[4])) {
		str[0] = str[5] = '\'';
		str[6] = '\0';
	} else
		// no, format it as an integer
		sprintf(str, "%d", (int)error);
	
	fprintf(stderr, "Error: %s (%s)\n", operation, str);
	
	exit(1);
}

void CheckALError (const char *operation) {
	ALenum alErr = alGetError();
	if (alErr == AL_NO_ERROR) return;
	const char *errFormat = NULL;
	switch (alErr) {
		case AL_INVALID_NAME: errFormat = "OpenAL Error: %s (AL_INVALID_NAME)"; break;
		case AL_INVALID_VALUE:  errFormat = "OpenAL Error: %s (AL_INVALID_VALUE)"; break;
		case AL_INVALID_ENUM:  errFormat = "OpenAL Error: %s (AL_INVALID_ENUM)"; break;
		case AL_INVALID_OPERATION: errFormat = "OpenAL Error: %s (AL_INVALID_OPERATION)"; break;
		case AL_OUT_OF_MEMORY: errFormat = "OpenAL Error: %s (AL_OUT_OF_MEMORY)"; break;
	}
	fprintf (stderr, errFormat, operation);
	exit(1);
	
}
