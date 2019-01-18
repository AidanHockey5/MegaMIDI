#ifndef SN76489_H_
#define SN76489_H_
#include <Arduino.h>
#include "PSGNotes.h"

const int MAX_CHANNELS_PSG = 3;

class SN76489
{
private:
    uint8_t _WE = 44;
    typedef struct
    {
        bool keyOn = false;
        uint8_t keyNumber = 0;
    } Channel;
public:
    SN76489();
    Channel channels[MAX_CHANNELS_PSG];
    uint16_t GetFrequencyFromLUT(uint8_t key);
    void SetChannelOn(uint8_t key);
    void SetChannelOff(uint8_t key);
    void SetVolume(uint8_t channel, uint8_t volume);
    void SetFrequency(uint8_t key, uint8_t channel);
    void Reset();
    void send(uint8_t data);
};
#endif

//Notes:
//PSG /WE on F6