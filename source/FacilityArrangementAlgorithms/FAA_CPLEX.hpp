/**
 * @file FAA_CPLEX.hpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FAA_CPLEX_HPP__
#define __FASOLVER_FAA_CPLEX_HPP__





#include "../Common/Common.hpp"
#include "../SubjectType/SubjectType.hpp"
#include "../Logger/Logger.hpp"
#include <ilcplex/ilocplex.h>





// A forward declaration of Facility
class Facility;





template<typename AreaInputType, typename SubjectCountInputType, typename UnitType, typename PriceType>
void facilityArrangementAlgorithm_CPLEX
(
	Facility *facility,
	UnaryMap<SubjectType<AreaInputType, SubjectCountInputType, UnitType, PriceType>> const &subject_types,
	BinaryMap<UnitType> const &total_flows,
	Logger const &logger,
	bool const first_step
)
{
	// Define types of variables based on parameters
	using CplexUnitTypeVar = std::conditional
	<
		std::is_same<UnitType, FASInteger>,
		IloIntVar,
		IloNumVar
	>;
	using SubjectCountOutputType = std::conditional
	<
		facility->_subject_count_output_type_integral,
		
	>;

	// Start preparation of a CPLEX model
	IloEnv cplex_environment;

	// CPLEX variables and constraints
	// Flows of objects (cplex_var_flow[{u,v}][{i,j}] = flow of objects from all objects
	// of type i located in u to all objects of type j located in v)
	BinaryPairMap<CplexUnitTypeVar> cplex_var_flow;
	// Objects produced by subjects (cplex_var_produced[u][i] = number of objects
	// produced by all subjects of type i located in u)
	BinaryMap<CplexUnitTypeVar> cplex_var_produced;
	// Number of subjects (cplex_var_subject_count[u][i] = number of subjects of type i
	// located in u)
	BinaryMap<>
}





#endif // __FASOLVER_FAA_CPLEX_HPP__
