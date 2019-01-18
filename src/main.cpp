#include <Arduino.h>
#include "Voice.h"
#include "YM2612.h"
#include "SN76489.h"
#include "SdFat.h"
#include "usb_midi_serial.h"


//Notes: Access serial connection using Arduino IDE. Ensure that teensy software suite has been installed https://www.pjrc.com/teensy/td_download.html
//In Arduino, look for Tools->Port->(Emulated Serial)
//Open serial monitor
//Device should reset. You may need to try this a couple times.
//----OR----
//Use SERIAL_CONNECT.bat file found in the 'tools' folder of this repository.

//Music
#include "Adjustments.h" //Look in this file for tuning & pitchbend settings

//MIDI
#define YM_CHANNEL 1
#define PSG_CHANNEL 2

//DEBUG
#define DLED 8

//IO
#define PROG_UP 5
#define PROG_DOWN 6
#define LFO_TOG 7

//Clocks
uint32_t masterClockFrequency = 8000000;

//Sound Chips
SN76489 sn76489 = SN76489();
YM2612 ym2612 = YM2612();

//SD Card
SdFat SD;
File file;
#define MAX_FILE_NAME_SIZE 128
char fileName[MAX_FILE_NAME_SIZE];
uint32_t numberOfFiles = 0;
uint32_t currentFileNumber = 0;

//Prototypes
void KeyOn(byte channel, byte key, byte velocity);
void KeyOff(byte channel, byte key, byte velocity);
void ProgramChange(byte channel, byte program);
void PitchChange(byte channel, int pitch);
void ControlChange(byte channel, byte control, byte value);
void SetVoice(Voice v);
void removeSVI();
void ReadVoiceData();
void HandleSerialIn();
void DumpVoiceData(Voice v);
void ShiftOctaveUp();
void ShiftOctaveDown();

void setup() 
{
  Serial.begin(115200);
  delay(20); //Wait for clocks to start
  sn76489.Reset();
  ym2612.Reset();
  usbMIDI.setHandleNoteOn(KeyOn);
  usbMIDI.setHandleNoteOff(KeyOff);
  usbMIDI.setHandleProgramChange(ProgramChange);
  usbMIDI.setHandlePitchChange(PitchChange);
  usbMIDI.setHandleControlChange(ControlChange);
  pinMode(DLED, OUTPUT);
  pinMode(PROG_UP, INPUT_PULLUP);
  pinMode(PROG_DOWN, INPUT_PULLUP);
  pinMode(LFO_TOG, INPUT_PULLUP);

  if(!SD.begin(20, SPI_HALF_SPEED))
  {
    digitalWrite(DLED, HIGH);
    while(true){Serial.println("SD Mount failed!");}
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

  if(file.isOpen())
    file.close();
  file.openNext(SD.vwd(), O_READ);
  if(!file)
  {
    digitalWrite(DLED, HIGH);
    while(true){Serial.println("File Read failed!");}
  }
  file.getName(fileName, MAX_FILE_NAME_SIZE);
  Serial.println(fileName);
  ReadVoiceData();
  ym2612.SetVoice(voices[0]);
  DumpVoiceData(voices[0]);
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
        break;
      }
      else if(l.startsWith("@:"+String(voiceCount)))
      {
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
        voiceCount++;
      }
      if(voiceCount == MAX_VOICES-1)
        break;
  }
  Serial.println("Done Reading Voice Data");
}

void DumpVoiceData(Voice v) //Used to check operator settings from loaded OPM file
{
  Serial.print("LFO: ");
  for(int i = 0; i<5; i++)
  {
    Serial.print(v.LFO[i]); Serial.print(" ");
  }
  Serial.println();
  Serial.print("CH: ");
  for(int i = 0; i<7; i++)
  {
    Serial.print(v.CH[i]); Serial.print(" ");
  }
  Serial.println();
  Serial.print("M1: ");
  for(int i = 0; i<11; i++)
  {
    Serial.print(v.M1[i]); Serial.print(" ");
  }
  Serial.println();
  Serial.print("C1: ");
  for(int i = 0; i<11; i++)
  {
    Serial.print(v.C1[i]); Serial.print(" ");
  }
  Serial.println();
  Serial.print("M2: ");
  for(int i = 0; i<11; i++)
  {
    Serial.print(v.M2[i]); Serial.print(" ");
  }
  Serial.println();
  Serial.print("C2: ");
  for(int i = 0; i<11; i++)
  {
    Serial.print(v.C2[i]); Serial.print(" ");
  }
  Serial.println();
}

void PitchChange(byte channel, int pitch)
{
  if(channel == YM_CHANNEL)
  {
    pitchBendYM = pitch;
    for(int i = 0; i<MAX_CHANNELS_YM; i++)
    {
      ym2612.AdjustPitch(i, pitch);
    }
  }
  else if(channel == PSG_CHANNEL)
  {
    for(int i = 0; i<MAX_CHANNELS_PSG; i++)
    {
      sn76489.PitchChange(i, pitch);
    }
  }
}

