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
 load : Load {"a facility" if fasf else "subject groups"} from {mode.upper()} file (overrides current
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
	return



def _editortd(mode: str):
	assert mode in {'fast', 'fasd'}
	fast = (mode == 'fast')
	COL_1_LEN = 20
	COL_2_LEN = 20
	COL_3_LEN = 10
	answer = None
	command = None
	uiprint(f'Interactive {"total flows" if fast else "distance"} editor')
	while command != 'quit':
		if answer == None:
			uiprint(f'''
Commands available within this editor:
 load : Load a {"set of total flows" if fast else "distance"} from {mode.upper()} file.
 new  : Create a new {"set of total flows" if fast else "distance"}.
 quit : Exit the editor.''')
		else:
			uiprint(f'''
Commands available within this editor:
 edit : Change value for a certain pair of {"subject groups" if fast else "points"}.
 load : Load a {"set of total flows" if fast else "distance"} from {mode.upper()} file (overrides
        current progress).
 new  : Create a new {"set of total flows" if fast else "distance"} (disregards unsaved
        changes).
 quit : Exit the editor (disregards unsaved changes).
 save : Save current {"total flows" if fast else "distance"} to a {mode.upper()} file.
 show : Print current {"set of total flows" if fast else "distance"}.''')
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
	uiprint('''Availbale commands in interactive mode
 cls   : Clear console.
 editf : Edit facility or create a new one.
 editg : Edit a set of subject groups or create a new one.
 editt : Edit a set of total flows or create a new one.
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
 editt : Edit a set of total flows or create a new one.
 help  : Show brief help message.
 hhelp : Show complete help message.
 quit  : Exit the program.

Command line syntax
 python fas.py -o <fileo> -f <filef> -g <fileg> -t <filet> [...]
Required arguments
 -o <fileo>
 --output <fileo> : The path to the output file. Output file will
        contain information about the arrangement produced by the
        program. Format of the output file depends on the chosen
        arrangement algorithm.
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
                      problem to be solved with CPLEX. Saves ar-
                      rangement in SOL format.
         mip_cubic  : Sets up a mixed integer cubic optimisation
                      problem to be solved with CPLEX. Saves ar-
                      rangement in SOL format.
        Omitting this argument is equivalent to -a mip_linear.
 -d (mN|moo|<filed>)
 --distance (mN|moo|<filed>) : The distance to use for computati-
        ons. The values have the following effect:
         mN      : N = 1, 2, ..., 50. Sets up Minkowski distance
                   of order N.
         moo     : Sets up Minkowski distance of order infinity.
         <filed> : The path to the distance file in FASD format.
        Omitting this argument is equivalent to -d m2.
 -l <filel>
 --log <filel> : The path to a log file.
        Omitting this argument will lead to logs being printed in
        console.''')
	return



def run():
	# Known commands and functions to call
	command_func = \
	{
		'cls'   : cmd_cls,
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
