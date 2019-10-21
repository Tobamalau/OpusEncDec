#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "opus.h"
#include <stdio.h>
#include "marioTestenc.c"

/*The frame size is hardcoded for this sample code but it d../../../../usr/local/include/opusoesn't have to be*/
#define FRAME_SIZE 960
#define SAMPLE_RATE 48000
#define CHANNELS 1
#define APPLICATION OPUS_APPLICATION_AUDIO
#define BITRATE 64000
#define NBBYTES 249

#define MAX_FRAME_SIZE (6*960)
#define MAX_PACKET_SIZE (3*1276)

int main(int argc, char **argv)
{
   opus_int16 in[FRAME_SIZE*CHANNELS];
   opus_int16 out[MAX_FRAME_SIZE*CHANNELS];
   unsigned char cbits[MAX_PACKET_SIZE];
   /*Holds the state of the encoder and decoder */
   int err;

   struct opusfile {
      OpusDecoder *decoder;
      int nbBytes;
      int max_frame_size;
      unsigned char *input;
      unsigned int inputLenpth;
      opus_int16 out[MAX_FRAME_SIZE*CHANNELS];
      unsigned char cbits[MAX_PACKET_SIZE];
   } OpusInstanz = {NULL, NBBYTES, MAX_FRAME_SIZE, NULL, MAX_PACKET_SIZE, NULL, NULL};


   int_fast8_t decodeOpus(struct opusfile *lib)
   {
      int frame_size = opus_decode(lib->decoder, lib->input, lib->nbBytes, lib->out, lib->max_frame_size, 0);
      if (frame_size<0)
      {
       fprintf(stderr, "decoder failed: %s\n", opus_strerror(frame_size));
       return EXIT_FAILURE;
      }
      return EXIT_SUCCESS;
   }

   OpusInstanz.decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
   if (err<0)
   {
      fprintf(stderr, "failed to create decoder: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }

   while (1)
   {
      OpusInstanz.input = &marioTestenc_opus;
      for(int i=0;i<NBBYTES;i++)
      {
         OpusInstanz.cbits[i] = marioTestenc_opus[i];
      }


      decodeOpus(&OpusInstanz);


   }
   /*Destroy the encoder state*/
   opus_decoder_destroy(OpusInstanz.decoder);
   return EXIT_SUCCESS;
}
