#include <iostream>
#include "opus.h"
#include "opusfunc.h"
#include <errno.h>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include <stdio.h>
#include <stdlib.h>
// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <cstring>

#include <ctime>

/*The frame size is hardcoded for this sample code but it d../../../../usr/local/include/opusoesn't have to be*/
#define FRAME_SIZE 960     //240=5ms, 960=20ms bei 48kHz
#define SAMPLE_RATE 48000 //Sampling rate of input signal (Hz)
#define CHANNELS 1
#define APPLICATION OPUS_APPLICATION_VOIP      //OPUS_APPLICATION_VOIP//OPUS_APPLICATION_AUDIO
#define BITRATE 128000   //bitrate in bits per second 768000=48kHz, 384000=24kHz, 192000=12kHz, 128000=8kHz (500 to 512000 bits per second)
#define MAX_FRAME_SIZE 6*960
#define MAX_PACKET_SIZE (3*1276)
#define VBR 0        // VBR = 1, CBR = 0
#define BANDWIDTH OPUS_BANDWIDTH_SUPERWIDEBAND //OPUS_BANDWIDTH_SUPERWIDEBAND:12kHz OPUS_BANDWIDTH_FULLBAND: 20kHz

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

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
int initSerial(int *serial_port);
OpusEncoder *initOpusEnc(int *err);
int readEncodeFrame(OpusEncoder *encoder, FILE *fin, int *nbBytes, char *cbits);


FILE *openFile(string filename, string mode) //const char *
{
   FILE *file = fopen(filename.c_str(), mode.c_str());
   if (file==NULL)
   {
      printf("failed to open input file:\n");
      return NULL;
   }
   return file;
}


