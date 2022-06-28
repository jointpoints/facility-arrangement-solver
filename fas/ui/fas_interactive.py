'''
Facility Arrangement Solver for Python
Version: 1.0.0
Author : Andrew Eliseev (JointPoints)
'''
from ui.console import *
from tools.arrangement import *
import os






# Util






def _editor(mode: str):
	assert mode in {'fasf', 'fasg'}
	fasf = (mode == 'fasf')
	COL_1_LEN = 20
	COL_2_LEN = 10
	COL_3_LEN = 10
	COL_4_LEN = 10
	answer = {}
	command = None
	uiprint(f'Interactive {"facility" if fasf else "subject groups"} editor')
	while command != 'quit':
		uiprint(f'\nYour current {"facility" if fasf else "set of subject groups"}:')
		uiprint(f' │{"Name of a point" if fasf else "Name of a group":^20}│{"x" if fasf else "Inp. cap.":^10}│{"y" if fasf else "Outp. cap.":^10}│{"Area":^10}│')
		uiprint(f' ├{"─" * 20}┼{"─" * 10}┼{"─" * 10}┼{"─" * 10}┤')
		if len(answer) == 0:
			uiprint(f' │{"<empty facility>" if fasf else "<empty set of subjec groups>":^53}│')
		else:
			for name in answer:
				uiprint(f' │{name:<20}│{answer[name].x if fasf else answer[name].input_capacity:>10}│{answer[name].y if fasf else answer[name].output_capacity:>10}│{answer[name].area:>10}│')
		uiprint(f'''
Commands available within this editor:
 add  : Add a new {"point" if fasf else "subject group"} or modify an existing one.
 del  : Remove an existing {"point" if fasf else "subject group"}.
 load : Load {"a facility" if fasf else "groups"} from {mode.upper()} file (overrides current
        progress).
 quit : Exit the editor (disregards unsaved changes).
 save : Save current {"facility" if fasf else "subject groups"} to a {mode.upper()} file.''')
		command = uiinput('Your command: ')
		# If user wants to add/edit a point/group
		if command == 'add':
			name = uiinput(f'    Enter the name of the {"point" if fasf else "group"}: ')
			if len(name) > COL_1_LEN:
				uiprint(f'    ERROR: The name is too long (max. {COL_1_LEN} symbols).')
				continue
			try:
				field1 = abs(int(uiinput(f'    Enter the {"x coordinate" if fasf else "input capacity"}: ')))
				if len(str(field1)) > COL_2_LEN:
					uiprint(f'    ERROR: The {"x" if fasf else "input capacity"} is too long (max. {COL_2_LEN} symbols).')
					continue
			except:
				uiprint(f'    ERROR: {"x" if fasf else "input capacity"} must be an integer.')
				continue
			try:
				field2 = abs(int(uiinput(f'    Enter the {"y coordinate" if fasf else "output capacity"}: ')))
				if len(str(field2)) > COL_3_LEN:
					uiprint(f'    ERROR: The {"y" if fasf else "output capacity"} is too long (max. {COL_3_LEN} symbols).')
					continue
			except:
				uiprint(f'    ERROR: {"y" if fasf else "output capacity"} must be an integer.')
				continue
			try:
				area = abs(int(uiinput('    Enter the area: ')))
				if len(str(area)) > COL_4_LEN:
					uiprint(f'    ERROR: The area is too long (max. {COL_4_LEN} symbols).')
					continue
			except:
				uiprint('    ERROR: Area must be an integer.')
				continue
			answer[name] = Point(field1, field2, area) if fasf else SubjectGroup(field1, field2, area)
		# If user wants to delete a point/group
		elif command == 'del':
			name = uiinput(f'    Enter the name of the {"point" if fasf else "group"}: ')
			if len(name) > COL_1_LEN:
				uiprint(f'    ERROR: The name is too long (max. {COL_1_LEN} symbols).')
				continue
			if name in answer:
				del answer[name]
		# If user wants to load a facility/subject groups from a file
		elif command == 'load':
			path = uiinput(f'    Enter the path to a {mode.upper()} file: ')
			try:
				answer = fas_load(path, mode)
			except RuntimeError as e:
				uiprint(f'    {e}')
				continue
		# If user wants to save the current facility/subject groups
		elif command == 'save':
			path = uiinput(f'    Enter the path to a {mode.upper()} file: ')
			try:
				fas_save(answer, path, mode)
			except RuntimeError as e:
				uiprint(f'    {e}')
				continue






# Commands






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
	_editor('fasf')
	return



def cmd_editg():
	_editor('fasg')
	return



def cmd_help():
	uiprint('''Availbale commands in interactive mode
 cls   : Clear console.
 editf : Edit facility or create a new one.
 editg : Edit a set of subject groups or create a new one.
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
 editg : Edit a set of subject groups or create a new one.
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
		'editg' : cmd_editg,
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
