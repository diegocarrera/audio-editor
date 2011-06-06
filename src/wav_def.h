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
   char rID[4];            // 'RIFF'
   long int rLen;
      
   char wID[4];            // 'WAVE'
      
   char fId[4];            // 'fmt '
   long int pcm_header_len;   // varies...
   short int wFormatTag;
   short int nChannels;      // 1,2 for stereo data is (l,r) pairs
   long int nSamplesPerSec;
   long int nAvgBytesPerSec;
   short int nBlockAlign;      
   short int nBitsPerSample;
}   WAV_HDR;

   
// header of wav file
typedef struct{
   char dId[4];            // 'data' or 'fact'
   long int dLen;
//   unsigned char *data;
}   CHUNK_HDR;


 
