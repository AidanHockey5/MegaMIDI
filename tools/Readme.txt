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
