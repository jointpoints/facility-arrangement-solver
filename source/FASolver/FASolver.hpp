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
#include <chrono>
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





/**
 * @class FacilityLayout
 * @brief Containter for the facility layout
 *
 * Facility layout is determined by a map of points where subjects can be put and a
 * metric that defines distances between them.
 */
template<typename CoordinateType>
	requires numeric<CoordinateType>
struct FacilityLayout final
{
	/// Collection of named points where subjects can be placed
	PointMap<CoordinateType> points;
	/// A metric to measure distances between points
	PlanarMetric             distance;



	/// @name Constructors & destructors
	/// @{
	
	/**
	 * @brief Constructor
	 *
	 * Constructs a new facility layout.
	 */
	explicit
	FacilityLayout(PointMap<CoordinateType> const& points, PlanarMetric const& distance) noexcept
		: points(points), distance(distance) {};
	
	/// @}
};





/**
 * @class FASolver
 * @brief Facility Arrangement Problem solver
 *
 * Objects of this class let run the optimisation processes for the fixed Facility
 * Arrangement Problem instance with the help of CPLEX.
 */
template<typename CoordinateType, typename UnitType, typename AreaType>
	requires numeric<CoordinateType> && numeric<UnitType> && numeric<AreaType>
class FASolver final
{
	/// CPLEX numerical type to store unit values (e.g., flows)
	using CplexUnitType = std::conditional<std::is_integral<UnitType>::value, IloInt, IloNum>::type;

	/// CPLEX numerical type to store area values
	using CplexAreaType = std::conditional<std::is_integral<AreaType>::value, IloInt, IloNum>::type;



	/// CPLEX environemnt responsible for memory management for all Concert Technology objects.
	IloEnv cplex_environment;

	/// @name Problem data
	/// @{

	/// Facility layout
	FacilityLayout<CoordinateType> const        facility_layout;
	/// Subject types and their features
	SubjectTypeMap<UnitType, AreaType> const&   types;
	/// Sequence of types
	std::vector<std::string>                    type_names;
	/// Price of a single subject (dupticates the \c price field of SubjectType for
	/// further convenience, may be deleted in the future)
	IloNumArray                                 cplex_data_price;
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
	 * Constructs a new FASolver object for the provided instance of the problem to
	 * solve.
	 * 
	 * @param facility_layout Description of the available facility layout.
	 * @param types Features of different types of subjects.
	 * @param total_flows A map of maps containing pairs of a kind `<i : <j : f>>`
	 *                    where \c f is the total flow from all subjects of type
	 *                    \c i into all subjects of type \c j.
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
	 * @brief Solve the problem
	 *
	 * Runs a solver for mixed-integer linear program associated with the given
	 * instance of the Facility Arrangement Problem.
	 * 
	 * @param alpha A hyperparameter of the problem.
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
	, types(types)
	, total_flows(total_flows)
{
	// Check that type names in `total_flows` completely coincide with type names in `types`
	uint64_t const point_count = this->facility_layout.points.size();
	uint64_t const type_count = types.size();
	// ... TODO ...

	// Generate CPLEX-compatible data out of the given information
	this->cplex_data_price = IloNumArray(this->cplex_environment, 0);
	for (auto const& [type, features] : types)
	{
		this->type_names.push_back(type);
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
	auto const initialisation_start_time = std::chrono::high_resolution_clock::now();
	logger.info("Initialisation of model (variables and constraints) has started...");
	for (auto const& type1 : this->type_names)
	{
		cplex_x_generated.emplace(type1, IloIntVarArray(this->cplex_environment, point_count, 0, IloIntMax));
		cplex_x_generated[type1].setNames(("g_" + type1).data());
		cplex_x_subject_count.emplace(type1, IloIntVarArray(this->cplex_environment, point_count, 0, IloIntMax));
		cplex_x_subject_count[type1].setNames(("n_" + type1).data());
		
		for (uint64_t point1_i = 0; point1_i < point_count; ++point1_i)
		{
			auto const& [point_name1, point1] = *std::next(this->facility_layout.points.begin(), point1_i);
				
			cplex_constr_out_flow.try_emplace({type1, point_name1}, IloIntExpr(this->cplex_environment, 0));
			cplex_constr_occupied_area.try_emplace(point_name1, IloIntExpr(this->cplex_environment, 0));
				
			cplex_constr_occupied_area[point_name1] += (CplexAreaType)this->types.at(type1).area * cplex_x_subject_count[type1][point1_i];

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
	auto const initialisation_runtime = std::chrono::high_resolution_clock::now() - initialisation_start_time;
	auto const initialisation_runtime_hms = std::chrono::hh_mm_ss(initialisation_runtime);
	logger.info("Initialisation of model has finished.");

	// Add constraints
	logger.info("Model preparation has started...");
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
					cplex_constr_in_flow[{type1, point_name}] <= cplex_x_subject_count[type1][point_i] * (CplexUnitType)this->types.at(type1).in_capacity
				);
				// (2)
				cplex_model.add
				(
					cplex_constr_out_flow[{type1, point_name}] <= cplex_x_subject_count[type1][point_i] * (CplexUnitType)this->types.at(type1).out_capacity
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
				IloSum(cplex_x_generated[type1]) == (CplexUnitType)this->types.at(type1).total_generated_units
			);
			// (7)
			cplex_model.add
			(
				IloSum(cplex_x_subject_count[type1]) == (IloInt)this->types.at(type1).initially_available + cplex_x_additional_subjects[type1_i]
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
	logger.info("Model has been prepared for computations.");

	IloCplex cplex(cplex_model);
	cplex.setOut(logger.getInfoCallback());
	cplex.setWarning(logger.getWarningCallback());
	cplex.setError(logger.getErrorCallback());
	logger.info("CPLEX output starts now");
	logger.info("========================= CPLEX OUTPUT START =========================");
	auto const computation_start_time = std::chrono::high_resolution_clock::now();
	cplex.solve();
	auto const computation_runtime = std::chrono::high_resolution_clock::now() - computation_start_time;
	auto const computation_runtime_hms = std::chrono::hh_mm_ss(computation_runtime);
	logger.info("========================= CPLEX OUTPUT  END  =========================");
	logger.info("CPLEX has halted, its output has been finished.");

	// Print the time
	auto const total_runtime_hms = std::chrono::hh_mm_ss(initialisation_runtime + computation_runtime);
	logger.info("\nTime statistics");
	logger.info("\tTime spent on model intialisation = "
		+ std::to_string(initialisation_runtime_hms.hours().count()) + " h. "
		+ std::to_string(initialisation_runtime_hms.minutes().count()) + " m. "
		+ std::to_string(initialisation_runtime_hms.seconds().count()) + " s.");
	logger.info("\t        Time spent on computation = "
		+ std::to_string(computation_runtime_hms.hours().count()) + " h. "
		+ std::to_string(computation_runtime_hms.minutes().count()) + " m. "
		+ std::to_string(computation_runtime_hms.seconds().count()) + " s.");
	logger.info("\t                                    ------------------------------");
	logger.info("\t                            TOTAL = "
		+ std::to_string(total_runtime_hms.hours().count()) + " h. "
		+ std::to_string(total_runtime_hms.minutes().count()) + " m. "
		+ std::to_string(total_runtime_hms.seconds().count()) + " s.");
	logger.info("\nLog finished.");

	return;
}





#endif // __FASOLVER_FASOLVER_HPP__
