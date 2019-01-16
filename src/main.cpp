#include <Arduino.h>
#include "LTC6903.h"
#include "YM2612.h"
#include "SdFat.h"
#include "usb_midi_serial.h"

#define mask(s) (~(~0<<s))

//Notes: Access serial connection using Arduino IDE. Ensure that teensy software suite has been installed https://www.pjrc.com/teensy/td_download.html
//In Arduino, look for Tools->Port->(Emulated Serial)
//Open serial monitor
//Device should reset. You may need to try this a couple times.
//----OR----
//Use SERIAL_CONNECT.bat file found in the 'tools' folder of this repository.

//DEBUG
#define DLED 17

//IO
#define PROG_UP 16
#define PROG_DOWN 15
#define LFO_TOG 14

//Music
#define TUNE 3 //Adjust this to add or subtract semitones to the final note.

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
uint32_t masterClockFrequency = 7670453; //7670453; //PAL 7600489 //NTSC 7670453

//Sound Chips
YM2612 ym2612 = YM2612();

//SD Card
SdFat SD;
File file;
#define MAX_FILE_NAME_SIZE 128
char fileName[MAX_FILE_NAME_SIZE];
uint32_t numberOfFiles = 0;
uint32_t currentFileNumber = 0;

int16_t pitchBend = 0;
uint8_t pitchBendRange = 2; //How many semitones would you like the pitch-bender to range? Standard = 2

//Voice data
#define MAX_VOICES 16
#define MAX_OCTAVE_SHIFT 5
Voice voices[MAX_VOICES];
uint8_t currentProgram = 0;
uint8_t maxValidVoices = 0;
uint8_t octaveShift = 0;
uint8_t lfoOn = false;
uint8_t lfoFrq = 0;
uint8_t lfoSens = 7;


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
void SetFrequency(uint16_t f, uint8_t channel);
void ToggleLFO();
float NoteToFrequency(uint8_t note);


float NoteToFrequency(uint8_t note)
{
    //Elegant note/freq system by diegodorado
    //Check out his project at https://github.com/diegodorado/arduinoProjects/tree/master/ym2612
    const static float freq[12] = 
    {
      //You can create your own note frequencies here. C4#-C5. There should be twelve entries.

      //A note set from my piano
      //278.0, 294.7, 312.2, 331.3, 350.7, 371.5, 393.7, 417.0, 442.0, 468.0, 496.2, 525.5

      //8MHz Tuned note set
      //261.63f,   277.18f,   293.66f,   311.13f,   329.63f,   349.23f,   369.99f,   392.00f,   415.30f,   440.00f,   466.16f,   493.88f

      //YM3438 datasheet note set
      277.2, 293.7, 311.1, 329.6, 349.2, 370.0, 392.0, 415.3, 440.0, 466.2, 493.9, 523.3
    }; 
    const static float multiplier[] = 
    {
      0.03125f,   0.0625f,   0.125f,   0.25f,   0.5f,   1.0f,   2.0f,   4.0f,   8.0f,   16.0f,   32.0f 
    }; 
    note += TUNE;
    return freq[note%12]*multiplier[(note/12)+octaveShift];
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

void setup() 
{
  ymClock.SetFrequency(masterClockFrequency); //PAL 7600489 //NTSC 7670453
  ym2612.Reset();
  Serial.begin(115200);
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
  SetVoice(voices[0]);
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

void SetVoice(Voice v)
{
  bool resetLFO = lfoOn;
  if(lfoOn)
    ToggleLFO();
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
  if(resetLFO)
    ToggleLFO();
}

void PitchChange(byte channel, int pitch)
{
  pitchBend = pitch;
  for(int i = 0; i<MAX_CHANNELS; i++)
  {
    if(ym2612.channels[i].keyOn)
    {
    float freqFrom = NoteToFrequency(ym2612.channels[i].keyNumber-pitchBendRange);
    float freqTo = NoteToFrequency(ym2612.channels[i].keyNumber+pitchBendRange);
    SetFrequency(map(pitchBend,-8192, 8191, freqFrom, freqTo), i);
    }
  }
}

void SetFrequency(uint16_t f, uint8_t channel)
{
  int block = 2;
  uint16_t frq;
  while(f >= 2048)
  {
    f /= 2;
    block++;
  }
  frq = (uint16_t)f;
  bool setA1 = channel > 2;
  ym2612.send(0xA4 + channel%3, ((frq >> 8) & mask(3)) | ((block & mask(3)) << 3), setA1);
  ym2612.send(0xA0 + channel%3, frq, setA1);
}

void KeyOn(byte channel, byte key, byte velocity)
{
  uint8_t openChannel = ym2612.SetChannelOn(key); 
  uint8_t offset = openChannel % 3;
  bool setA1 = openChannel > 2;
  if(openChannel == 0xFF)
    return;
  if(pitchBend == 0)
    SetFrequency(NoteToFrequency(key), openChannel);
  else
  {
    float freqFrom = NoteToFrequency(key-pitchBendRange);
    float freqTo = NoteToFrequency(key+pitchBendRange);
    SetFrequency(map(pitchBend, -8192, 8191, freqFrom, freqTo), openChannel);
  }
  ym2612.send(0x28, 0xF0 + offset + (setA1 << 2));  
}

void KeyOff(byte channel, byte key, byte velocity)
{
  uint8_t closedChannel = ym2612.SetChannelOff(key);
  uint8_t offset = closedChannel % 3;
  bool setA1 = closedChannel > 2;
  ym2612.send(0x28, 0x00 + offset + (setA1 << 2));
}

void ControlChange(byte channel, byte control, byte value)
{
  lfoFrq = map(value, 0, 127, 0, 7);
  if(lfoOn)
  {
    uint8_t lfo = (1 << 3) | lfoFrq;
    ym2612.send(0x22, lfo);
    Serial.println(lfoFrq);
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
        ym2612.send(0xB4 + i, lrAmsFms, a1); // Speaker and LMS
      }
    }
  }
}

