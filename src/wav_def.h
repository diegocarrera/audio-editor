/******************* FILE : wav_def.h ********************/



/****** NOTICE: LIMITATIONS ON USE AND DISTRIBUTION ********\
  This software is provided on an as-is basis, it may be
  distributed in an unlimited fashion without charge provided
  that it is used for scholarly purposes - it is not for
  commercial use or profit. This notice must remain unaltered.

  Software by Dr Fred DePiero - CalPoly State University

\******************** END OF NOTICE ************************/



// header of wav file
typedef struct{
   char rID[4];               // 'RIFF'   //4 bits
   long int rLen;                      //4 bits
      
   char wID[4];               // 'WAVE'  4bits
      
   char fId[4];               // 'fmt '   4 bits
   long int pcm_header_len;   // varies... 4 bits
   short int wFormatTag;      //2bits
   short int nChannels;       // 1,2 for stereo data is (l,r) pairs,   2bits
   long int nSamplesPerSec;   // 4 bits (SampleRate)
   long int nAvgBytesPerSec;  //ByteRate, 4 bits
   short int nBlockAlign;     //BlockAlign, 2bits    
   short int nBitsPerSample;  //2bits
}   WAV_HDR;

   
// header of wav file
typedef struct{
   char dId[4];            // 'data' or 'fact', 4 bits (36-40)
   long int dLen;          // 4 bits (40-44)
   //unsigned char *data;    
}   CHUNK_HDR;


 
