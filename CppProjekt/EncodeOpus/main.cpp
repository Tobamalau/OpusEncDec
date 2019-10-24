#include <iostream>
#include "opus.h"
#include <errno.h>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <cstring>

/*The frame size is hardcoded for this sample code but it d../../../../usr/local/include/opusoesn't have to be*/
#define FRAME_SIZE 960
#define SAMPLE_RATE 48000 //Sampling rate of input signal (Hz)
#define CHANNELS 1
#define APPLICATION OPUS_APPLICATION_AUDIO
#define BITRATE 384000   //bitrate in bits per second 768000=48kHz, 384000=24kHz, 192000=12kHz, 128000=8kHz (500 to 512000 bits per second)
#define MAX_FRAME_SIZE 6*960
#define MAX_PACKET_SIZE (3*1276)
#define VBR 1        // VBR = 0, CBR = 1

using namespace std;

string pathBashSkript = "/src/tools/OpusEncDec/CppProjekt/makecarray.sh";
string path = "/src/tools/OpusEncDec/MusikRohdateien/";
string filename = "youtube";
#if VBR
   string mode = "_vbr";
#else
   string mode = "_cbr";
#endif

const char *homedir;




int main()
{

   if ((homedir = getenv("HOME")) == NULL) {
       homedir = getpwuid(getuid())->pw_dir;
   }

   FILE *fin;
   FILE *fout;
   FILE *freadme;
   opus_int16 in[FRAME_SIZE*CHANNELS];
   opus_int16 out[MAX_FRAME_SIZE*CHANNELS];
   unsigned char cbits[MAX_PACKET_SIZE];
   int nbBytes;
   /*Holds the state of the encoder and decoder */
   OpusEncoder *encoder;
   OpusDecoder *decoder;
   int err;

   /*Create a new encoder state */
   encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, APPLICATION, &err);   //SAMPLE_RATE: Sampling rate of input signal (Hz)
   if (err<0)
   {
      printf("failed to create an encoder: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }
   /* Set the desired bit-rate. You can also set other parameters if needed.
      The Opus library is designed to have good defaults, so only set
      parameters you know you need. Doing otherwise is likely to result
      in worse quality, but better. */
   err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE));                //BITRATE: bitrate in bits per second
   if (err<0)
   {
      printf("failed to set bitrate: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }
   err = opus_encoder_ctl(encoder, OPUS_SET_VBR(VBR));                //None variable Bitrate
   if (err<0)
   {
      printf("failed to set bitrate: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }
   {
   stringstream tmp;
   tmp << SAMPLE_RATE/1000;
   string inFile = homedir + path + filename + tmp.str() + "_11" + ".wav";
   fin = fopen(inFile.c_str(), "r");
   }
   if (fin==NULL)
   {
      printf("failed to open input file:\n");

      return EXIT_FAILURE;
   }
   /* Create a new decoder state. */
   decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
   if (err<0)
   {
      printf("failed to create decoder:\n");
      return EXIT_FAILURE;
   }

   stringstream tmp;
   tmp << SAMPLE_RATE/1000 << "_" << BITRATE/16/1000 << mode;
   string gg = tmp.str();
   string outFileoext = homedir + path + filename + tmp.str(); //"/home/tobi/src/opusTrivial_example/MusikRohdateien/youtube2412_11_cbr.opus";
   string outFile = outFileoext + ".opus";
   fout = fopen(outFile.c_str(), "w");

   if (fout==NULL)
   {
      printf("failed to open output file:\n");
      return EXIT_FAILURE;
   }
   char NBbytes[10000];
   uint NBbytesCnt = 0;
   int loopcnt = 0;
   while (1)
   {
      int i;
      unsigned char pcm_bytes[MAX_FRAME_SIZE*CHANNELS*2];
      int frame_size;

      /* Read a 16 bits/sample audio frame. */
      fread(pcm_bytes, sizeof(short)*CHANNELS, FRAME_SIZE, fin);
      if (feof(fin))
      {
         char cstr[] = {'}','\0'};
         sprintf(NBbytes, "%s%s", NBbytes, cstr);
         NBbytesCnt+=sizeof(cstr);
         break;
      }
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
      //printf("%d,", nbBytes);
      string nBytes;
      if(loopcnt == 1)
         nBytes = "int NBbytes[] = {" + to_string(nbBytes);
      else
         nBytes = "; " + to_string(nbBytes);

      char cstr[nBytes.size() + 1];
      sprintf(NBbytes, "%s%s", NBbytes, nBytes.c_str());
      NBbytesCnt+=nBytes.length();

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
   }
   /*Destroy the encoder state*/
   opus_encoder_destroy(encoder);
   opus_decoder_destroy(decoder);
   fclose(fin);
   fclose(fout);
   string cmd = "xxd -i " + outFile + " " + outFileoext + ".c" ;
   system(cmd.c_str());

   string readmeFile = outFileoext + ".c";
   freadme = fopen(readmeFile.c_str(), "a");
   if (freadme==NULL)
   {
      printf("failed to open ReadmeFile file:\n");
      return EXIT_FAILURE;
   }
   fwrite(NBbytes, sizeof(char), NBbytesCnt-1, freadme);
   fclose(freadme);
   return EXIT_SUCCESS;
}
