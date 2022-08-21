'''
Facility Arrangement Solver for Python
Version: 1.0.0
Author : Andrew Eliseev (JointPoints)
'''
from ui.console import *
from tools.arrangement import *
import os






# Util






def _editorfg(mode: str):
	assert mode in {'fasf', 'fasg'}
	fasf = (mode == 'fasf')
	COL_1_LEN = 20
	COL_2_LEN = 10
	COL_3_LEN = 10
	COL_4_LEN = 10
	answer = {}
	command = None
	uiprint(f'\033[4mInteractive {"facility" if fasf else "subject groups"} editor\033[0m:')
	while command != 'quit':
		uiprint(f'\nYour current {"facility" if fasf else "set of subject groups"}:')
		uiprint(f' │{"Name of a point" if fasf else "Name of a group":^20}│{"x" if fasf else "Inp. cap.":^10}│{"y" if fasf else "Outp. cap.":^10}│{"Area":^10}│')
		uiprint(f' ├{"─" * 20}┼{"─" * 10}┼{"─" * 10}┼{"─" * 10}┤')
		if len(answer) == 0:
			uiprint(f' │{"<empty facility>" if fasf else "<empty set of subject groups>":^53}│')
		else:
			for name in answer:
				uiprint(f' │{name:<20}│{answer[name].x if fasf else answer[name].input_capacity:>10}│{answer[name].y if fasf else answer[name].output_capacity:>10}│{answer[name].area:>10}│')
		uiprint(f'''
\033[4mCommands available within this editor\033[0m:
 \033[1madd\033[0m  : Add a new {"point" if fasf else "subject group"} or modify an existing one.
 \033[1mdel\033[0m  : Remove an existing {"point" if fasf else "subject group"}.
 \033[1mload\033[0m : Load {"a facility" if fasf else "subject groups"} from {mode.upper()} file (overrides current
        progress).
 \033[1mquit\033[0m : Exit the editor (disregards unsaved changes).
 \033[1msave\033[0m : Save current {"facility" if fasf else "subject groups"} to a {mode.upper()} file.''')
		command = uiinput('Your command: ')
		# If user wants to add/edit a point/group
		if command == 'add':
			name = uiinput(f'    Enter the name of the {"point" if fasf else "group"}: ')
			if len(name) > COL_1_LEN:
				uiprint(f'    ERROR: The name is too long (max. {COL_1_LEN} symbols).')
				continue
			try:
				field1 = float(uiinput('    Enter the x coordinate: ')) if fasf else abs(int(uiinput('    Enter the input capacity: ')))
				if len(str(field1)) > COL_2_LEN:
					uiprint(f'    ERROR: The {"x" if fasf else "input capacity"} is too long (max. {COL_2_LEN} symbols).')
					continue
			except:
				uiprint(f'    ERROR: {"x" if fasf else "input capacity"} must be {"a real number" if fasf else "an integer"}.')
				continue
			try:
				field2 = float(uiinput('    Enter the y coordinate: ')) if fasf else abs(int(uiinput('    Enter the output capacity: ')))
				if len(str(field2)) > COL_3_LEN:
					uiprint(f'    ERROR: The {"y" if fasf else "output capacity"} is too long (max. {COL_3_LEN} symbols).')
					continue
			except:
				uiprint(f'    ERROR: {"y" if fasf else "output capacity"} must be {"a real number" if fasf else "an integer"}.')
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
	return



