#include "YM2612.h"

YM2612::YM2612()
{
    DDRC = 0xFF;
    PORTC = 0x00;
    DDRF = 0xFF;
    PORTF |= 0x3C; //_A1 LOW, _A0 LOW, _IC HIGH, _WR HIGH, _RD HIGH, _CS HIGH
}

void YM2612::Reset()
{
    digitalWriteFast(_IC, LOW);  //_IC HIGH
    delayMicroseconds(25);
    digitalWriteFast(_IC, HIGH); //_IC HIGH
}

void YM2612::send(unsigned char addr, unsigned char data, bool setA1)
{
    digitalWriteFast(_A1, setA1);
    digitalWriteFast(_A0, LOW);
    digitalWriteFast(_CS, LOW);
    PORTC = addr;
    digitalWriteFast(_WR, LOW);
    delayMicroseconds(1);
    digitalWriteFast(_WR, HIGH);
    digitalWriteFast(_CS, HIGH);
    digitalWriteFast(_A0, HIGH);
    digitalWriteFast(_CS, LOW);
    PORTC = data;
    digitalWriteFast(_WR, LOW);
    delayMicroseconds(1);
    digitalWriteFast(_WR, HIGH);
    digitalWriteFast(_CS, HIGH);
    digitalWriteFast(_A1, LOW);
    digitalWriteFast(_A0, LOW);
}

uint8_t YM2612::SetChannelOn(uint8_t key)
{
    for(int i = 0; i<MAX_CHANNELS_YM; i++)
    {
        if(!channels[i].keyOn)
        {
            // if(channels[i].keyNumber == key)
            //     continue;
            channels[i].keyOn = true;
            channels[i].keyNumber = key;
            channels[i].blockNumber = key/12;
            return i;
        }
        
    }
    return 0xFF;
}

uint8_t YM2612::SetChannelOff(uint8_t key)
{
    for(int i = 0; i<MAX_CHANNELS_YM; i++)
    {
        if(channels[i].keyNumber == key)
        {
            channels[i].keyOn = false;
            return i;
        }
    }
    return 0;
}

//Notes
// DIGITAL BUS = PC0-PC7
// IC = F0/38
// CS = F1/39
// WR = F2/40
// RD = F3/41
// A0 = F4/42
// A1 = F5/43