#include <Arduino.h>
#include "LTC6903.h"
#include "YM2612.h"
#include "SdFat.h"

//DEBUG
#define DLED 17

//IO
#define PROG_UP 16
#define PROG_DOWN 15

//OPM File Format https://vgmrips.net/wiki/OPM_File_Format
typedef struct
{
  uint8_t LFO[5];
  uint8_t CH[7];
  uint8_t M1[11];
  uint8_t C1[11];
  uint8_t M2[11];
  uint8_t C2[11];
} Voice;

//Clocks
LTC6903 ymClock(24);
uint32_t masterClockFrequency = 7670453;

//Sound Chips
YM2612 ym2612 = YM2612();

//SD Card
SdFat SD;
File file;
#define MAX_FILE_NAME_SIZE 128
char fileName[MAX_FILE_NAME_SIZE];
uint32_t numberOfFiles = 0;
uint32_t currentFileNumber = 0;

//C# to C (music note frequencies, not programming languages)
float notes[]
{
  277.2, 293.7, 311.1, 329.6, 349.2, 370.0, 392.0, 415.3, 440.0, 466.2, 493.9, 523.3
};
uint16_t fNumberNotes[12];

//Voice data
#define MAX_VOICES 16
Voice voices[MAX_VOICES];
uint8_t currentProgram = 0;
uint8_t maxValidVoices = 0;


//Prototypes
void GenerateNoteSet();
void KeyOn(byte channel, byte key, byte velocity);
void KeyOff(byte channel, byte key, byte velocity);
void ProgramChange(byte channel, byte program);
void SetVoice(Voice v);
void removeSVI();
void ReadVoiceData();
void HandleSerialIn();


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
  ymClock.SetFrequency(masterClockFrequency); //PAL 7600489 //NTSC 7670453
  ym2612.Reset();
  GenerateNoteSet();
  Serial1.begin(9600);
  usbMIDI.setHandleNoteOn(KeyOn);
  usbMIDI.setHandleNoteOff(KeyOff);
  usbMIDI.setHandleProgramChange(ProgramChange);
  pinMode(DLED, OUTPUT);
  pinMode(PROG_UP, INPUT_PULLUP);
  pinMode(PROG_DOWN, INPUT_PULLUP);

  if(!SD.begin(20, SPI_HALF_SPEED))
  {
    digitalWrite(DLED, HIGH);
    while(true){Serial1.println("SD Mount failed!");}
  }
  removeSVI();

  //Count total files
  File countFile;
  while ( countFile.openNext( SD.vwd(), O_READ ))
  {
    countFile.close();
    numberOfFiles++;
  }
  countFile.close();
  SD.vwd()->rewind();

  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!DEBUG FILE
  if(file.isOpen())
    file.close();
  file.openNext(SD.vwd(), O_READ);
  if(!file)
  {
    digitalWrite(DLED, HIGH);
    while(true){Serial1.println("File Read failed!");}
  }
  ReadVoiceData();
  SetVoice(voices[1]);
}

void removeSVI() //Sometimes, Windows likes to place invisible files in our SD card without asking... GTFO!
{
  File nextFile;
  nextFile.openNext(SD.vwd(), O_READ);
  char name[MAX_FILE_NAME_SIZE];
  nextFile.getName(name, MAX_FILE_NAME_SIZE);
  String n = String(name);
  if(n == "System Volume Information")
  {
      if(!nextFile.rmRfStar())
        Serial.println("Failed to remove SVI file");
  }
  SD.vwd()->rewind();
  nextFile.close();
}

