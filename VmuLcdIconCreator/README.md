# Protofall's VMU Savefile Icon Creator

This program will take a 48 wide by 32 high input image and create a 1bpp binary that can easily be displayed on a VMU screen. The input image must be a PNG now since I changed over to using libPNG for image reading (And writing)

Dependencies:

+ libPNG

Usage:
`./VmuLcdIconCreator --input [png_filename] --output-binary [filename] *--output-png [filename] *--invert`

The `--invert` parameter is optional, this will flip the vertical axis of the image. If writting a DC program that renders to the VMU then you'll want to set this flag (Since the VMU is upside down in the controller), but if you wanted to use this for a VMU in hand-held mode then you omit this flag.

