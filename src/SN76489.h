#ifndef SN76489_H_
#define SN76489_H_
#include <Arduino.h>
#include "Adjustments.h"

//SN76489 MIDI driver example by: https://github.com/cdodd/teensy-sn76489-midi-synth/blob/master/teensy-sn76489-midi-synth.ino
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
    const long clockHz = 4000000;
    const uint8_t attenuationRegister[4] = {0x10, 0x30, 0x50, 0x70};
    const uint8_t frequencyRegister[3] = {0x00, 0x20, 0x40};
    const uint8_t ledPin[4] = {5, 9, 10, 12};
    uint8_t currentNote[4] = {0, 0, 0, 0};
    uint8_t currentVelocity[4] = {0, 0, 0, 0};
    int currentPitchBend[3] = {8192, 8192, 8192};
public:
    SN76489();
    Channel channels[MAX_CHANNELS_PSG];
    void SetChannelOn(uint8_t key, uint8_t velocity);
    void SetChannelOff(uint8_t key);
    void UpdateAttenuation(uint8_t voice);
    void SetSquareFrequency(uint8_t voice, int frequencyData);
    bool UpdateSquarePitch(uint8_t voice);
    void PitchChange(uint8_t channel, int pitch);
    void Reset();
    void send(uint8_t data);
};
#endif

//Notes:
//PSG /WE on F6