About:

VGM2OPM is a small command-line tool, which allows to extract OPN instruments
from *.vgm or *.vgz files and save them as *.opm file, which can be used with
VOPM VSTi.  Tool can process  one or few files,  or  whole directory with all
sub-directories at once. It also removes copies of instruments with different
volumes (extracts only most loud ones of same).  All files will be saved into
tool directory under same name as original files, but with *.opm extension.


Usage:

VGM2OPM FILENAME.VGM (or .VGZ) - process one VGM file
VGM2OPM FILE1.VGM FILE2.VGM .. - process few VGM files
VGM2OPM PATH\* - process all VGM files from directory and all sub-directores


Notes:

Tool was tested with VGM v1.80 files. Tool was not crash-tested, by processing
thousands of files at once, for example.
 
To compile source code you'll need zlib library from http://www.zlib.net/.


History:

v1.0 01.09.08 - Initial release.


mailto: shiru@mail.ru
homepage: shiru.untergrund.net