/**
 * @file Facility.hpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FACILITY_HPP__
#define __FASOLVER_FACILITY_HPP__





#include "../Common/Common.hpp"





/**
 * @class Facility
 * @brief brief
 *
 * details
 */
class Facility
{
	/// @name Metadata
	/// @{

	/// Says whether the coordinates of points are integral
	bool _coordinate_type_integral;
	/// Says whether the area capacities of points are integral
	bool _area_capacity_type_integral;
	/// Says whether the areas of subjects are integral
	bool _area_consumption_type_integral;
	/// Says whether the units of objects are integral
	bool _unit_type_integral;
	/// Says whether distances between points are integral
	bool _distance_type_integral;

	/// @}



	/// @name Data
	/// @{

	/// Points that make up a facility
	std::shared_ptr<void> _points;
	/// A functor to measure distances between points
	std::shared_ptr<void> _distance;
	/// Flows between points

	/// @}
};





#endif // __FASOLVER_FACILITY_HPP__
