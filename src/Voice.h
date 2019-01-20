#ifndef VOICE_H_
#define VOICE_H_
#define MAX_VOICES 16
//Voice data
static unsigned char currentProgram = 0;
static unsigned char maxValidVoices = 0;

//OPM File Format https://vgmrips.net/wiki/OPM_File_Format
typedef struct
{
  unsigned char LFO[5];
  unsigned char CH[7];
  unsigned char M1[11];
  unsigned char C1[11];
  unsigned char M2[11];
  unsigned char C2[11];
} Voice;

static Voice voices[MAX_VOICES];

#endif