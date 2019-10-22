#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "opus.h"
#include <stdio.h>
//#include "marioTestenc.c"
#include "opusFile.c"
#include "opusTobi.h"

/*The frame size is hardcoded for this sample code but it d../../../../usr/local/include/opusoesn't have to be*/
#define FRAME_SIZE 960
#define SAMPLE_RATE 48000
//#define CHANNELS 1
#define APPLICATION OPUS_APPLICATION_AUDIO
#define BITRATE 64000
#define NBBYTES 249

//#define MAX_FRAME_SIZE (6*960)
//#define MAX_PACKET_SIZE (3*1276)
#define VERBOSE 0

/*struct opus {
   OpusDecoder *decoder;
   int nbBytes;
   int max_frame_size;
   unsigned char *input;
   unsigned int inputLenpth;
   opus_int16 out[MAX_FRAME_SIZE*CHANNELS];
   unsigned char cbits[MAX_PACKET_SIZE];
   unsigned char pcm_bytes[MAX_FRAME_SIZE*CHANNELS*2];
} */
   struct opus OpusInstanz = {NULL, NBBYTES, MAX_FRAME_SIZE, NULL, MAX_PACKET_SIZE, {}, {}, {}};

struct frame {
   struct opus *opus_t;
   uint loopcnt;
   uint nbbytessum;
} FrameInstanz = {NULL, 0, 0};

volatile int NBbytes[] = {249,134,135,258,189,161,161,161,161,161,161,161,161,161,161,161,161,161,161,161,
                          104,114,207,175,181,162,184,161,161,161,161,145,177,161,161,157,165,152,137,193,
                          144,179,154,168,161,161,161,161,161,161,161,137,128,161,148,142,166};

int_fast8_t decodeOpusFrame(struct opus *opus_t)
{
   //memset(opus_t->pcm_bytes, '\0', sizeof(opus_t->pcm_bytes));
   int frame_size = opus_decode(opus_t->decoder, opus_t->input, opus_t->nbBytes, opus_t->out, opus_t->max_frame_size, 0);
   if (frame_size<0)
   {
    fprintf(stderr, "decoder failed: %s\n", opus_strerror(frame_size));
    return EXIT_FAILURE;
   }
   /* Convert to little-endian ordering.*/
   for(int i=0;i<CHANNELS*frame_size;i++)
   {
      opus_t->pcm_bytes[2*i]=opus_t->out[i]&0xFF;
      opus_t->pcm_bytes[2*i+1]=(opus_t->out[i]>>8)&0xFF;
#if VERBOSE
      printf("%o\t%o\t",opus_t->pcm_bytes[2*i], opus_t->pcm_bytes[2*i+1]);
      if(i%10 == 0)
         printf("\n");
#endif
   }
   return EXIT_SUCCESS;
}

int initOpus(struct opus *opus_t)
{
   int err;
   opus_t->decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
   if (err<0)
   {
      printf("failed to create decoder: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }
   opus_t->input = marioTestenc_opus;
   return EXIT_SUCCESS;
}



int main()
{
   /*Holds the state of the encoder and decoder */

   uint loopcnt = 0;
   uint nbbytessum = 0;
   int len = sizeof(NBbytes) / sizeof(NBbytes[0]);
   initOpus(&OpusInstanz);
   FrameInstanz.opus_t = &OpusInstanz;

   while (1)
   {
      OpusInstanz.input = marioTestenc_opus + nbbytessum;
#if VERBOSE
      printf("NBbytes:%d, NBbytes: %d", len, NBbytes[loopcnt]);
#endif
      OpusInstanz.nbBytes = NBbytes[loopcnt];
      for(int i=0;i<OpusInstanz.nbBytes;i++)
      {
         OpusInstanz.cbits[i] = marioTestenc_opus[(i)+(nbbytessum)];
         //OpusInstanz.cbits[i] = marioTestenc_opus[(i)+(NBBYTES*loopcnt)];
      }

      decodeOpusFrame(&OpusInstanz);
      nbbytessum += OpusInstanz.nbBytes;
      loopcnt++;
      if(len-1<loopcnt)
      {
         printf("nbbytessum:%d\tloopcnt:%d",nbbytessum, loopcnt);
         break;
      }

   }
   /*Destroy the encoder state*/
   opus_decoder_destroy(OpusInstanz.decoder);
   return EXIT_SUCCESS;
}
