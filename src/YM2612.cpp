#include "YM2612.h"

YM2612::YM2612()
{
    DDRA = 0xFF;
    PORTA = 0x00;
    DDRF = 0xFF;
    PORTF = 0x0F; //_A1 LOW, _A0 LOW, _IC HIGH, _WR HIGH, _RD HIGH, _CS HIGH
}

void YM2612::Reset()
{
    digitalWriteFast(_IC, LOW);  //_IC HIGH
    delayMicroseconds(25);
    digitalWriteFast(_IC, HIGH); //_IC HIGH
}

void YM2612::send(unsigned char addr, unsigned char data, bool setA1) //0x52 = A1 LOW, 0x53 = A1 HIGH
{
    digitalWriteFast(_A1, setA1);
    digitalWriteFast(_A0, LOW);
    digitalWriteFast(_CS, LOW);
    PORTA = addr;
    digitalWriteFast(_WR, LOW);
    delayMicroseconds(1);
    digitalWriteFast(_WR, HIGH);
    digitalWriteFast(_CS, HIGH);
    digitalWriteFast(_A0, HIGH);
    digitalWriteFast(_CS, LOW);
    PORTA = data;
    digitalWriteFast(_WR, LOW);
    delayMicroseconds(1);
    digitalWriteFast(_WR, HIGH);
    digitalWriteFast(_CS, HIGH);
}

//Notes
// DIGITAL BUS = PA0-PA7
// IC = F0/38
// CS = F1/39
// WR = F2/40
// RD = F3/41
// A0 = F4/42
// A1 = F5/43