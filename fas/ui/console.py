'''
Facility Arrangement Solver for Python
Version: 1.0.0
Author : Andrew Eliseev (JointPoints)
'''



def uiprint(output: str):
	lines = output.split('\n')
	for line in lines:
		print(' ║ │ ' + line)
	return



def uifinish(mode=None):
	if mode != 'cls':
		print(f' {"╨" if mode == "quit" else "║"} └' + '─' * 66)
	return
