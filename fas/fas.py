'''
Facility Arrangement Solver for Python
Version: 1.0.0
Author : Andrew Eliseev (JointPoints)
'''
from tools.arrangement import *
from ui import fas_interactive
from re import match
from sys import argv



def parse_args(args):
	answer = {}
	arg_translate = \
	{
		'-o'             : 'output',
		'--output'       : 'output',
		'-f'             : 'facility',
		'--facility'     : 'facility',
		'-g'             : 'groups',
		'--groups'       : 'groups',
		'-t'             : 'total_flows',
		'--totalflows'   : 'total_flows',
		'-a'             : 'algo',
		'--algorithm'    : 'algo',
		'-d'             : 'dist',
		'--distance'     : 'dist',
		'--forcevanilla' : 'force_vanilla',
		'-l'             : 'log',
		'--log'          : 'log',
	}
	arg_specification = \
	{
		'output'        : {None},
		'facility'      : {None},
		'groups'        : {None},
		'total_flows'   : {None},
		'algo'          : {'linear', 'cpr_linear', 'cpr_linear_gfred'},
		'dist'          : {f'm{N}' for N in range(1, 51)} | {'moo', None},
		'force_vanilla' : {0},
		'log'           : {None},
	}
	arg_expected = True
	curr_arg = None
	for arg in args:
		if arg_expected:
			if arg in arg_translate:
				curr_arg = arg_translate[arg]
				if 0 in arg_specification[curr_arg]:
					answer[curr_arg] = True
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
		arg_expected = (not arg_expected) or (0 in arg_specification[curr_arg])
	if 'algo' not in answer:
		answer['algo'] = 'cpr_linear'
	if 'dist' not in answer:
		answer['dist'] = 'm2'
	if 'force_vanilla' not in answer:
		answer['force_vanilla'] = False
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
		'cpr_linear'       : arrange_cpr_linear,
		'cpr_linear_gfred' : arrange_cpr_linear_gfred,
	}
	# Try to load data
	try:
		if match('g[0-9]+:[.0-9]+x[0-9]+:[.0-9]+x[0-9]+', kwargs['facility']) != None:
			kwargs['facility'] = kwargs['facility'][1:]
			row_count = int(kwargs['facility'][:kwargs['facility'].index(':')])
			kwargs['facility'] = kwargs['facility'][kwargs['facility'].index(':')+1:]
			row_step = float(kwargs['facility'][:kwargs['facility'].index('x')])
			kwargs['facility'] = kwargs['facility'][kwargs['facility'].index('x')+1:]
			column_count = int(kwargs['facility'][:kwargs['facility'].index(':')])
			kwargs['facility'] = kwargs['facility'][kwargs['facility'].index(':')+1:]
			column_step = float(kwargs['facility'][:kwargs['facility'].index('x')])
			kwargs['facility'] = kwargs['facility'][kwargs['facility'].index('x')+1:]
			area = int(kwargs['facility'])
			if row_count * column_count * area == 0:
				raise ValueError()
			points = {f'({i},{j})' : Point(column_step * i, row_step * j, area) for i in range(column_count) for j in range(row_count)}
			grid_size = (column_count, row_count) if not kwargs['force_vanilla'] else None
		else:
			points = fas_load(kwargs['facility'], 'fasf')
			grid_size = None
		groups = fas_load(kwargs['groups'], 'fasg')
		total_flows = fas_load(kwargs['total_flows'], 'fast')
	except RuntimeError as e:
		print(e)
		exit(1)
	except ValueError:
		print('ERROR: Grid parameters are invalid.')
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
	# Run an arrangement algorithm
	try:
		algo[kwargs['algo']](points, distance, groups, total_flows, kwargs['output'], kwargs['log'], grid_size)
	except RuntimeError as e:
		print(e)
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
