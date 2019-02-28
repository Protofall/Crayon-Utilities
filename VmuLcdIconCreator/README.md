# Protofall's VMU Savefile Icon Creator

This program will take a 32 wide by 48 high input image and create a 1bpp binary that can easily be displayed on a VMU screen. The input image can be in any format stb_image.h can load (However some of the supported formats don't work like paletted PNGs).

Usage:
`./VmuLcdIconCreator [Source_file_path] [Dest_file_path] [--invert]*`

The `--invert` parameter is optional, this will flip the vertical axis of the image. If writting a DC program that renders to the VMU then you'll want to set this flag (Since the VMU is upside down in the controller), but if you wanted to use this for a VMU in hand-held mode then you omit this flag.

Dependencies:

+ stb_image.h

