/**
 * @file Facility.hpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FACILITY_HPP__
#define __FASOLVER_FACILITY_HPP__





#include "../PlanarMetric/PlanarMetric.hpp"
#include "../SubjectType/SubjectType.hpp"
#include "../Logger/Logger.hpp"
#include <ilcplex/ilocplex.h>
#include <thread>
#include <mutex>
#include <future>
#include <random>





/**
 * @class FacilityLayout
 * @brief Containter for the facility layout
 *
 * Facility layout is determined by a map of points where subjects can be put and a
 * metric that defines distances between them.
 */
template<typename CoordinateType, typename AreaType>
	requires numeric<CoordinateType> && numeric<AreaType>
struct FacilityLayout final
{
	/// Collection of named points where subjects can be placed.
	PointMap<CoordinateType, AreaType> points;
	/// A metric to measure distances between points.
	PlanarMetric                       distance;



	/// @name Constructors & destructors
	/// @{
	
	/**
	 * @brief Constructor
	 *
	 * Constructs a new facility layout.
	 */
	explicit
	FacilityLayout(PointMap<CoordinateType, AreaType> const& points, PlanarMetric const& distance) noexcept
		: points(points), distance(distance) {};
	
	/// @}
};





/**
 * @class FacilityArrangement
 * @brief Container for the facility arrangement
 *
 * Facility layout with set subject placement and calculated flows.
 */
template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
struct FacilityArrangement final
{
	/// Collection of named points where subjects have already been placed.
	FacilityArrangementPointMap<CoordinateType, AreaType, UnitType> points;
	/// A metric to measure distances between points
	PlanarMetric distance;



	FacilityArrangement(void) = default;

	FacilityArrangement(FacilityArrangement const&) = default;

	FacilityArrangement(FacilityLayout<CoordinateType, AreaType> const& facility_layout)
		: distance(facility_layout.distance)
	{
		for (auto const& [point_name, point] : facility_layout.points)
			this->points[point_name] = point;
		return;
	};
};










// Namespaces





/**
 * @namespace fa::util
 * @brief Utilities used for facility arrangement generation
 *
 * These functions are not expected to be called from outside of fa namespace.
 */
