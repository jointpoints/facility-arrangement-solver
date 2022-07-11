'''
Facility Arrangement Solver for Python
Version: 1.0.0
Author : Andrew Eliseev (JointPoints)
'''
from tools.arrangement import *
from ui import fas_interactive
from sys import argv



def parse_args(args):
	answer = {}
	arg_translate = \
	{
		'-o'           : 'output',
		'--output'     : 'output',
		'-f'           : 'facility',
		'--facility'   : 'facility',
		'-g'           : 'groups',
		'--groups'     : 'groups',
		'-t'           : 'total_flows',
		'--totalflows' : 'total_flows',
		'-a'           : 'algo',
		'--algorithm'  : 'algo',
		'-d'           : 'dist',
		'--distance'   : 'dist',
		'-l'           : 'log',
		'--log'        : 'log',
	}
	arg_specification = \
	{
		'output'      : {None},
		'facility'    : {None},
		'groups'      : {None},
		'total_flows' : {None},
		'algo'        : {'mip_linear', 'mip_cubic'},
		'dist'        : {f'm{N}' for N in range(1, 51)} | {'moo', None},
		'log'         : {None}
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
	if 'log' not in answer:
		answer['log'] = None
	if ('output' not in answer) or ('facility' not in answer) or ('groups' not in answer):
		print('ERROR: One or more required arguments are missing. To get help execute:')
		print('\tpython fas.py')
		print('\thelp')
		exit(1)
	return answer



def run(**kwargs):
	algo = \
	{
		'mip_linear' : arrange_0,
	}
	# Try to load data
	try:
		points = fas_load(kwargs['facility'], 'fasf')
		groups = fas_load(kwargs['groups'], 'fasg')
		total_flows = fas_load(kwargs['total_flows'], 'fast')
	except RuntimeError as e:
		print(e)
		exit(1)
	# Compute the distance
	distance = {}
	if kwargs['dist'] in {f'm{_}' for _ in range(1, 51)}:
		order = int(kwargs['dist'][1:])
		for point1_name in points:
			for point2_name in points:
				if order == 1:
					distance[(point1_name, point2_name)] = abs(points[point1_name].x - points[point2_name].x) + abs(points[point1_name].y - points[point2_name].y)
				else:
					distance[(point1_name, point2_name)] = (abs(points[point1_name].x - points[point2_name].x)**order + abs(points[point1_name].y - points[point2_name].y)**order)**(1/order)
	elif kwargs['dist'] == 'moo':
		for point1_name in points:
			for point2_name in points:
				distance[(point1_name, point2_name)] = max(abs(points[point1_name].x - points[point2_name].x), abs(points[point1_name].y - points[point2_name].y))
	algo[kwargs['algo']](points, distance, groups, total_flows, kwargs['output'], kwargs['log'])
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
