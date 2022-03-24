/**
 * @file FASolver.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FASOLVER_HPP__
#define __FASOLVER_FASOLVER_HPP__





#include "../PlanarMetric/PlanarMetric.hpp"
#include "../SubjectType/SubjectType.hpp"
#include <ilcplex/ilocplex.h>
#include <type_traits>
#include <numeric>
#include <vector>
#include <set>





template<typename X>
X successValue(void)
{
	if constexpr(std::same_as<X, bool>)
		return true;
	else if constexpr(std::integral<X>)
		return 0;
};

#define FASOLVER_ASSERT(expr, succ_message, fail_message) \
	if (auto result = expr; result == SuccessValue<decltype(result)>())





template<typename UnitType>
	requires numeric<UnitType>
using UnitCapacityMap = std::map<std::string, UnitType>;

template<typename UnitType>
	requires numeric<UnitType>
using FlowMap = std::map<std::string, UnitCapacityMap<UnitType>>;

template<typename CoordinateType>
	requires numeric<CoordinateType>
using DistanceMap = std::map<std::pair<Point<CoordinateType>, Point<CoordinateType>>, long double>;





template<typename CoordinateType>
	requires numeric<CoordinateType>
struct FacilityLayout final
{
	PointMap<CoordinateType> points;
	metric::PlanarMetric     distance;



	explicit
	FacilityLayout(PointMap<CoordinateType> const& points, metric::PlanarMetric const& distance)
		: points(points), distance(distance) {};
};





/**
 * @class FASolver
 * @brief Facility Arrangement Problem solver
 *
 * Runs the optimisation process for the given Facility Arrangement Problem instance
 * with the help of CPLEX.
 */
template<typename CoordinateType, typename UnitType, typename AreaType>
	requires numeric<CoordinateType> && numeric<UnitType> && numeric<AreaType>
class FASolver final
{
	/// Type of CPLEX array to store unit values (e.g., flows)
	using CplexUnitArray = std::conditional<std::is_integral<UnitType>::value, IloIntArray, IloNumArray>::type;

	/// Type of CPLEX array to store area values
	using CplexAreaArray = std::conditional<std::is_integral<AreaType>::value, IloIntArray, IloNumArray>::type;



	/// CPLEX environemnt responsible for memory allocation of all Concert objects.
	IloEnv cplex_environment;

	/// Log file
	std::fstream log;

	/// @name Problem data
	/// @{

	/// Facility layout
	FacilityLayout<CoordinateType> const    facility_layout;
	/// Sequence of types
	std::vector<std::string>                type_names;
	/// Input capacities
	CplexUnitArray                          cplex_in_capacity;
	/// Output capacities
	CplexUnitArray                          cplex_out_capacity;
	/// Total number of units all subjects of each type are supposed to generate
	CplexUnitArray                          cplex_total_generated_units;
	/// Areas
	CplexAreaArray                          cplex_area;
	/// Initially available subjects
	IloIntArray                             cplex_initially_available;
	/// Price of a single subject
	IloNumArray                             cplex_price;
	/// Total flows
	FlowMap<UnitType> const                 total_flows;

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



	/// @name Solvers
	/// @{

	/**
	 * @brief Solve MILP version of the problem
	 *
	 * Runs a solver for mixed-integer linear program associated with the given
	 * instance of Facility Arrangement Problem.
	 */
	void optimise(long double const alpha = 1.L) const;

	/// @}
};





// Definitions of FASolver template class member functions





template<typename CoordinateType, typename UnitType, typename AreaType>
	requires numeric<CoordinateType> && numeric<UnitType> && numeric<AreaType>
FASolver<CoordinateType, UnitType, AreaType>::FASolver(FacilityLayout<CoordinateType> const&        facility_layout,
                                                       SubjectTypeMap<UnitType, AreaType> const&    types,
                                                       FlowMap<UnitType> const&                     total_flows)
	: facility_layout(facility_layout)
	, total_flows(total_flows)
{
	// Check that type names in `total_flows` completely coincide with type names in `types`
	uint64_t const point_count = this->facility_layout.points.size();
	uint64_t const type_count = types.size();
	// ... TODO ...

	// Generate CPLEX-compatible data out of the given information
	this->type_names = std::vector<std::string>();
	this->cplex_in_capacity = CplexUnitArray(this->cplex_environment, 0);
	this->cplex_out_capacity = CplexUnitArray(this->cplex_environment, 0);
	this->cplex_total_generated_units = CplexUnitArray(this->cplex_environment, 0);
	this->cplex_area = CplexAreaArray(this->cplex_environment, 0);
	this->cplex_initially_available = IloIntArray(this->cplex_environment, 0);
	this->cplex_price = IloNumArray(this->cplex_environment, 0);
	for (auto const& [type, features] : types)
	{
		this->type_names.push_back(type);
		this->cplex_in_capacity.add(features.in_capacity);
		this->cplex_out_capacity.add(features.out_capacity);
		this->cplex_total_generated_units.add(features.total_generated_units);
		this->cplex_area.add(features.area);
		this->cplex_initially_available.add(features.initially_available);
		this->cplex_price.add(features.price);
	}

	return;
}



