/**
 * @file Facility.hpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FACILITY_HPP__
#define __FASOLVER_FACILITY_HPP__





#include "../Common/Common.hpp"
#include "../PlanarMetric/PlanarMetric.hpp"
#include "../Logger/Logger.hpp"
// Facility arrangement algorithms
#include "../FacilityArrangementAlgorithms/FAA_CPLEX.hpp"





class FacilityArrangementStrategy
{
	/// Possible arrangement algorithms
	static
	enum Algorithm
	{
		FASTRAT_CPLEX,
		FASTRAT_GMC
	};

	/// Sequence of algorithms
	std::vector<Algorithm> sequence;



	/// @name Constructors & destructors
	/// @{

	/// Not default-constructable
	FacilityArrangementStrategy(void) = delete;

	/// By-value constructor
	FacilityArrangementStrategy(Algorithm const algorithm);

	/// By-value constructor
	FacilityArrangementStrategy(std::vector<Algorithm> const &sequence);

	/// @}



public:
	/// @name Operators
	/// @{

	FacilityArrangementStrategy const operator>>(FacilityArrangementStrategy const &other);

	/// @}



friend Facility;
};





/**
 * @class Facility
 * @brief Facility layout and arrangement
 *
 * The objects of this class represent the \e plan of a facility alongside with the
 * interface to \e arrange user's subjects within the facility and the way to organise
 * the \e flow of objects among them.
 */
class Facility
{
	/// @name Metadata
	/// @{

	/// Says whether distances between points are integral
	bool _distance_type_integral;
	/// Says whether coordinates of points are integral
	bool _coordinate_type_integral;
	/// Says whether area capacities of points are integral
	bool _area_input_type_integral;
	/// Says whether subjects are counted by integers
	bool _subject_count_output_type_integral;
	/// Says whether subjects are not counted at all
	bool _subject_count_output_type_none;
	/// Says whether units of objects are integral
	bool _unit_type_integral;

	/// @}



	/// @name Data
	/// @{

	/// Points that make up a facility
	std::unique_ptr<void> _points;
	/// A functor to measure distances between points
	std::unique_ptr<void> _distance;
	/// Flows between points
	std::unique_ptr<void> _flow;

	/// @}



public:
	/// Available arrangement algorithms
	static
	struct
	{
		FacilityArrangementStrategy CPLEX;
		FacilityArrangementStrategy GeneticMonteCarlo;
	} strategy;



	/// @name Constructors & destructors
	/// @{

	/**
	 * @brief By-value constructor
	 *
	 * Creates a facility with an empty arrangement for the given points and the given
	 * distance function.
	 * 
	 * @param points Points where subjects can be placed.
	 * @param distance A distance function.
	 */
	template<typename DistanceType, typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType>
	explicit inline
	Facility
	(
		UnaryMap<Point<CoordinateType, AreaInputType, SubjectCountOutputType>> const &points,
		PlanarMetric<DistanceType, CoordinateType, AreaInputType, SubjectCountOutputType> const &distance
	);

	/// @}



	/// @name Modifiers
	/// @{

	/**
	 * @brief Arrange subjects within the facility
	 *
	 * Performs an optimal or suboptimal arrangement of subjects and object flows within
	 * the given facility layout. Optimality/suboptimality and execution time depends on
	 * the strategy chosen for arrangement. Possible options are listed in the table
	 * below.
	 * <table>
	 * <tr align="center">
	 *     <th>Strategy</th>
	 *     <th>Solution quality</th>
	 *     <th>Time cost</th>
	 *     <th>Description</th>
	 * </tr>
	 * <tr align="center">
	 *     <td><tt>FASTRAT_CPLEX</tt></td>
	 *     <td>Optimal</td>
	 *     <td>Expensive</td>
	 *     <td>Launches CPLEX to find an exact solution to the optimisation problem.</td>
	 * </tr>
	 * </table>
	 * 
	 * @param subject_types Subjects to be placed within the facility.
	 * @param total_flows Total flow of objects between all possible pairs of types.
	 *                    Value `total_flows[i][j]` shows how many objects \b must be
	 *                    transported in total from all objects of type `i` to all
	 *                    objects of type `j`.
	 * @param strategy Arrangement strategy. Must be one of the values stated in the
	 *                 table above.
	 * 
	 * @returns Nothing. All changes are made to the internal data structures.
	 */
	template<typename AreaInputType, typename SubjectCountInputType, typename UnitType, typename PriceType>
	void arrange
	(
		UnaryMap<SubjectType<AreaInputType, SubjectCountInputType, UnitType, PriceType>> const &subject_types,
		BinaryMap<UnitType> const &total_flows,
		FacilityArrangementStrategy const strategy = Facility::strategy.CPLEX
	);

	/// @}
};










// Methods of Facility










template<typename DistanceType, typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType>
Facility::Facility
(
	UnaryMap<Point<CoordinateType, AreaInputType, SubjectCountOutputType>> const &points,
	PlanarMetric<DistanceType, CoordinateType, AreaInputType, SubjectCountOutputType> const &distance
)
{
	this->_points.reset(reinterpret_cast<void *>(new UnaryMap<Point<CoordinateType, AreaInputType, SubjectCountOutputType>>(points)));
	this->_coordinate_type_integral = std::is_same<CoordinateType, FASInteger>::value;
	this->_area_input_type_integral = std::is_same<AreaInputType, FASInteger>::value;
	this->_subject_count_output_type_integral = std::is_same<SubjectCountOutputType, FASInteger>::value;
	this->_subject_count_output_type_none = std::is_same<SubjectCountOutputType, FASNone>::value;

	this->_distance.reset(reinterpret_cast<void *>(new PlanarMetric(distance)));
	this->_distance_type_integral = std::is_same<DistanceType, FASInteger>::value;

	return;
}





template<typename AreaInputType, typename SubjectCountInputType, typename UnitType, typename PriceType>
void Facility::arrange
(
	UnaryMap<SubjectType<AreaInputType, SubjectCountInputType, UnitType, PriceType>> const &subject_types,
	BinaryMap<UnitType> const &total_flows,
	FacilityArrangementStrategy const strategy
)
{
	// Start logging
	int logger_status_code = 0;
	Logger logger("./test.txt", &logger_status_code);
	if (logger_status_code != FASOLVER_LOGGER_STATUS_OK)
		throw 1; // TODO
	
	// Create interpretation of strategy
	std::map
	<
		FacilityArrangementStrategy::Algorithm,
		void (*)
		(
			Facility *,
			UnaryMap<SubjectType<AreaInputType, SubjectCountInputType, UnitType, PriceType>> const &,
			BinaryMap<UnitType> const &,
			Logger const &,
			bool const
		)
	> const arrangement_algorithm
	{
		{FacilityArrangementStrategy::Algorithm::FASTRAT_CPLEX, &facilityArrangementAlgorithm_CPLEX}
	};

	// Execute strategy
	for (auto const &strategy_step : strategy.sequence)
		*arrangement_algorithm.at(strategy_step)(this, subject_types, total_flows, logger, true);
}





#endif // __FASOLVER_FACILITY_HPP__
