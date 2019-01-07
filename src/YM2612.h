#ifndef YM2612_H_
#define YM2612_H_
#include <Arduino.h>
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