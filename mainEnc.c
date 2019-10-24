#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "opus.h"
#include <stdio.h>


/*The frame size is hardcoded for this sample code but it d../../../../usr/local/include/opusoesn't have to be*/
#define FRAME_SIZE 960
#define SAMPLE_RATE 24000 //Sampling rate of input signal (Hz)
#define CHANNELS 1
#define APPLICATION OPUS_APPLICATION_AUDIO
#define BITRATE 192000   //bitrate in bits per second 768000=48kHz, 384000=24kHz, 192000=12kHz (500 to 512000 bits per second)
#define MAX_FRAME_SIZE 6*960
#define MAX_PACKET_SIZE (3*1276)


char Path[] = "/home/tobi/src/opusTrivial_example/MusikRohdateien/";
char Filename[] = "youtube24_11.wav";

char *my_itoa(int wert, int laenge);

int main(int argc, char **argv)
{
   char *inFile;
   FILE *fin;
   char *outFile;
   FILE *fout;
   opus_int16 in[FRAME_SIZE*CHANNELS];
   opus_int16 out[MAX_FRAME_SIZE*CHANNELS];
   unsigned char cbits[MAX_PACKET_SIZE];
   int nbBytes;
   /*Holds the state of the encoder and decoder */
   OpusEncoder *encoder;
   OpusDecoder *decoder;
   int err;

   char buffer[2];
   printf("Binary value = %s\n", buffer);

   if (!argc)
   {
      printf("usage: trivial_example input.pcm output.pcm\n");
      fprintf(stderr, "input and output are 16-bit little-endian raw files\n");
      return EXIT_FAILURE;
   }

   /*Create a new encoder state */
   encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, APPLICATION, &err);   //SAMPLE_RATE: Sampling rate of input signal (Hz)
   if (err<0)
   {
      fprintf(stderr, "failed to create an encoder: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }
   /* Set the desired bit-rate. You can also set other parameters if needed.
      The Opus library is designed to have good defaults, so only set
      parameters you know you need. Doing otherwise is likely to result
      in worse quality, but better. */
   err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE));                //BITRATE: bitrate in bits per second
   if (err<0)
   {
      fprintf(stderr, "failed to set bitrate: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }
   err = opus_encoder_ctl(encoder, OPUS_SET_VBR(0));                //None variable Bitrate
   if (err<0)
   {
      fprintf(stderr, "failed to set bitrate: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }

   inFile = strcat(Path, Filename);
   fin = fopen(inFile, "r");
   if (fin==NULL)
   {
      fprintf(stderr, "failed to open input file: %s\n", strerror(errno));

      return EXIT_FAILURE;
   }
   /* Create a new decoder state. */
   decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
   if (err<0)
   {
      fprintf(stderr, "failed to create decoder: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }
   //outFile = "/home/tobi/src/opusTrivial_example/MarioTestEncDec.wav";
   //outFile = "/home/tobi/src/opusTrivial_example/marioTestenc.opus";
   outFile = "/home/tobi/src/opusTrivial_example/MusikRohdateien/youtube2412_11_cbr.opus";
   fout = fopen(outFile, "w");
   if (fout==NULL)
   {
      fprintf(stderr, "failed to open output file: %s\n", strerror(errno));
      return EXIT_FAILURE;
   }
   int loopcnt = 0;
   while (1)
   {
      int i;
      unsigned char pcm_bytes[MAX_FRAME_SIZE*CHANNELS*2];
      int frame_size;

      /* Read a 16 bits/sample audio frame. */
      fread(pcm_bytes, sizeof(short)*CHANNELS, FRAME_SIZE, fin);
      if (feof(fin))
         break;
      loopcnt++;
      /* Convert from little-endian ordering. */ //Baut sich aus 1d 3f eine 16Bit Zahl
      for (i=0;i<CHANNELS*FRAME_SIZE;i++)
         in[i]=pcm_bytes[2*i+1]<<8|pcm_bytes[2*i];

      /* Encode the frame. */
      nbBytes = opus_encode(encoder, in, FRAME_SIZE, cbits, MAX_PACKET_SIZE);
      if (nbBytes<0)
      {
         fprintf(stderr, "encode failed: %s\n", opus_strerror(nbBytes));
         return EXIT_FAILURE;
      }
      printf("%d,", nbBytes);
      /*int bandwith = opus_packet_get_bandwidth(cbits);
      int nb_channels = opus_packet_get_nb_channels(cbits);
      int nb_frames = opus_packet_get_nb_frames(cbits, nbBytes);*/
      fwrite(cbits, sizeof(char), nbBytes, fout);


      /* Decode the data. In this example, frame_size will be constant because
         the encoder is using a constant frame size. However, that may not
         be the case for all encoders, so the decoder must always check
         the frame size returned. */
      frame_size = opus_decode(decoder, cbits, nbBytes, out, MAX_FRAME_SIZE, 0);
      if (frame_size<0)
      {
         fprintf(stderr, "decoder failed: %s\n", opus_strerror(frame_size));
         return EXIT_FAILURE;
      }
      if(loopcnt == 1)
         i=27;
      else
         i=0;
      /* Convert to little-endian ordering. */
      for(i;i<CHANNELS*frame_size;i++)
      {
         pcm_bytes[2*i]=out[i]&0xFF;
         pcm_bytes[2*i+1]=(out[i]>>8)&0xFF;
      }
      i=0;
      /* Write the decoded audio to file. */
      //fwrite(pcm_bytes, sizeof(short), frame_size*CHANNELS, fout);
   }
   /*Destroy the encoder state*/
   opus_encoder_destroy(encoder);
   opus_decoder_destroy(decoder);
   fclose(fin);
   fclose(fout);
   return EXIT_SUCCESS;
}


char *my_itoa(int wert, int laenge) {
   char *ret =(char *) malloc(laenge+1 * sizeof(char));
   int i;

   for(i  =0; i < laenge; i++) {
      ret[laenge-i-1] = (wert % 10) + 48;
      wert = wert / 10;
   }
   ret[laenge]='\0';
   return ret;
}