void ToggleLFO()
{
  lfoOn = !lfoOn;
  Serial.print("LFO: "); Serial.println(lfoOn == true ? "ON": "OFF");
  Voice v = voices[currentProgram];
  uint8_t AMD1R;
  if(lfoOn)
  {
    uint8_t lfo = (1 << 3) | lfoFrq;
    ym2612.send(0x22, lfo);
    //This is a bulky way to do this, but it works so....
    for(int a1 = 0; a1<=1; a1++)
    {
      for(int i=0; i<3; i++)
      {
        //Op. 1
        AMD1R = (v.M1[10] << 7) | v.M1[1];
        AMD1R |= 1 << 7;
        ym2612.send(0x60 + i, AMD1R, a1); 

        //Op. 2
        AMD1R = (v.C1[10] << 7) | v.C1[1];
        AMD1R |= 1 << 7;
        ym2612.send(0x64 + i, AMD1R, a1); 

        //Op. 3
        AMD1R = (v.M2[10] << 7) | v.M2[1];
        AMD1R |= 1 << 7;
        ym2612.send(0x68 + i, AMD1R, a1); 

        //Op. 4
        AMD1R = (v.C2[10] << 7) | v.C2[1];
        AMD1R |= 1 << 7;
        ym2612.send(0x6C + i, AMD1R, a1); 

        uint8_t lrAmsFms = 0xC0 + (3 << 4);
        lrAmsFms |= lfoSens;
        ym2612.send(0xB4 + i, lrAmsFms, a1); // Speaker and LMS
      }
    }
  }
  else
  {
    ym2612.send(0x22, 0x00); // LFO off
    for(int a1 = 0; a1<=1; a1++)
    {
      for(int i=0; i<3; i++)
      {
        //Op. 1
        AMD1R = (v.M1[10] << 7) | v.M1[1];
        AMD1R &= ~(1 << 7);
        ym2612.send(0x60 + i, AMD1R, a1); 

        //Op. 2
        AMD1R = (v.C1[10] << 7) | v.C1[1];
        AMD1R &= ~(1 << 7);
        ym2612.send(0x64 + i, AMD1R, a1); 

        //Op. 3
        AMD1R = (v.M2[10] << 7) | v.M2[1];
        AMD1R &= ~(1 << 7);
        ym2612.send(0x68 + i, AMD1R, a1); 

        //Op. 4
        AMD1R = (v.C2[10] << 7) | v.C2[1];
        AMD1R &= ~(1 << 7);
        ym2612.send(0x6C + i, AMD1R, a1); 

        uint8_t lrAmsFms = 0xC0;
        ym2612.send(0xB4 + i, lrAmsFms, a1); // Speaker and LMS
      }
    }
  }
}

uint8_t lastProgram = 0;
void ProgramChange(byte channel, byte program)
{
  if(program == 255)
    program = 0;
  program %= maxValidVoices;
  currentProgram = program;
  SetVoice(voices[currentProgram]);
  Serial.print("Current Voice Number: "); Serial.print(currentProgram); Serial.print("/"); Serial.println(maxValidVoices-1);
  DumpVoiceData(voices[currentProgram]);
  lastProgram = program;
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
        ToggleLFO();
        return;
      }
      case '+': //Move up one voice in current OPM file
      {
        ProgramChange(0, currentProgram+1);
        return;
      }
      case '-': //Move down one voice in current OPM file
      {
        ProgramChange(0, currentProgram-1);
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
    SetVoice(voices[0]);
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
    ToggleLFO();
    delay(200);
  }
}