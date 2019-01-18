#include "SN76489.h"

SN76489::SN76489()
{
    DDRC = 0xFF;
    PORTC = 0x00;
    DDRF = 0xFF;
    PORTF |= 0x02; //_WE HIGH
}

void SN76489::Reset()
{
    send(0x9F);
    send(0xBF);
    send(0xDF);
    send(0xFF);  
}

void SN76489::send(uint8_t data)
{
    //Byte 1
    // 1   REG ADDR        DATA
    //|1| |R0|R1|R2| |F6||F7|F8|F9|

    //Byte 2
    //  0           DATA
    //|0|0| |F0|F1|F2|F3|F4|F5|

    digitalWriteFast(_WE, HIGH);
    PORTC = data;
    digitalWriteFast(_WE, LOW);
    delayMicroseconds(14);
    digitalWriteFast(_WE, HIGH);
}

void SN76489::SetVolume(uint8_t channel, uint8_t volume)
{
    if(channel < 3)
    {
        send(LATCH_CMD | (channel << 5) | TYPE_VOL | (volume & 0b1111));
    }
}

void SN76489::SetChannelOn(uint8_t key)
{
    if(key > sizeof(G_notes))
        return;

    uint8_t channel = 0xFF;
    for(int i = 0; i<MAX_CHANNELS_PSG; i++)
    {
        if(!channels[i].keyOn)
        {
            channels[i].keyOn = true;
            channels[i].keyNumber = key;
            channel = i;
            break;
        }
    }
    if(channel < MAX_CHANNELS_PSG)
    {
        uint16_t frq;
        if ( (key < LOWEST_NOTE) || (key > HIGHEST_NOTE) ) return;
        frq =  pgm_read_word(&G_notes[key]);
        send( LATCH_CMD | (channel << 5) | TYPE_TONE | (frq & 0b1111) );
        if(frq > 0b1111)
            send(DATA_CMD | (frq >> 4));
        SetVolume(channel, VOL_MAX);
    }
}

void SN76489::SetChannelOff(uint8_t key)
{
    uint8_t channel = 0xFF;
    for(int i = 0; i<MAX_CHANNELS_PSG; i++)
    {
        if(channels[i].keyNumber == key)
        {
            channels[i].keyOn = false;
            channel = i;
            break;
        }
    }
    if(channel == 0xFF)
        return;
    SetVolume(channel, VOL_OFF);
}

//Notes
// DIGITAL BUS = PC0-PC7
// WE = F6/44