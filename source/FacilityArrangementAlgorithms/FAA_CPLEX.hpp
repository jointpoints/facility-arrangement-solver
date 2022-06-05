/**
 * @file FAA_CPLEX.hpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FAA_CPLEX_HPP__
#define __FASOLVER_FAA_CPLEX_HPP__





#include "../Common/Common.hpp"
#include "FAA_Common.hpp"
#include <ilcplex/ilocplex.h>





namespace faa
{





/**
 * @brief CPLEX-based solver
 *
 * details
 */
FACILITY_ARRANGEMENT_ALGORITHM(CPLEX)
{
	// Define types of variables based on parameters
#define DEFINE_CPLEX_TYPE(name, ...) \
using name = std::conditional        \
<                                    \
    __VA_ARGS__,                     \
    IloInt,                          \
    IloNum                           \
>::type
	DEFINE_CPLEX_TYPE(CplexDistanceType, std::is_same<DistanceType, FASInteger>::value);
	DEFINE_CPLEX_TYPE(CplexAreaOutputType, std::is_same<SubjectCountOutputType, FASInteger>::value && std::is_same<AreaInputType, FASInteger>::value);
	DEFINE_CPLEX_TYPE(CplexSubjectCountOutputType, std::is_same<SubjectCountOutputType, FASInteger>::value);
	DEFINE_CPLEX_TYPE(CplexUnitOutputType, std::is_same<UnitOutputType, FASInteger>::value);
#undef DEFINE_CPLEX_TYPE
#define DEFINE_CPLEX_TYPE_VAR(name, ...) \
using name = std::conditional            \
<                                        \
    __VA_ARGS__,                         \
    IloIntVar,                           \
    IloNumVar                            \
>::type;
	DEFINE_CPLEX_TYPE_VAR(CplexSubjectCountOutputTypeVar, std::is_same<SubjectCountOutputType, FASInteger>::value);
	DEFINE_CPLEX_TYPE_VAR(CplexUnitOutputTypeVar, std::is_same<UnitOutputType, FASInteger>::value);
#undef DEFINE_CPLEX_TYPE_VAR

	// Start preparation of a CPLEX model
	IloEnv cplex_environment;

	// CPLEX variables
	// Flows of objects (cplex_var_flow[{u,v}][{i,j}] = flow of objects from all objects
	// of type i located in u to all objects of type j located in v)
	BinaryPairMap<CplexUnitOutputTypeVar> cplex_var_flow;
	// Objects produced by subjects (cplex_var_produced[u][i] = number of objects
	// produced by all subjects of type i located in u)
	BinaryMap<CplexUnitOutputTypeVar> cplex_var_produced;
	// Number of subjects (cplex_var_subject_count[u][i] = number of subjects of type i
	// located in u)
	BinaryMap<CplexSubjectCountOutputTypeVar> cplex_var_subject_count;
	// Number of additionally bought subjects (cplex_var_bought_subject_count[i] = number
	// of additionally bought subjects of type i)
	UnaryMap<CplexSubjectCountOutputTypeVar> cplex_var_bought_subject_count;

	// Objective function 1
	IloExpr cplex_objective1(cplex_environment);

	// Constraints:
	// (1) - input of a subject cannot exceed its input capacity
	// (2) - output of a subject cannot exceed its output capacity
	// (3) - total area of all subjects placed into a point cannot exceed its area
	//       capacity
	// (4) - relaxed Kirchhoff constraint, output of a subject is not greater than its
	//       input
	// (5) - total flow requirements are met
	// (6) - production requirements are met
	// (7) - all subjects are placed into points

	// CPLEX model
	IloModel cplex_model(cplex_environment);

	// Add constraints to the model
	for (auto const &[type1_name, type1] : subject_types)
	{
		IloExpr cplex_constr_produced_sum(cplex_environment);
		IloExpr cplex_constr_total_subject_count(cplex_environment);
		for (auto const &[point1_name, point1] : points)
		{
			IloExpr cplex_constr_in_sum(cplex_environment);
			IloExpr cplex_constr_out_sum(cplex_environment);
			for (auto const &[type2_name, type2] : subject_types)
				for (auto const &[point2_name, point2] : points)
				{
					// Initialise cplex_var_flow
					cplex_var_flow[{point1_name, point2_name}].try_emplace
					(
						{type1_name, type2_name},
						cplex_environment, 0, std::numeric_limits<CplexUnitOutputType>::max(), ("f(" + type1_name + "," + type2_name + ")(" + point1_name + "," + point2_name + ")").data()
					);
					cplex_var_flow[{point2_name, point1_name}].try_emplace
					(
						{type2_name, type1_name},
						cplex_environment, 0, std::numeric_limits<CplexUnitOutputType>::max(), ("f(" + type2_name + "," + type1_name + ")(" + point2_name + "," + point1_name + ")").data()
					);
					// Aggregate constraints
					cplex_constr_in_sum += cplex_var_flow[{point2_name, point1_name}][{type2_name, type1_name}];
					cplex_constr_out_sum += cplex_var_flow[{point1_name, point2_name}][{type1_name, type2_name}];
					cplex_objective1 += (CplexDistanceType)distance(point1, point2) * cplex_var_flow[{point1_name, point2_name}][{type1_name, type2_name}];
				}
			// Initialise cplex_var_produced and cplex_var_subject_count
			cplex_var_produced[point1_name].try_emplace
			(
				type1_name,
				cplex_environment, 0, std::numeric_limits<CplexUnitOutputType>::max(), ("p(" + point1_name + ")(" + type1_name + ")").data()
			);
			cplex_var_subject_count[point1_name].try_emplace
			(
				type1_name,
				cplex_environment, 0, std::numeric_limits<CplexSubjectCountOutputType>::max(), ("n(" + point1_name + ")(" + type1_name + ")").data()
			);
			// Aggregate constraints
			cplex_constr_produced_sum += cplex_var_produced[point1_name][type1_name];
			cplex_constr_total_subject_count += cplex_var_subject_count[point1_name][type1_name];
			// (1)
			cplex_model.add
			(
				cplex_constr_in_sum <= (CplexUnitOutputType)type1.in_capacity * cplex_var_subject_count[point1_name][type1_name]
			);
			// (2)
			cplex_model.add
			(
				cplex_constr_out_sum <= (CplexUnitOutputType)type1.out_capacity * cplex_var_subject_count[point1_name][type1_name]
			);
			// (4)
			cplex_model.add
			(
				cplex_constr_out_sum <= cplex_var_produced[point1_name][type1_name] + cplex_constr_in_sum
			);
		}
		// Production plan calculation; deliberately separated into positive and negative
		// parts to minimise error in case of floating point numbers
		CplexUnitOutputType cplex_scal_total_in_plan(0);
		CplexUnitOutputType cplex_scal_total_out_plan(0);
		for (auto const &[type2_name, type2] : subject_types)
		{
			cplex_scal_total_in_plan += total_flows.at(type2_name).at(type1_name);
			cplex_scal_total_out_plan += total_flows.at(type1_name).at(type2_name);
			IloExpr cplex_constr_type1_type2_total_flow(cplex_environment);
			for (auto const &[point1_name, point1] : points)
				for (auto const &[point2_name, point2] : points)
					cplex_constr_type1_type2_total_flow += cplex_var_flow[{point1_name, point2_name}][{type1_name, type2_name}];
			// (5)
			cplex_model.add
			(
				cplex_constr_type1_type2_total_flow == (CplexUnitOutputType)total_flows.at(type1_name).at(type2_name)
			);
		}
		// (6)
		cplex_model.add
		(
			cplex_constr_produced_sum == std::max(cplex_scal_total_out_plan - cplex_scal_total_in_plan, (CplexUnitOutputType)0)
		);
		// Initialise cplex_var_bought_subject_count
		cplex_var_bought_subject_count.try_emplace
		(
			type1_name,
			cplex_environment, 0, std::numeric_limits<CplexSubjectCountOutputType>::max(), ("nt(" + type1_name + ")").data()
		);
		// (7)
		cplex_model.add
		(
			cplex_constr_total_subject_count == (CplexSubjectCountOutputType)type1.initially_available + cplex_var_bought_subject_count[type1_name]
		);
	}
	for (auto const &[point_name, point] : points)
	{
		IloExpr cplex_constr_occupied_area(cplex_environment);
		for (auto const &[type_name, type] : subject_types)
			cplex_constr_occupied_area += (CplexAreaOutputType)type.area * cplex_var_subject_count[point_name][type_name];
		// (3)
		cplex_model.add
		(
			cplex_constr_occupied_area <= (CplexAreaOutputType)point.areaTotal()
		);
	}

	// Set objective functions
	cplex_model.add(IloMinimize(cplex_environment, cplex_objective1));

	// Solve
	IloCplex cplex(cplex_model);
	cplex.setOut(logger.getInfoCallback());
	cplex.setWarning(logger.getWarningCallback());
	cplex.setError(logger.getErrorCallback());
	cplex.solve();

	cplex_environment.end();

	return;
}





} // faa





#endif // __FASOLVER_FAA_CPLEX_HPP__
