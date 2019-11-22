import sys
from cx_Freeze import setup, Executable

build_exe_options = {}
setup(  name = "MegaFlasher",
        version = "1.0.0",
        description = "Flash your Mega MIDI",
        options = {"build_exe": build_exe_options},
        executables = [Executable("MegaFlasher.py")])