namespace fa::util
{





/**
 * @class ThreadReturn
 * @brief Structure returned by each generating thread
 */
template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
struct ThreadReturn
{
	FacilityArrangement<CoordinateType, AreaType, UnitType> facility_arrangement;
	long double objective_value;
};





/**
 * @brief Generate a portion of Monte-Carlo samples
 *
 * Used by fa::produceMC to be run by a single thread to generate some part of
 * Monte-Carlo samples. Each thread returns the best found arrangement, the main thread
 * is then supposed to choose the best one among them.
 * 
 * @param facility_layout Description of the available facility layout.
 * @param types Features of different types of subjects.
 * @param total_flows A map of maps containing pairs of a kind `<i : <j : f>>`
 *                    where \c f is the total flow from all subjects of type
 *                    \c i into all subjects of type \c j.
 * @param return_value An object to store the return value of the thread.
 * @param logger A logger.
 * @param logger_mutex A mutex shared by all the threads to prevent multiple threads
 *                     writing to the logger simultaneously.
 * @param thread_id An ID to distinguish this thread.
 * @param workload Number of samples for a thread to generate.
 * @param seed A seed to use for a pseudorandom number generator.
 * @param max_attempts Maximum number of attempts to make to generate one facility
 *                     arrangement.
 * 
 * @returns Nothing. Return value is handed back to the main thread via \c return_value.
 */
template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
void threadMC(FacilityLayout<CoordinateType, AreaType> const& facility_layout,
              SubjectTypeMap<AreaType, UnitType> const&       types,
              FlowMap<UnitType> const&                        total_flows,
              std::promise<ThreadReturn<CoordinateType, AreaType, UnitType>> return_value,
              Logger const& logger,
              std::mutex&   logger_mutex,
              uint16_t const thread_id,
              uint64_t const workload,
              uint64_t const seed,
              uint64_t const max_attempts)
{
	FacilityArrangement<CoordinateType, AreaType, UnitType> answer;
	long double best_objective_value = std::numeric_limits<long double>::infinity();
	std::unique_lock logger_lock(logger_mutex, std::defer_lock);

	std::mt19937_64 prng(seed);
	std::uniform_int_distribution<> distribution_points(0, facility_layout.points.size() - 1);

	uint64_t skipped_sample_count = 0;
	for (uint64_t sample_i = 0; sample_i < workload; ++sample_i)
	{
		FacilityArrangement<CoordinateType, AreaType, UnitType> current(facility_layout);

		// Arrange subjects randomly within the given layout
		for (auto const& [type_name, type] : types)
		{
			for (uint64_t subject_i = 0; subject_i < type.initially_available; ++subject_i)
			{
				// Try to place a subject of type `type_name` somewhere in the facility
				auto point_it = std::next(current.points.begin(), distribution_points(prng));
				auto selected_point_name = std::string(point_it->first);
				auto& selected_point = point_it->second;
				uint64_t attempt_i = 0;
				while ((!selected_point.addSubject(type_name, type.area)) && (attempt_i++ < max_attempts))
				{
					point_it = std::next(current.points.begin(), distribution_points(prng));
					selected_point_name = std::string(point_it->first);
					selected_point = point_it->second;
				}
				if (attempt_i == max_attempts)
					goto skip_sample;
			}
		}

		{
			// After subjects are arranged, find an optimal flow with CPLEX
			// CPLEX & variables
			IloEnv cplex_environment;
			IloModel cplex_model(cplex_environment);
			// 1. Flows
			std::map<std::pair<std::string, std::string>, std::map<std::pair<std::string, std::string>, IloIntVar>> cplex_x_flow;
			// 2. Generated units
			//std::map<std::string, IloIntVarArray> cplex_x_generated;
			std::map<std::string, std::map<std::string, IloIntVar>> cplex_x_generated;

			// Total flow cost (one of the terms of the objective function)
			IloNumExpr cplex_total_flow_cost(cplex_environment, 0.0);

			// Constraint (1): input does not exceed the input capacity (iterated over all types and then over all points)
			std::map<std::pair<std::string, std::string>, IloIntExpr> cplex_constr_in_flow;
			// Constraint (2): output does not exceed the output capacity (iterated over all types and then over all points)
			std::map<std::pair<std::string, std::string>, IloIntExpr> cplex_constr_out_flow;
			// Constraint (3): the area occupied by the subjects in one place/point does not exceed its area capacity (iterated over all points)
			//     (omitted)
			// Constraint (4): weak Kirchhoff condition
			//     reuse `cplex_constr_out_flow` and `cplex_constr_in_flow` for this
			// Constraint (5): flow from all subjects of type i to all subjects of type j equals the respective total flow
			// Constraint (6): all generated units by subjects of each type add up to the respective total number of generated units
			// Constraint (7): all subjects are placed somewhere
			//     (omitted)

			// Initialisation of variables, `cplex_total_flow_cost` and constraints
			//auto const initialisation_start_time = std::chrono::high_resolution_clock::now();
			//logger.info("Initialisation of model (variables and constraints) has started...");
			for (auto const& [type_name, type] : types)
				for (auto const& [point_name, point] : current.points)
				{
					if (point.subject_count.contains(type_name))
					{
						cplex_constr_in_flow.emplace(std::make_pair(type_name, point_name), IloIntExpr(cplex_environment, 0));
						cplex_constr_out_flow.emplace(std::make_pair(type_name, point_name), IloIntExpr(cplex_environment, 0));
						cplex_x_generated[type_name][point_name] = IloIntVar(cplex_environment, 0, IloIntMax, ("g_" + type_name + "_" + point_name).data());
					}
				}
			for (auto const& [type1_name, type1] : types)
				for (auto const& [point1_name, point1] : current.points)
					if (point1.subject_count.contains(type1_name))
						for (auto const& [type2_name, type2] : types)
							if (total_flows.at(type1_name).at(type2_name) != 0)
							{
								cplex_x_flow.insert({{type1_name, type2_name}, {}});
								
								for (auto const& [point2_name, point2] : current.points)
									if (point2.subject_count.contains(type2_name))
									{
										cplex_x_flow[{type1_name, type2_name}].insert({{point1_name, point2_name}, IloIntVar(cplex_environment, 0, IloIntMax, ("f_" + type1_name + type2_name + "_" + point1_name + "," + point2_name).data())});
										
										cplex_total_flow_cost += (IloNum)current.distance(point1, point2) * cplex_x_flow[{type1_name, type2_name}][{point1_name, point2_name}];
										
										cplex_constr_in_flow[{type2_name, point2_name}] += cplex_x_flow[{type1_name, type2_name}][{point1_name, point2_name}];
										cplex_constr_out_flow[{type1_name, point1_name}] += cplex_x_flow[{type1_name, type2_name}][{point1_name, point2_name}];
									}
							}
			//auto const initialisation_runtime = std::chrono::high_resolution_clock::now() - initialisation_start_time;
			//auto const initialisation_runtime_hms = std::chrono::hh_mm_ss(initialisation_runtime);
			//logger.info("Initialisation of model has finished.");

			// Add constraints
			for (auto const& [type1_name, type1] : types)
			{
				uint64_t point_i = 0;
				for (auto const& [point_name, point] : current.points)
					if (point.subject_count.contains(type1_name))
					{
						// (1)
						cplex_model.add
						(
							cplex_constr_in_flow[{type1_name, point_name}] <= (IloInt)point.subject_count.at(type1_name) * (IloNum)type1.in_capacity
						);
						// (2)
						cplex_model.add
						(
							cplex_constr_out_flow[{type1_name, point_name}] <= (IloInt)point.subject_count.at(type1_name) * (IloNum)type1.out_capacity
						);
						// (4)
						cplex_model.add
						(
							cplex_constr_out_flow[{type1_name, point_name}] <= cplex_x_generated[type1_name][point_name] + cplex_constr_in_flow[{type1_name, point_name}]
						);
						++point_i;
					}
				for (auto const& [type2_name, type2] : types)
					if (total_flows.at(type1_name).at(type2_name) != 0)
					{
						IloIntExpr accumulated_sum(cplex_environment, 0);
						for (auto const& [point1_name, point1] : current.points)
							for (auto const& [point2_name, point2] : current.points)
								if ((point1.subject_count.contains(type1_name)) && (point2.subject_count.contains(type2_name)))
									accumulated_sum += cplex_x_flow[{type1_name, type2_name}][{point1_name, point2_name}];
						// (5)
						cplex_model.add
						(
							accumulated_sum == (IloNum)(total_flows.at(type1_name).at(type2_name))
						);
					}
				// (6)
				IloIntExpr accumulated_sum(cplex_environment, 0);
				for (auto const& [point_name, point] : current.points)
						if (point.subject_count.contains(type1_name))
							accumulated_sum += cplex_x_generated[type1_name][point_name];
				cplex_model.add
				(
					accumulated_sum == (IloInt)types.at(type1_name).total_generated_units
				);
			}

			// Objective function
			cplex_model.add(IloMinimize(cplex_environment, cplex_total_flow_cost));

			IloCplex cplex(cplex_model);
			cplex.setOut(cplex_environment.getNullStream());
			cplex.setWarning(cplex_environment.getNullStream());
			cplex.setError(cplex_environment.getNullStream());
			cplex.setParam(IloCplex::Param::Threads, 1);
			cplex.solve();

			if (cplex.getStatus() == IloCplex::Status::Infeasible)
			{
				cplex_environment.end();
				goto skip_sample;
			}
			if (cplex.getObjValue() < best_objective_value)
			{
				answer = current;
				for (auto& [point1_name, point1] : answer.points)
					for (auto const& [type1_name, type1_subject_count] : point1.subject_count)
					{
						point1.generated_unit_count[type1_name] = cplex.getValue(cplex_x_generated[type1_name][point1_name]);
						for (auto const& [point2_name, point2] : answer.points)
							for (auto const& [type2_name, type2_subject_count] : point2.subject_count)
								if (total_flows.at(type1_name).at(type2_name) != 0)
									point1.out_flows[{type1_name, type2_name}][point2_name] = cplex.getValue(cplex_x_flow[{type1_name, type2_name}][{point1_name, point2_name}]);
					}
				best_objective_value = cplex.getObjValue();
			}

			cplex_environment.end();
			goto finish_sample;
		}

		skip_sample:
		++skipped_sample_count;
		finish_sample:
		if ((sample_i + 1) % 100 == 0)
		{
			logger_lock.lock();
			logger.info("Thread #" + std::to_string(thread_id) + " reports:");
			logger.info("\tTotal sample construction iterations = " + std::to_string(sample_i + 1) + "/" + std::to_string(workload) + "\n");
			logger.info("\t                          Successful = " + std::to_string(sample_i + 1 - skipped_sample_count) + "/" + std::to_string(sample_i + 1));
			if (skipped_sample_count > 0)
				logger.warning("\t                             Skipped = " + std::to_string(skipped_sample_count) + "/" + std::to_string(sample_i + 1));
			logger.info("\t      The best found total flow cost = " + std::to_string(best_objective_value));
			logger_lock.unlock();
		}
	}

	return_value.set_value({answer, best_objective_value});

	return;
}





} // fa::util