void ReadVoiceData()
{
  size_t n;
  uint8_t voiceCount = 0;
  char * pEnd;
  uint8_t vDataRaw[6][11];
  const size_t LINE_DIM = 60;
  char line[LINE_DIM];
  while ((n = file.fgets(line, sizeof(line))) > 0) 
  {
      String l = line;
      //Ignore comments
      if(l.startsWith("//"))
        continue;
      if(l.startsWith("@:"+String(voiceCount)+" no Name"))
      {
        maxValidVoices = voiceCount;
      }
      else if(l.startsWith("@:"+String(voiceCount)))
      {
        voiceCount++;
        for(int i=0; i<6; i++)
        {
          file.fgets(line, sizeof(line));
          l = line;
          l.replace("LFO: ", "");
          l.replace("CH: ", "");
          l.replace("M1: ", "");
          l.replace("C1: ", "");
          l.replace("M2: ", "");
          l.replace("C2: ", "");
          l.toCharArray(line, sizeof(line), 0);

          vDataRaw[i][0] = strtoul(line, &pEnd, 10); 
          for(int j = 1; j<11; j++)
          {
            vDataRaw[i][j] = strtoul(pEnd, &pEnd, 10);
          }
        }

        for(int i=0; i<5; i++) //LFO
          voices[voiceCount].LFO[i] = vDataRaw[0][i];
        for(int i=0; i<7; i++) //CH
          voices[voiceCount].CH[i] = vDataRaw[1][i];
        for(int i=0; i<11; i++) //M1
          voices[voiceCount].M1[i] = vDataRaw[2][i];
        for(int i=0; i<11; i++) //C1
          voices[voiceCount].C1[i] = vDataRaw[3][i];
        for(int i=0; i<11; i++) //M2
          voices[voiceCount].M2[i] = vDataRaw[4][i];
        for(int i=0; i<11; i++) //C2
          voices[voiceCount].C2[i] = vDataRaw[5][i];
      }
      if(voiceCount == MAX_VOICES-1)
        break;
  }
  Serial1.println("Done Reading Voice Data");
}

