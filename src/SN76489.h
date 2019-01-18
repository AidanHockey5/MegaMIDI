#ifndef SN76489_H_
#define SN76489_H_
#include <Arduino.h>
#include "PSGNotes.h"

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
    const byte attenuationRegister[4] = {0x10, 0x30, 0x50, 0x70};
    const byte frequencyRegister[3] = {0x00, 0x20, 0x40};
    const byte ledPin[4] = {5, 9, 10, 12};
    byte currentNote[4] = {0, 0, 0, 0};
    byte currentVelocity[4] = {0, 0, 0, 0};
    int currentPitchBend[3] = {8192, 8192, 8192};
public:
    SN76489();
    Channel channels[MAX_CHANNELS_PSG];
    uint16_t GetFrequencyFromLUT(uint8_t key);
    void SetChannelOn(uint8_t key, uint8_t velocity);
    void SetChannelOff(uint8_t key);
    void SetVolume(uint8_t channel, uint8_t volume);
    void SetFrequency(uint8_t key, uint8_t channel);
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