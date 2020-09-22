import subprocess

prog = './' + 'DreamcastSavefileIconTool'

subprocess.run([prog, '-i', 'textures/single_test1.png', 'textures/single_test2.png', 'textures/single_test3.png', '-o', 'image1.bin', '--out-pal', 'palette1.bin'])
subprocess.run([prog, '-i', 'textures/three_test.png', '-o', 'image2.bin', '--out-pal', 'palette2.bin'])
# Has 28 colours, colour count is reduced
subprocess.run([prog, '-i', 'textures/lines.png', '-o', 'image3.bin', '--out-pal', 'palette3.bin'])
# This one generates a warning because the Dreamcast BIOS can't render it
subprocess.run([prog, '-i', 'textures/single_test1.png', 'textures/three_test.png', '-o', 'imageW.bin', '--out-pal', 'paletteW.bin'])
