/**
 * @file Facility.hpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FACILITY_HPP__
#define __FASOLVER_FACILITY_HPP__





#include "../PlanarMetric/PlanarMetric.hpp"
#include "../Logger/Logger.hpp"
#include <ilcplex/ilocplex.h>
#include <thread>
#include <mutex>
#include <future>
#include <random>





/**
 * @class FacilityLayout
 * @brief Containter for facility layouts
 *
 * Facility layout represents a plan of a facility. This plan shows places where
 * different subjects \b can be placed (these places are represented with objects of
 * class Point) and also determins how to compute distance between any two points on this
 * plan (distance is determined by an object of class PlanarMetric).
 * 
 * Facility layout is given and known in advance.
 * 
 * @f[
 * \text{The user has the ability to...} \\
 * \text{FacilityArrangement} \left\{
 * \begin{align}
 *     \text{FacilityLayout} &\left\{
 *     \begin{aligned}
 *         &\text{... know points where subjects can be placed.} \\
 *         &\text{... compute distance between such points.} \\
 *         &\text{... know the area available in each point.}
 *     \end{aligned}
 *     \right. \\
 *     &\text{... see how many subjects of each type are located in each point.} \\
 *     &\text{... know how many objects are produced by subjects located in each point.} \\
 *     &\text{... see how many objects go from subjects located in one point into subjects located in another point.}
 * \end{align}
 * \right.
 * @f]
 */
template<typename CoordinateType, typename AreaType>
	requires numeric<CoordinateType> && numeric<AreaType>
struct FacilityLayout final
{
	/// A map containing pairs of a kind `<n : p>` where \c n is a name of a point and \c p is a Point called \c n
	PointMap<CoordinateType, AreaType> points;
	/// A functor to measure distances between points
	PlanarMetric                       distance;



	/// @name Constructors & destructors
	/// @{
	
	/// By-value constructor
	explicit
	FacilityLayout(PointMap<CoordinateType, AreaType> const& points, PlanarMetric const& distance) noexcept
		: points(points), distance(distance) {};
	
	/// @}
};





/**
 * @class FacilityArrangement
 * @brief Container for facility arrangements
 *
 * Facility arrangement resembles facility layout very much, however, there is a
 * significant difference between the two. While FacilityLayout is \e just a plan of a
 * facility, facility arrangement is a plan \e enhanced by the information about the
 * actual placement of each and every subject and flows of objects among them.
 * 
 * Facility layout is given and known in advance. Facility arrangement is an output of
 * the solver, something one wants to receive in the end.
 * 
 * @f[
 * \text{The user has the ability to...} \\
 * \text{FacilityArrangement} \left\{
 * \begin{align}
 *     \text{FacilityLayout} &\left\{
 *     \begin{aligned}
 *         &\text{... know points where subjects can be placed.} \\
 *         &\text{... compute distance between such points.} \\
 *         &\text{... know the area available in each point.}
 *     \end{aligned}
 *     \right. \\
 *     &\text{... see how many subjects of each type are located in each point.} \\
 *     &\text{... know how many objects are produced by subjects located in each point.} \\
 *     &\text{... see how many objects go from subjects located in one point into subjects located in another point.}
 * \end{align}
 * \right.
 * @f]
 */
template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
struct FacilityArrangement final
{
	/// A map containing pairs of a kind `<n : p>` where \c n is a name of a point and \c p is a FacilityArrangementPoint called \c n
	FacilityArrangementPointMap<CoordinateType, AreaType, UnitType> points;
	/// A metric to measure distances between points
	PlanarMetric distance;



	/// @name Constructors & destructors
	/// @{
	
	/// Default constructor
	FacilityArrangement(void) = default;

	/// Copy constructor
	FacilityArrangement(FacilityArrangement const&) = default;

	/**
	 * @brief Constructor from FacilityLayout
	 *
	 * Constructs an empty arrangement for the given layout. Empty arrangement is the
	 * arrangement where all points are empty (contain no subjects).
	 * 
	 * @param facility_layout The facility layout to construct the empty arrangement for.
	 */
	FacilityArrangement(FacilityLayout<CoordinateType, AreaType> const& facility_layout)
		: distance(facility_layout.distance)
	{
		for (auto const& [point_name, point] : facility_layout.points)
			this->points[point_name] = point;
		return;
	};

	/// @}
};





#endif // __FASOLVER_FACILITY_HPP__
