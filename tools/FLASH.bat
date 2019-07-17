@echo off
title Mega MIDI AVRDUDE Firmware Flasher Script
echo This program will flash your firmware automatically. Make sure you have ran the PlatformIO "Compile" function first to generate a .hex
echo Press "CONTROL+C" to exit at any time.
echo .
echo .
echo .
:start
echo Please enter the COM port number that your Arduino is on (not the Mega MIDI, the Arduino programmer)
echo (Do not include the 'COM' prefix, just the number. You can find this COM port number in the Arduino IDE under Tools--Port)
set /p com="COM: "

if "%com%" == "" goto start

echo Please select your programming speed, or press enter to automatically choose the default 19200 baud
echo (Type the number only, i.e. '1' or '2')
echo 1) 19200
echo 2) 115200
echo 3) 1000000

set /p speed="Enter Speed #: "

if "%speed%"=="" goto flash1
if "%speed%"=="1" goto flash1
if "%speed%"=="2" goto flash2
if "%speed%"=="3" goto flash3

goto start 

:reflash
cd "..\..\..\tools\"
echo Press any key to program again using the same settings.
pause >nul
if "%speed%"=="" goto flash1
if "%speed%"=="1" goto flash1
if "%speed%"=="2" goto flash2
if "%speed%"=="3" goto flash3


:flash1
@echo on
cd "..\.pio\build\teensy*\"
"..\..\..\tools\FLASH_FIRMWARE\avrdude.exe" -c arduino -p usb1286 -P COM%com% -b 19200 -U flash:w:"firmware.hex":a -U lfuse:w:0x5E:m -U hfuse:w:0xDF:m -U efuse:w:0xF3:m 
@echo off
goto reflash

:flash2
@echo on
cd "..\.pio\build\teensy*\"
"..\..\..\tools\FLASH_FIRMWARE\avrdude.exe" -c arduino -p usb1286 -P COM%com% -b 115200 -U flash:w:"firmware.hex":a -U lfuse:w:0x5E:m -U hfuse:w:0xDF:m -U efuse:w:0xF3:m 
@echo off
goto reflash

:flash3
@echo on
cd "..\.pio\build\teensy*\"
"..\..\..\tools\FLASH_FIRMWARE\avrdude.exe" -c arduino -p usb1286 -P COM%com% -b 1000000 -U flash:w:"firmware.hex":a -U lfuse:w:0x5E:m -U hfuse:w:0xDF:m -U efuse:w:0xF3:m 
@echo off
goto reflash

pause