int main(int argc, char *argv[])
{
   FILE *fin, *fout, *foutWave;
   opus_int16 in[FRAME_SIZE*CHANNELS];
   opus_int16 out[MAX_FRAME_SIZE*CHANNELS];
   char cbits[MAX_PACKET_SIZE];
   int nbBytes;
   OpusEncoder *encoder;
   OpusDecoder *decoder;
   int err;
   stringstream tmp, tmp2;
   tmp << SAMPLE_RATE/1000;
   tmp2 << SAMPLE_RATE/1000 << "_" << BITRATE/16/1000 << mode;
   if ((homedir = getenv("HOME")) == NULL)
       homedir = getpwuid(getuid())->pw_dir;
   string inFile = homedir + path + filename + tmp.str() + "_11" + ".wav";
   string outFileoext = homedir + path + filename + tmp2.str();
   string outFile = outFileoext + ".opus";
   string outFileWave = outFileoext + ".wav";
   char NBbytes[10000];
   uint NBbytesCnt = 0;
   int loopcnt = 0;


   cout << "1: Communicate via UART" << endl;
   cout << "2: Send via Uart one Frame" << endl;
   cout << "3: Send via Uart continuously" << endl;
   cout << "5: Read wav File and generate C Array" << endl;
   cout << "9: Quitt" << endl;
   int branch;
   if(argc > 1)
      branch = atoi(argv[1]);
   else
      cin >> branch;

   switch(branch)
   {
   case 1:  /*Communicate via UART*/
   {
      std::string msg;
      int serial_port;
      if(!initSerial(&serial_port))
         return EXIT_FAILURE;
      do{
         cout << "Give any Message (q=quitt):" << endl;
         cin >> msg;
         msg.append(1, 0xff);
         msg.append(1, 0x01);
         const char* tmp = msg.c_str();
         int size = msg.length();
         write(serial_port, tmp, size);

         char read_buf [256];
         memset(&read_buf, '\0', sizeof(read_buf));
         int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
         // n is the number of bytes read. n may be 0 if no bytes were received, and can also be -1 to signal an error.
         if (num_bytes < 0) {
             printf("Error reading: %s", strerror(errno));
         }
         printf("Read %i bytes. Received message: %s", num_bytes, read_buf);
      }while(msg[0] != 'q');
      close(serial_port);
      break;
   }
   case 2:/*Send via Uart one Frame*/
   {
      fin = fopen(inFile.c_str(), "r");
      if (fin==NULL)
      {
         printf("failed to open input file:\n");
         return EXIT_FAILURE;
      }
      encoder = initOpusEnc(&err);
      if(err < 0 || encoder == NULL)
         return EXIT_FAILURE;
      readEncodeFrame(encoder, fin, &nbBytes, cbits);
      u_int16_t headerlength = 0;
      char *header = getOpusPacketHeader(OPUSPACKETPERREQUEST, &nbBytes, &headerlength);
      int serial_port;
      if(!initSerial(&serial_port))
         return EXIT_FAILURE;
      int payloadSize = (nbBytes + HEADERMEMSYZE(OPUSPACKETPERREQUEST)) * sizeof(char);
      char payload[payloadSize];
      memset(payload, '\0', payloadSize);
      for(int i=0; i<payloadSize; i++){
          if (i<HEADERMEMSYZE(OPUSPACKETPERREQUEST))
              payload[i]=header[i];
          else
              payload[i] = cbits[i-HEADERMEMSYZE(OPUSPACKETPERREQUEST)];
      }
      write(serial_port, payload, payloadSize);
      sleep(1);         char read_buf [256];
      memset(&read_buf, '\0', sizeof(read_buf));
      int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));

      opus_encoder_destroy(encoder);
      fclose(fin); 
      close(serial_port);
      break;
   }
   case 3:/*Send via Uart continuously*/
   {
      int serial_port;
      double elapsed_secs;
      fin = fopen(inFile.c_str(), "r");
      if (fin==NULL)
      {
         printf("failed to open input file:\n");
         return EXIT_FAILURE;
      }
      encoder = initOpusEnc(&err);
      if(err < 0 || encoder == NULL)
         return EXIT_FAILURE;
      do{
         clock_t begin = clock();
         if(!readEncodeFrame(encoder, fin, &nbBytes, cbits))
            break;
         u_int16_t headerlength = 0;
         char *header = getOpusPacketHeader(OPUSPACKETPERREQUEST, &nbBytes, &headerlength);
         if(!initSerial(&serial_port))
            return EXIT_FAILURE;

         elapsed_secs = double(clock() - begin) / CLOCKS_PER_SEC;
         cout << "initSerial: " << elapsed_secs << endl;

         int payloadSize = (nbBytes + HEADERMEMSYZE(OPUSPACKETPERREQUEST)) * sizeof(char);
         char payload[payloadSize];
         memset(payload, '\0', payloadSize);
         for(int i=0; i<payloadSize; i++)
         {
             if (i<HEADERMEMSYZE(OPUSPACKETPERREQUEST))
                 payload[i]= header[i];
             else
                 payload[i] = cbits[i-HEADERMEMSYZE(OPUSPACKETPERREQUEST)];
         }
         elapsed_secs = double(clock() - begin) / CLOCKS_PER_SEC;
         cout << "bevore write: " << elapsed_secs << endl;

         int writtenBytes = write(serial_port, payload, payloadSize);

         elapsed_secs = double(clock() - begin) / CLOCKS_PER_SEC;
         cout << "write: " << elapsed_secs << endl;
         cout << "payloadSize: " << payloadSize << endl; //endl ist wichtig sonst wird zeile nicht direkt ausgegeben
         char read_buf [6];
         memset(&read_buf, '\0', sizeof(read_buf));
         elapsed_secs = double(clock() - begin) / CLOCKS_PER_SEC;
         cout << "before read: " << elapsed_secs << endl;
         do{
            int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
            if(num_bytes>0)
               cout << "\t" << read_buf[0] << read_buf[1] << read_buf[2] << read_buf[3] << read_buf[4]<< "\t" << read_buf[5] << endl;

         }while(read_buf[0] != 'r');
         loopcnt++;
         clock_t end = clock();
         elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
         cout  << ANSI_COLOR_RED << "looptime: " << elapsed_secs << ANSI_COLOR_RESET << endl;
      }while(1);
      sleep(1);
      opus_encoder_destroy(encoder);
      fclose(fin);
      close(serial_port);
      break;
   }
   case 5:  /*Read wav File and generate C Array*/
   {

      encoder = initOpusEnc(&err);
      if(err < 0 || encoder == NULL)
         return EXIT_FAILURE;

      fin = fopen(inFile.c_str(), "r");
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
      fout = fopen(outFile.c_str(), "w");
      if (fout==NULL)
      {
         printf("failed to open output file:\n");
         return EXIT_FAILURE;
      }
      foutWave = fopen(outFileWave.c_str(), "w");
      if (fout==NULL)
      {
         printf("failed to open outFileWave:\n");
         return EXIT_FAILURE;
      }

      while (1)
      {
         int i;
         unsigned char pcm_bytes[MAX_FRAME_SIZE*CHANNELS*2];
         int frame_size;

         /* Read a 16 bits/sample audio frame. */
         fread(pcm_bytes, sizeof(short)*CHANNELS, FRAME_SIZE, fin);
         if (feof(fin))
         {
            char cstr[] = {'}', ';','\0'};
            sprintf(NBbytes, "%s%s", NBbytes, cstr);
            NBbytesCnt+=sizeof(cstr);
            break;
         }

         /* Convert from little-endian ordering. */ //Baut sich aus 1d 3f eine 16Bit Zahl
         for (i=0;i<CHANNELS*FRAME_SIZE;i++)
            in[i]=pcm_bytes[2*i+1]<<8|pcm_bytes[2*i];

         /* Encode the frame. */
         nbBytes = opus_encode(encoder, in, FRAME_SIZE, reinterpret_cast<unsigned char*>(cbits), MAX_PACKET_SIZE);
         if (nbBytes<0)
         {
            fprintf(stderr, "encode failed: %s\n", opus_strerror(nbBytes));
            return EXIT_FAILURE;
         }
         loopcnt++;
         //printf("%d,", nbBytes);
         string nBytes;
         if(loopcnt == 1)
            nBytes = "int NBbytes[] = {" + to_string(nbBytes);
         else
            if(loopcnt%20 != 0)
               nBytes = ", " + to_string(nbBytes);
            else
               nBytes = ",\n" + to_string(nbBytes);


         sprintf(NBbytes, "%s%s", NBbytes, nBytes.c_str());
         NBbytesCnt+=nBytes.length();

         fwrite(cbits, sizeof(char), nbBytes, fout);


         frame_size = opus_decode(decoder, reinterpret_cast<unsigned char*>(cbits), nbBytes, out, MAX_FRAME_SIZE, 0);
         if (frame_size<0)
         {
            fprintf(stderr, "decoder failed: %s\n", opus_strerror(frame_size));
            return EXIT_FAILURE;
         }
         if(loopcnt == 1)
         {  //Byte 24,25 ist Sample Rate
            i=27;
            //pcm_bytes[24]=0x40;
            //pcm_bytes[25]=0x1F;
         }
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
         fwrite(pcm_bytes, sizeof(short), frame_size*CHANNELS, foutWave);
      }
      /*Destroy the encoder state*/
      opus_encoder_destroy(encoder);
      opus_decoder_destroy(decoder);
      fclose(fin);
      fclose(fout);
      fclose(foutWave);
      string cmd = "xxd -i " + outFile + " " + outFileoext + ".c" ;
      system(cmd.c_str());

      string readmeFile = outFileoext + ".c";
      fout = fopen(readmeFile.c_str(), "a");
      if (fout==NULL)
      {
         printf("failed to open ReadmeFile file:\n");
         return EXIT_FAILURE;
      }
      fwrite(NBbytes, sizeof(char), NBbytesCnt-1, fout);
      //fwrite(to_string(NBbytesCnt).c_str(), sizeof(char), sizeof(to_string(NBbytesCnt).c_str())/sizeof(to_string(NBbytesCnt).c_str()[0]), freadme);    //write NBbytes count
      fclose(fout);
      cmd.clear();
      //cmd = "rm " + outFile;
      system(cmd.c_str());
      break;
   }
   case 9:
   {
      return EXIT_SUCCESS;
      break;
   }
   default:
   {
      cout << "wrong Input" << endl;
   }


   }

}

