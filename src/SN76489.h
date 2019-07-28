#ifndef SN76489_H_
#define SN76489_H_
#include <Arduino.h>
#include "Adjustments.h"

//SN76489 MIDI driver example by: https://github.com/cdodd/teensy-sn76489-midi-synth/blob/master/teensy-sn76489-midi-synth.ino
const int MAX_CHANNELS_PSG = 3;
const int noise = 3;

class SN76489
{
private:
    uint8_t _WE = 36;
    typedef struct
    {
        bool keyOn = false;
        bool sustained = false;
        uint8_t keyNumber = 0;
    } Channel;
    const long clockHz = 4000000;
    const uint8_t attenuationRegister[4] = {0x10, 0x30, 0x50, 0x70};
    const uint8_t frequencyRegister[3] = {0x00, 0x20, 0x40};
    uint8_t currentNote[4] = {0, 0, 0, 0};
    uint8_t currentVelocity[4] = {0, 0, 0, 0};
    int currentPitchBend[3] = {8192, 8192, 8192};
public:
    SN76489();
    Channel channels[MAX_CHANNELS_PSG];
    Channel noiseChannel;
    void SetChannelOn(uint8_t key, uint8_t velocity, bool velocityEnabled);
    void SetChannelOff(uint8_t key);
    void SetNoiseOn(uint8_t key, uint8_t velocity, bool velocityEnabled);
    void MIDISetNoiseControl(byte control, byte value);
    bool UpdateNoiseControl();
    void SetNoiseOff(uint8_t key);
    void ClampSustainedKeys();
    void ReleaseSustainedKeys();
    void UpdateAttenuation(uint8_t voice);
    void SetSquareFrequency(uint8_t voice, int frequencyData);
    bool UpdateSquarePitch(uint8_t voice);
    void PitchChange(uint8_t channel, int pitch);
    void Reset();
    void send(uint8_t data);
};
#endif

//Notes
// DIGITAL BUS = PF0-PF7
// WE = PE4/36
// RDY = PE5/37