'''
Facility Arrangement Solver for Python
Version: 1.0.0
Author : Andrew Eliseev (JointPoints)
'''
from tools.components import *
from re import match
import sys
import cplex



def arrange_cplex_compressed_linear(points: "dict[str, Point]", distance, groups: "dict[str, SubjectGroup]", total_flows: TotalFlows, arrangement_path: str = None, log_path: str = None, grid_size: tuple = None, is_fas_interactive: bool = False):
	ceil = lambda x: x if x == int(x) else int(x) + 1
	# Compute the sufficient number of subjects for each group
	total_subject_count = {i : max(ceil(total_flows.get_in_flow(i) / groups[i].input_capacity), ceil(total_flows.get_out_flow(i) / groups[i].output_capacity)) for i in groups}
	# Compute total production by each subject group
	total_production_count = {i : total_flows.get_out_flow(i) - total_flows.get_in_flow(i) for i in groups}
	# CPLEX model
	cplex_model = cplex.Cplex()
	cplex_model.objective.set_sense(cplex_model.objective.sense.minimize)
	# CPLEX variables
	cplex_model.variables.add \
	(
		names=[f'f({i},{j})[{u},{v}]' for i in groups for j in groups for u in points for v in points],
		lb=[0] * len(groups)**2 * len(points)**2,
		ub=[total_flows[(i, j)] for i in groups for j in groups for u in points for v in points],
		types=[cplex_model.variables.type.integer] * len(groups)**2 * len(points)**2
	)
	cplex_model.variables.add \
	(
		names=[f'n({i})[{u}]' for i in groups for u in points],
		lb=[0] * len(groups) * len(points),
		ub=[total_subject_count[i] for i in groups for u in points],
		types=[cplex_model.variables.type.integer] * len(groups) * len(points)
	)
	cplex_model.variables.add \
	(
		names=[f'g({i})[{u}]' for i in groups for u in points],
		lb=[-total_production_count[i] if total_production_count[i] >= 0 else total_production_count[i] for i in groups for u in points],
		ub=[total_production_count[i] if total_production_count[i] >= 0 else 0 for i in groups for u in points],
		types=[cplex_model.variables.type.integer] * len(groups) * len(points)
	)
	# Objective function
	cplex_model.objective.set_linear(tuple((f'f({i},{j})[{u},{v}]', distance[(u, v)]) for i in groups for j in groups for u in points for v in points))
	# Constraints
	for i in groups:
		for u in points:
			cplex_constr_output = cplex.SparsePair \
			(
				ind=tuple(f'f({i},{j})[{u},{v}]' for j in groups for v in points) + (f'n({i})[{u}]',),
				val=(1,) * len(groups) * len(points)                              + (- groups[i].output_capacity,)
			)
			cplex_constr_input = cplex.SparsePair \
			(
				ind=tuple(f'f({j},{i})[{v},{u}]' for j in groups for v in points) + (f'n({i})[{u}]',),
				val=(1,) * len(groups) * len(points)                              + (- groups[i].input_capacity,)
			)
			cplex_constr_kirchhoff = cplex.SparsePair \
			(
				ind=tuple(f'f({i},{j})[{u},{v}]' for j in groups for v in points) + (f'g({i})[{u}]',) + tuple(f'f({j},{i})[{v},{u}]' for j in groups for v in points if j != i),
				val=(1,) * len(groups) * len(points)                              + (- 1,)            + tuple(- 1 for j in groups for v in points if j != i)
			)
			cplex_model.linear_constraints.add \
			(
				lin_expr=(cplex_constr_output, cplex_constr_input, cplex_constr_kirchhoff),
				senses=('L', 'L', 'E'),
				rhs=(0, 0, 0)
			)
		cplex_constr_total_production = cplex.SparsePair \
		(
			ind=tuple(f'g({i})[{u}]' for u in points),
			val=(1,) * len(points)
		)
		cplex_constr_total_subject_count = cplex.SparsePair \
		(
			ind=tuple(f'n({i})[{u}]' for u in points),
			val=(1,) * len(points)
		)
		cplex_model.linear_constraints.add \
		(
			lin_expr=(cplex_constr_total_production, cplex_constr_total_subject_count),
			senses=('E', 'E'),
			rhs=(total_production_count[i], total_subject_count[i])
		)
		for j in groups:
			cplex_constr_total_flow = cplex.SparsePair \
			(
				ind=tuple(f'f({i},{j})[{u},{v}]' for u in points for v in points),
				val=(1,) * len(points)**2
			)
			cplex_model.linear_constraints.add \
			(
				lin_expr=(cplex_constr_total_flow,),
				senses=('E',),
				rhs=(total_flows[(i, j)],)
			)
	for u in points:
		cplex_constr_area = cplex.SparsePair \
		(
			ind=tuple(f'n({i})[{u}]' for i in groups),
			val=tuple(groups[i].area for i in groups)
		)
		cplex_model.linear_constraints.add \
		(
			lin_expr=(cplex_constr_area,),
			senses=('L',),
			rhs=(points[u].area,)
		)
	# Grid-specific constraints
	if grid_size != None:
		cplex_constr_grid_first_row = cplex.SparsePair \
		(
			ind=tuple(f'n({i})[({x},0)]' for i in groups for x in range(grid_size[0])),
			val=(1,) * len(groups) * grid_size[0]
		)
		cplex_constr_grid_first_column = cplex.SparsePair \
		(
			ind=tuple(f'n({i})[(0,{y})]' for i in groups for y in range(grid_size[1])),
			val=(1,) * len(groups) * grid_size[1]
		)
		cplex_constr_grid_left_half = cplex.SparsePair \
		(
			ind=tuple(f'n({i})[({x},{y})]' for i in groups for x in range(grid_size[0]) for y in range(grid_size[1])),
			val=tuple(1 if x <= ceil(grid_size[0] / 2) else -1 for i in groups for x in range(grid_size[0]) for y in range(grid_size[1]))
		)
		cplex_constr_grid_upper_half = cplex.SparsePair \
		(
			ind=tuple(f'n({i})[({x},{y})]' for i in groups for x in range(grid_size[0]) for y in range(grid_size[1])),
			val=tuple(1 if x <= ceil(grid_size[1] / 2) else -1 for i in groups for x in range(grid_size[0]) for y in range(grid_size[1]))
		)
		cplex_model.linear_constraints.add \
		(
			lin_expr=(cplex_constr_grid_first_row, cplex_constr_grid_first_column, cplex_constr_grid_left_half, cplex_constr_grid_upper_half),
			senses=('G', 'G', 'G', 'G'),
			rhs=(1, 1, 0, 0)
		)
	# Set up logs
	if log_path != None:
		try:
			log_file = open(log_path, 'w')
		except:
			log_file = open('bad_log_file_name_backup_save.log', 'w')
		cplex_model.set_log_stream(log_file)
		cplex_model.set_error_stream(log_file)
		cplex_model.set_warning_stream(log_file)
		cplex_model.set_results_stream(log_file)
	elif is_fas_interactive:
		cplex_model.set_log_stream(sys.stdout, lambda x : ' ║ │' + x)
		cplex_model.set_error_stream(sys.stderr, lambda x : ' ║ │' + x)
		cplex_model.set_warning_stream(sys.stderr, lambda x : ' ║ │' + x)
		cplex_model.set_results_stream(sys.stderr, lambda x : ' ║ │' + x)
	# Solve
	cplex_model.solve()
	if (cplex_model.solution.get_status() == 103):
		raise RuntimeError('ERROR: No feasible solution.')
	# Save the solution
	if arrangement_path != None:
		try:
			cplex_model.solution.write(arrangement_path)
		except:
			cplex_model.solution.write('bad_output_file_name_backup_save.sol')
	# Close all streams
	if log_path != None:
		log_file.close()

	return



