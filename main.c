#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "opus.h"
#include <stdio.h>
//#include "marioTestenc.c"
#include "opusFile.c"
#include "opusTobi.h"


struct opus OpusInstanz = {NULL, NBBYTES, NULL, {}, {}};
struct frame FrameInstanz = {NULL, 0, 0};

volatile int NBbytes[] = {249,134,135,258,189,161,161,161,161,161,161,161,161,161,161,161,161,161,161,161,
                          104,114,207,175,181,162,184,161,161,161,161,145,177,161,161,157,165,152,137,193,
                          144,179,154,168,161,161,161,161,161,161,161,137,128,161,148,142,166};

int main()
{
   /*Holds the state of the encoder and decoder */

   uint_fast16_t loopcnt = 0;
   uint_fast16_t nbbytessum = 0;
   uint_fast16_t len = sizeof(NBbytes) / sizeof(NBbytes[0]);
   initOpus(&OpusInstanz);
   FrameInstanz.opus_t = &OpusInstanz;

   while (len>loopcnt)
   {
      OpusInstanz.input = marioTestenc_opus + nbbytessum;
      OpusInstanz.nbBytes = NBbytes[loopcnt];
      decodeOpusFrame(&OpusInstanz);
      nbbytessum += OpusInstanz.nbBytes;
      loopcnt++;
   }
   printf("nbbytessum:%d\tloopcnt:%d",nbbytessum, loopcnt);
   /*Destroy the encoder state*/
   opus_decoder_destroy(OpusInstanz.decoder);
   return EXIT_SUCCESS;
}
