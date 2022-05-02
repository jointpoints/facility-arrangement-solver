/**
 * @file FAHeuristic_GeneticMonteCarlo.hpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FAHEURISTIC_GENETIC_MONTE_CARLO_HPP__
#define __FASOLVER_FAHEURISTIC_GENETIC_MONTE_CARLO_HPP__





#include "../FAHeuristic/FAHeuristic.hpp"





// Namespaces





namespace fa::util
{





/**
 * @brief Generate a portion of Genetic Monte-Carlo samples
 *
 * Used by fa::produceMC to be run by a single thread to generate some part of Genetic
 * Monte-Carlo samples. Each thread returns the best found arrangement, the main thread
 * is then supposed to choose the best one among them.
 * 
 * @param facility_arrangement The initial arrangement of subjects. In the first
 *                             generation is expected to be empty. Otherwise, is expected
 *                             to be the best arrangement found in the previous
 *                             generation.
 * @param types Features of different types of subjects.
 * @param total_flows A map of maps containing pairs of a kind `<i : <j : f>>` where \c f
 *                    is the total flow from all subjects of type \c i into all subjects
 *                    of type \c j.
 * @param return_value An object to store the return value of the thread.
 * @param logger A logger.
 * @param logger_mutex A mutex shared by all the threads to prevent multiple threads
 *                     writing to the logger simultaneously.
 * @param thread_id An ID to distinguish this thread.
 * @param generation_i The ordinal number of the current generation.
 * @param workload Number of samples for the thread to generate.
 * @param seed A seed to use for a pseudorandom number generator.
 * @param max_attempts Maximum number of attempts to make to generate one facility
 *                     arrangement.
 * 
 * @returns Nothing. Return value is handed back to the main thread via \c return_value.
 */