/**
 * @namespace fa
 * @brief Tools to generate facility arrangements
 *
 * These tools might be helpful to find a feasible solution for the given instance of
 * Facility Arrangement Problem with reasonably low value of the objective function
 * before running the actual CPLEX solver for the main problem.
 */
namespace fa
{





/**
 * @brief Generate a facility arrangement with Monte-Carlo
 *
 * Uses Monte-Carlo simulation to find a feasible solution. Pseudorandomly generates
 * multiple facility arrangements, computes the objective function for them and returns
 * one of the arrangements with the smallest found value of the objective function.
 * 
 * @param facility_layout Description of the available facility layout.
 * @param types Features of different types of subjects.
 * @param total_flows A map of maps containing pairs of a kind `<i : <j : f>>`
 *                    where \c f is the total flow from all subjects of type
 *                    \c i into all subjects of type \c j.
 * @param thread_count Number of threads to use to generate arrangements concurrently.
 *                     If set to `0`, the number of threads will be equal to the number
 *                     of available logical cores in the system.
 * @param workload Number of samples for \b each thread to generate.
 * 
 * @returns A facility arrangement with the best found value of the objective function.
 */
template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
FacilityArrangement<CoordinateType, AreaType, UnitType> const produceMC(FacilityLayout<CoordinateType, AreaType> const& facility_layout,
                                                                        SubjectTypeMap<AreaType, UnitType> const&       types,
                                                                        FlowMap<UnitType> const&                        total_flows,
                                                                        Logger const&  logger,
                                                                        uint16_t       thread_count = 0,
                                                                        uint64_t const workload     = 1000,
                                                                        uint64_t const max_attempts = 1000)
{
	// Get the number of available threads
	{
		uint16_t const max_thread_count = std::thread::hardware_concurrency();
		// !!!!! TODO !!!!! Assertions
		if (thread_count == 0)
			thread_count = max_thread_count;
	}

	FacilityArrangement<CoordinateType, AreaType, UnitType> answer;
	long double best_objective_value = std::numeric_limits<long double>::infinity();

	// Start threads and wait for them to finish computations
	logger.info("Starting feasible arrangement search in " + std::to_string(thread_count) + " threads.");
	logger.info("Each thread will generate " + std::to_string(workload) + " samples.");
	logger.info("Each sample will take at most " + std::to_string(max_attempts) + " attempts to generate.");
	std::vector<std::thread> threads;
	std::vector<std::future<util::ThreadReturn<CoordinateType, AreaType, UnitType>>> futures;
	std::mutex logger_mutex;
	std::seed_seq seed_sequence{9299U, 2020U, 2022U, 218U};
	std::vector<uint32_t> seeds(thread_count);
	seed_sequence.generate(seeds.begin(), seeds.end());
	for (uint16_t thread_i = 0; thread_i < thread_count; ++thread_i)
	{
		std::promise<util::ThreadReturn<CoordinateType, AreaType, UnitType>> promise;
		futures.push_back(promise.get_future());
		threads.push_back(std::thread
		(
			util::threadMC<CoordinateType, AreaType, UnitType>,
			std::cref(facility_layout),
			std::cref(types),
			std::cref(total_flows),
			std::move(promise),
			std::cref(logger),
			std::ref(logger_mutex),
			thread_i,
			workload,
			seeds[thread_i],
			max_attempts
		));
	}
	for (uint16_t thread_i = 0; thread_i < thread_count; ++thread_i)
	{
		util::ThreadReturn<CoordinateType, AreaType, UnitType> thread_return(futures[thread_i].get());
		if (thread_return.objective_value < best_objective_value)
		{
			answer = thread_return.facility_arrangement;
			best_objective_value = thread_return.objective_value;
		}
		threads[thread_i].join();
	}
	logger.info("A facility arrangement was found with the total flow cost of " + std::to_string(best_objective_value));
	
	return answer;
}





} // fa





#endif // __FASOLVER_FACILITY_HPP__
