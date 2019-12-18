# Mega MIDI

https://www.aidanlawrence.com/mega-midi-a-playable-version-of-my-hardware-sega-genesis-synth/

[Instruction Manual](https://www.aidanlawrence.com/wp-content/uploads/2019/07/MegaMidi5Manual.pdf)

[Store Link](https://www.aidanlawrence.com/product/mega-midi-5/)

This project contains the source material for the Mega MIDI, a MIDI-controlled Sega Genesis/Megadrive Synthesizer that uses the genuine sound chips from the console itself. The primary FM sound source is the Yamaha YM2612, but I have also included the SN76489 PSG which was also present on the Sega Genesis. This synthesizer does not use emulated sound - it's the real deal. Therefore, you are limited to the limitations of the actual hardware, which I will explain in more detail down below.

# Why Not Just Emulate?
You'd be hard-pressed to find an emulated YM2612 core that sounds exactly like the real YM2612. There are some that get close, but nothing compares to the real thing. Yamaha's FM synthesis is pretty tricky to emulate in the first place, but the YM2612's signature gritty DAC is mostly to blame for the lack of truly accurate emulation. Not with The Mega MIDI though. Since you are using the real YM2612 and SN76489 ICs, you can experience playing the Sega Genesis as a musical instrument with the true fidelity and accuracy as the real thing.

# Play Any Instrument from Any Genesis Video Game Music
The Mega MIDI reads in ".opm" files. OPM files were originally designed for the VOPM virtual synthesizer. The OPM format stores the register data for FM patches that you can use to easily program your YM2612 with. FM synthesis is very complicated, so while you still have the option to create your own FM patches, it would be way easier to just directly take the FM voice patches from the games themselves and use them on your Mega MIDI. With the Mega MIDI, you can, in fact, DIRECTLY play FM patches from any Sega Genesis game's music. These aren't "sound-alike" patches either, they are the actual identical patches lifted directly from the game's source. Do you like ThunderForce's Guitar patch in the track Metal Squad? Play it. Want to try your hand playing Chemical Plant Zone's bass patch? Go for it. Any track. Any instrument. Instantly.

# Where Do I find OPM files?
There are two ways:

1) There is a massive pack of ripped OPM files ready to go that spans almost the entire Genesis library. You can find just about anything in this collection. [Click here to download it.](https://www.aidanlawrence.com/wp-content/uploads/2019/03/2612org-OPMs.zip)
One thing to note: This is a REALLY big collection of files. I don't recommend putting them ALL on your SD card at once as load times will begin to suffer. 

2) You can convert any OPN2(YM2612) .VGM file to an OPM patch using the vgm2opm tool courtesy of [Shiru](https://shiru.untergrund.net) and https://github.com/vampirefrog/fmtools

I have included a windows-compiled binary of vgm2opm within the [tools](https://github.com/AidanHockey5/MegaMIDI/tree/master/tools) directory of this repository along with a batch-file to automatically execute the tool. To use this tool, simply place your desired .vgm/vgz files within the VGM_IN folder, double-click the CONVERT_VGM_OPM.bat file, then retrieve your OPM patch files in the OPM_OUT folder.

# USB MIDI and Traditional DIN MIDI Compatible
Want to control The Mega MIDI through software like Ableton, FL Studio, or any other DAW? You can! The Mega MIDI will show up like any other MIDI-compatible instrument and is able to receive native USB MIDI commands without any sort of serial bridge. 
Prefer old-school traditional 5-pin DIN MIDI instead? Go for it! Bust out that classic MIDI controller and plug it in with zero additional setup required.

# Crystal-Clear Amplification 
Now featuring a modified version of the Mega Amp Mod! Beautifully clear amplification for both the YM3438 and the YM2612!

# 7 "Favorite" Buttons to Quickly Swap Between Your Favorite Patches
While you do have access to every .opm file on your Mega MIDI's SD card, sometimes you'll find the *perfect* patch that you'll want to keep for easy-access later. Simply hold-down one of the seven "favorite" buttons at the bottom of the board and your currently loaded patch will be saved into EEPROM. Because favorite patches are saved to EEPROM, you can access them instantaneously without any load times. Perfect for swapping patches on-the-fly!

# Real Hardware, Real Limitations
Because you are working with the real sound chips, you are also working under their real limitations. The YM2612 is a six-channel sound chip, meaning you can play six individual notes at once. The SN76489 PSG is a 3-channel square-wave-only (excluding noise) chip. In total, you can have nine total channels of sound if you're playing both the PSG and the YM2612 at the same time.

While some might find this a bit limiting, others may enjoy the challenge of getting as much sound as they possibly can within the limits of the genuine hardware. Countless other video game music composers did it, now you can experience it too!

# Open-source
All of my source material has been made open-source. Want to build your own Mega MIDI from scratch? Go for it! Have some modifications in mind? I'd love to see them! If you do decide your own version, please credit me: Aidan Lawrence and refer back to this original repository. Please do not resell this project.

# Tools
The firmware for this project was developed in [Visual Studio Code](https://code.visualstudio.com/) with the [PlatformIO extention.](https://docs.platformio.org/en/latest/ide/vscode.html) I used the Teensyduino (ArduinoTeensy) toolchain.

The schematics/PCB files were made with [KiCAD.](http://kicad-pcb.org/) Specifically, version 5.0.2.

Precompiled firmware HEX files can be found here:
https://github.com/AidanHockey5/MegaMIDI/releases/

Here is the schematic:
![Here is the schematic so far](https://github.com/AidanHockey5/MegaMIDI/raw/master/Schematic/YM2612_MIDI_SMD/YM2612_MIDI_SMD.png)