int initSerial(int *serial_port)
{
   /*https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/*/
   *serial_port = open("/dev/ttyACM0", O_RDWR);

   // Check for errors
   if (*serial_port < 0) {
       printf("Error %i from open: %s\n", errno, strerror(errno));
       return 0;
   }
   // Create new termios struc, we call it 'tty' for convention
   struct termios tty;
   memset(&tty, 0, sizeof tty);

   // Read in existing settings, and handle any error
   if(tcgetattr(*serial_port, &tty) != 0) {
       printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
       return 0;
   }

   tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
   //tty.c_cflag |= PARENB;  // Set parity bit, enabling parity
   tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
   //tty.c_cflag |= CSTOPB;  // Set stop field, two stop bits used in communication
   tty.c_cflag |= CS8; // 8 bits per byte (most common)
   tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
   //tty.c_cflag |= CRTSCTS;  // Enable RTS/CTS hardware flow control
   tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
   tty.c_lflag &= ~ICANON;
   tty.c_lflag &= ~ECHO; // echo
   tty.c_lflag &= ~ECHOE; // Disable erasure
   tty.c_lflag &= ~ECHONL; // Disable new-line echo
   tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
   tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
   tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

   tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
   tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
   // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT IN LINUX)
   // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT IN LINUX)
   tty.c_cc[VTIME] = 0;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
   tty.c_cc[VMIN] = 0;

   // Set in/out baud rate to B0,  B50,  B75,  B110,  B134,  B150,  B200, B300, B600, B1200, B1800, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800
   cfsetispeed(&tty, B460800);
   cfsetospeed(&tty, B460800);

   // Save tty settings, also checking for error
   if (tcsetattr(*serial_port, TCSANOW, &tty) != 0) {
       printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
       return 0;
   }

   return 1;
}

