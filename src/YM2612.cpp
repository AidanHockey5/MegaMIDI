#include "YM2612.h"

YM2612::YM2612()
{
    DDRF = 0xFF;
    PORTF = 0x00;
    DDRC = 0xFF;
    PORTC |= 0x3C; //_A1 LOW, _A0 LOW, _IC HIGH, _WR HIGH, _RD HIGH, _CS HIGH
}

void YM2612::Reset()
{
    digitalWriteFast(_IC, LOW);  //_IC HIGH
    delayMicroseconds(25);
    digitalWriteFast(_IC, HIGH); //_IC HIGH
    delayMicroseconds(25);
}

void YM2612::send(unsigned char addr, unsigned char data, bool setA1)
{
    digitalWriteFast(_A1, setA1);
    digitalWriteFast(_A0, LOW);
    digitalWriteFast(_CS, LOW);
    PORTF = addr;
    digitalWriteFast(_WR, LOW);
    delayMicroseconds(1);
    digitalWriteFast(_WR, HIGH);
    digitalWriteFast(_CS, HIGH);
    digitalWriteFast(_A0, HIGH);
    digitalWriteFast(_CS, LOW);
    PORTF = data;
    digitalWriteFast(_WR, LOW);
    delayMicroseconds(1);
    digitalWriteFast(_WR, HIGH);
    digitalWriteFast(_CS, HIGH);
    digitalWriteFast(_A1, LOW);
    digitalWriteFast(_A0, LOW);
}

void YM2612::SetFrequency(uint16_t frequency, uint8_t channel)
{
  int block = 2;
  
  uint16_t frq;
  while(frequency >= 2048)
  {
    frequency /= 2;
    block++;
  }
  frq = (uint16_t)frequency;
  bool setA1 = channel > 2;
  send(0xA4 + channel%3, ((frq >> 8) & mask(3)) | ((block & mask(3)) << 3), setA1);
  send(0xA0 + channel%3, frq, setA1);
}

float YM2612::NoteToFrequency(uint8_t note)
{
    //Elegant note/freq system by diegodorado
    //Check out his project at https://github.com/diegodorado/arduinoProjects/tree/master/ym2612
    const static float freq[12] = 
    {
      //You can create your own note frequencies here. C4#-C5. There should be twelve entries.
      //YM3438 datasheet note set
      277.2, 293.7, 311.1, 329.6, 349.2, 370.0, 392.0, 415.3, 440.0, 466.2, 493.9, 523.3

    }; 
    const static float multiplier[] = 
    {
      0.03125f,   0.0625f,   0.125f,   0.25f,   0.5f,   1.0f,   2.0f,   4.0f,   8.0f,   16.0f,   32.0f 
    }; 
    float f = freq[note%12];
    return (f+(f*TUNE))*multiplier[(note/12)+octaveShift];
}

void YM2612::SetChannelOn(uint8_t key, uint8_t velocity)
{
    uint8_t openChannel = 0xFF;
    for(int i = 0; i<MAX_CHANNELS_YM; i++)
    {
        if(!channels[i].keyOn)
        {
            // if(channels[i].keyNumber == key)
            //     continue;
            channels[i].keyOn = true;
            channels[i].keyNumber = key;
            channels[i].blockNumber = key/12;
            openChannel = i;
            break;
        }
    }
    uint8_t offset = openChannel % 3;
    bool setA1 = openChannel > 2;
    if(openChannel == 0xFF)
      return;
    if(pitchBendYM == 0)
    {
      SetFrequency(NoteToFrequency(key), openChannel);
    }
    else
    {
      float freqFrom = NoteToFrequency(key-pitchBendYMRange);
      float freqTo = NoteToFrequency(key+pitchBendYMRange);
      SetFrequency(map(pitchBendYM, -8192, 8192, freqFrom, freqTo), openChannel);
    }
    send(0x28, 0xF0 + offset + (setA1 << 2));  
}

void YM2612::SetChannelOff(uint8_t key)
{
    uint8_t closedChannel = 0xFF;
    for(int i = 0; i<MAX_CHANNELS_YM; i++)
    {
        if(channels[i].keyNumber == key)
        {
            channels[i].keyOn = false;
            closedChannel = i;
            break;
        }
    }
    if(closedChannel == 0xFF)
        return;
    uint8_t offset = closedChannel % 3;
    bool setA1 = closedChannel > 2;
    send(0x28, 0x00 + offset + (setA1 << 2));
}

