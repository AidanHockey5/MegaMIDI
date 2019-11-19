#MACOS Note! Requires SSL command to be ran!
#run the following line in terminal
#                                      /Applications/Python\ 3.6/Install\ Certificates.command
#Replace '3.6' with what ever version of python your running


import sys
import glob
import serial
import requests
import json
import urllib
import os
from io import StringIO

def GetOS():
    os = ""
    if sys.platform.startswith('win'):
        os = "WIN"
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        os = "LIN"
    elif sys.platform.startswith('darwin'):
        os = "OSX"
    else:
        raise EnvironmentError('Unsupported platform')
    return os

OPERATING_SYSTEM = GetOS()
SCRIPT_LOCATION = sys.path[0]
AVRDUDE_LOCATION = ""
if(OPERATING_SYSTEM == "WIN"):
    AVRDUDE_LOCATION = SCRIPT_LOCATION+"\\FLASH_FIRMWARE"
elif(OPERATING_SYSTEM == "LIN" or OPERATING_SYSTEM == "OSX"):
    AVRDUDE_LOCATION = SCRIPT_LOCATION+"/FLASH_FIRMWARE"
def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if OPERATING_SYSTEM == "WIN":
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif OPERATING_SYSTEM == "LIN":
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif OPERATING_SYSTEM == "OSX":
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

print("---Megaflasher Python---")
print("This script will detect your ArduinoISP serial port and invoke Avrdude for you.")
print("Make sure your Arduino is programmed with the ArduinoISP example program from the Arduino IDE")
print("and is set to 115200 baud (uncomment line 143 in the ArduinoISP program).")
print("===================")
print("")

inputValid = False
ports = serial_ports()
selectedPort = 0
while inputValid == False:
    print("Please select your serial port by typing-in the boxed number that preceeds it")
    ports = serial_ports()

    if(len(ports) == 0):
        print("No serial devices found. Please connect your programmer and reload this program.")
        input()
        quit()

    i = 0
    for port in ports:
        print("[" + str(i) + "] " + port)
        i += 1
    selectedPort = int(input("-->"))
    if(selectedPort < 0 or selectedPort > i):
        print("Invalid selection, please try again")
    else:
        inputValid = True
        print("OK")
        print(ports[selectedPort] + " selected")

print("Fetching latest firmware...")
r = requests.get('https://api.github.com/repos/AidanHockey5/MegaMIDI/releases')
if(r.ok):
    repoItems = json.loads(r.text or r.content)
    latest = repoItems[0]["assets"][0]["browser_download_url"]
    print(latest)
    if(OPERATING_SYSTEM == "WIN"):
        urllib.request.urlretrieve (latest, AVRDUDE_LOCATION+"\\firmware.hex")
    elif(OPERATING_SYSTEM == "LIN" or OPERATING_SYSTEM == "OSX"):
        urllib.request.urlretrieve (latest, AVRDUDE_LOCATION+"/firmware.hex")
    print("OK")
    print("Invoking AVRDude at 115200 baud, please wait...")

    if(OPERATING_SYSTEM == "WIN"):
        os.system(AVRDUDE_LOCATION+"\\avrdude.exe -c arduino -p usb1286 -P" + ports[selectedPort] + " -b 115200 -U flash:w:\"" + AVRDUDE_LOCATION + "\\firmware.hex\"" + ":a -U lfuse:w:0x5E:m -U hfuse:w:0xDF:m -U efuse:w:0xF3:m")
    elif(OPERATING_SYSTEM == "LIN"):
        os.system(AVRDUDE_LOCATION+"/avrdude -c arduino -p usb1286 -P" + ports[selectedPort] + " -b 115200 -U flash:w:\"" + AVRDUDE_LOCATION + "/firmware.hex\"" + ":a -U lfuse:w:0x5E:m -U hfuse:w:0xDF:m -U efuse:w:0xF3:m")
    elif(OPERATING_SYSTEM == "OSX"):
        os.system(AVRDUDE_LOCATION+"/avrdude_osx" + " -C" + AVRDUDE_LOCATION+"/avrdude.conf" + " -v -pusb1286 -carduino -P"   + ports[selectedPort] + " -b115200 -U flash:w:\"" + AVRDUDE_LOCATION + "/firmware.hex\"" + ":a -U lfuse:w:0x5E:m -U hfuse:w:0xDF:m -U efuse:w:0xF3:m")