template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
void threadGMC(FacilityArrangement<CoordinateType, AreaType, UnitType> facility_arrangement,
               SubjectTypeMap<AreaType, UnitType>                      types,
               FlowMap<UnitType> const&                                total_flows,
               std::promise<ThreadReturn<CoordinateType, AreaType, UnitType>> return_value,
               Logger const& logger,
               std::mutex&   logger_mutex,
               uint16_t const thread_id,
               uint16_t const generation_i,
               uint64_t const workload,
               uint64_t const seed,
               uint64_t const max_attempts)
{
	FacilityArrangement<CoordinateType, AreaType, UnitType> answer;
	long double best_objective_value = std::numeric_limits<long double>::infinity();
	std::unique_lock logger_lock(logger_mutex, std::defer_lock);

	std::mt19937_64 prng(seed);
	std::uniform_int_distribution<> distribution_points(0, facility_arrangement.points.size() - 1);

	// Fix (2^`generation_i` - 1) / 2^`generation_i` subjects in the given facility
	if (generation_i > 0)
	{
		double subject_count = [&types](){double answer = 0; for (auto const& [type_name, type] : types) answer += type.initially_available; return answer;}();
		std::uniform_int_distribution<> distribution_types(0, types.size() - 1);
		for (auto& [type_name, type] : types)
			type.initially_available = 0;
		for (uint64_t subject_i = 0; subject_i / subject_count <= 1.L / (1U << generation_i); ++subject_i)
		{
			auto point_it = std::next(facility_arrangement.points.begin(), distribution_points(prng));
			while (point_it->second.countSubjects() == 0)
				point_it = std::next(facility_arrangement.points.begin(), distribution_points(prng));
			auto type_it = std::next(types.begin(), distribution_types(prng));
			while (point_it->second.countSubjects(type_it->first) == 0)
				type_it = std::next(types.begin(), distribution_types(prng));
			point_it->second.removeSubject(type_it->first, type_it->second.area);
			++(type_it->second.initially_available);
		}
	}

	uint64_t skipped_sample_count = 0;
	for (uint64_t sample_i = 0; sample_i < workload; ++sample_i)
	{
		FacilityArrangement<CoordinateType, AreaType, UnitType> current(facility_arrangement);

		// Arrange subjects randomly within the given layout
		for (auto const& [type_name, type] : types)
		{
			for (uint64_t subject_i = 0; subject_i < type.initially_available; ++subject_i)
			{
				// Try to place a subject of type `type_name` somewhere in the facility
				auto point_it = std::next(current.points.begin(), distribution_points(prng));
				uint64_t attempt_i = 0;
				for (; (attempt_i < max_attempts) && (!point_it->second.addSubject(type_name, type.area)); ++attempt_i)
					point_it = std::next(current.points.begin(), distribution_points(prng));
				if (attempt_i >= max_attempts)
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
			std::map<std::string, std::map<std::string, IloIntVar>> cplex_x_produced;

			// Total flow cost (one of the terms of the objective function)
			IloNumExpr cplex_total_flow_cost(cplex_environment, 0.0);

			// Constraint (1): input does not exceed the input capacity (iterated over all types and then over all points)
			std::map<std::pair<std::string, std::string>, IloIntExpr> cplex_constr_in_flow;
			// Constraint (2): output does not exceed the output capacity (iterated over all types and then over all points)
			std::map<std::pair<std::string, std::string>, IloIntExpr> cplex_constr_out_flow;
			// Constraint (3): the area occupied by the subjects in one place/point does not exceed its area capacity (iterated over all points)
			//     (satisfied by construction)
			// Constraint (4): weak Kirchhoff condition
			//     reuse `cplex_constr_out_flow` and `cplex_constr_in_flow` for this
			// Constraint (5): flow from all subjects of type i to all subjects of type j equals the respective total flow
			// Constraint (6): all generated units by subjects of each type add up to the respective total number of generated units
			// Constraint (7): all subjects are placed somewhere
			//     (satisfied by construction)

			// Initialisation of variables, `cplex_total_flow_cost` and constraints
			for (auto const& [type_name, type] : types)
				for (auto const& [point_name, point] : current.points)
					if (point.countSubjects(type_name) > 0)
					{
						cplex_constr_in_flow[{type_name, point_name}] = IloIntExpr(cplex_environment, 0);
						cplex_constr_out_flow[{type_name, point_name}] = IloIntExpr(cplex_environment, 0);
						cplex_x_produced[type_name][point_name] = IloIntVar(cplex_environment, 0, IloIntMax, ("g_" + type_name + "_" + point_name).data());
					}
			for (auto const& [type1_name, type1] : types)
				for (auto const& [type2_name, type2] : types)
					if (total_flows.at(type1_name).at(type2_name) != 0)
						for (auto const& [point1_name, point1] : current.points)
							if (point1.countSubjects(type1_name) > 0)
								for (auto const& [point2_name, point2] : current.points)
									if (point2.countSubjects(type2_name) > 0)
									{
										cplex_x_flow[{type1_name, type2_name}][{point1_name, point2_name}] = IloIntVar(cplex_environment, 0, IloIntMax, ("f_" + type1_name + type2_name + "_" + point1_name + "," + point2_name).data());
										
										cplex_total_flow_cost += (IloNum)current.distance(point1, point2) * cplex_x_flow[{type1_name, type2_name}][{point1_name, point2_name}];
										
										cplex_constr_in_flow[{type2_name, point2_name}] += cplex_x_flow[{type1_name, type2_name}][{point1_name, point2_name}];
										cplex_constr_out_flow[{type1_name, point1_name}] += cplex_x_flow[{type1_name, type2_name}][{point1_name, point2_name}];
									}

			// Add constraints
			for (auto const& [type1_name, type1] : types)
			{
				uint64_t point_i = 0;
				for (auto const& [point_name, point] : current.points)
					if (point.countSubjects(type1_name) > 0)
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
							cplex_constr_out_flow[{type1_name, point_name}] <= cplex_x_produced[type1_name][point_name] + cplex_constr_in_flow[{type1_name, point_name}]
						);
						++point_i;
					}
				for (auto const& [type2_name, type2] : types)
					if (total_flows.at(type1_name).at(type2_name) != 0)
					{
						IloIntExpr accumulated_sum(cplex_environment, 0);
						for (auto const& [point1_name, point1] : current.points)
							for (auto const& [point2_name, point2] : current.points)
								if (point1.countSubjects(type1_name) > 0 && point2.countSubjects(type2_name) > 0)
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
					if (point.countSubjects(type1_name) > 0)
						accumulated_sum += cplex_x_produced[type1_name][point_name];
				cplex_model.add
				(
					accumulated_sum == (IloInt)type1.production_target
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
						if (type1_subject_count != 0)
						{
							if (point1.produced_unit_count.contains(type1_name))
								point1.produced_unit_count[type1_name] += cplex.getValue(cplex_x_produced[type1_name][point1_name]);
							else
								point1.produced_unit_count[type1_name] = cplex.getValue(cplex_x_produced[type1_name][point1_name]);
							for (auto const& [point2_name, point2] : answer.points)
								for (auto const& [type2_name, type2_subject_count] : point2.subject_count)
									if (total_flows.at(type1_name).at(type2_name) != 0 && type2_subject_count != 0)
										point1.out_flows[{type1_name, type2_name}][point2_name] = cplex.getValue(cplex_x_flow[{type1_name, type2_name}][{point1_name, point2_name}]);
						}
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
			logger.info("Generation #" + std::to_string(generation_i + 1) + ", thread #" + std::to_string(thread_id) + " reports:");
			logger.info("\t   Processed samples = " + std::to_string(sample_i + 1) + "/" + std::to_string(workload) + "\n");
			logger.info("\t          Successful = " + std::to_string(sample_i + 1 - skipped_sample_count) + "/" + std::to_string(sample_i + 1));
			if (skipped_sample_count > 0)
				logger.warning("\t             Skipped = " + std::to_string(skipped_sample_count) + "/" + std::to_string(sample_i + 1));
			logger.info("\tBest total flow cost = " + std::to_string(best_objective_value));
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
 * @brief Generate a facility arrangement with Genetic Monte-Carlo
 *
 * Uses Monte-Carlo simulation to find a feasible solution. In each generation
 * pseudorandomly generates multiple facility arrangements, computes the objective
 * function for them and selects one of the arrangements with the smallest found value of
 * the objective function. The selected facility arrangement is passed as the initial
 * starting point for the next generation where a part of subjects is pseudorandomly
 * fixed (each thread fixes different subset of subjects) while the rest of the subjects
 * are pseudorandomly shuffled. The share of fixed subjects in each generation is
 * calculated by the formula
 * 
 * @f[\frac{2^\text{<generation no.>} - 1}{2^\text{<generation no.>}}@f]
 * 
 * where generations are numbered starting with @f$0@f$. A facility arrangement with the
 * best value of the objective function is selected as the starting point for each
 * consecutive generation. The selected arrangement in the last generation is returned to
 * the user.
 * 
 * @param facility_layout Description of the available facility layout.
 * @param types Features of different types of subjects.
 * @param total_flows A map of maps containing pairs of a kind `<i : <j : f>>`
 *                    where \c f is the total flow from all subjects of type
 *                    \c i into all subjects of type \c j.
 * @param logger A logger that is going to collect the logs and print them.
 * @param thread_count Number of threads to use to generate arrangements concurrently.
 *                     If set to `0`, the number of threads will be equal to the number
 *                     of available logical cores in the system.
 * @param generation_count Number of generations to simulate. If set to `1`, the result
 *                         of the algorithm is identical to the regular Monte-Carlo.
 * @param workload Number of samples for \b each thread to generate in \b each
 *                 generation.
 * @param max_attempts The maximum number of attempts to spend on generation of a single
 *                     arrangement. If all available subjects can't be placed in the
 *                     facility layout after `max_attempts` attempts, the algorithm gives
 *                     up and starts arranging subjects anew (all subjects that were
 *                     fixed at the start of the generation simulation remain fixed and
 *                     untouched).
 * 
 * @returns A facility arrangement with the best found value of the objective function.
 */
template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
FacilityArrangement<CoordinateType, AreaType, UnitType> const GMC(FacilityLayout<CoordinateType, AreaType> const& facility_layout,
                                                                  SubjectTypeMap<AreaType, UnitType> const&       types,
                                                                  FlowMap<UnitType> const&                        total_flows,
                                                                  Logger const&  logger,
                                                                  uint16_t       thread_count     = 0,
                                                                  uint16_t const generation_count = 1,
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

	FacilityArrangement<CoordinateType, AreaType, UnitType> answer(facility_layout);
	long double best_objective_value = std::numeric_limits<long double>::infinity();

	// Start threads and wait for them to finish computations
	logger.info("Starting feasible arrangement search with Genetic Monte-Carlo heuristic.");
	logger.info("Search will be done within " + std::to_string(generation_count) + " generations, " + std::to_string(thread_count) + " threads.");
	logger.info("Each thread will generate " + std::to_string(workload) + " samples per generation.");
	logger.info("Each sample will take at most " + std::to_string(max_attempts) + " attempts to generate.");
	std::vector<std::thread> threads;
	std::vector<std::future<util::ThreadReturn<CoordinateType, AreaType, UnitType>>> futures;
	std::mutex logger_mutex;
	std::seed_seq seed_sequence{9299U, 4521U, 2022U, 218U};
	std::vector<uint32_t> seeds(thread_count);
	seed_sequence.generate(seeds.begin(), seeds.end());
	for (uint16_t generation_i = 0; generation_i < generation_count; ++generation_i)
	{
		for (uint16_t thread_i = 0; thread_i < thread_count; ++thread_i)
		{
			std::promise<util::ThreadReturn<CoordinateType, AreaType, UnitType>> promise;
			futures.push_back(promise.get_future());
			threads.push_back(std::thread
			(
				util::threadGMC<CoordinateType, AreaType, UnitType>,
				answer,
				types,
				std::cref(total_flows),
				std::move(promise),
				std::cref(logger),
				std::ref(logger_mutex),
				thread_i,
				generation_i,
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
		futures.clear();
		threads.clear();
		logger.info("Generation #" + std::to_string(generation_i + 1) + " produced a facility arrangement with the total flow cost of " + std::to_string(best_objective_value));
	}
	logger.info("A facility arrangement was found with the total flow cost of " + std::to_string(best_objective_value));
	
	return answer;
}





} // fa





#endif // __FASOLVER_FAHEURISTIC_GENETIC_MONTE_CARLO_HPP__
