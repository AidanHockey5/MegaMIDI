#ifndef YM2612_H_
#define YM2612_H_
#include <Arduino.h>

const int MAX_CHANNELS = 6;

typedef struct
{
    bool keyOn = false;
    uint8_t keyNumber = 0;
    uint8_t blockNumber = 0;
} Channel;

class YM2612
{
private:
    uint8_t _IC = 38;
    uint8_t _CS = 39;
    uint8_t _WR = 40;
    uint8_t _RD = 41;
    uint8_t _A0 = 42;
    uint8_t _A1 = 43;
public:
    YM2612();
    Channel channels[MAX_CHANNELS];
    uint8_t SetChannelOn(uint8_t key);
    uint8_t SetChannelOff(uint8_t key);
    void Reset();
    void send(unsigned char addr, unsigned char data, bool setA1=0);
};
#endif

//Notes
// DIGITAL BUS = PA0-PA7
// IC = F0/38
// CS = F1/39
// WR = F2/40
// RD = F3/41
// A0 = F4/42
// A1 = F5/43