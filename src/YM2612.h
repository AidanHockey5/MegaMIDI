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
    uint8_t _IC = 10;
    uint8_t _CS = 11;
    uint8_t _WR = 12;
    uint8_t _RD = 13;
    uint8_t _A0 = 14;
    uint8_t _A1 = 15;
    typedef struct
    {
        bool keyOn = false;
        bool sustained = false;
        uint8_t keyNumber = 0;
        uint8_t blockNumber = 0;
        uint8_t index = 0;
    } Channel;
    uint8_t lfoFrq = 0;
    uint8_t lfoSens = 7;
    int8_t octaveShift = 0;
    unsigned char bank0[0xB7-0x21]; //Shadow registers
    unsigned char bank1[0xB7-0x30];
    Voice currentVoice;
public:
    YM2612();
    Channel channels[MAX_CHANNELS_YM];
    bool lfoOn = false;
    int8_t GetOctaveShift();
    void SetOctaveShift(int8_t shift);
    void SetChannelOn(uint8_t key, uint8_t velocity, bool velocityEnabled);
    void SetChannelOff(uint8_t key);
    void SetVoice(Voice v);
    float NoteToFrequency(uint8_t note);
    void SetFrequency(uint16_t frequency, uint8_t channel);
    void AdjustLFO(uint8_t value);
    void AdjustPitch(uint8_t channel, int pitch);
    void ReleaseSustainedKeys();
    void ClampSustainedKeys();
    uint16_t CalcFNumber(float note);
    void ShiftOctaveUp();
    void ShiftOctaveDown();
    void ToggleLFO();
    void Reset();
    void send(unsigned char addr, unsigned char data, bool setA1=0);
    void DumpShadowRegisters();
    uint8_t GetShadowValue(uint8_t addr, bool bank);
};
#endif

//Notes
// DIGITAL BUS = PF0-PF7
// IC = PC0/10
// CS = PC1/11
// WR = PC2/12
// RD = PC3/13
// A0 = PC4/14
// A1 = PC5/15