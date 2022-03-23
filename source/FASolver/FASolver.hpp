/**
 * @file FASolver.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FASOLVER_HPP__
#define __FASOLVER_FASOLVER_HPP__





#include "../PlanarMetric/PlanarMetric.hpp"
#include <ilcplex/ilocplex.h>
#include <map>
#include <set>





template<typename CoordinateType>
struct FacilityLayout final
{
	POINT_MAP_TYPE       points;
	metric::PlanarMetric distance;
};





/**
 * @class FASolver
 * @brief Facility Arrangement Problem solver
 *
 * Runs the optimisation process for the given Facility Arrangement Problem instance
 * with the help of CPLEX.
 */
template<typename CoordinateType, typename UnitType>
class FASolver final
{
	/// CPLEX environemnt responsible for memory allocation of all Concert objects.
	IloEnv cplex_environment;

	/// @name Problem data
	/// @{
	/// Facility layout
	FacilityLayout<CoordinateType> const    facility_layout;
	/// Types of subjects
	std::set<std::string> const             types;
	/// Input capacities
	std::map<std::string, UnitType> const   in_capacities;
	/// Output capacities
	std::map<std::string, UnitType> const   out_capacities;
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
	FASolver(FacilityLayout<CoordinateType> const& facility_layout,
	         std::set<std::string> const& types,
	         std::map<std::string, UnitType> const& in_capacities,
	         std::map<std::string, UnitType> const& out_capacities)
		: facility_layout(facility_layout)
		, types(types)
		, in_capacities(in_capacities)
		, out_capacities(out_capacities) {};

	/**
	 * @brief Destructor
	 *
	 * Destroys the solver and the associated CPLEX environemnt.
	 */
	~FASolver(void)
	{
		this->cplex_environment.end();
	};

	/// @}
};





#endif // __FASOLVER_FASOLVER_HPP__
