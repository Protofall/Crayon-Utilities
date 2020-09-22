import subprocess

prog = './' + 'DreamcastEyecatcherTool'

subprocess.run([prog, '-i', 'textures/simple1.png', '-o', 'eyecatcher1.bin', '-m', '1'])
subprocess.run([prog, '-i', 'textures/simple1.png', '-o', 'eyecatcher2.bin', '-m', '2'])
subprocess.run([prog, '-i', 'textures/simple1.png', '-o', 'eyecatcher3.bin', '-m', '3'])
subprocess.run([prog, '-i', 'textures/lines.png', '-o', 'eyecatcher_lines2.bin', '-m', '2', '-p', 'proc_lines2.png'])
	#Has 28 colours, colour count is reduced
subprocess.run([prog, '-i', 'textures/lines.png', '-o', 'eyecatcher_lines3.bin', '-m', '3', '-p', 'proc_lines3.png'])