OpusEncoder *initOpusEnc(int *err)
{
   OpusEncoder *encoder;
   //int err;
   /*Create a new encoder state */
   encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, APPLICATION, err);   //SAMPLE_RATE: Sampling rate of input signal (Hz)
   if (*err<0)
   {
      printf("failed to create an encoder: %s\n", opus_strerror(*err));
      return NULL;
   }
   /* Set the desired bit-rate. You can also set other parameters if needed.
      The Opus library is designed to have good defaults, so only set
      parameters you know you need. Doing otherwise is likely to result
      in worse quality, but better. */
   *err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE));                //BITRATE: bitrate in bits per second
   if (*err<0)
   {
      printf("failed to set bitrate: %s\n", opus_strerror(*err));
      return NULL;
   }
   *err = opus_encoder_ctl(encoder, OPUS_SET_VBR(VBR));                //None variable Bitrate
   if (*err<0)
   {
      printf("failed to set bitrate: %s\n", opus_strerror(*err));
      return NULL;
   }
   *err = opus_encoder_ctl(encoder, OPUS_SET_BANDWIDTH(BANDWIDTH));                //None variable Bitrate
   if (*err<0)
   {
      printf("failed to set bitrate: %s\n", opus_strerror(*err));
      return NULL;
   }
   return encoder;
}
int readEncodeFrame(OpusEncoder *encoder, FILE *fin, int *nbBytes, char *cbits)
{
   char pcm_bytes[MAX_FRAME_SIZE*CHANNELS*2];
   opus_int16 in[FRAME_SIZE*CHANNELS];
   /* Read a 16 bits/sample audio frame. */
   fread(pcm_bytes, sizeof(short)*CHANNELS, FRAME_SIZE, fin);
   if (feof(fin))
   {
      //char cstr[] = {'}', ';','\0'};
      //sprintf(NBbytes, "%s%s", NBbytes, cstr);
      return 0;
   }

   opus_int16 eins = (pcm_bytes[25]<<8)|(pcm_bytes[24]&0xff);
   opus_int16 zei = (pcm_bytes[25]<<8);

   /* Convert from little-endian ordering. */ //Baut sich aus 1d 3f eine 16Bit Zahl
   for (int i=0;i<CHANNELS*FRAME_SIZE;i++)
      in[i]=(pcm_bytes[2*i+1]<<8)|(pcm_bytes[2*i]&0xff);

   /* Encode the frame. */
   *nbBytes = opus_encode(encoder, in, FRAME_SIZE, reinterpret_cast<unsigned char*>(cbits), MAX_PACKET_SIZE);
   if (*nbBytes<0)
   {
      fprintf(stderr, "encode failed: %s\n", opus_strerror(*nbBytes));
      return 0;
   }
   return 1;

}
