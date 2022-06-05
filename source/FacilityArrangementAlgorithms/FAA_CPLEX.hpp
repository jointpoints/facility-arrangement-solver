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
	using CplexUnitOutputTypeVar = std::conditional
	<
		std::is_same<UnitOutputType, FASInteger>::value,
		IloIntVar,
		IloNumVar
	>::type;
	using CplexSubjectCountOutputTypeVar = std::conditional
	<
		std::is_same<SubjectCountOutputType, FASInteger>::value,
		IloIntVar,
		IloNumVar
	>::type;

	// Start preparation of a CPLEX model
	IloEnv cplex_environment;

	// CPLEX variables and constraints
	// Flows of objects (cplex_var_flow[{u,v}][{i,j}] = flow of objects from all objects
	// of type i located in u to all objects of type j located in v)
	BinaryPairMap<CplexUnitOutputTypeVar> cplex_var_flow;
	// Objects produced by subjects (cplex_var_produced[u][i] = number of objects
	// produced by all subjects of type i located in u)
	BinaryMap<CplexUnitOutputTypeVar> cplex_var_produced;
	// Number of subjects (cplex_var_subject_count[u][i] = number of subjects of type i
	// located in u)
	BinaryMap<CplexSubjectCountOutputTypeVar> cplex_var_subject_count;

	cplex_environment.end();

	return;
}





} // faa





#endif // __FASOLVER_FAA_CPLEX_HPP__
