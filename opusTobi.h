/*header for Opus functions created by Tobias*/

#ifndef OPUS_TOBI_H
#define OPUS_TOBI_H

#include "opus.h"

#define CHANNELS 1
#define MAX_FRAME_SIZE (6*960)
#define MAX_PACKET_SIZE (3*1276)


struct opus {
   OpusDecoder *decoder;
   int nbBytes;
   int max_frame_size;
   unsigned char *input;
   unsigned int inputLenpth;
   opus_int16 out[MAX_FRAME_SIZE*CHANNELS];
   unsigned char cbits[MAX_PACKET_SIZE];
   unsigned char pcm_bytes[MAX_FRAME_SIZE*CHANNELS*2];
};


#endif
