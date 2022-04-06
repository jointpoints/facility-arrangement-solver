/**
 * @file Facility.hpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FACILITY_HPP__
#define __FASOLVER_FACILITY_HPP__





#include "../PlanarMetric/PlanarMetric.hpp"





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



	FacilityArrangement(FacilityLayout<CoordinateType, AreaType> const& layout)
		: distance(layout.distance)
	{
		for (auto const& [point_name, point] : layout.points)
			this->points[point_name] = point;
		return;
	};
};





#endif // __FASOLVER_FACILITY_HPP__