void KeyOn(byte channel, byte key, byte velocity)
{
  if(channel == YM_CHANNEL)
  {
    ym2612.SetChannelOn(key+SEMITONE_ADJ_YM);
  }
  else if(channel == PSG_CHANNEL)
  {
    sn76489.SetChannelOn(key+SEMITONE_ADJ_PSG, velocity);
  }
}

void KeyOff(byte channel, byte key, byte velocity)
{
  if(channel == YM_CHANNEL)
  {
    ym2612.SetChannelOff(key+SEMITONE_ADJ_YM);
  }
  else if(channel == PSG_CHANNEL)
  {
    sn76489.SetChannelOff(key+SEMITONE_ADJ_PSG);
  }
}

void ControlChange(byte channel, byte control, byte value)
{
  if(control == 0x01)
  {
    ym2612.AdjustLFO(value);
  }
}

uint8_t lastProgram = 0;
void ProgramChange(byte channel, byte program)
{
  if(channel == YM_CHANNEL)
  {
    if(program == 255)
      program = 0;
    program %= maxValidVoices;
    currentProgram = program;
    ym2612.SetVoice(voices[currentProgram]);
    Serial.print("Current Voice Number: "); Serial.print(currentProgram); Serial.print("/"); Serial.println(maxValidVoices-1);
    DumpVoiceData(voices[currentProgram]);
    lastProgram = program;
  }
}

void ShiftOctaveUp()
{
  if(octaveShift == MAX_OCTAVE_SHIFT)
    return;
  octaveShift++;
  Serial.print("Octave Shift Up: "); Serial.print(octaveShift); Serial.print("/"); Serial.println(MAX_OCTAVE_SHIFT);
}

void ShiftOctaveDown()
{
  if(octaveShift == 0)
    return;
  octaveShift--;
  Serial.print("Octave Shift Down: "); Serial.print(octaveShift); Serial.print("/"); Serial.println(MAX_OCTAVE_SHIFT);
}

void HandleSerialIn()
{
  while(Serial.available())
  {
    char serialCmd = Serial.read();
    switch(serialCmd)
    {
      case 'o': //Dump current voice operator info
      {
        Serial.print("Current Voice Number: "); Serial.print(currentProgram); Serial.print("/"); Serial.println(maxValidVoices-1);
        DumpVoiceData(voices[currentProgram]);
        return;
      }
      case 'l':
      {
        ym2612.ToggleLFO();
        return;
      }
      case '+': //Move up one voice in current OPM file
      {
        ProgramChange(YM_CHANNEL, currentProgram+1);
        return;
      }
      case '-': //Move down one voice in current OPM file
      {
        ProgramChange(YM_CHANNEL, currentProgram-1);
        return;
      }
      case '>':
      {
        ShiftOctaveUp();
        return;
      }
      case '<':
      {
        ShiftOctaveDown();
        return;
      }
      case '?':
      {
        Serial.println(fileName);
        return;
      }
      case 'r': //Request a new opm file. format:    r:myOpmFile.opm
      {
        String req = Serial.readString();
        req.remove(0, 1); //Remove colon character
        SD.vwd()->rewind();
        bool fileFound = false;
        Serial.print("REQUEST: ");Serial.println(req);
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
          if(!fileFound)
          {
            Serial.println("Error: File not found.");
            return;
          }
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
      Serial.println("Failed to read file");
      digitalWrite(DLED, HIGH);
      while(true){}
    }
    ReadVoiceData();
    ym2612.SetVoice(voices[0]);
    currentProgram = 0;
  }
}

void loop() 
{
  usbMIDI.read();

  if(Serial.available() > 0)
    HandleSerialIn();
  if(!digitalReadFast(PROG_UP))
  {
    ProgramChange(0, currentProgram+1);
    delay(200);
  }
  if(!digitalReadFast(PROG_DOWN))
  {
    ProgramChange(0, currentProgram-1);
    delay(200);
  }
  if(!digitalReadFast(LFO_TOG))
  {
    ym2612.ToggleLFO();
    delay(200);
  }
}



//Notes precalcuated, keeping here for people to reference it if they need it.
//C4# to C5 
// float notes[]
// {
//   277.2, 293.7, 311.1, 329.6, 349.2, 370.0, 392.0, 415.3, 440.0, 466.2, 493.9, 523.3
// };
// uint16_t fNumberNotes[12];
// void GenerateNoteSet()
// {
//   for(int i = 0; i < 12; i++)
//   {
//     //YM3438 manual f-number formula
//     //F-Number = (144 x fNote x 2^20 / fm) / 2^(B-1)
//     //fNote = desired note frequency
//     //fm = YM clock frequency
//     //B = Block data, in this case, it's 4
//     uint16_t F = (144*notes[i]*(pow(2, 20))/masterClockFrequency) / pow(2, 4-1);
//     fNumberNotes[i] = F;
//   }
// }