void YM2612::SetVoice(Voice v)
{
  currentVoice = v;
  bool resetLFO = lfoOn;
  if(lfoOn)
    ToggleLFO();
  send(0x22, 0x00); // LFO off
  send(0x27, 0x00); // CH3 Normal
  for(int i = 0; i<7; i++) //Turn off all channels
  {
    send(0x28, i);
  }
  send(0x2B, 0x00); // DAC off

  for(int a1 = 0; a1<=1; a1++)
  {
    for(int i=0; i<3; i++)
    {
          uint8_t DT1MUL, TL, RSAR, AMD1R, D2R, D1LRR = 0;

          //Operator 1
          DT1MUL = (v.M1[8] << 4) | v.M1[7];
          TL = v.M1[5];
          RSAR = (v.M1[6] << 6) | v.M1[0];
          AMD1R = (v.M1[10] << 7) | v.M1[1];
          D2R = v.M1[2];
          D1LRR = (v.M1[4] << 4) | v.M1[3];

          // if(i == 0 && a1 == 0)
          // {
          // Serial.print("DT1MUL: "); Serial.println(DT1MUL, HEX);
          // Serial.print("TL: "); Serial.println(TL, HEX);
          // Serial.print("RSAR: "); Serial.println(RSAR, HEX);
          // Serial.print("AMD1R: "); Serial.println(AMD1R, HEX);
          // Serial.print("D2R: "); Serial.println(D2R, HEX);
          // Serial.print("D1LRR: "); Serial.println(D1LRR, HEX);
          // }


          send(0x30 + i, DT1MUL, a1); //DT1/Mul
          send(0x40 + i, TL, a1); //Total Level
          send(0x50 + i, RSAR, a1); //RS/AR
          send(0x60 + i, AMD1R, a1); //AM/D1R
          send(0x70 + i, D2R, a1); //D2R
          send(0x80 + i, D1LRR, a1); //D1L/RR
          send(0x90 + i, 0x00, a1); //SSG EG
          
          //Operator 2
          DT1MUL = (v.C1[8] << 4) | v.C1[7];
          TL = v.C1[5];
          RSAR = (v.C1[6] << 6) | v.C1[0];
          AMD1R = (v.C1[10] << 7) | v.C1[1];
          D2R = v.C1[2];
          D1LRR = (v.C1[4] << 4) | v.C1[3];
          send(0x34 + i, DT1MUL, a1); //DT1/Mul
          send(0x44 + i, TL, a1); //Total Level
          send(0x54 + i, RSAR, a1); //RS/AR
          send(0x64 + i, AMD1R, a1); //AM/D1R
          send(0x74 + i, D2R, a1); //D2R
          send(0x84 + i, D1LRR, a1); //D1L/RR
          send(0x94 + i, 0x00, a1); //SSG EG
           
          //Operator 3
          DT1MUL = (v.M2[8] << 4) | v.M2[7];
          TL = v.M2[5];
          RSAR = (v.M2[6] << 6) | v.M2[0];
          AMD1R = (v.M2[10] << 7) | v.M2[1];
          D2R = v.M2[2];
          D1LRR = (v.M2[4] << 4) | v.M2[3];
          send(0x38 + i, DT1MUL, a1); //DT1/Mul
          send(0x48 + i, TL, a1); //Total Level
          send(0x58 + i, RSAR, a1); //RS/AR
          send(0x68 + i, AMD1R, a1); //AM/D1R
          send(0x78 + i, D2R, a1); //D2R
          send(0x88 + i, D1LRR, a1); //D1L/RR
          send(0x98 + i, 0x00, a1); //SSG EG
                   
          //Operator 4
          DT1MUL = (v.C2[8] << 4) | v.C2[7];
          TL = v.C2[5];
          RSAR = (v.C2[6] << 6) | v.C2[0];
          AMD1R = (v.C2[10] << 7) | v.C2[1];
          D2R = v.C2[2];
          D1LRR = (v.C2[4] << 4) | v.C2[3];
          send(0x3C + i, DT1MUL, a1); //DT1/Mul
          send(0x4C + i, TL, a1); //Total Level
          send(0x5C + i, RSAR, a1); //RS/AR
          send(0x6C + i, AMD1R, a1); //AM/D1R
          send(0x7C + i, D2R, a1); //D2R
          send(0x8C + i, D1LRR, a1); //D1L/RR
          send(0x9C + i, 0x00, a1); //SSG EG

          uint8_t FBALGO = (v.CH[1] << 3) | v.CH[2];
          send(0xB0 + i, FBALGO); // Ch FB/Algo
          send(0xB4 + i, 0xC0); // Both Spks on

          send(0x28, 0x00 + i + (a1 << 2)); //Keys off
    }
  }
  if(resetLFO)
    ToggleLFO();
}

