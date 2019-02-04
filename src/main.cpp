#include <Arduino.h>
#include "Voice.h"
#include "YM2612.h"
#include "SN76489.h"
#include "SdFat.h"
#include <MIDI.h>
#include <Encoder.h>
#include <LiquidCrystal.h>
#include "LCDChars.h"
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
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

//DEBUG
#define DLED 8

//INPUT
#define PROG_UP 5
#define PROG_DOWN 6
#define LFO_TOG 7
#define ENC_BTN 0
Encoder encoder(18, 19);
long encoderPos = 0;

//LCD
#define LCD_ROWS 4
#define LCD_COLS 20
uint16_t fileNameScrollIndex = 0;
String fileScroll;
uint8_t lcdSelectionIndex = 0;
LiquidCrystal lcd(25, 26, 10, 11, 12, 13, 14, 15, 16, 17); //Same data bus as sound chips + PB5 & PB6

//Clocks
uint32_t masterClockFrequency = 8000000;

//Sound Chips
SN76489 sn76489 = SN76489();
YM2612 ym2612 = YM2612();

//SD Card
SdFat SD;
File file;
#define FIRST_FILE 0x00
#define NEXT_FILE 0x01
#define PREV_FILE 0x02
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
bool LoadFile(byte strategy);
bool LoadFile(String req);
void SetVoice(Voice v);
void removeSVI();
void ReadVoiceData();
void HandleSerialIn();
void DumpVoiceData(Voice v);
void ResetSoundChips();
void HandleRotaryButtonDown();
void HandleRotaryEncoder();
void LCDInit();
void ScrollFileNameLCD();

void setup() 
{
  Serial.begin(115200);
  lcd.createChar(0, arrowCharLeft);
  lcd.createChar(1, arrowCharRight);
  lcd.begin(LCD_COLS, LCD_ROWS);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  delay(20); //Wait for clocks to start
  sn76489.Reset();
  ym2612.Reset();
  usbMIDI.setHandleNoteOn(KeyOn);
  usbMIDI.setHandleNoteOff(KeyOff);
  usbMIDI.setHandleProgramChange(ProgramChange);
  usbMIDI.setHandlePitchChange(PitchChange);
  usbMIDI.setHandleControlChange(ControlChange);

  MIDI.setHandleNoteOn(KeyOn);
  MIDI.setHandleNoteOff(KeyOff);
  MIDI.setHandleProgramChange(ProgramChange);
  MIDI.setHandlePitchBend(PitchChange);
  MIDI.setHandleControlChange(ControlChange);

  pinMode(DLED, OUTPUT);
  pinMode(PROG_UP, INPUT_PULLUP);
  pinMode(PROG_DOWN, INPUT_PULLUP);
  pinMode(LFO_TOG, INPUT_PULLUP);
  pinMode(ENC_BTN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_BTN), HandleRotaryButtonDown, FALLING);

  if(!SD.begin(20, SPI_HALF_SPEED))
  {
    digitalWrite(DLED, HIGH);
    while(true){Serial.println("SD Mount failed!");}
  }
  removeSVI();
  LoadFile(FIRST_FILE);
  ReadVoiceData();
  ym2612.SetVoice(voices[0]);
  DumpVoiceData(voices[0]);
  LCDInit();
}

