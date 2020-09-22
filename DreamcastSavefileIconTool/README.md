# Protofall's Dreamcast Savefile Icon Creator

This program will take a set of PNGs that are all 32 pixels wide and a multiple of 32 pixels high, stitch them together and make a PAL4BPP ARGB4444 texture in the form of a image and palette binaries. These binaries can then be added to a Dreamcast savefile as the icon on the BIOS screen. You can have between 1 and 3 frames of animation (Each frame is 32 by 32). Any higher won't render correctly due to have the DC BIOS works so a warning is thrown when creating the binaries for inputs with more than 3 frames

Dependencies:

+ libpng
+ math
+ scons (Build language, use pip3)

Run `scons` to compile the program.
Run `./DreamcastSavefileIconTool --input-image [png_filename_1] [png_filename_2] (etc.) --output-image [filename] -- output-palette [filename] --preview [filename]` to create the image and palette binaries. You must provide input and output-binary paths

### Credits

MrNeo240 has a similar tool on his website and he helped explain how the format worked

Kresna and Falco Girgis for identifying the DC BIOS can only handle 3 frames of animation
