# Protofall's Dreamcast Eyecatcher Creator

This program will take a single PNG that is 72 pixels wide 56 pixels high and depending on what you set "Mode" to, you either make a non-paletted ARGB4444 binary (1) or a PAL8BPP (2) or PAL4BPP (3) ARGB4444 binary (The palette and image is in one binary). These binaries can then be added to a Dreamcast savefile as the eyecatcher on the BIOS screen (Displays on the VMU screen to the left).

Dependencies:

+ libpng
+ math
+ scons (Build language, use pip3)

Run `scons` to compile the program.
Run `./DreamcastEyecatcherTool --input-image [png_filename] --output-binary [filename] --mode [1 - 3] --preview [filename]` to create eyecatcher binary. You must provide input png and output binary paths