bool LoadFile(byte strategy) //Request a file with NEXT, PREV, FIRST commands
{
  File nextFile;
  memset(fileName, 0x00, MAX_FILE_NAME_SIZE);
  switch(strategy)
  {
    case FIRST_FILE:
    {
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
      currentFileNumber = 0;
      return true;
    break;
    }
    case NEXT_FILE:
    {
      if(currentFileNumber+1 >= numberOfFiles)
      {
          SD.vwd()->rewind();
          currentFileNumber = 0;
      }
      else
          currentFileNumber++;
      nextFile.openNext(SD.vwd(), O_READ);
      nextFile.getName(fileName, MAX_FILE_NAME_SIZE);
      nextFile.close();
      break;
    }
    case PREV_FILE:
    {
      if(currentFileNumber != 0)
      {
        currentFileNumber--;
        SD.vwd()->rewind();
        for(uint32_t i = 0; i<=currentFileNumber; i++)
        {
          nextFile.close();
          nextFile.openNext(SD.vwd(), O_READ);
        }
        nextFile.getName(fileName, MAX_FILE_NAME_SIZE);
        nextFile.close();
      }
      else
      {
        currentFileNumber = numberOfFiles-1;
        SD.vwd()->rewind();
        for(uint32_t i = 0; i<=currentFileNumber; i++)
        {
          nextFile.close();
          nextFile.openNext(SD.vwd(), O_READ);
        }
        nextFile.getName(fileName, MAX_FILE_NAME_SIZE);
        nextFile.close();
      }
      break;
    }
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
  LCDInit();
  return true;
}

bool LoadFile(String req) //Request a file (string) to load
{
  bool fileFound = false;
  char searchFn[MAX_FILE_NAME_SIZE];
  SD.vwd()->rewind();
  Serial.print("REQUEST: "); Serial.println(req);
  File nextFile;
  for(uint32_t i = 0; i<numberOfFiles; i++)
  {
    nextFile.close();
    nextFile.openNext(SD.vwd(), O_READ);
    nextFile.getName(searchFn, MAX_FILE_NAME_SIZE);
    String tmpFN = String(searchFn);
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
    Serial.println("Error: File not found!");
    return false;
  }
  memset(fileName, 0x00, MAX_FILE_NAME_SIZE);
  strncpy(fileName, searchFn, MAX_FILE_NAME_SIZE);
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
  LCDInit();
  return true;
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

void HandleRotaryButtonDown()
{
  lcd.setCursor(0, lcdSelectionIndex);
  lcd.print(" ");
  lcdSelectionIndex++;
  lcdSelectionIndex %= 4;
  lcd.setCursor(0, lcdSelectionIndex);
  lcd.write((uint8_t)0); //Arrow Left
}

void LCDInit()
{
  lcd.clear();
  lcd.home();
  lcdSelectionIndex = 0;
  lcd.setCursor(0, lcdSelectionIndex);
  lcd.print(" ");
  lcd.setCursor(0, lcdSelectionIndex);
  lcd.write((uint8_t)0); //Arrow Left
  lcd.setCursor(1, 0);
  String fn = fileName;
  fn = fn.remove(LCD_COLS-1);
  fileScroll = fileName;
  lcd.print(fn);
  lcd.setCursor(1, 1);
  lcd.print("                   ");
  lcd.setCursor(1, 1);
  lcd.print("Voice # ");
  lcd.print(currentProgram);
  lcd.print("/");
  lcd.print(maxValidVoices-1);
  lcd.setCursor(1, 2);
  lcd.print("LFO: ");
  lcd.print(ym2612.lfoOn ? "ON" : "OFF");
}

void ResetSoundChips()
{
  ym2612.Reset();
  sn76489.Reset();
  ym2612.SetVoice(voices[currentProgram]);
  Serial.println("Soundchips Reset");
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
    ym2612.SetChannelOn(key+SEMITONE_ADJ_YM, velocity);
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
      program = maxValidVoices-1;
    program %= maxValidVoices;
    currentProgram = program;
    lcd.setCursor(1, 1);
    lcd.print("                   ");
    lcd.setCursor(1, 1);
    lcd.print("Voice # ");
    lcd.print(currentProgram);
    lcd.print("/");
    lcd.print(maxValidVoices-1);
    ym2612.SetVoice(voices[currentProgram]);
    Serial.print("Current Voice Number: "); Serial.print(currentProgram); Serial.print("/"); Serial.println(maxValidVoices-1);
    DumpVoiceData(voices[currentProgram]);
    lastProgram = program;
  }
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
      case 'l': //Toggle the Low Frequency Oscillator
      {
        ym2612.ToggleLFO();
        lcd.setCursor(1, 2);
        lcd.print("        ");
        lcd.setCursor(1, 2);
        lcd.print("LFO: ");
        lcd.print(ym2612.lfoOn ? "ON" : "OFF");
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
      case '>': //Move the entire keyboard up one ocatave for the YM2612
      {
        ym2612.ShiftOctaveUp();
        return;
      }
      case '<': //Move the entire keyboard down one ocatave for the YM2612
      {
        ym2612.ShiftOctaveDown();
        return;
      }
      case '?': //List the currently loaded OPM file
      {
        Serial.println(fileName);
        return;
      }
      case '!': //Reset the sound chips
      {
        ResetSoundChips();
        return;
      }
      case 'v': //Toggle velocity sensitivity for the PSG
      {
        sn76489.ToggleVelocitySensitivity();
        return;
      }
      case 'r': //Request a new opm file. format:    r:myOpmFile.opm
      {
        String req = Serial.readString();
        req.remove(0, 1); //Remove colon character
        LoadFile(req);
      }
      break;
      default:
        continue;
    }
  }
}

