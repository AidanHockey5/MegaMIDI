Here are a few tools to help you operate the YM2612 MIDI device
You need to make sure you have installed the Teensyduino files from PJRC!
https://www.pjrc.com/teensy/td_download.html


SERIAL CONNECTIONS
--------------------------------------------------
You can use the SERIAL_CONNECT.bat file to open up a virtual serial port in order to send commands to the device.

Inside of the SERIAL_CONNECT.bat file reads:
::This batch file will open the teensy_gateway program and a putty serial window
::Keep the black teensy_gateway window open and work inside of the putty window
::If everything is working, you should see the text "teensy_gateway" inside of the putty window.
::Use the putty window as if you were using the Arduino Serial console.
::Type-in your commands, then hit enter to execute them



CONVERT VGM FILES TO OPM FILES
--------------------------------------------------
You can use this tool to extract the voices out of VGM files and place them in an .OPM file to use with your YM2612 MIDI device
The VGM files MUST contain YM2612/YM3438 instrument data

Place your VGM/VGZ files in the VGM_IN and run the "CONVERT_VGM_OPM.bat" file.
After the program runs, go into the OPM_OUT folder and you should see your new .OPM files.
Add these .OPM files to the SD card of your YM2612 MIDI project.


PROGRAMMING
--------------------------------------------------
While you can easily use something like an ATMEL ICE programmer and Atmel Studio, you can also use a plain ol' Arduino board to act
as a programmer.

You can use the ArduinoISP sketch from the Arduino IDE to turn any standard Arduno Uno/Nano/Mega into a programmer.
It is reccomended that you change the ArduinoISP programmer speed to  #define BAUDRATE	1000000 (line 144), but the default
setting of 19200 works too, though it is very slow.


Pin connections:
The Mega MIDI 6-pin programming connector looks like this:

      MISO °. . 5V 
      SCK   . . MOSI
      RST   . . GND

Arduino | Mega MIDI programming header
GND     | GND
10      | RST
11      | MOSI
12      | MISO
13      | SCK

You do not need to connect 5V.

Run FLASH.bat and choose your COM port number and programming speed.

The SD card will not read properly after being programmed with an ISP device. Remove the SD card, reinsert the card, then press the RESET button on the Mega MIDI board.

Default AVRDUDE command is:
avrdude -c arduino -p usb1286 -P COM16 -b 19200 -U flash:w:"LOCATION_OF_YOUR_PROJECT_FOLDER\.pioenvs\teensy20pp\firmware.hex":a -U lfuse:w:0x5E:m -U hfuse:w:0xDF:m -U efuse:w:0xF3:m 
