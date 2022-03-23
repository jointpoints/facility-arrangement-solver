/**
 * @file FASolver.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FASOLVER_HPP__
#define __FASOLVER_FASOLVER_HPP__





#include "../PlanarMetric/PlanarMetric.hpp"
#include "../SubjectType/SubjectType.hpp"
#include <ilcplex/ilocplex.h>
#include <set>





template<typename UnitType>
using UnitCapacityMap = std::map<std::string, uint64_t>;

template<typename UnitType>
using FlowMap = std::map<std::string, UnitCapacityMap<UnitType>>;





template<typename CoordinateType>
struct FacilityLayout final
{
	PointMap<CoordinateType> points;
	metric::PlanarMetric     distance;
};





/**
 * @class FASolver
 * @brief Facility Arrangement Problem solver
 *
 * Runs the optimisation process for the given Facility Arrangement Problem instance
 * with the help of CPLEX.
 */
template<typename CoordinateType, typename UnitType, typename AreaType>
class FASolver final
{
	/// CPLEX environemnt responsible for memory allocation of all Concert objects.
	IloEnv cplex_environment;



	/// @name Problem data
	/// @{

	/// Facility layout
	FacilityLayout<CoordinateType> const        facility_layout;
	/// Types of subjects and their properties
	SubjectTypeMap<UnitType, AreaType> const    types;
	/// Total flows
	FlowMap<UnitType> const                     total_flows;

	/// @}



public:
	/// @name Constructors and destructors
	/// @{
	
	FASolver(void) = delete;
	
	/**
	 * @brief Constructor from data
	 *
	 * Constructs a new FASolver object relying on the provided data about the instance
	 * of the problem to solve.
	 */
	explicit
	FASolver(FacilityLayout<CoordinateType> const&      facility_layout,
	         SubjectTypeMap<UnitType, AreaType> const&  types,
	         FlowMap<UnitType> const&                   total_flows);

	/**
	 * @brief Destructor
	 *
	 * Destroys the solver and the associated CPLEX environemnt.
	 */
	~FASolver(void);

	/// @}
};





// Definitions of FASolver template class member functions





template<typename CoordinateType, typename UnitType, typename AreaType>
FASolver<CoordinateType, UnitType, AreaType>::FASolver(FacilityLayout<CoordinateType> const&        facility_layout,
                                                       SubjectTypeMap<UnitType, AreaType> const&    types,
                                                       FlowMap<UnitType> const&                     total_flows)
	: facility_layout(facility_layout)
	, types(types)
	, total_flows(total_flows)
{
	return;
}



template<typename CoordinateType, typename UnitType, typename AreaType>
FASolver<CoordinateType, UnitType, AreaType>::~FASolver(void)
{
	this->cplex_environment.end();
	return;
}





#endif // __FASOLVER_FASOLVER_HPP__