void YM2612::AdjustLFO(uint8_t value)
{
    lfoFrq = map(value, 0, 127, 0, 7);
    if(lfoOn)
    {
        uint8_t lfo = (1 << 3) | lfoFrq;
        send(0x22, lfo);
        if(lfoSens > 7)
        {
            Serial.println("LFO Sensitivity out of range! (Must be 0-7)");
            return;
        }
        uint8_t lrAmsFms = 0xC0 + (3 << 4);
        lrAmsFms |= lfoSens;
        for(int a1 = 0; a1<=1; a1++)
        {
            for(int i=0; i<3; i++)
            {
                send(0xB4 + i, lrAmsFms, a1); // Speaker and LMS
            }
        }
    }
}

void YM2612::AdjustPitch(uint8_t channel, int pitch)
{
    float freqFrom = NoteToFrequency(channels[channel].keyNumber-pitchBendYMRange);
    float freqTo = NoteToFrequency(channels[channel].keyNumber+pitchBendYMRange);
    pitchBendYM = pitch;
    SetFrequency(map(pitch,-8192, 8192, freqFrom, freqTo), channel);
}

void YM2612::ToggleLFO()
{
  lfoOn = !lfoOn;
  Serial.print("LFO: "); Serial.println(lfoOn == true ? "ON": "OFF");
  Voice v = currentVoice;
  uint8_t AMD1R = 0;
  if(lfoOn)
  {
    uint8_t lfo = (1 << 3) | lfoFrq;
    send(0x22, lfo);
    //This is a bulky way to do this, but it works so....
    for(int a1 = 0; a1<=1; a1++)
    {
      for(int i=0; i<3; i++)
      {
        //Op. 1
        AMD1R = (v.M1[10] << 7) | v.M1[1];
        AMD1R |= 1 << 7;
        send(0x60 + i, AMD1R, a1); 

        //Op. 2
        AMD1R = (v.C1[10] << 7) | v.C1[1];
        AMD1R |= 1 << 7;
        send(0x64 + i, AMD1R, a1); 

        //Op. 3
        AMD1R = (v.M2[10] << 7) | v.M2[1];
        AMD1R |= 1 << 7;
        send(0x68 + i, AMD1R, a1); 

        //Op. 4
        AMD1R = (v.C2[10] << 7) | v.C2[1];
        AMD1R |= 1 << 7;
        send(0x6C + i, AMD1R, a1); 

        uint8_t lrAmsFms = 0xC0 + (3 << 4);
        lrAmsFms |= lfoSens;
        send(0xB4 + i, lrAmsFms, a1); // Speaker and LMS
      }
    }
  }
  else
  {
    send(0x22, 0x00); // LFO off
    Reset();
    delay(1);
    SetVoice(v);
  }
  digitalWriteFast(leds[0], lfoOn);
}

void YM2612::ShiftOctaveUp()
{
  if(octaveShift == MAX_OCTAVE_SHIFT)
    return;
  octaveShift++;
  Serial.print("Octave Shift Up: "); Serial.print(octaveShift); Serial.print("/"); Serial.println(MAX_OCTAVE_SHIFT);
}

void YM2612::ShiftOctaveDown()
{
  if(octaveShift == 0)
    return;
  octaveShift--;
  Serial.print("Octave Shift Down: "); Serial.print(octaveShift); Serial.print("/"); Serial.println(MAX_OCTAVE_SHIFT);
}

uint16_t YM2612::CalcFNumber(float note)
{
  const uint32_t clockFrq = 8000000;
  return (144*note*(pow(2, 20))/clockFrq) / pow(2, 4-1);
}

//Notes
// DIGITAL BUS = PF0-PF7
// IC = PC0/10
// CS = PC1/11
// WR = PC2/12
// RD = PC3/13
// A0 = PC4/14
// A1 = PC5/15