def arrange_1(points: "dict[str, Point]", distance, groups: "dict[str, SubjectGroup]", flows: "dict[str, dict[str, list[list[int]]]]", arrangement_path: str = None, log_path: str = None, is_fas_interactive: bool = False):
	# CPLEX model
	cplex_model = cplex.Cplex()
	cplex_model.objective.set_sense(cplex_model.objective.sense.minimize)
	# CPLEX variables
	cplex_model.variables.add \
	(
		names=[f'f({i} : {p}, {j} : {q})' for i in groups for p in range(len(flows[i][i])) for j in groups for q in range(len(flows[j][j]))],
		lb=[flows[i][j][p][q] for i in groups for p in range(len(flows[i][i])) for j in groups for q in range(len(flows[j][j]))],
		ub=[flows[i][j][p][q] for i in groups for p in range(len(flows[i][i])) for j in groups for q in range(len(flows[j][j]))],
		types=[cplex_model.variables.type.integer for i in groups for p in range(len(flows[i][i])) for j in groups for q in range(len(flows[j][j]))]
	)
	cplex_model.variables.add \
	(
		names=[f'b({i} : {p})[{u}]' for i in groups for p in range(len(flows[i][i])) for u in points],
		lb=[0 for i in groups for p in range(len(flows[i][i])) for u in points],
		ub=[1 for i in groups for p in range(len(flows[i][i])) for u in points],
		types=[cplex_model.variables.type.integer for i in groups for p in range(len(flows[i][i])) for u in points]
	)
	# Objective function
	quardatic_coeffs = [[[], []] for idx in range(cplex_model.variables.get_num())]
	for i in groups:
		for p in range(len(flows[i][i])):
			for u in points:
				idx = cplex_model.variables.get_indices(f'b({i} : {p})[{u}]')
				quardatic_coeffs[idx] = [[f'b({j} : {q})[{v}]' for j in groups for q in range(len(flows[j][j])) for v in points], [distance[(u, v)] * (flows[i][j][p][q] + flows[j][i][q][p]) for j in groups for q in range(len(flows[j][j])) for v in points]]
	cplex_model.objective.set_quadratic(quardatic_coeffs)
	# Constraints
	for i in groups:
		for p in range(len(flows[i][i])):
			cplex_model.linear_constraints.add \
			(
				lin_expr=(cplex.SparsePair \
				(
					ind=tuple(f'b({i} : {p})[{u}]' for u in points),
					val=(1,) * len(points)
				),),
				senses=('E',),
				rhs=(1,)
			)
	for u in points:
		cplex_model.linear_constraints.add \
		(
			lin_expr=(cplex.SparsePair \
			(
				ind=tuple(f'b({i} : {p})[{u}]' for i in groups for p in range(len(flows[i][i]))),
				val=tuple(1 for i in groups for p in range(len(flows[i][i])))
			),),
			senses=('L',),
			rhs=(points[u].area,)
		)
	# Set up logs
	if log_path != None:
		try:
			log_file = open(log_path, 'w')
		except:
			log_file = open('bad_log_file_name_emergency_save.log', 'w')
		cplex_model.set_log_stream(log_file)
		cplex_model.set_error_stream(log_file)
		cplex_model.set_warning_stream(log_file)
		cplex_model.set_results_stream(log_file)
	elif is_fas_interactive:
		cplex_model.set_log_stream(sys.stdout, lambda x : ' ║ │' + x)
		cplex_model.set_error_stream(sys.stderr, lambda x : ' ║ │' + x)
		cplex_model.set_warning_stream(sys.stderr, lambda x : ' ║ │' + x)
		cplex_model.set_results_stream(sys.stderr, lambda x : ' ║ │' + x)
	# Solve
	cplex_model.solve()
	if (cplex_model.solution.get_status() == 103):
		raise RuntimeError('ERROR: No feasible solution.')
	# Save the solution
	if arrangement_path != None:
		try:
			cplex_model.solution.write(arrangement_path)
		except:
			cplex_model.solution.write('bad_output_file_name_emergency_save.sol')
	# Close all streams
	if log_path != None:
		log_file.close()

	return
