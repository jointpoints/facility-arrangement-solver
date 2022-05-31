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
	bool _unit_output_type_integral;

	/// @}



	/// @name Data
	/// @{

	/// Points that make up a facility
	std::unique_ptr<void> _points;
	/// A functor to measure distances between points
	std::unique_ptr<void> _distance;
	/// Flows between points
	std::unique_ptr<void> _flows;

	/// @}



public:
	/// Available arrangement algorithms
	static
	struct
	{
		FacilityArrangementStrategy CPLEX;
		FacilityArrangementStrategy MonteCarlo;
	} strategy_blocks;



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
		PlanarMetric<DistanceType> const &distance
	);

	/// @}



	/// @name Modifiers
	/// @{

	/**
	 * @brief Arrange subjects within the facility
	 *
	 * Performs an optimal or suboptimal arrangement of subjects and object flows within
	 * the given facility layout. Optimality/suboptimality and execution time depends on
	 * the strategy chosen for arrangement. Arrangement strategy is a sequence of
	 * arrangement algorithms executed one after another. Available arrangement
	 * algorithms are:
	 * <table>
	 * <tr align="center">
	 *     <th>Algorithm</th>
	 *     <th>Trivial name</th>
	 *     <th>Solution quality</th>
	 *     <th>Time cost</th>
	 *     <th>Referenced function</th>
	 * </tr>
	 * <tr align="center">
	 *     <td><tt>Facility::strategy_blocks.CPLEX</tt></td>
	 *     <td>CPLEX-based solver</td>
	 *     <td>Optimal</td>
	 *     <td>Expensive</td>
	 *     <td>facilityArrangementAlgorithm_CPLEX</td>
	 * </tr>
	 * <tr align="center">
	 *     <td><tt>Facility::strategy_blocks.MonteCarlo</tt></td>
	 *     <td>Monte-Carlo generator</td>
	 *     <td>Suboptimal</td>
	 *     <td>Reasonable</td>
	 *     <td>facilityArrangementAlgorithm_CPLEX</td>
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
	template
	<
		typename Subj_AreaInputType,
		typename SubjectCountInputType,
		typename UnitInputType,
		typename PriceType
	>
	void arrange
	(
		UnaryMap<SubjectType<Subj_AreaInputType, SubjectCountInputType, UnitInputType, PriceType>> const &subject_types,
		BinaryMap<UnitInputType> const &total_flows,
		FacilityArrangementStrategy const strategy = Facility::strategy_blocks.CPLEX,
		bool const warm_start = false
	);

	/// @}
};










// Methods of Facility










template<typename DistanceType, typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType>
Facility::Facility
(
	UnaryMap<Point<CoordinateType, AreaInputType, SubjectCountOutputType>> const &points,
	PlanarMetric<DistanceType> const &distance
)
{
	this->_points.reset(static_cast<void *>(new UnaryMap<Point<CoordinateType, AreaInputType, SubjectCountOutputType>>(points)));
	this->_coordinate_type_integral = std::is_same<CoordinateType, FASInteger>::value;
	this->_area_input_type_integral = std::is_same<AreaInputType, FASInteger>::value;
	this->_subject_count_output_type_integral = std::is_same<SubjectCountOutputType, FASInteger>::value;
	this->_subject_count_output_type_none = std::is_same<SubjectCountOutputType, FASNone>::value;

	this->_distance.reset(static_cast<void *>(new PlanarMetric(distance)));
	this->_distance_type_integral = std::is_same<DistanceType, FASInteger>::value;

	return;
}





template
<
	typename Subj_AreaInputType,
	typename SubjectCountInputType,
	typename UnitInputType,
	typename PriceType
>
void Facility::arrange
(
	UnaryMap<SubjectType<Subj_AreaInputType, SubjectCountInputType, UnitInputType, PriceType>> const &subject_types,
	BinaryMap<UnitInputType> const &total_flows,
	FacilityArrangementStrategy const strategy,
	bool const warm_start
)
{
	// Start logging
	int logger_status_code = 0;
	Logger logger("./test.txt", &logger_status_code);
	if (logger_status_code != FASOLVER_LOGGER_STATUS_OK)
		throw 1; // TODO
	
	// Reestablish types
	RUNTIME_CONDITIONAL(DistanceType, this->_distance_type_integral, FASInteger, FASFloat,
	RUNTIME_CONDITIONAL(CoordinateType, this->_coordinate_type_integral, FASInteger, FASFloat,
	RUNTIME_CONDITIONAL(AreaInputType, this->_area_input_type_integral, FASInteger, FASFloat,
	RUNTIME_CONDITIONAL(SubjectCountOutputType, this->_subject_count_output_type_integral, FASInteger, FASFloat,
		// Within the context of defined types, define the rest of the types
		using UnitOutputType = std::conditional
		<
			std::is_same<SubjectCountOutputType, FASInteger>::value && std::is_same<UnitInputType, FASInteger>::value,
			FASInteger,
			FASFloat
		>::type;
		using FacilityArrangementAlgorithm = void (*)
		(
			UnaryMap<Point<CoordinateType, AreaInputType, SubjectCountOutputType>> &,
			PlanarMetric<DistanceType> const &,
			BinaryPairMap<UnitOutputType> &,
			UnaryMap<SubjectType<AreaInputType, SubjectCountInputType, UnitInputType, PriceType>> const &,
			BinaryMap<UnitInputType> const &,
			Logger const &,
			bool const
		);

		// Define the interpretation of strategy blocks
		std::map<FacilityArrangementStrategy::Algorithm, FacilityArrangementAlgorithm> const algorithms
		{
			{FacilityArrangementStrategy::Algorithm::FASTRAT_CPLEX, &facilityArrangementAlgorithm_CPLEX<DistanceType, CoordinateType, AreaInputType, SubjectCountInputType, SubjectCountOutputType, UnitInputType, UnitOutputType, PriceType>}
		};

		// Reinterpret the facility data
		auto *explicit_points = static_cast<UnaryMap<Point<CoordinateType, AreaInputType, SubjectCountOutputType>> *>(this->_points.get());
		auto *explicit_distance = static_cast<PlanarMetric<DistanceType> *>(this->_distance.get());
		auto *explicit_flows = static_cast<BinaryPairMap<UnitOutputType> *>(this->_flows.get());

		// Convert subject types so that their area type coincides with the one of the facility
		UnaryMap<SubjectType<AreaInputType, SubjectCountInputType, UnitInputType, PriceType>> converted_subject_types;
		for (auto const &[type_name, type] : subject_types)
			converted_subject_types[type_name] = type;
		
		// Execute the arrangement strategy
		for (auto step_it = strategy.sequence.begin(); step_it != strategy.sequence.end(); ++step_it)
			*algorithms.at(*step_it)(*explicit_points, *explicit_distance, *explicit_flows, converted_subject_types, total_flows, logger, step_it != strategy.sequence.begin() || warm_start);
	))))
}





#endif // __FASOLVER_FACILITY_HPP__
