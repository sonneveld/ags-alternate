//
//  sampleloader.cpp
//  EngineMac
//
//  Created by Nick Sonneveld on 18/04/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "sampleloader.h"

#include <iostream>
#include <vector>

#import <AudioToolbox/AudioToolbox.h>
#import <OpenAL/al.h>
#import <OpenAL/alc.h>
#import <CoreAudio/CoreAudioTypes.h>

#include "allegro.h"
#include "auderr.h"


// given a sample file, produce a bundle of buffers

#define READ_BUF_SIZE (512*1024)
#define SOURCE_BUF_SIZE (512*1204)


struct AudioFileStreamClientState {
  AudioStreamBasicDescription srcASBD;
  int asbd_set;  // small check but we should get format before any packets
  
  AudioStreamBasicDescription destASBD;
  
  AudioConverterRef audioConverter;
  
  std::vector<ALuint> *buffers;
};

struct AudioConverterClientState {
  AudioFileStreamClientState *streamState;
  
  UInt32							inNumberBytes;
  UInt32							inNumberPackets;
  const void *					inInputData;
  AudioStreamPacketDescription	*inPacketDescriptions;
  
  int packets_processed;  // number of packets sent to converter.
};

static void stream_on_prop_callback(void *						inClientData,
                              AudioFileStreamID			inAudioFileStream,
                              AudioFileStreamPropertyID	inPropertyID,
                              UInt32 *					ioFlags){
  
  AudioFileStreamClientState *streamState = (AudioFileStreamClientState *)inClientData;
  
  if (inPropertyID == kAudioFileStreamProperty_DataFormat) {
    UInt32 size = sizeof(streamState->srcASBD) ;
    memset( &streamState->srcASBD, 0, size);
    CheckError( AudioFileStreamGetProperty(inAudioFileStream, kAudioFileStreamProperty_DataFormat, &size, &streamState->srcASBD), "coudln't read property" );
    
    streamState->asbd_set = 1;
        
    CheckError(AudioConverterNew(&streamState->srcASBD, &streamState->destASBD, &streamState->audioConverter), "couldn't create converter");
  }
  
}


static OSStatus convert_fill_callback(	AudioConverterRef				inAudioConverter,
                                              UInt32*							ioNumberDataPackets,
                                              AudioBufferList*				ioData,
                                              AudioStreamPacketDescription**	outDataPacketDescription,
                                              void*							inUserData) {
  
  AudioConverterClientState *convertState  = (AudioConverterClientState *) inUserData;
  
  int packets_remaining = convertState->inNumberPackets - convertState->packets_processed;
  
  if (packets_remaining <= 0) {
    *ioNumberDataPackets = 0;
    return 'mpty';  // AudioConverterFillComplexBuffer will return this value
  }
  
  *ioNumberDataPackets = packets_remaining;
  
  ioData->mNumberBuffers = 1;
  ioData->mBuffers[0].mNumberChannels = 2;
  ioData->mBuffers[0].mDataByteSize = convertState->inNumberBytes  ;
  ioData->mBuffers[0].mData = (void *)convertState->inInputData;
  
  if (outDataPacketDescription) {
    *outDataPacketDescription = convertState->inPacketDescriptions;
  }
  
  convertState->packets_processed = convertState->inNumberPackets;
  
  return noErr;
  
}

