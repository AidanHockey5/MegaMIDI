#include <Arduino.h>
#include "LTC6903.h"
#include "YM2612.h"

LTC6903 ymClock(20);
YM2612 ym2612 = YM2612();

uint32_t masterClockFrequency = 7670453;

//C# to C (music note frequencies, not programming languages)
float notes[]
{
  277.2, 293.7, 311.1, 329.6, 349.2, 370.0, 392.0, 415.3, 440.0, 466.2, 493.9, 523.3
};

uint16_t fNumberNotes[12];

void GenerateNoteSet();
void KeyOn(byte channel, byte key, byte velocity);
void KeyOff(byte channel, byte key, byte velocity);


void GenerateNoteSet()
{
  for(int i = 0; i < 12; i++)
  {
    //YM3438 manual f-number formula
    //F-Number = (144 x fNote x 2^20 / fm) / 2^(B-1)
    //fNote = desired note frequency
    //fm = YM clock frequency
    //B = Block data, in this case, it's 4
    uint16_t F = (144*notes[i]*(pow(2, 20))/masterClockFrequency) / pow(2, 4-1);
    fNumberNotes[i] = F;
  }
}

void setup() 
{
  GenerateNoteSet();

  usbMIDI.setHandleNoteOn(KeyOn);
  usbMIDI.setHandleNoteOff(KeyOff);

  pinMode(17, OUTPUT);
  ymClock.SetFrequency(masterClockFrequency); //PAL 7600489 //NTSC 7670453
  delay(2000);

  ym2612.Reset();

  delay(100);

  ym2612.send(0x22, 0x00); // LFO off
  ym2612.send(0x27, 0x00); // CH3 Normal
  ym2612.send(0x28, 0x00); // Note off (channel 0)
  ym2612.send(0x28, 0x01); // Note off (channel 1)
  ym2612.send(0x28, 0x02); // Note off (channel 2)
  ym2612.send(0x28, 0x04); // Note off (channel 3)
  ym2612.send(0x28, 0x05); // Note off (channel 4)
  ym2612.send(0x28, 0x06); // Note off (channel 5)
  ym2612.send(0x2B, 0x00); // DAC off

  for(int a1 = 0; a1<=1; a1++)
  {
    for(int i=0; i<3; i++)
    {
          //Operator 1
          ym2612.send(0x30 + i, 0x71, a1); //DT1/Mul
          ym2612.send(0x40 + i, 0x23, a1); //Total Level
          ym2612.send(0x50 + i, 0x5F, a1); //RS/AR
          ym2612.send(0x60 + i, 0x05, a1); //AM/D1R
          ym2612.send(0x70 + i, 0x02, a1); //D2R
          ym2612.send(0x80 + i, 0x11, a1); //D1L/RR
          ym2612.send(0x90 + i, 0x00, a1); //SSG EG
           
          //Operator 2
          ym2612.send(0x34 + i, 0x0D, a1); //DT1/Mul
          ym2612.send(0x44 + i, 0x2D, a1); //Total Level
          ym2612.send(0x54 + i, 0x99, a1); //RS/AR
          ym2612.send(0x64 + i, 0x05, a1); //AM/D1R
          ym2612.send(0x74 + i, 0x02, a1); //D2R
          ym2612.send(0x84 + i, 0x11, a1); //D1L/RR
          ym2612.send(0x94 + i, 0x00, a1); //SSG EG
           
         //Operator 3
          ym2612.send(0x38 + i, 0x33, a1); //DT1/Mul
          ym2612.send(0x48 + i, 0x26, a1); //Total Level
          ym2612.send(0x58 + i, 0x5F, a1); //RS/AR
          ym2612.send(0x68 + i, 0x05, a1); //AM/D1R
          ym2612.send(0x78 + i, 0x02, a1); //D2R
          ym2612.send(0x88 + i, 0x11, a1); //D1L/RR
          ym2612.send(0x98 + i, 0x00, a1); //SSG EG
                   
         //Operator 4
          ym2612.send(0x3C + i, 0x01, a1); //DT1/Mul
          ym2612.send(0x4C + i, 0x00, a1); //Total Level
          ym2612.send(0x5C + i, 0x94, a1); //RS/AR
          ym2612.send(0x6C + i, 0x07, a1); //AM/D1R
          ym2612.send(0x7C + i, 0x02, a1); //D2R
          ym2612.send(0x8C + i, 0xA6, a1); //D1L/RR
          ym2612.send(0x9C + i, 0x00, a1); //SSG EG
          
          ym2612.send(0xB0 + i, 0x32); // Ch FB/Algo
          ym2612.send(0xB4 + i, 0xC0); // Both Spks on
          ym2612.send(0xA4 + i, 0x22); // Set Freq MSB
          ym2612.send(0xA0 + i, 0x69); // Freq LSB
    }
  }
  ym2612.send(0xB4, 0xC0); // Both speakers on
  ym2612.send(0x28, 0x00); // Key off

  Serial1.begin(9600);
}


void KeyOn(byte channel, byte key, byte velocity)
{
  uint8_t offset, block, msb, lsb;
  uint8_t openChannel = ym2612.SetChannelOn(key); 
  offset = openChannel % 3;
  block = key / 12;
  key = key % 12;
  lsb = fNumberNotes[key] % 256;
  msb = fNumberNotes[key] >> 8;

  bool setA1 = openChannel > 2;

  if(openChannel == 0xFF)
    return;

  Serial1.print("OFFSET: "); Serial1.println(offset);
  Serial1.print("CHANNEL: "); Serial1.println(openChannel);
  Serial1.print("A1: "); Serial1.println(setA1);
  ym2612.send(0xA4 + offset, (block << 3) + msb, setA1);
  ym2612.send(0xA0 + offset, lsb, setA1);
  ym2612.send(0x28, 0xF0 + offset + (setA1 << 2));
}

void KeyOff(byte channel, byte key, byte velocity)
{
  uint8_t closedChannel = ym2612.SetChannelOff(key);
  bool setA1 = closedChannel > 2;
  ym2612.send(0x28, 0x00 + closedChannel%3 + (setA1 << 2));
}

void loop() 
{
  usbMIDI.read();
}