template<typename CoordinateType, typename UnitType, typename AreaType>
	requires numeric<CoordinateType> && numeric<UnitType> && numeric<AreaType>
FASolver<CoordinateType, UnitType, AreaType>::~FASolver(void)
{
	this->cplex_environment.end();
	return;
}



template<typename CoordinateType, typename UnitType, typename AreaType>
	requires numeric<CoordinateType> && numeric<UnitType> && numeric<AreaType>
void FASolver<CoordinateType, UnitType, AreaType>::optimise(long double const alpha) const
{
	IloModel cplex_model(this->cplex_environment);

	uint64_t const point_count = this->facility_layout.points.size();
	uint64_t const type_count = this->type_names.size();

	// Variables
	// 1. Flows
	std::map<std::pair<std::string, std::string>, std::map<std::pair<std::string, std::string>, IloIntVar>> cplex_x_flow;
	// 2. Generated units
	std::map<std::string, IloIntVarArray> cplex_x_generated;
	// 3. Number of subjects in each point
	std::map<std::string, IloIntVarArray> cplex_x_subject_count;
	// 4. Additional subjects to buy
	IloIntVarArray cplex_x_additional_subjects(this->cplex_environment, type_count, 0, IloInfinity);

	// Total flow cost
	IloNumExpr cplex_total_flow_cost(this->cplex_environment, 0.0);

	// Constraint: input does not exceed the input capacity
	std::map<std::pair<std::string, std::string>, IloNumExpr> cplex_in_flow;

	// Constraint: output does not exceed the output capacity
	std::map<std::pair<std::string, std::string>, IloNumExpr> cplex_out_flow;

	// Initialisation of variables, `cplex_total_flow_cost` and constraints
	for (auto const& type1 : this->type_names)
	{
		cplex_x_generated.emplace(type1, IloIntVarArray(this->cplex_environment, point_count, 0, IloInfinity));
		cplex_x_subject_count.emplace(type1, IloIntVarArray(this->cplex_environment, point_count, 0, IloInfinity));
		for (auto const& type2 : this->type_names)
		{
			cplex_x_flow.insert({{type1, type2}, {}});
			for (auto const& [point_name1, point1] : this->facility_layout.points)
			{
				cplex_in_flow.try_emplace({type1, point_name1}, IloNumExpr(this->cplex_environment, 0.0));
				cplex_out_flow.try_emplace({type1, point_name1}, IloNumExpr(this->cplex_environment, 0.0));
				for (auto const& [point_name2, point2] : this->facility_layout.points)
				{
					cplex_x_flow[{type1, type2}].insert({{point_name1, point_name2}, IloIntVar(this->cplex_environment, 0, IloInfinity)});
					cplex_total_flow_cost += (IloNum)this->facility_layout.distance(point1, point2) * cplex_x_flow[{type1, type2}][{point_name1, point_name2}];
					cplex_in_flow[{type1, point_name1}] += cplex_x_flow[{type2, type1}][{point_name2, point_name1}];
					cplex_out_flow[{type1, point_name1}] += cplex_x_flow[{type1, type2}][{point_name1, point_name2}];
				}
			}
		}
		// Add constraints that have already been finalised
		for (auto const& [point_name, point] : this->facility_layout.points)
			cplex_model.add(cplex_in_flow[{}])
	}

	// Objective function
	cplex_model.add(IloMinimize(this->cplex_environment,
		/* additional cost */        alpha  * IloScalProd(this->cplex_price, cplex_x_additional_subjects) +
		/* total flow cost */ (2.L - alpha) * cplex_total_flow_cost
	));
}





#endif // __FASOLVER_FASOLVER_HPP__
