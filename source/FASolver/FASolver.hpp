/**
 * @file FASolver.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FASOLVER_HPP__
#define __FASOLVER_FASOLVER_HPP__





#include "../Facility/Facility.hpp"
#include "../SubjectType/SubjectType.hpp"
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





/**
 * @class FASolver
 * @brief Facility Arrangement Problem solver
 *
 * Objects of this class let run the optimisation processes for the fixed Facility
 * Arrangement Problem instance with the help of CPLEX.
 */
template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<UnitType> && numeric<AreaType>
class FASolver final
{
	/// CPLEX numerical type to store unit values (e.g., flows).
	using CplexUnitType = std::conditional<std::is_integral<UnitType>::value, IloInt, IloNum>::type;

	/// CPLEX numerical type to store area values.
	using CplexAreaType = std::conditional<std::is_integral<AreaType>::value, IloInt, IloNum>::type;



	/// CPLEX environemnt responsible for memory management for all Concert Technology objects.
	IloEnv cplex_environment;

	/// @name Problem data
	/// @{

	/// Facility layout.
	FacilityLayout<CoordinateType, AreaType> const  facility_layout;
	/// Subject types and their features.
	SubjectTypeMap<AreaType, UnitType> const&       types;
	/// Sequence of types.
	std::vector<std::string>                        type_names;
	/// Price of a single subject (dupticates the \c price field of SubjectType for
	/// further convenience, may be deleted in the future).
	IloNumArray                                     cplex_data_price;
	/// Total flows.
	FlowMap<UnitType> const                         total_flows;

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
	FASolver(FacilityLayout<CoordinateType, AreaType> const&    facility_layout,
	         SubjectTypeMap<AreaType, UnitType> const&          types,
	         FlowMap<UnitType> const&                           total_flows);

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





template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<UnitType> && numeric<AreaType>
FASolver<CoordinateType, AreaType, UnitType>::FASolver(FacilityLayout<CoordinateType, AreaType> const&  facility_layout,
                                                       SubjectTypeMap<AreaType, UnitType> const&        types,
                                                       FlowMap<UnitType> const&                         total_flows)
	: cplex_environment()
	, facility_layout(facility_layout)
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



template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<UnitType> && numeric<AreaType>
FASolver<CoordinateType, AreaType, UnitType>::~FASolver(void)
{
	this->cplex_environment.end();
	return;
}



template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<UnitType> && numeric<AreaType>
void FASolver<CoordinateType, AreaType, UnitType>::optimise(long double const alpha) const
{
	// Set up a logger
	int logger_status = 0;
	Logger logger("test.txt", logger_status);
	// !!!!! TODO: Logger assertions !!!!!
	logger.info("Log started.");

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
	std::map<std::string, std::map<std::string, IloIntExpr>> cplex_constr_in_flow;
	// Constraint (2): output does not exceed the output capacity (iterated over all types and then over all points)
	std::map<std::string, std::map<std::string, IloIntExpr>> cplex_constr_out_flow;
	// Constraint (3): the area occupied by the subjects in one place/point does not exceed its area capacity (iterated over all points)
	std::map<std::string, IloIntExpr> cplex_constr_occupied_area;
	// Constraint (4): weak Kirchhoff condition
	//     reuse `cplex_constr_out_flow` and `cplex_constr_in_flow` for this
	// Constraint (5): flow from all subjects of type i to all subjects of type j equals the respective total flow
	// Constraint (6): all generated units by subjects of each type add up to the respective total number of generated units
	// Constraint (7): all subjects are placed somewhere

	// Initialisation of variables, `cplex_total_flow_cost` and constraints
	auto const initialisation_start_time = std::chrono::high_resolution_clock::now();
	logger.info("Initialisation of variables and constraints started...");
	for (auto const& type_name : this->type_names)
	{
		cplex_constr_in_flow[type_name];
		cplex_constr_out_flow[type_name];
		for (auto const& [point_name, point] : this->facility_layout.points)
		{
			cplex_constr_in_flow[type_name].emplace(point_name, IloIntExpr(this->cplex_environment, 0));
			cplex_constr_out_flow[type_name].emplace(point_name, IloIntExpr(this->cplex_environment, 0));
		}
	}
	for (auto const& type1 : this->type_names)
	{
		cplex_x_generated.emplace(type1, IloIntVarArray(this->cplex_environment, point_count, 0, IloIntMax));
		cplex_x_generated[type1].setNames(("g_" + type1).data());
		cplex_x_subject_count.emplace(type1, IloIntVarArray(this->cplex_environment, point_count, 0, IloIntMax));
		cplex_x_subject_count[type1].setNames(("n_" + type1).data());
		
		uint64_t point1_i = 0;
		for (auto const& [point_name1, point1] : this->facility_layout.points)
		{
			cplex_constr_occupied_area.try_emplace(point_name1, IloIntExpr(this->cplex_environment, 0));
				
			cplex_constr_occupied_area[point_name1] += (CplexAreaType)this->types.at(type1).area * cplex_x_subject_count[type1][point1_i++];

			for (auto const& type2 : this->type_names)
				if (this->total_flows.at(type1).at(type2) != 0)
				{
					cplex_x_flow.insert({{type1, type2}, {}});
					
					for (auto const& [point_name2, point2] : this->facility_layout.points)
					{
						cplex_x_flow[{type1, type2}].insert({{point_name1, point_name2}, IloIntVar(this->cplex_environment, 0, IloIntMax, ("f_" + type1 + type2 + "_" + point_name1 + "," + point_name2).data())});
						
						cplex_total_flow_cost += (IloNum)this->facility_layout.distance(point1, point2) * cplex_x_flow[{type1, type2}][{point_name1, point_name2}];
						
						cplex_constr_in_flow[type2][point_name2] += cplex_x_flow[{type1, type2}][{point_name1, point_name2}];
						cplex_constr_out_flow[type1][point_name1] += cplex_x_flow[{type1, type2}][{point_name1, point_name2}];
					}
				}
		}
	}
	auto const initialisation_runtime = std::chrono::high_resolution_clock::now() - initialisation_start_time;
	auto const initialisation_runtime_hms = std::chrono::hh_mm_ss(initialisation_runtime);
	logger.info("Initialisation of variables and constraints finished.");

	// Find feasible solution with the help of heuristic
	logger.info("Execution of Simple Monte-Carlo heuristic started...");
	FacilityArrangement<CoordinateType, AreaType, UnitType> const feasible_solution = fa::produceMC(
		this->facility_layout,
		this->types,
		this->total_flows,
		logger,
		0,
		(uint64_t)std::sqrt(point_count) * type_count * type_count
	);
	logger.info("Execution of heuristic finished.");
	
	IloNumVarArray cplex_xs_aggregator(this->cplex_environment);
	IloNumArray cplex_feasxs_aggregator(this->cplex_environment);

	// Add constraints and translate the feasible solution into the language of CPLEX
	logger.info("Model preparation started...");
	IloModel cplex_model(this->cplex_environment);
	{
		uint64_t type1_i = 0;
		for (auto const& type1_name : this->type_names)
		{
			uint64_t point_i = 0;
			for (auto const& [point_name, point] : this->facility_layout.points)
			{
				// (1)
				cplex_model.add
				(
					cplex_constr_in_flow[type1_name][point_name] <= cplex_x_subject_count[type1_name][point_i] * (CplexUnitType)this->types.at(type1_name).in_capacity
				);
				// (2)
				cplex_model.add
				(
					cplex_constr_out_flow[type1_name][point_name] <= cplex_x_subject_count[type1_name][point_i] * (CplexUnitType)this->types.at(type1_name).out_capacity
				);
				// (4)
				cplex_model.add
				(
					cplex_constr_out_flow[type1_name][point_name] <= cplex_x_generated[type1_name][point_i] + cplex_constr_in_flow[type1_name][point_name]
				);
				// Translate subject count and generated units count
				cplex_xs_aggregator.add(cplex_x_subject_count[type1_name][point_i]);
				cplex_xs_aggregator.add(cplex_x_generated[type1_name][point_i]);
				cplex_feasxs_aggregator.add(feasible_solution.points.at(point_name).subject_count.contains(type1_name) ? feasible_solution.points.at(point_name).subject_count.at(type1_name) : 0);
				cplex_feasxs_aggregator.add(feasible_solution.points.at(point_name).generated_unit_count.contains(type1_name) ? feasible_solution.points.at(point_name).generated_unit_count.at(type1_name) : 0);
				cplex_model.add(cplex_x_subject_count[type1_name][point_i] == (feasible_solution.points.at(point_name).subject_count.contains(type1_name) ? feasible_solution.points.at(point_name).subject_count.at(type1_name) : 0));
				cplex_model.add(cplex_x_generated[type1_name][point_i] == (feasible_solution.points.at(point_name).generated_unit_count.contains(type1_name) ? feasible_solution.points.at(point_name).generated_unit_count.at(type1_name) : 0));
				++point_i;
			}
			for (auto const& type2_name : this->type_names)
				if (this->total_flows.at(type1_name).at(type2_name) != 0)
				{
					IloIntExpr accumulated_sum(this->cplex_environment, 0);
					for (auto const& [point1_name, point1] : this->facility_layout.points)
					{
						for (auto const& [point2_name, point2] : this->facility_layout.points)
						{
							accumulated_sum += cplex_x_flow[{type1_name, type2_name}][{point1_name, point2_name}];
							cplex_xs_aggregator.add(cplex_x_flow[{type1_name, type2_name}][{point1_name, point2_name}]);
							cplex_feasxs_aggregator.add((feasible_solution.points.at(point1_name).out_flows.contains({type1_name, type2_name}) && feasible_solution.points.at(point1_name).out_flows.at({type1_name, type2_name}).contains(point2_name)) ? feasible_solution.points.at(point1_name).out_flows.at({type1_name, type2_name}).at(point2_name) : 0);
							cplex_model.add(cplex_x_flow[{type1_name, type2_name}][{point1_name, point2_name}] == ((feasible_solution.points.at(point1_name).out_flows.contains({type1_name, type2_name}) && feasible_solution.points.at(point1_name).out_flows.at({type1_name, type2_name}).contains(point2_name)) ? feasible_solution.points.at(point1_name).out_flows.at({type1_name, type2_name}).at(point2_name) : 0));
						}
					}
					// (5)
					cplex_model.add
					(
						accumulated_sum == (IloNum)(this->total_flows.at(type1_name).at(type2_name))
					);
				}
			// (6)
			cplex_model.add
			(
				IloSum(cplex_x_generated[type1_name]) == (CplexUnitType)this->types.at(type1_name).total_generated_units
			);
			// (7)
			cplex_model.add
			(
				IloSum(cplex_x_subject_count[type1_name]) == (IloInt)this->types.at(type1_name).initially_available + cplex_x_additional_subjects[type1_i]
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
	logger.info("CPLEX output starts now.");
	logger.info("========================= CPLEX OUTPUT START =========================");
	auto const computation_start_time = std::chrono::high_resolution_clock::now();
	cplex.addMIPStart(cplex_xs_aggregator, cplex_feasxs_aggregator, IloCplex::MIPStartEffort::MIPStartCheckFeas);
	cplex.solve();
	auto const computation_runtime = std::chrono::high_resolution_clock::now() - computation_start_time;
	auto const computation_runtime_hms = std::chrono::hh_mm_ss(computation_runtime);
	logger.info("========================= CPLEX OUTPUT  END  =========================");
	logger.info("CPLEX halted, its output finished.");

	// Print the time
	auto const total_runtime_hms = std::chrono::hh_mm_ss(initialisation_runtime + computation_runtime);
	logger.info("\nTime statistics");
	logger.info("\tTime spent on intialisation = "
		+ std::to_string(initialisation_runtime_hms.hours().count()) + " h. "
		+ std::to_string(initialisation_runtime_hms.minutes().count()) + " m. "
		+ std::to_string(initialisation_runtime_hms.seconds().count()) + " s.");
	logger.info("\t  Time spent on computation = "
		+ std::to_string(computation_runtime_hms.hours().count()) + " h. "
		+ std::to_string(computation_runtime_hms.minutes().count()) + " m. "
		+ std::to_string(computation_runtime_hms.seconds().count()) + " s.");
	logger.info("\t                              --------------------------------");
	logger.info("\t                      TOTAL = "
		+ std::to_string(total_runtime_hms.hours().count()) + " h. "
		+ std::to_string(total_runtime_hms.minutes().count()) + " m. "
		+ std::to_string(total_runtime_hms.seconds().count()) + " s.");
	logger.info("\nLog finished.");

	cplex.exportModel("test.lp");

	return;
}





#endif // __FASOLVER_FASOLVER_HPP__
