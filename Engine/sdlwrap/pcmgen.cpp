//
//  pcmgen.cpp
//  EngineMac
//
//  Created by Nick Sonneveld on 16/04/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "pcmgen.h"

#include <iostream>


#import <AudioToolbox/AudioToolbox.h>
#import <OpenAL/al.h>
#import <OpenAL/alc.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "allegro.h"
#include "auderr.h"

static const int _ALOGG_BUFFER_SIZE = 16*1024;
static const int _ALOGG_NUM_BUFFER = 4;



// CALLBACKS
// =============================================================================

// ov callback notes
// http://xiph.org/vorbis/doc/vorbisfile/callbacks.html


// PACK FILE CALLBACKS

static size_t _packfileread(void *ptr, size_t size, size_t count, ALW_PACKFILE *packfile) {
  return alw_pack_fread(ptr, count*size, packfile);
}

static int _packfileclose(ALW_PACKFILE *packfile) {
  return alw_pack_fclose(packfile);
}

static ov_callbacks OV_CALLBACKS_PACKFILE = {
  (size_t (*)(void *, size_t, size_t, void *))  _packfileread,
  (int (*)(void *, ogg_int64_t, int))           0,
  (int (*)(void *))                             _packfileclose,
  (long (*)(void *))                            0 
};



// MEMORY BUF CALLBACKS

struct OggStaticBuffer {
  void *data;
  int pos;
  int data_len;
};


static size_t _bufread(void *ptr, size_t size, size_t count, OggStaticBuffer *statbuf) {
  size_t avail = (statbuf->data_len-statbuf->pos)/size;
  
  if (avail < count)
    count = avail;
  
  memcpy(ptr, ((char*)statbuf->data)+statbuf->pos, count*size);
  statbuf->pos += count*size;
  
  return count;
}
static int _bufseek(OggStaticBuffer *statbuf, ogg_int64_t offset, int origin) {
  switch (origin) {
    case SEEK_SET: //0
      statbuf->pos = offset;
      break;
    case SEEK_CUR: // 1
      statbuf->pos += offset;
      break;
    case SEEK_END: // 2
      statbuf->pos = statbuf->data_len + offset;
      break;
    default:
      return -1;
  }
  return 0;
}

static int _bufclose(OggStaticBuffer *statbuf) {
  delete statbuf;
  return 0;
}

static long _buftell(OggStaticBuffer *statbuf) {
  return statbuf->pos;
}

static ov_callbacks OV_CALLBACKS_BUF = {
  (size_t (*)(void *, size_t, size_t, void *))  _bufread,
  (int (*)(void *, ogg_int64_t, int))           _bufseek,
  (int (*)(void *))                             _bufclose,
  (long (*)(void *))                            _buftell
};






// OGG PCM GENERATOR
// =============================================================================

class OggPcmGenerator : public IStaticPcmGenerator 
{
private:
  OggVorbis_File _vf;
  int _vf_set;
  
  int _channels;
  int _rate;
  ALenum _buf_format;
  
  int _current_bitstream;
  
  bool _decode_eof;
  bool _decode_failure;
  bool _is_looping;    // affects fill openal buffer
  
  void _init_self() {
    _channels = -1;
    _rate = -1;
    _buf_format = 0;
    _current_bitstream = -1;
    _decode_eof = false;
    _decode_failure = false;
    _is_looping = false;
    
    _vf_set = 0;
  }
  
  void _init_info() {
    vorbis_info *vi=ov_info(&_vf,-1);
    _channels = vi->channels;
    _rate = vi->rate;
    
    switch (_channels) {
      case 1: _buf_format = AL_FORMAT_MONO16; break;
      case 2: _buf_format = AL_FORMAT_STEREO16; break;
      default: throw open_error();
    }
  }
  
public:
  
  OggPcmGenerator(ALW_PACKFILE *packfile) {
    _init_self();
    
    if(ov_open_callbacks(packfile, &_vf, NULL, 0, OV_CALLBACKS_PACKFILE) < 0) {
      fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");
      throw open_error();
    }  
    _vf_set = 1;
  
    _init_info();
  }
  
  OggPcmGenerator(void *data, int data_len) {
    _init_self();
    
    OggStaticBuffer *sbuf = new OggStaticBuffer();
    sbuf->data = data;
    sbuf->data_len = data_len;
    
    if(ov_open_callbacks(sbuf, &_vf, NULL, 0, OV_CALLBACKS_BUF) < 0) {
      fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");
      throw open_error();
    }  
    _vf_set = 1;
    
    _init_info();
  }
  
  ~OggPcmGenerator() {
    Close();
  }
  
  virtual void Close() {
    if (_vf_set) {
      ov_clear(&_vf);
    }
  }
  
  virtual FillBufferResult FillOpenAlBuffer(ALuint bid) {
    if (_decode_eof) return FBR_EOF;
    if (_decode_failure) return FBR_FAILURE;
    
    char buf[_ALOGG_BUFFER_SIZE];
    long filled = 0;
        
    while (filled < _ALOGG_BUFFER_SIZE) {
      long count = ov_read(&_vf,
                           buf+filled,
                           _ALOGG_BUFFER_SIZE-filled,
                           0,2,1,
                           &_current_bitstream);
      
      // error condition
      if (count < 0) {
        // we could try and render what we have before error 
        // but we shouldn't be encouraging distributing buggy oggs?
        _decode_failure = true;
        return FBR_FAILURE;
      }
      
      // eof condition
      if (count == 0) {
        if (_is_looping) {
          ov_pcm_seek_lap(&_vf, 0); // rewind ogg
        } else {
          _decode_eof = true;
          break;
        }
      }
      
      // normal condition
      filled += count;
    }
    
    if (filled <= 0)
      return FBR_EOF;
    
    // copy from buf to AL buffer
    alBufferData(bid, _buf_format, buf, filled, _rate);
    CheckALError ("Couldn't buffer data");
    
    return FBR_OK;
  }

  
  virtual ALenum GetOpenAlBufferFormat() {
    return _buf_format;
  }
  
  virtual double BytesPosInMs(long bytes) {
    return (double)bytes * 1000.0 / sizeof(Uint16) / _channels / _rate; 
  }
  
  virtual bool HasDecodeEof() {
    return _decode_eof;
  }
  
  // for static version
  
  virtual double GetTotalMs() {
    return ov_time_total(&_vf, -1) * 1000.0;
  }
  
  virtual void SeekMs(double positionMs) {
    ov_time_seek_lap(&_vf, positionMs/1000.0);
  }
  
  virtual void SetLoopMode(bool is_looping) {
    _is_looping = is_looping;
  }
  
  
  
};



// PLAIN PCM GENERATOR
// =============================================================================

// ...





IStaticPcmGenerator *pcmgen_from_ogg_packfile(ALW_PACKFILE *packfile) {
  return new OggPcmGenerator(packfile);
}
IStaticPcmGenerator *pcmgen_from_ogg_buffer(void *data, int data_len) {
  return new OggPcmGenerator(data, data_len);
}
