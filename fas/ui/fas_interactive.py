'''
Facility Arrangement Solver for Python
Version: 1.0.0
Author : Andrew Eliseev (JointPoints)
'''
from ui.console import *
from tools.arrangement import *
import os



def cmd_help():
	uiprint('''Availbale commands in interactive mode
 cls   : Clear console.
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



def run():
	# Known commands and functions to call
	command_func = \
	{
		'cls'   : cmd_cls,
		'help'  : cmd_help,
		'hhelp' : cmd_hhelp
	}
	cmd_cls()
	# Receive a command
	command = None
	while command != 'quit':
		command = input(' ╠═╕ ')
		print(' ║ │')
		if command not in command_func and command != 'quit':
			uiprint('ERROR: Command not found.')
			uifinish()
			continue
		if command != 'quit':
			command_func[command]()
		uifinish(mode=command)
	pass
