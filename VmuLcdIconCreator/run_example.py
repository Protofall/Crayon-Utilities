import subprocess

prog = './' + 'VmuLcdBitmapTool'

# For displaying in handheld mode
subprocess.run([prog, '-i', 'images/icon_VMU_LCD.png', '-o', 'vmu_lcd1.bin', '-p', 'preview1.png'])
# For displaying via DC controller
subprocess.run([prog, '-i', 'images/icon_VMU_LCD.png', '-o', 'vmu_lcd2.bin', '-p', 'preview2.png', '--inv'])
# For greyscale
subprocess.run([prog, '-i', 'images/icon_VMU_LCD.png', '-o', 'vmu_lcd3.bin', '-p', 'preview3.png', '--inv', '-f 60'])
# Scale example
subprocess.run([prog, '-i', 'images/sure.png', '-o', 'vmu_lcd4.bin', '-p', 'preview4.png', '-s 6'])