static  void stream_on_packet_callback( void *							inClientData,
                             UInt32							inNumberBytes,
                             UInt32							inNumberPackets,
                             const void *					inInputData,
                             AudioStreamPacketDescription	*inPacketDescriptions){
  
  AudioFileStreamClientState *streamState = (AudioFileStreamClientState*)inClientData;
  
  AudioConverterClientState convertState;
  memset(&convertState, 0, sizeof(convertState));
  convertState.streamState = streamState;
  convertState.inNumberBytes = inNumberBytes;
  convertState.inNumberPackets = inNumberPackets;
  convertState.inInputData = inInputData;
  convertState.inPacketDescriptions = inPacketDescriptions;
  convertState.packets_processed = 0;
  
  // create audio buffer list
  AudioBufferList *buffers;
	UInt32 ablSize = offsetof(AudioBufferList, mBuffers[0]) + (sizeof(AudioBuffer) * 1); // 1 buffer
	buffers = (AudioBufferList*)malloc (ablSize);
	
	// allocate sample buffer
	char * sampleBuffer =  (char*)malloc(SOURCE_BUF_SIZE); // 4/18/11 - fix 1
	
	buffers->mNumberBuffers = 1;
	buffers->mBuffers[0].mNumberChannels = 2;
	buffers->mBuffers[0].mDataByteSize = SOURCE_BUF_SIZE;
	buffers->mBuffers[0].mData = sampleBuffer;
  
  int maxNumPackets = SOURCE_BUF_SIZE /4;
  unsigned int numPackets = maxNumPackets;
  
  int done = 0;
  
  while (!done) {
    
    // reset the size
    buffers->mBuffers[0].mDataByteSize = SOURCE_BUF_SIZE;
    
    int result = AudioConverterFillComplexBuffer(streamState->audioConverter, convert_fill_callback, (void*)&convertState, (UInt32*)&numPackets, buffers, 0);
    
    if (result == 0) {
      // more to go because we ran out of out buffer space.
    } else if (result == 'mpty') {
      done = 1;
    } else {
      CheckError(result  , "Couldn't convert audio buffer");
    }
    
    //printf("stream: converted data!  %d packets  = %d bytes of buffer\n", numPackets, buffers->mBuffers[0].mDataByteSize);
    
    ALuint buffer;
    alGenBuffers(1, &buffer);
    CheckALError("couldn't generate buffer");
    
    alBufferData(buffer, AL_FORMAT_STEREO16, buffers->mBuffers[0].mData, buffers->mBuffers[0].mDataByteSize, streamState->destASBD.mSampleRate);
    CheckALError("couldn't buffer data");
    
    streamState->buffers->push_back(buffer);
  }
  
  free(sampleBuffer);
  free(buffers);
}




int CreateOpenAlBuffersFromSample(const char *filename, std::vector<ALuint> &buffers) {
  AudioFileStreamClientState streamState;
  memset(&streamState, 0, sizeof(streamState));
  
  // check out FillOutASBDForPCM  <-- after we know this way works :)
  // wanted
  // frame is a collection of samples
  // packet is a collection of frames (1 for uncompressed)
  memset(&streamState.destASBD, 0, sizeof(streamState.destASBD));
  streamState.destASBD.mSampleRate = 44100.0;
  streamState.destASBD.mChannelsPerFrame = 2;
	streamState.destASBD.mBitsPerChannel = 16;
	streamState.destASBD.mFormatID = kAudioFormatLinearPCM;
	streamState.destASBD.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
  /* 1 frame = 1 packet when using PCM */
  streamState.destASBD.mFramesPerPacket = 1;// for uncompressed, this is 1
  streamState.destASBD.mBytesPerPacket = streamState.destASBD.mBytesPerFrame = sizeof(UInt16) * streamState.destASBD.mChannelsPerFrame;
  streamState.buffers = &buffers;
  
  AudioFileStreamID filestream;
  CheckError(  AudioFileStreamOpen(&streamState, &stream_on_prop_callback, &stream_on_packet_callback, 0, &filestream) ,
             "coudln't open stream");
  
  ALW_PACKFILE *packfile = alw_pack_fopen(filename, "r");
  if (packfile == 0)
    return -1;
  
  char buf[READ_BUF_SIZE];
  
  for(;;) {
    long count = alw_pack_fread(buf, READ_BUF_SIZE, packfile);
    if (count == 0)
      break;
    if (count < 0)
      return -1;
    CheckError(AudioFileStreamParseBytes(filestream, count, buf, 0), 
               "Couldn't parse bytes in stream");
  }
  
  alw_pack_fclose(packfile);
  CheckError( AudioFileStreamClose(filestream), "couldn't close");
  
  return 0;
}
