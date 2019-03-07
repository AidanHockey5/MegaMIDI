#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "Voice.h"

static const unsigned char leds[] = {1, 3, 4, 5, 6, 7, 24, 27};
static bool psgReady = false;
typedef struct 
{
    Voice v;
    unsigned char index = 0xFF;
} FavoriteVoice;


#endif