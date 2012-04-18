//
//  sampleloader.h
//  EngineMac
//
//  Created by Nick Sonneveld on 18/04/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef EngineMac_sampleloader_h
#define EngineMac_sampleloader_h

#include <vector>
#import <OpenAL/al.h>

extern int CreateOpenAlBuffersFromSample(const char *filename, std::vector<ALuint> &buffers);

#endif
