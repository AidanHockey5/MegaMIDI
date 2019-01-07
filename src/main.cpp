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
  pinMode(17, OUTPUT);
  ymClock.SetFrequency(masterClockFrequency); //PAL 7600489 //NTSC 7670453
  delay(2000);

  ym2612.Reset();

  delay(100);

  ym2612.send(0x22, 0x00); // LFO off
  ym2612.send(0x27, 0x00); // Note off (channel 0)
  ym2612.send(0x28, 0x01); // Note off (channel 1)
  ym2612.send(0x28, 0x02); // Note off (channel 2)
  ym2612.send(0x28, 0x04); // Note off (channel 3)
  ym2612.send(0x28, 0x05); // Note off (channel 4)
  ym2612.send(0x28, 0x06); // Note off (channel 5)
  ym2612.send(0x2B, 0x00); // DAC off
  ym2612.send(0x30, 0x71); //
  ym2612.send(0x34, 0x0D); //
  ym2612.send(0x38, 0x33); //
  ym2612.send(0x3C, 0x01); // DT1/MUL
  ym2612.send(0x40, 0x23); //
  ym2612.send(0x44, 0x2D); //
  ym2612.send(0x48, 0x26); //
  ym2612.send(0x4C, 0x00); // Total level
  ym2612.send(0x50, 0x5F); //
  ym2612.send(0x54, 0x99); //
  ym2612.send(0x58, 0x5F); //
  ym2612.send(0x5C, 0x94); // RS/AR 
  ym2612.send(0x60, 0x05); //
  ym2612.send(0x64, 0x05); //
  ym2612.send(0x68, 0x05); //
  ym2612.send(0x6C, 0x07); // AM/D1R
  ym2612.send(0x70, 0x02); //
  ym2612.send(0x74, 0x02); //
  ym2612.send(0x78, 0x02); //
  ym2612.send(0x7C, 0x02); // D2R
  ym2612.send(0x80, 0x11); //
  ym2612.send(0x84, 0x11); //
  ym2612.send(0x88, 0x11); //
  ym2612.send(0x8C, 0xA6); // D1L/RR
  ym2612.send(0x90, 0x00); //
  ym2612.send(0x94, 0x00); //
  ym2612.send(0x98, 0x00); //
  ym2612.send(0x9C, 0x00); // Proprietary
  ym2612.send(0xB0, 0x32); // Feedback/algorithm
  ym2612.send(0xB4, 0xC0); // Both speakers on
  ym2612.send(0x28, 0x00); // Key off
  
  ym2612.send(0xA4, 0x22); // 
  ym2612.send(0xA0, 0x69); // Set frequency


  // PORTF = 0xFF;
  // PORTA = 0xFF;
  Serial.begin(9600);
  for(int i = 0;i<12; i++)
  {
    Serial.print(fNumberNotes[i]); Serial.print(", ");
  }
  Serial.println();
}


void setKey(uint8_t key)
{
  uint8_t offset, block, msb, lsb;
  offset = 0; //Adjust this for other channels
  block = key / 12;
  key = key % 12;
  lsb = fNumberNotes[key] % 256;
  msb = fNumberNotes[key] >> 8;
  ym2612.send(0xa4 + offset, (block << 3) + msb);
  ym2612.send(0xa0 + offset, lsb);
}

uint8_t k = 0;

void loop() 
{
  PORTC = 0xFF;
  ym2612.send(0x28, 0xF0);
  delay(150);
  ym2612.send(0x28, 0x00);
  PORTC = 0x00;
  delay(150);

  setKey(k++);
  if(k == 95)
    k = 0;
}