def _editortd(mode: str):
	assert mode in {'fast', 'fasd'}
	fast = (mode == 'fast')
	COL_1_LEN = 20
	COL_2_LEN = 20
	COL_3_LEN = 10
	answer = None
	command = None
	uiprint(f'\033[4mInteractive {"total flows" if fast else "distance"} editor\033[0m:')
	while command != 'quit':
		if answer == None:
			uiprint(f'''
\033[4mCommands available within this editor\033[0m:
 \033[1mload\033[0m : Load a {"set of total flows" if fast else "distance"} from {mode.upper()} file.
 \033[1mnew\033[0m  : Create a new {"set of total flows" if fast else "distance"}.
 \033[1mquit\033[0m : Exit the editor.''')
		else:
			uiprint(f'''
\033[4mCommands available within this editor\033[0m:
 \033[1medit\033[0m : Change value for a certain pair of {"subject groups" if fast else "points"}.
 \033[1mload\033[0m : Load a {"set of total flows" if fast else "distance"} from {mode.upper()} file (overrides
        current progress).
 \033[1mnew\033[0m  : Create a new {"set of total flows" if fast else "distance"} (disregards unsaved
        changes).
 \033[1mquit\033[0m : Exit the editor (disregards unsaved changes).
 \033[1msave\033[0m : Save current {"total flows" if fast else "distance"} to a {mode.upper()} file.
 \033[1mshow\033[0m : Print current {"set of total flows" if fast else "distance"}.''')
		command = uiinput('Your command: ')
		# If user wants to add/edit a point/group
		if command == 'edit' and answer != None:
			name1 = uiinput(f'    Enter the name of the source {"subject group" if fast else "point"}: ')
			if len(name1) > COL_1_LEN:
				uiprint(f'    ERROR: The name is too long (max. {COL_1_LEN} symbols).')
				continue
			if name1 not in (answer.groups if fast else answer):
				uiprint(f'    ERROR: Unknown {"subject group" if fast else "point"}.')
				continue
			name2 = uiinput(f'    Enter the name of the target {"subject group" if fast else "point"}: ')
			if len(name2) > COL_2_LEN:
				uiprint(f'    ERROR: The name is too long (max. {COL_2_LEN} symbols).')
				continue
			if name2 not in (answer.groups if fast else answer):
				uiprint(f'    ERROR: Unknown {"subject group" if fast else "point"}.')
				continue
			try:
				new_value = abs((int if fast else float)(uiinput('    Enter new value: ')))
				if len(str(new_value)) > COL_3_LEN:
					uiprint(f'    ERROR: The value is too long (max. {COL_3_LEN} symbols).')
					continue
			except:
				uiprint('    ERROR: Wrong format of the value.')
				continue
			if fast:
				answer.total_flow[(name1, name2)] = new_value
			else:
				answer[(name1, name2)] = new_value
		# If user wants to load a facility/subject groups from a file
		elif command == 'load':
			path = uiinput(f'    Enter the path to a {mode.upper()} file: ')
			try:
				answer = fas_load(path, mode)
			except RuntimeError as e:
				uiprint(f'    {e}')
				answer = None
				continue
		# If user wants to create new total flows/distance
		elif command == 'new':
			uiprint(f'    {"Total flows" if fast else "Distance"} must be associated with certain {"subject groups" if fast else "facility"}.')
			path = uiinput(f'    Enter the path to the associated {"FASG" if fast else "FASF"} file: ')
			try:
				associated = fas_load(path, 'fasg' if fast else 'fasf')
				answer = TotalFlows(associated) if fast else None
			except RuntimeError as e:
				uiprint(str(e))
				answer = None
				continue
			data_container = answer.total_flow if fast else answer
			for name1 in answer.groups if fast else answer:
				for name2 in answer.groups if fast else answer:
					try:
						data_container[(name1, name2)] = abs(int(uiinput(f'    Enter {"flow" if fast else "distance"} from {name1} to {name2}: ')))
					except:
						uiprint(f'    {"Flow" if fast else "Distance"} must be {"integer" if fast else "a number"}.')
						answer = None
						break
				else:
					continue
				break
		# If user wants to save the current total flows/distance
		elif command == 'save':
			path = uiinput(f'    Enter the path to a {mode.upper()} file: ')
			try:
				fas_save(answer, path, mode)
			except RuntimeError as e:
				uiprint(f'    {e}')
				continue
		# If user wants to see the table with all the values
		elif command == 'show' and answer != None:
			uiprint(f'\nYour current {"set of total flows" if fast else "distance"}:')
			uiprint(f' │{"Group 1" if fast else "Point 1":^20}│{"Group 2" if fast else "Point 2":^20}│{"Value":^10}│')
			uiprint(f' ├{"─" * 20}┼{"─" * 20}┼{"─" * 10}┤')
			for name in (answer.total_flow if fast else answer):
				uiprint(f' │{name[0]:<20}│{name[1]:<20}│{(answer.total_flow if fast else answer)[name]:>10}│')
	return






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



def cmd_editd():
	_editortd('fasd')
	return



def cmd_editf():
	_editorfg('fasf')
	return



def cmd_editg():
	_editorfg('fasg')
	return



def cmd_editt():
	_editortd('fast')
	return



def cmd_help():
	uiprint('''\033[4mAvailbale commands in interactive mode\033[0m
 \033[1mcls\033[0m   : Clear console.
 \033[1meditf\033[0m : Edit facility or create a new one.
 \033[1meditg\033[0m : Edit a set of subject groups or create a new one.
 \033[1meditt\033[0m : Edit a set of total flows or create a new one.
 \033[1mhelp\033[0m  : Show brief help message.
 \033[1mhhelp\033[0m : Show complete help message.
 \033[1mquit\033[0m  : Exit the program.

\033[4mCommand line syntax\033[0m
 python fas.py -o <fileo> -f <valuef> -g <fileg> -t <filet> [...]
To get more information, type "hhelp".''')
	return



