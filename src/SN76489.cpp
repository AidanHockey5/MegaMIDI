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

void SN76489::SetChannelOn(uint8_t key, uint8_t velocity)
{
    bool updateAttenuationFlag;
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
    if(channel == 0xFF)
        return;

    if(channel < MAX_CHANNELS_PSG)
    {
        currentNote[channel] = key;
        currentVelocity[channel] = velocity;
        updateAttenuationFlag = UpdateSquarePitch(channel);
        if (updateAttenuationFlag) 
            UpdateAttenuation(channel);
    }
}

void SN76489::PitchChange(uint8_t channel, int pitch)
{
    if (channel < 0 || channel > 2)
        return;
    currentPitchBend[channel] = pitch;
    UpdateSquarePitch(channel);
}

bool SN76489::UpdateSquarePitch(uint8_t voice)
{
    float pitchInHz;
    unsigned int frequencyData;
    if (voice < 0 || voice > 2)
        return false;
    pitchInHz = 440 * pow(2, (float(currentNote[voice] - 69) / 12) + (float(currentPitchBend[voice] - 8192) / ((unsigned int)4096 * 12)));
    frequencyData = clockHz / float(32 * pitchInHz);
    if (frequencyData > 1023)
        return false;
    SetSquareFrequency(voice, frequencyData);
    return true;
}

void SN76489::SetSquareFrequency(uint8_t voice, int frequencyData)
{
    if (voice < 0 || voice > 2)
        return;
    send(0x80 | frequencyRegister[voice] | (frequencyData & 0x0f));
    send(frequencyData >> 4);
}

void SN76489::UpdateAttenuation(uint8_t voice)
{
    uint8_t attenuationValue;
    if (voice < 0 || voice > 3) 
        return;
    attenuationValue = (127 - currentVelocity[voice]) >> 3;
    send(0x80 | attenuationRegister[voice] | attenuationValue);
}

uint16_t SN76489::GetFrequencyFromLUT(uint8_t key)
{
    if ( (key < LOWEST_NOTE) || (key > HIGHEST_NOTE) ) return 0x00;
    return pgm_read_word(&G_notes[key]);
}

void SN76489::SetFrequency(uint8_t key, uint8_t channel)
{
    uint16_t frq;
    if ( (key < LOWEST_NOTE) || (key > HIGHEST_NOTE) ) return;
    frq =  GetFrequencyFromLUT(key);
    send( LATCH_CMD | (channel << 5) | TYPE_TONE | (frq & 0b1111) );
    if(frq > 0b1111)
        send(DATA_CMD | (frq >> 4));
    SetVolume(channel, VOL_MAX);
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
    if (key != currentNote[channel])
        return;
    currentVelocity[channel] = 0;
    UpdateAttenuation(channel);
}

//Notes
// DIGITAL BUS = PC0-PC7
// WE = F6/44