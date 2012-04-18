//
//  pcmgen.h
//  EngineMac
//
//  Created by Nick Sonneveld on 17/04/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef EngineMac_pcmgen_h
#define EngineMac_pcmgen_h

#import <OpenAL/al.h>

typedef struct ALW_PACKFILE ALW_PACKFILE;


enum FillBufferResult {
  FBR_OK = 0, // the buffer was filled with some data
  FBR_EOF = 1,  // no data available, buffer was not touched.
  FBR_FAILURE= -1   // something happened, we can't guarantee state of things
};


// EXCEPTIONS
// =============================================================================

class file_error { } ;
class open_error : public file_error { } ;
class close_error : public file_error { } ;
class write_error : public file_error { } ;




// INTERFACES
// =============================================================================

class IStreamingPcmGenerator
{
public:
  virtual ~IStreamingPcmGenerator() {}
  virtual void Close() = 0;
  //virtual long Read(char *buf, int length) = 0;
  virtual FillBufferResult FillOpenAlBuffer(ALuint bid) = 0;
  virtual ALenum GetOpenAlBufferFormat() = 0;
  virtual double BytesPosInMs(long bytes) = 0; // for rendering to time
  virtual bool HasDecodeEof() = 0;
};

class IStaticPcmGenerator : public IStreamingPcmGenerator {
public:
  virtual double GetTotalMs() = 0;
  virtual void SeekMs(double positionMs) = 0;
  virtual void SetLoopMode(bool is_looping) = 0;
};


extern IStaticPcmGenerator *pcmgen_from_ogg_packfile(ALW_PACKFILE *packfile);
extern IStaticPcmGenerator *pcmgen_from_ogg_buffer(void *data, int data_len);


#endif
