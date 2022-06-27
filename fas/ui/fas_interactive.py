'''
Facility Arrangement Solver for Python
Version: 1.0.0
Author : Andrew Eliseev (JointPoints)
'''
from ui.console import *
from tools.arrangement import *
import os



def cmd_cls():
	os.system('cls' if os.name in ('nt', 'dos') else 'clear')
	print('╔' + '═' * 68 + '╗')
	print('║ Facility Arrangement Solver for Python                     v.1.0.0 ║')
	print('║' + ' ' * 68 + '║')
	print('║ interactive mode                Andrew Eliseev (JointPoints), 2022 ║')
	print('╚╦' + '═' * 67 + '╝')
	print(' ║')
	print(' ║ Type "help" to get the list of available commands.')
	print(' ║')
	return



def cmd_editf():
	MAX_NAME_LEN = 20
	MAX_X_LEN = 10
	MAX_Y_LEN = 10
	MAX_AREA_LEN = 10
	points = {}
	command = None
	uiprint('Interactive facility editor')
	while command != 'quit':
		uiprint('\nYour current facility:')
		uiprint(f' │{"Name of a point":^20}│{"x":^10}│{"y":^10}│{"Area":^10}│')
		uiprint(f' ├{"─" * 20}┼{"─" * 10}┼{"─" * 10}┼{"─" * 10}┤')
		if len(points) == 0:
			uiprint(f' │{"<empty facility>":^53}│')
		else:
			for point_name in points:
				uiprint(f' │{point_name:<20}│{points[point_name].x:>10}│{points[point_name].y:>10}│{points[point_name].area:>10}│')
		uiprint('''
Commands available within this editor:
 add  : Add a new point or modify an existing one.
 del  : Remove an existing point.
 load : Load a facility from FASF file (overrides the current
        facility).
 quit : Exit the editor (disregards unsaved changes).
 save : Save current facility to a FASF file.''')
		command = uiinput('Your command: ')
		# If user wants to add/edit a point
		if command == 'add':
			point_name = uiinput('    Enter the name of the point: ')
			if len(point_name) > MAX_NAME_LEN:
				uiprint(f'    ERROR: The name is too long (max. {MAX_NAME_LEN} symbols).')
				continue
			try:
				x = abs(int(uiinput('    Enter the x coordinate: ')))
				if len(str(x)) > MAX_X_LEN:
					uiprint(f'    ERROR: The x is too long (max. {MAX_X_LEN} symbols).')
					continue
			except:
				uiprint('    ERROR: x must be an integer.')
				continue
			try:
				y = abs(int(uiinput('    Enter the y coordinate: ')))
				if len(str(y)) > MAX_Y_LEN:
					uiprint(f'    ERROR: The y is too long (max. {MAX_Y_LEN} symbols).')
					continue
			except:
				uiprint('    ERROR: y must be an integer.')
				continue
			try:
				area = abs(int(uiinput('    Enter the area: ')))
				if len(str(area)) > MAX_AREA_LEN:
					uiprint(f'    ERROR: The area is too long (max. {MAX_AREA_LEN} symbols).')
					continue
			except:
				uiprint('    ERROR: Area must be an integer.')
				continue
			points[point_name] = Point(x, y, area)
		# If user wants to delete a point
		elif command == 'del':
			point_name = uiinput('    Enter the name of the point: ')
			if len(point_name) > MAX_NAME_LEN:
				uiprint(f'    ERROR: The name is too long (max. {MAX_NAME_LEN} symbols).')
				continue
			if point_name in points:
				del points[point_name]
		# If user wants to load a facility from a file
		elif command == 'load':
			facility_path = uiinput('    Enter the path to a FASF file: ')
			try:
				points = load_facility(facility_path)
			except RuntimeError as e:
				uiprint(f'    {e}')
				continue
		# If user wants to save the current facility
		elif command == 'save':
			facility_path = uiinput('    Enter the path to a FASF file: ')
			try:
				save_facility(points, facility_path)
			except RuntimeError as e:
				uiprint(f'    {e}')
				continue
	return



def cmd_help():
	uiprint('''Availbale commands in interactive mode
 cls   : Clear console.
 editf : Edit facility or create a new one.
 help  : Show brief help message.
 hhelp : Show complete help message.
 quit  : Exit the program.

Command line syntax
 python fas.py -o <fileo> -f <filef> -g <fileg> -t <filet> [...]
To get more information, type "hhelp".''')
	return



def cmd_hhelp():
	uiprint('''Availbale commands in interactive mode
 cls   : Clear console.
 editf : Edit facility or create a new one.
 help  : Show brief help message.
 hhelp : Show complete help message.
 quit  : Exit the program.

Command line syntax
 python fas.py -o <fileo> -f <filef> -g <fileg> -t <filet> [...]
Required arguments
 -o <fileo>
 --output <fileo> : The path to the output file. Output file will
        contain information about the arrangement produced by the
        program. Arrangement will be saved in FASA format.
 -f <filef>
 --facility <filef> : The path to a facility layout file in FASF
        format.
 -g <fileg>
 --groups <fileg> : The path to a subject groups description file
        in FASG format.
 -t <filet>
 -totalflows <filet> : The path to a total flows description file
        in FAST format.
Optional arguments
 -a <valuea>
 --algorithm <valuea> : The algorithm to use for arrangement.
        Possible values of <valuea> are:
         mip_linear : Sets up a mixed integer linear optimisation
                      problem to be solved with CPLEX.
         mip_cubic  : Sets up a mixed integer cubic optimisation
                      problem to be solved with CPLEX.
        Omitting this argument is equivalent to -a mip_linear.
 -d (mN|moo|<filed>)
 --distance (mN|moo|<filed>) : The distance to use for computati-
        ons. The values have the following effect:
         mN      : N = 1, 2, ..., 50. Sets up Minkowski distance
                   of order N.
         moo     : Sets up Minkowski distance of order infinity.
         <filed> : The path to the distance file in FASD format.
        Omitting this argument is equivalent to -d m2.''')
	return



def run():
	# Known commands and functions to call
	command_func = \
	{
		'cls'   : cmd_cls,
		'editf' : cmd_editf,
		'help'  : cmd_help,
		'hhelp' : cmd_hhelp,
	}
	cmd_cls()
	# Receive a command
	command = None
	while command != 'quit':
		command = input(' ╠═╕ ')
		print(' ║ │')
		if command not in command_func and command != 'quit':
			uiprint('ERROR: Unknown command.')
			uifinish()
			continue
		if command != 'quit':
			command_func[command]()
		uifinish(mode=command)
	pass
