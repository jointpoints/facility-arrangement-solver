'''
Facility Arrangement Solver for Python
Version: 1.0.0
Author : Andrew Eliseev (JointPoints)
'''
from tools.arrangement import *
from ui import fas_interactive
from sys import argv
import os



def parse_args(args):
	answer = {}
	arg_translate = \
	{
		'-o'          : 'output',
		'--output'    : 'output',
		'-f'          : 'facility',
		'--facility'  : 'facility',
		'-g'          : 'groups',
		'--groups'    : 'groups',
		'-a'          : 'algo',
		'--algorithm' : 'algo',
		'-d'          : 'dist',
		'--distance'  : 'dist',
	}
	arg_specification = \
	{
		'output'   : {None},
		'facility' : {None},
		'groups'   : {None},
		'algo'     : {'mip_linear', 'mip_cubic'},
		'dist'     : {f'm{N}' for N in range(1, 51)} | {'moo', None},
	}
	arg_expected = True
	curr_arg = None
	for arg in args:
		if arg_expected:
			if arg in arg_translate:
				curr_arg = arg_translate[arg]
			else:
				print('ERROR: Unknown command line argument. To get help execute:')
				print('\tpython fas.py')
				print('\thelp')
				exit(1)
		else:
			if (arg in arg_specification[curr_arg]) or (None in arg_specification[curr_arg]):
				answer[curr_arg] = arg
			else:
				print('ERROR: Invalid value of an argument. To get help execute:')
				print('\tpython fas.py')
				print('\thelp')
				exit(1)
		arg_expected = not arg_expected
	if 'algo' not in answer:
		answer['algo'] = 'mip_linear'
	if 'dist' not in answer:
		answer['dist'] = 'm2'
	if ('output' not in answer) or ('facility' not in answer) or ('groups' not in answer):
		print('ERROR: One or more required arguments are missing. To get help execute:')
		print('\tpython fas.py')
		print('\thelp')
		exit(1)
	return answer



def run(**kwargs):
	points = None
	algo = \
	{
		'mip_linear' : arrange_0,
	}
	# Try to load facility
	if os.path.isfile(kwargs['facility']):
		points = load_facility(kwargs['facility'])
	else:
		print('ERROR: Facility file is invalid.')
		exit(1)
	return



def main():
	args = argv[1:]
	# If there are no input arguments given, launch an interactive mode
	if len(args) == 0:
		fas_interactive.run()
	else:
		run(**parse_args(args))
	return



if __name__ == '__main__':
	main()
