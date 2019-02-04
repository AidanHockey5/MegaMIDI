#ifndef YM2612_H_
#define YM2612_H_
#include <Arduino.h>
#include "Adjustments.h"
#include "Voice.h"
#define mask(s) (~(~0<<s))
const int MAX_CHANNELS_YM = 6;

class YM2612
{
private:
    uint8_t _IC = 38;
    uint8_t _CS = 39;
    uint8_t _WR = 40;
    uint8_t _RD = 41;
    uint8_t _A0 = 42;
    uint8_t _A1 = 43;
    typedef struct
    {
        bool keyOn = false;
        uint8_t keyNumber = 0;
        uint8_t blockNumber = 0;
    } Channel;
    uint8_t lfoFrq = 0;
    uint8_t lfoSens = 7;
    uint8_t octaveShift = 0;
    Voice currentVoice;
public:
    YM2612();
    Channel channels[MAX_CHANNELS_YM];
    bool lfoOn = false;
    void SetChannelOn(uint8_t key, uint8_t velocity);
    void SetChannelOff(uint8_t key);
    void SetVoice(Voice v);
    float NoteToFrequency(uint8_t note);
    void SetFrequency(uint16_t frequency, uint8_t channel);
    void AdjustLFO(uint8_t value);
    void AdjustPitch(uint8_t channel, int pitch);
    uint16_t CalcFNumber(float note);
    void ShiftOctaveUp();
    void ShiftOctaveDown();
    void ToggleLFO();
    void Reset();
    void send(unsigned char addr, unsigned char data, bool setA1=0);
};
#endif

//Notes
// DIGITAL BUS = PC0-PC7
// IC = F0/38
// CS = F1/39
// WR = F2/40
// RD = F3/41
// A0 = F4/42
// A1 = F5/43