/**
 * @file Facility.hpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FACILITY_HPP__
#define __FASOLVER_FACILITY_HPP__





#include "../Common/Common.hpp"
#include "../PlanarMetric/PlanarMetric.hpp"





enum ArrangementStrategy
{
	CPLEX,
	GMC_CPLEX
};





/**
 * @class Facility
 * @brief Facility layout and arrangement
 *
 * The objects of this class represent the \e plan of a facility alongside with the way
 * to \e arrange user's subjects within the facility and the way to organise the \e flow
 * of objects among them.
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

	template<typename AreaInputType, typename SubjectCountInputType, typename UnitType, typename PriceType>
	void arrange
	(
		UnaryMap<SubjectType<AreaInputType, SubjectCountInputType, UnitType, PriceType>> const &subject_types,
		ArrangementStrategy const strategy = GMC_CPLEX
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
	ArrangementStrategy const strategy
)
{}





#endif // __FASOLVER_FACILITY_HPP__
