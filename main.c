#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "opus.h"
#include <stdio.h>
#include "opusTobi.h"



int main()
{
   /*Holds the state of the encoder and decoder */
   uint_fast8_t newFrame = 1;
   struct opus OpusInstanz = {NULL, NBBYTES, NULL, {}, {}};
   struct frame FrameInstanz = {&OpusInstanz, 0, 0};
   uint8_t bufferNr = 0;

   initOpusFrame(&FrameInstanz);

   getPcm(&FrameInstanz, bufferNr);
   bufferNr ^= (1 << 0);

   //while ()
   while(1)
   {
      if(newFrame)
      {

         getPcm(&FrameInstanz, bufferNr);
         //printf("pcm:%o", OpusInstanz.pcm_bytes[0]);
         newFrame = 0;

      }

      if (getchar() == 'g')
      {
         newFrame = 1;
         bufferNr ^= (1 << 0);
      }

      if(FrameInstanz.nbbytescnt-1<FrameInstanz.loopcnt)
         break;
   }
   printf("nbbytessum:%ld\tloopcnt:%ld", FrameInstanz.nbbytessum, FrameInstanz.loopcnt);
   /*Destroy the encoder state*/
   opus_decoder_destroy(OpusInstanz.decoder);
   return EXIT_SUCCESS;
}