void HandleRotaryEncoder()
{
  long enc = encoder.read();
  if(enc != encoderPos && enc % 4 == 0) //Rotary encoders often have multiple steps between each descrete 'click.' In this case, it is 4
  {
    bool isEncoderUp = enc > encoderPos;
    switch(lcdSelectionIndex)
    {
      case 0:
      {
        LoadFile(isEncoderUp ? NEXT_FILE : PREV_FILE);
        LCDInit();
      break;
      }
      case 1:
      {
        ProgramChange(YM_CHANNEL, isEncoderUp ? currentProgram+1 : currentProgram-1);
      break;
      }
      case 2:
      {
        ym2612.ToggleLFO();
        lcd.setCursor(1, 2);
        lcd.print("        ");
        lcd.setCursor(1, 2);
        lcd.print("LFO: ");
        lcd.print(ym2612.lfoOn ? "ON" : "OFF");
      break;
      }
      case 3:
      {

      break;
      }
    }
    encoderPos = enc;
  }
}

uint32_t prevMilli = 0;
uint16_t scrollDelay = 500;
void ScrollFileNameLCD()
{
  // if(lcdSelectionIndex != 0)
  //   return;
  if(strlen(fileName) <= LCD_COLS-1)
    return;
  uint32_t curMilli = millis();
  if(curMilli - prevMilli >= scrollDelay)
  {
    prevMilli = curMilli;
    //Clear top line
    lcd.setCursor(0, 0);
    lcd.println("");
    lcd.setCursor(0, 0);

    //Draw filename substring    
    lcd.setCursor(1, 0);
    String sbStr = fileScroll.substring(fileNameScrollIndex, fileNameScrollIndex+LCD_COLS-1);
    fileNameScrollIndex++;
    if(fileNameScrollIndex+(LCD_COLS-2) >= strlen(fileName))
    {
      fileNameScrollIndex = 0;
      scrollDelay *= 5;
    }
    else
      scrollDelay = 500;
    lcd.print(sbStr);

    //Redraw selection arrow
    if(lcdSelectionIndex == 0)
    {
      lcd.setCursor(0, lcdSelectionIndex);
      lcd.write(" ");
      lcd.setCursor(0, lcdSelectionIndex);
      lcd.write((uint8_t)0); //Arrow Left
    }
    else
    {
      lcd.setCursor(0,0);
      lcd.write(" ");
      lcd.setCursor(0,0);
    }    
  }
}

void loop() 
{
  usbMIDI.read();
  MIDI.read();
  HandleRotaryEncoder();
  ScrollFileNameLCD();
  
  if(Serial.available() > 0)
    HandleSerialIn();

  if(!digitalReadFast(LFO_TOG))
  {
    ym2612.ToggleLFO();
    delay(200);
  }
}