void SetVoice(Voice v)
{
  ym2612.send(0x22, 0x00); // LFO off
  ym2612.send(0x27, 0x00); // CH3 Normal
  for(int i = 0; i<7; i++) //Turn off all channels
  {
    ym2612.send(0x28, i);
  }
  ym2612.send(0x2B, 0x00); // DAC off

  for(int a1 = 0; a1<=1; a1++)
  {
    for(int i=0; i<3; i++)
    {
          uint8_t DT1MUL, TL, RSAR, AMD1R, D2R, D1LRR;

          //Operator 1
          DT1MUL = (v.M1[8] << 4) | v.M1[7];
          TL = v.M1[5];
          RSAR = (v.M1[7] << 6) | v.M1[0];
          AMD1R = (v.M1[10] << 7) | v.M1[1];
          D2R = v.M1[2];
          D1LRR = (v.M1[4] << 4) | v.M1[3];
          ym2612.send(0x30 + i, DT1MUL, a1); //DT1/Mul
          ym2612.send(0x40 + i, TL, a1); //Total Level
          ym2612.send(0x50 + i, RSAR, a1); //RS/AR
          ym2612.send(0x60 + i, AMD1R, a1); //AM/D1R
          ym2612.send(0x70 + i, D2R, a1); //D2R
          ym2612.send(0x80 + i, D1LRR, a1); //D1L/RR
          ym2612.send(0x90 + i, 0x00, a1); //SSG EG
          
          //Operator 2
          DT1MUL = (v.C1[8] << 4) | v.C1[7];
          TL = v.C1[5];
          RSAR = (v.C1[7] << 6) | v.C1[0];
          AMD1R = (v.C1[10] << 7) | v.C1[1];
          D2R = v.C1[2];
          D1LRR = (v.C1[4] << 4) | v.C1[3];
          ym2612.send(0x34 + i, DT1MUL, a1); //DT1/Mul
          ym2612.send(0x44 + i, TL, a1); //Total Level
          ym2612.send(0x54 + i, RSAR, a1); //RS/AR
          ym2612.send(0x64 + i, AMD1R, a1); //AM/D1R
          ym2612.send(0x74 + i, D2R, a1); //D2R
          ym2612.send(0x84 + i, D1LRR, a1); //D1L/RR
          ym2612.send(0x94 + i, 0x00, a1); //SSG EG
           
          //Operator 3
          DT1MUL = (v.M2[8] << 4) | v.M2[7];
          TL = v.M2[5];
          RSAR = (v.M2[7] << 6) | v.M2[0];
          AMD1R = (v.M2[10] << 7) | v.M2[1];
          D2R = v.M2[2];
          D1LRR = (v.M2[4] << 4) | v.M2[3];
          ym2612.send(0x38 + i, DT1MUL, a1); //DT1/Mul
          ym2612.send(0x48 + i, TL, a1); //Total Level
          ym2612.send(0x58 + i, RSAR, a1); //RS/AR
          ym2612.send(0x68 + i, AMD1R, a1); //AM/D1R
          ym2612.send(0x78 + i, D2R, a1); //D2R
          ym2612.send(0x88 + i, D1LRR, a1); //D1L/RR
          ym2612.send(0x98 + i, 0x00, a1); //SSG EG
                   
          //Operator 4
          DT1MUL = (v.C2[8] << 4) | v.C2[7];
          TL = v.C2[5];
          RSAR = (v.C2[7] << 6) | v.C2[0];
          AMD1R = (v.C2[10] << 7) | v.C2[1];
          D2R = v.C2[2];
          D1LRR = (v.C2[4] << 4) | v.C2[3];
          ym2612.send(0x3C + i, DT1MUL, a1); //DT1/Mul
          ym2612.send(0x4C + i, TL, a1); //Total Level
          ym2612.send(0x5C + i, RSAR, a1); //RS/AR
          ym2612.send(0x6C + i, AMD1R, a1); //AM/D1R
          ym2612.send(0x7C + i, D2R, a1); //D2R
          ym2612.send(0x8C + i, D1LRR, a1); //D1L/RR
          ym2612.send(0x9C + i, 0x00, a1); //SSG EG

          uint8_t FBALGO = (v.CH[1] << 3) | v.CH[3];
          ym2612.send(0xB0 + i, FBALGO); // Ch FB/Algo
          ym2612.send(0xB4 + i, 0xC0); // Both Spks on

          ym2612.send(0x28, 0x00 + i + (a1 << 2)); //Keys off
    }
  }
  //ym2612.send(0xB4, 0xC0); // Both speakers on
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

  // Serial1.print("OFFSET: "); Serial1.println(offset);
  // Serial1.print("CHANNEL: "); Serial1.println(openChannel);
  // Serial1.print("A1: "); Serial1.println(setA1);
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

uint8_t lastProgram = 0;
void ProgramChange(byte channel, byte program)
{
  program %= 16;

  if(program > lastProgram)
  {
    if(currentProgram + 1 < maxValidVoices)
      currentProgram++;
  }
  else if(program < lastProgram)
  {
    if(currentProgram > 0)
      currentProgram--;
  }
  SetVoice(voices[currentProgram]);
  lastProgram = program;
}

void HandleSerialIn()
{
  while(Serial1.available())
  {
    char serialCmd = Serial1.read();
    switch(serialCmd)
    {
      case 'r':
      {
        String req = Serial1.readString();
        req.remove(0, 1); //Remove colon character
        SD.vwd()->rewind();
        bool fileFound = false;
        Serial1.print("REQUEST: ");Serial1.println(req);
        File nextFile;
        for(uint32_t i = 0; i<numberOfFiles; i++)
        {
            nextFile.close();
            nextFile.openNext(SD.vwd(), O_READ);
            nextFile.getName(fileName, MAX_FILE_NAME_SIZE);
            String tmpFN = String(fileName);
            tmpFN.trim();
            req.trim();
            if(tmpFN == req)
            {
              currentFileNumber = i;
              fileFound = true;
              break;
            }
          }
          nextFile.close();
      }
      break;
      default:
        continue;
    }
    if(file.isOpen())
      file.close();
    file = SD.open(fileName, FILE_READ);
    if(!file)
    {
      Serial1.println("Failed to read file");
      digitalWrite(DLED, HIGH);
      while(true){}
    }
    ReadVoiceData();
    SetVoice(voices[0]);
    currentProgram = 0;
  }
}

void loop() 
{
  usbMIDI.read();
  if(Serial1.available() > 0)
    HandleSerialIn();
  if(!digitalReadFast(PROG_UP))
  {
    ProgramChange(0, lastProgram+1);
    delay(200);
  }
  if(!digitalReadFast(PROG_DOWN))
  {
    ProgramChange(0, lastProgram-1);
    delay(200);
  }
}