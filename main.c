#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "opus.h"
#include <stdio.h>
#include "opusTobi.h"






uint_fast16_t len = 57;//sizeof(NBbytes) / sizeof(NBbytes[0]);



int main()
{
   /*Holds the state of the encoder and decoder */
   uint_fast8_t newFrame = 0;
   struct opus OpusInstanz = {NULL, NBBYTES, NULL, {}, {}};
   struct frame FrameInstanz = {&OpusInstanz, 0, 0};

   initOpus(&OpusInstanz);
   //FrameInstanz.opus_t = &OpusInstanz;

   //while ()
   while(1)
   {
      if(newFrame)
      {
         getPcm(&FrameInstanz);
         printf("pcm:%o", OpusInstanz.pcm_bytes[0]);
      }

      if (1)//(getchar() == 'g')
      {
        newFrame = 1;
      }

      if(len-1<FrameInstanz.loopcnt)
         break;
   }
   printf("nbbytessum:%d\tloopcnt:%d", FrameInstanz.nbbytessum, FrameInstanz.loopcnt);
   /*Destroy the encoder state*/
   opus_decoder_destroy(OpusInstanz.decoder);
   return EXIT_SUCCESS;
}