def cmd_hhelp():
	uiprint('''\033[4mAvailbale commands in interactive mode\033[0m:
 \033[1mcls\033[0m   : Clear console.
 \033[1meditf\033[0m : Edit facility or create a new one.
 \033[1meditg\033[0m : Edit a set of subject groups or create a new one.
 \033[1meditt\033[0m : Edit a set of total flows or create a new one.
 \033[1mhelp\033[0m  : Show brief help message.
 \033[1mhhelp\033[0m : Show complete help message.
 \033[1mquit\033[0m  : Exit the program.

\033[4mCommand line syntax\033[0m:
 python fas.py -o <fileo> -f <valuef> -g <fileg> -t <filet> [...]
\033[4mRequired arguments\033[0m:
 \033[1m-o <fileo>\033[0m
 \033[1m--output <fileo>\033[0m : The path to the output file. Output file will
        contain information about the arrangement produced by the
        program. Output file is saved in SOL format.
 \033[1m-f <filef>\033[0m
 \033[1m--facility <valuef>\033[0m : The facility layout. Possible values of
        <valuef> are:
         <pathf> : The path to a facility layout file in FASF
                   format.
         gN:HxM:WxA : A grid facility with N rows and M columns.
                      The distance between any two consecutive
                      rows is H. The distance between any two
                      consecutive columns is W. Each point has
                      area A. N, M, and A must be natural, H and
                      W must be positive.
        Note, that passing gN:HxM:WxA as the value might also
        enable some additional modifications in the algorithms to
        allow a faster convergence to an answer. To disable these
        modifications, use --forcevanilla argument (see below).
 \033[1m-g <fileg>\033[0m
 \033[1m--groups <fileg>\033[0m : The path to a subject groups description file
        in FASG format.
 \033[1m-t <filet>\033[0m
 \033[1m-totalflows <filet>\033[0m : The path to a total flows description file
        in FAST format.
\033[4mOptional arguments\033[0m:
 \033[1m-a <valuea>\033[0m
 \033[1m--algorithm <valuea>\033[0m : The algorithm to use for arrangement.
        Possible values of <valuea> are:
         linear : Use CPLEX solver to solve an integer optimisa-
                  tion problem using the linear model. This algo-
                  rithm produces an \033[4moptimal\033[0m solution.
         cpr_linear : Use CPLEX solver to solve an integer opti-
                      misation problem using the compressed line-
                      ar model. This algorithm produces an \033[4mopti\033[0m-
                      \033[4mmal\033[0m solution.
         linear_gfred : Use CPLEX solver to solve multiple inte-
                        ger optimisation problems using the line-
                        ar model on facilities of reduced sizes.
                        This algorithm produces a \033[4msuboptimal\033[0m so-
                        lution unless proven that its solutions
                        are optimal.
         cpr_linear_gfred : Use CPLEX solver to solve multiple
                            integer optimisation problems using
                            the compressed linear model on faci-
                            lities of reduced sizes. This algo-
                            rithm produces a \033[4msuboptimal\033[0m solution
                            unless proven that its solutions are
                            optimal.
        Note, that X_gfred algorithms only work for grid facili-
        ties generated by gN:HxM:WxA value of -f key (see above).
        Calling them with any other facility is equivalent to
        -a X. Calling them with --forcevanilla (see below) is
        equivalent to -a X --forcevanilla.
        Omitting this argument is equivalent to -a cpr_linear.
 \033[1m-d (mN|moo|<filed>)\033[0m
 \033[1m--distance (mN|moo|<filed>)\033[0m : The distance to use for computati-
        ons. The values have the following effect:
         mN      : N = 1, 2, ..., 50. Sets Minkowski distance of
                   order N.
         moo     : Sets Minkowski distance of the infinte order.
         <filed> : The path to a distance file in FASD format.
        Omitting this argument is equivalent to -d m2.
 \033[1m--forcevanilla\033[0m : Disable all possible optimisations designed for
        grid facility layouts. Omitting this argument will lead
        to the usage of special versions of the arrangement algo-
        rithms that may accelerate computations for grid facili-
        lities.
 \033[1m-l <filel>\033[0m
 \033[1m--log <filel>\033[0m : The path to a log file.
        Omitting this argument will lead to logs being printed in
        console.''')
	return



def run():
	# Known commands and functions to call
	command_func = \
	{
		'cls'   : cmd_cls,
		#'editd' : cmd_editd,
		'editf' : cmd_editf,
		'editg' : cmd_editg,
		'editt' : cmd_editt,
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
