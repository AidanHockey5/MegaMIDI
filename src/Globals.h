#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "Voice.h"

#define MIDI_MFG_ID 0xFF
#define MIDI_DEVICE_ID 0x05

static const unsigned char leds[] = {1, 3, 4, 5, 6, 7, 24, 27};
extern bool YMsustainEnabled;
extern bool PSGsustainEnabled;
typedef struct 
{
    Voice v;
    unsigned char index = 0xFF; //Use this as a simple way to check EEPROM for valid saved voice
    char fileName[20+1]; //+1 for null terminator
    unsigned char voiceNumber = 0;
    signed char octaveShift = 0;
} FavoriteVoice;

enum OperationMode
{
    STANDALONE, VST
};

static OperationMode operationMode = STANDALONE;

#endif