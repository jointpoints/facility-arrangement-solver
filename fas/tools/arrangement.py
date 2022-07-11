'''
Facility Arrangement Solver for Python
Version: 1.0.0
Author : Andrew Eliseev (JointPoints)
'''
from tools.components import *
import cplex



def arrange_0(points: "dict[str, Point]", distance, groups: "dict[str, SubjectGroup]", total_flows: TotalFlows, arrangement_path: str = None):
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

	return
