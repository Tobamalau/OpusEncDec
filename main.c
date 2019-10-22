#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "opus.h"
#include <stdio.h>
//#include "marioTestenc.c"
#include "opusFile.c"

/*The frame size is hardcoded for this sample code but it d../../../../usr/local/include/opusoesn't have to be*/
#define FRAME_SIZE 960
#define SAMPLE_RATE 48000
#define CHANNELS 1
#define APPLICATION OPUS_APPLICATION_AUDIO
#define BITRATE 64000
#define NBBYTES 249

#define MAX_FRAME_SIZE (6*960)
#define MAX_PACKET_SIZE (3*1276)

struct opusfile {
   OpusDecoder *decoder;
   int nbBytes;
   int max_frame_size;
   unsigned char *input;
   unsigned int inputLenpth;
   opus_int16 out[MAX_FRAME_SIZE*CHANNELS];
   unsigned char cbits[MAX_PACKET_SIZE];
   //unsigned char pcm_bytes[MAX_FRAME_SIZE*CHANNELS*2];
} OpusInstanz = {NULL, NBBYTES, MAX_FRAME_SIZE, NULL, MAX_PACKET_SIZE, {}, {}};

int NBbytes[] = {249,134,135,258,189,161,161,161,161,161,161,161,161,161,161,161,161,161,161,161,104,114,207,175,181,162,184,161,161,161,161,145,177,161,161,157,165,152,137,193,144,179,154,168,161,161,161,161,161,161,161,137,128,161,148,142,166};

int_fast8_t decodeOpus(struct opusfile *lib)
{
   //memset(lib->pcm_bytes, '\0', sizeof(lib->pcm_bytes));
   int frame_size = opus_decode(lib->decoder, lib->input, lib->nbBytes, lib->out, lib->max_frame_size, 0);
   if (frame_size<0)
   {
    fprintf(stderr, "decoder failed: %s\n", opus_strerror(frame_size));
    return EXIT_FAILURE;
   }
   return EXIT_SUCCESS;
}


int main()
{
   /*Holds the state of the encoder and decoder */
   int err;
   uint loopcnt = 0;

   OpusInstanz.decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
   if (err<0)
   {
      fprintf(stderr, "failed to create decoder: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }
   uint nbbytessum = 0;


   OpusInstanz.input = marioTestenc_opus;
   while (1)
   {
      OpusInstanz.input = marioTestenc_opus + nbbytessum;
      OpusInstanz.nbBytes = NBbytes[loopcnt];
      for(int i=0;i<OpusInstanz.nbBytes;i++)
      {
         OpusInstanz.cbits[i] = marioTestenc_opus[(i)+(nbbytessum)];
         //OpusInstanz.cbits[i] = marioTestenc_opus[(i)+(NBBYTES*loopcnt)];
      }

      decodeOpus(&OpusInstanz);
      nbbytessum += OpusInstanz.nbBytes;
      loopcnt++;
      if(sizeof(NBbytes)<loopcnt)
      {
         printf("nbbytessum:%d\tloopcnt:%d",nbbytessum, loopcnt);
         break;
      }

      /*
      unsigned char pcm_bytes[MAX_FRAME_SIZE*CHANNELS*2];
      /* Convert to little-endian ordering.
      for(int i=0;i<CHANNELS*OpusInstanz.framesize;i++)
      {
         pcm_bytes[2*i]=OpusInstanz.out[i]&0xFF;
         pcm_bytes[2*i+1]=(OpusInstanz.out[i]>>8)&0xFF;
         printf("\n%o\t%o",pcm_bytes[2*i], pcm_bytes[2*i+1]);
      }*/
   }
   /*Destroy the encoder state*/
   opus_decoder_destroy(OpusInstanz.decoder);
   return EXIT_SUCCESS;
}
