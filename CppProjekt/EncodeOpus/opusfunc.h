#ifndef OPUSFUNC_H
#define OPUSFUNC_H

#include "opus.h"

#include <stdlib.h>

#define FRAME_SIZE 960
#define APPLICATION OPUS_APPLICATION_AUDIO
//#define BITRATE 64000
#define NBBYTES 249
#define OPUSCHANNELS 1
#define MAX_FRAME_SIZE (6*960)
#define MAX_PACKET_SIZE (3*1276)
#define SAMPLE_RATE 48000   //input Sample Rate of opus file
#define OPUSPACKETIDENTIFIER 0xff
#define OPUSPACKETPERREQUEST 1
#define OPUSPACKETMAXCNT 10
#define HEADERMEMSYZE(X) (X * 2 + 2)



extern int NBbytes[];

struct opus {
   OpusDecoder *decoder;
   int nbBytes;
   const unsigned char *input;
   opus_int16 out[MAX_FRAME_SIZE*OPUSCHANNELS];
   int16_t pcm_bytes[2][MAX_FRAME_SIZE*OPUSCHANNELS];
};

struct frame {
   struct opus *opus_t;
   u_int16_t loopcnt;
   u_int16_t nbbytessum;
   u_int16_t nbbytescnt;
};

int8_t decodeOpusFrame(struct opus *opus_t, u_int8_t bufferNr);
int8_t initOpus(struct opus *opus_t);
int8_t initOpusFrame(struct frame *frame_t);
void getPcm(struct frame *frame_t, u_int8_t bufferNr);
char *getOpusPacketHeader(u_int8_t framecnt, int *framesize, u_int16_t *payloadlength);
bool isOpusPacket(const unsigned char *msgBuffer, u_int16_t msgLength);
const unsigned char *saveOpusPacket(const unsigned char *msgBuffer, u_int16_t msgLength);
const unsigned char *getOpusFrameFromPacket(const unsigned char *msgBuffer, u_int8_t pos);
#endif // OPUSFUNC_H
