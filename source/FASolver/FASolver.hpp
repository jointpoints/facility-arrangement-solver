/**
 * @file FASolver.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FASOLVER_HPP__
#define __FASOLVER_FASOLVER_HPP__





#include "../PlanarMetric/PlanarMetric.hpp"
#include "../SubjectType/SubjectType.hpp"
#include "../Logger/Logger.hpp"
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
	PlanarMetric             distance;



	explicit
	FacilityLayout(PointMap<CoordinateType> const& points, PlanarMetric const& distance)
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
	CplexUnitArray                          cplex_data_in_capacity;
	/// Output capacities
	CplexUnitArray                          cplex_data_out_capacity;
	/// Total number of units all subjects of each type are supposed to generate
	CplexUnitArray                          cplex_data_total_generated_units;
	/// Areas
	CplexAreaArray                          cplex_data_area;
	/// Initially available subjects
	IloIntArray                             cplex_data_initially_available;
	/// Price of a single subject
	IloNumArray                             cplex_data_price;
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
	this->cplex_data_in_capacity = CplexUnitArray(this->cplex_environment, 0);
	this->cplex_data_out_capacity = CplexUnitArray(this->cplex_environment, 0);
	this->cplex_data_total_generated_units = CplexUnitArray(this->cplex_environment, 0);
	this->cplex_data_area = CplexAreaArray(this->cplex_environment, 0);
	this->cplex_data_initially_available = IloIntArray(this->cplex_environment, 0);
	this->cplex_data_price = IloNumArray(this->cplex_environment, 0);
	for (auto const& [type, features] : types)
	{
		this->type_names.push_back(type);
		this->cplex_data_in_capacity.add(features.in_capacity);
		this->cplex_data_out_capacity.add(features.out_capacity);
		this->cplex_data_total_generated_units.add(features.total_generated_units);
		this->cplex_data_area.add(features.area);
		this->cplex_data_initially_available.add(features.initially_available);
		this->cplex_data_price.add(features.price);
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
	// Set up a logger
	int logger_status = 0;
	Logger logger("test.txt", logger_status);
	// !!!!! TODO: Logger assertions !!!!!
	std::cout << logger_status << '\n';
	logger.info("Log started.");

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
	IloIntVarArray cplex_x_additional_subjects(this->cplex_environment, type_count, 0, IloIntMax);
	cplex_x_additional_subjects.setNames("nt");

	// Total flow cost (one of the terms of the objective function)
	IloNumExpr cplex_total_flow_cost(this->cplex_environment, 0.0);

	// Constraint (1): input does not exceed the input capacity (iterated over all types and then over all points)
	std::map<std::pair<std::string, std::string>, IloIntExpr> cplex_constr_in_flow;
	// Constraint (2): output does not exceed the output capacity (iterated over all types and then over all points)
	std::map<std::pair<std::string, std::string>, IloIntExpr> cplex_constr_out_flow;
	// Constraint (3): the area occupied by the subjects in one place/point does not exceed its area capacity (iterated over all points)
	std::map<std::string, IloIntExpr> cplex_constr_occupied_area;
	// Constraint (4): weak Kirchhoff condition
	//     reuse `cplex_constr_out_flow` and `cplex_constr_in_flow` for this
	// Constraint (5): flow from all subjects of type i to all subjects of type j equals the respective total flow
	// Constraint (6): all generated units by subjects of each type add up to the respective total number of generated units
	// Constraint (7): all subjects are placed somewhere

	// Initialisation of variables, `cplex_total_flow_cost` and constraints
	for (uint64_t type1_i = 0; type1_i < type_count; ++type1_i)
	{
		auto const& type1 = this->type_names[type1_i];
		
		cplex_x_generated.emplace(type1, IloIntVarArray(this->cplex_environment, point_count, 0, IloIntMax));
		cplex_x_generated[type1].setNames(("g_" + type1).data());
		cplex_x_subject_count.emplace(type1, IloIntVarArray(this->cplex_environment, point_count, 0, IloIntMax));
		cplex_x_subject_count[type1].setNames(("n_" + type1).data());
		
		for (uint64_t point1_i = 0; point1_i < point_count; ++point1_i)
		{
			auto const& [point_name1, point1] = *std::next(this->facility_layout.points.begin(), point1_i);
				
			cplex_constr_out_flow.try_emplace({type1, point_name1}, IloIntExpr(this->cplex_environment, 0));
			cplex_constr_occupied_area.try_emplace(point_name1, IloIntExpr(this->cplex_environment, 0));
				
			cplex_constr_occupied_area[point_name1] += cplex_data_area[type1_i] * cplex_x_subject_count[type1][point1_i];

			for (auto const& type2 : this->type_names)
			{
				cplex_x_flow.insert({{type1, type2}, {}});
				
				for (auto const& [point_name2, point2] : this->facility_layout.points)
				{
					cplex_constr_in_flow.try_emplace({type2, point_name2}, IloIntExpr(this->cplex_environment, 0));
					cplex_x_flow[{type1, type2}].insert({{point_name1, point_name2}, IloIntVar(this->cplex_environment, 0, IloIntMax, ("f_" + type1 + type2 + "_" + point_name1 + "," + point_name2).data())});
					
					cplex_total_flow_cost += (IloNum)this->facility_layout.distance(point1, point2) * cplex_x_flow[{type1, type2}][{point_name1, point_name2}];
					
					cplex_constr_in_flow[{type2, point_name2}] += cplex_x_flow[{type1, type2}][{point_name1, point_name2}];
					cplex_constr_out_flow[{type1, point_name1}] += cplex_x_flow[{type1, type2}][{point_name1, point_name2}];
				}
			}
		}
	}

	// Add constraints
	{
		uint64_t type1_i = 0;
		for (auto const& type1 : this->type_names)
		{
			uint64_t point_i = 0;
			for (auto const& [point_name, point] : this->facility_layout.points)
			{
				// (1)
				cplex_model.add
				(
					cplex_constr_in_flow[{type1, point_name}] <= cplex_x_subject_count[type1][point_i] * this->cplex_data_in_capacity[type1_i]
				);
				// (2)
				cplex_model.add
				(
					cplex_constr_out_flow[{type1, point_name}] <= cplex_x_subject_count[type1][point_i] * this->cplex_data_out_capacity[type1_i]
				);
				// (4)
				cplex_model.add
				(
					cplex_constr_out_flow[{type1, point_name}] <= cplex_x_generated[type1][point_i] + cplex_constr_in_flow[{type1, point_name}]
				);
				++point_i;
			}
			for (auto const& type2 : this->type_names)
			{
				IloIntExpr accumulated_sum(this->cplex_environment, 0);
				for (auto const& [point_name1, point1] : this->facility_layout.points)
					for (auto const& [point_name2, point2] : this->facility_layout.points)
						accumulated_sum += cplex_x_flow[{type1, type2}][{point_name1, point_name2}];
				// (5)
				cplex_model.add
				(
					accumulated_sum == (IloNum)(this->total_flows.at(type1).at(type2))
				);
			}
			// (6)
			cplex_model.add
			(
				IloSum(cplex_x_generated[type1]) == this->cplex_data_total_generated_units[type1_i]
			);
			// (7)
			cplex_model.add
			(
				IloSum(cplex_x_subject_count[type1]) == this->cplex_data_initially_available[type1_i] + cplex_x_additional_subjects[type1_i]
			);
			++type1_i;
		}
		for (auto const& [point_name, point] : this->facility_layout.points)
		{
			// (3)
			cplex_model.add
			(
				cplex_constr_occupied_area[point_name] <= (IloInt)point.capacity
			);
		}
	}

	// Objective function
	cplex_model.add(IloMinimize(this->cplex_environment,
		/* additional cost */        alpha  * IloScalProd(this->cplex_data_price, cplex_x_additional_subjects) +
		/* total flow cost */ (2.L - alpha) * cplex_total_flow_cost
	));

	IloCplex cplex(cplex_model);
	cplex.solve();
	//cplex.exportModel("test.lp");

	return;
}





#endif // __FASOLVER_FASOLVER_HPP__
