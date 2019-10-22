/*lib for Opos functions created by Tobias*/

#include "opusTobi.h"
#include <stdio.h>


int_fast8_t decodeOpusFrame(struct opus *opus_t)
{
   //memset(opus_t->pcm_bytes, '\0', sizeof(opus_t->pcm_bytes));
   int frame_size = opus_decode(opus_t->decoder, opus_t->input, opus_t->nbBytes, opus_t->out, MAX_FRAME_SIZE, 0);
   if (frame_size<0)
   {
    printf("decoder failed: %s\n", opus_strerror(frame_size));
    return 0;
   }
   /* Convert to little-endian ordering.*/
   for(int i=0;i<OPUSCHANNELS*frame_size;i++)
   {
      opus_t->pcm_bytes[2*i]=opus_t->out[i]&0xFF;
      opus_t->pcm_bytes[2*i+1]=(opus_t->out[i]>>8)&0xFF;
#if VERBOSE
      printf("%o\t%o\t",opus_t->pcm_bytes[2*i], opus_t->pcm_bytes[2*i+1]);
      if(i%10 == 0)
         printf("\n");
#endif
   }
   return 1;
}

int initOpus(struct opus *opus_t)
{
   int err;
   opus_t->decoder = opus_decoder_create(SAMPLE_RATE, OPUSCHANNELS, &err);
   if (err<0)
   {
      printf("failed to create decoder: %s\n", opus_strerror(err));
      return 0;
   }
   return 1;
}
