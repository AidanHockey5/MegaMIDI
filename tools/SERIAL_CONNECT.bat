start ./Terminal/teensy_gateway.exe
start ./Terminal/putty.exe -telnet 127.0.0.1 28541

::This batch file will open the teensy_gateway program and a putty serial window
::Keep the black teensy_gateway window open and work inside of the putty window
::If everything is working, you should see the text "teensy_gateway" inside of the putty window.
::Use the putty window as if you were using the Arduino Serial console.
::Type-in your commands, then hit enter to execute them