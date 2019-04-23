# Mega MIDI

https://www.aidanlawrence.com/mega-midi-a-playable-version-of-my-hardware-sega-genesis-synth/

This project contains the source material for the Mega MIDI, a MIDI-controlled Sega Genesis/Megadrive Synthesizer that uses the genuine sound chips from the console itself. The primary FM sound source is the Yamaha YM2612, but I have also included the SN76489 PSG which was also present on the Sega Genesis. This synthesizer does not use emulated sound - it's the real deal. Therefore, you are limited to the limitations of the actual hardware, which I will explain in more detail down below.

# Why Not Just Emulate?
You'd be hard-pressed to find an emulated YM2612 core that sounds exactly like the real YM2612. There are some that get close, but nothing compares to the real thing. Yamaha's FM synthesis is pretty tricky to emulate in the first place, but the YM2612's signature gritty DAC is mostly to blame for the lack of truly accurate emulation. Not with The Mega MIDI though. Since you are using the real YM2612 and SN76489 ICs, you can experience playing the Sega Genesis as a musical instrument with the true fidelity and accuracy as the real thing.

# Play Any Instrument from Any Genesis Video Game Music
The Mega MIDI reads in ".opm" files. OPM files were originally designed for the VOPM virtual synthesizer. The OPM format stores the register data for FM patches that you can use to easily program your YM2612 with. FM synthesis is very complicated, so while you still have the option to create your own FM patches, it would be way easier to just directly take the FM voice patches from the games themselves and use them on your Mega MIDI. With the Mega MIDI, you can, in fact, DIRECTLY play FM patches from any Sega Genesis game's music. These aren't "sound-alike" patches either, they are the actual identical patches lifted directly from the game's source. Do you like ThunderForce's Guitar patch in the track Metal Squad? Play it. Want to try your hand playing Chemical Plant Zone's bass patch? Go for it. Any track. Any instrument. Instantly.

# Where Do I find OPM files?
There are two ways:

1) There is a massive pack of ripped OPM files ready to go that spans almost the entire Genesis library. You can find just about anything in this collection. [Click here to download it.](https://www.aidanlawrence.com/wp-content/uploads/2019/03/2612org-OPMs.zip)

2) You can convert any OPN2(YM2612) .VGM file to an OPM patch using the vgm2opm tool courtesy of [Shiru](https://shiru.untergrund.net) https://github.com/vampirefrog/fmtools

I have included a windows-compiled binary of vgm2opm within the [tools](https://github.com/AidanHockey5/MegaMIDI/tree/master/tools) directory of this repository along with a batch-file to automatically execute the tool. To use this tool, simply place your desired .vgm/vgz files within the VGM_IN folder, double-click the CONVERT_VGM_OPM.bat file, then retrieve your OPM patch files in the OPM_OUT folder.

Here is the schematic so far. I'll release these files once I'm done with them.
![Here is the schematic so far](https://i.imgur.com/WZymTlD.jpg)
