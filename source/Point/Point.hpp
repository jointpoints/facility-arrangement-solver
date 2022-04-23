/**
 * @file Point.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_POINT_HPP__
#define __FASOLVER_POINT_HPP__





#include <concepts>
#include <cstdint> // fixed-width integers
#include <map>
//#include <ranges>
#include <string>





template<typename X>
concept numeric = std::integral<X> || std::floating_point<X>;





template<typename UnitType>
	requires numeric<UnitType>
using UnitMap = std::map<std::string, UnitType>;

template<typename UnitType>
	requires numeric<UnitType>
using FlowMap = std::map<std::string, UnitMap<UnitType>>;

template<typename UnitType>
	requires numeric<UnitType>
using FacilityArrangementFlowMap = std::map<std::pair<std::string, std::string>, std::map<std::string, UnitType>>;





/**
 * @class Point
 * @brief 2D point with capacity
 *
 * Represents a place within a facility where subjects can be placed.
 */
template<typename CoordinateType, typename AreaType>
	requires numeric<CoordinateType> && numeric<AreaType>
struct Point
{
	/// Abscissa, \b may be used by PlanarMetric to compute distances between several points.
	CoordinateType x;
	/// Ordinate, \b may be used by PlanarMetric to compute distances between several points.
	CoordinateType y;
	/// The area available in the point.
	AreaType       capacity;



	/// @name Constructors & destructors
	/// @{
	
	/// Default constructor
	Point(void) = default;
	
	/// By-values constructor
	explicit
	Point(CoordinateType const x, CoordinateType const y, AreaType const capacity)
		: x(x), y(y), capacity(capacity) {};
	
	/// @}



	/// @name Operators
	/// @{

	/**
	 * @brief Strict less
	 *
	 * Used to induce a strict partial order over points. If @f$a@f$ and @f$b@f$ are two
	 * points, then
	 * 
	 * @f[a < b \Leftrightarrow \left[ \begin{aligned} &a_x < b_x \\ &\left\{ \begin{aligned} &a_x = b_x \\ &a_y < b_y \end{aligned} \right. \end{aligned} \right. .@f]
	 * 
	 * This order is needed to store points in maps.
	 */
	/*inline bool const operator<(Point const& other) const
	{
		return (this->x < other.x) || ((this->x == other.x) && (this->y < other.y));
	}*/

	/// @}
};





/**
 * @class FacilityArrangementPoint
 * @brief 2D point with capacity to be used in completed facility arrangements
 *
 * Represents a place within a facility where subjects have been placed and their flows
 * have been assigned.
 */
template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
struct FacilityArrangementPoint final
	: public Point<CoordinateType, AreaType>
{
	/// The unoccupied area available in the point
	AreaType                             remaining_capacity;
	/// A map containing pairs of a kind `<t : n>` where \c t is a name of a subject type and \c n is a number of subjects of type \c t placed into this point
	UnitMap<uint64_t>                    subject_count;
	/// A map containing pairs of a kind `<t : n>` where \c t is a name of a subject type and \c n is a number of objects of type \c t produced by all subjects located in this point
	UnitMap<UnitType>                    produced_unit_count;
	/// A map containing pairs of a kind `<{t1, t2} : <p : n>>` where \c t1 and \c t2 are names of subject types, \c p is a name of a point and \c n is a number of objects sent from all subjects of type \c t1 located in this point into all subjects of type \c t2 located in \c p
	FacilityArrangementFlowMap<UnitType> out_flows;



	/// @name Constructors & destructors
	/// @{
	
	/// Default constructor
	FacilityArrangementPoint(void) = default;

	/// Copy constructor
	FacilityArrangementPoint(FacilityArrangementPoint<CoordinateType, AreaType, UnitType> const&) = default;

	/**
	 * @brief Constructor from a regular Point
	 *
	 * Copies values of all the properties of a regular point, calls default constructors
	 * for everything else.
	 * 
	 * @param point A point to copy the data from.
	 */
	FacilityArrangementPoint(Point<CoordinateType, AreaType> const& point);

	/// @}



	/// @name Access
	/// @{
	
	/**
	 * @brief Count the total number of subjects
	 *
	 * Counts the total number of all subjects located in this point.
	 * 
	 * @returns The number of placed subjects.
	 */
	uint64_t const countSubjects(void) const;

	/**
	 * @brief Count the number of subjects
	 *
	 * Counts the number of subjects of the given type located in this point.
	 * 
	 * @param type_name The name of the type.
	 * 
	 * @returns The number of placed subjects of type \c type_name.
	 */ 
	uint64_t const countSubjects(std::string const type_name) const;

	/// @}



	/// @name Modifiers
	/// @{
	
	/**
	 * @brief Place subject into point
	 *
	 * Places a new subject of the given type into this point.
	 * 
	 * @param type_name The name of the type.
	 * @param area The area to be occupied by the new subject.
	 * 
	 * @returns \c true if the subject was successfully placed, \c false if the available
	 * free area in the point is less than \c area.
	 */
	bool const addSubject(std::string const type_name, AreaType const area);

	/**
	 * @brief Remove subject from point
	 *
	 * Remove a subject of the given type from this point.
	 * 
	 * @param type_name The name of the type.
	 * @param area The area to be freed.
	 */
	void removeSubject(std::string const type_name, AreaType const area);

	/// @}
};





template<typename CoordinateType, typename AreaType>
	requires numeric<CoordinateType> && numeric<AreaType>
using PointMap = std::map<std::string, Point<CoordinateType, AreaType>>;

template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
using FacilityArrangementPointMap = std::map<std::string, FacilityArrangementPoint<CoordinateType, AreaType, UnitType>>;










// Definitions of FacilityArrangementPoint template struct member functions





template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
FacilityArrangementPoint<CoordinateType, AreaType, UnitType>::FacilityArrangementPoint(Point<CoordinateType, AreaType> const& point)
	: remaining_capacity(point.capacity)
	, subject_count()
	, produced_unit_count()
	, out_flows()
{
	this->x = point.x;
	this->y = point.y;
	this->capacity = point.capacity;
	return;
}





template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
uint64_t const FacilityArrangementPoint<CoordinateType, AreaType, UnitType>::countSubjects(void) const
{
	uint64_t answer = 0;
	for (auto const& [type_name, type_count] : this->subject_count)
		answer += type_count;
	return answer;
}





template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
uint64_t const FacilityArrangementPoint<CoordinateType, AreaType, UnitType>::countSubjects(std::string const type_name) const
{
	return this->subject_count.contains(type_name) ? this->subject_count.at(type_name) : 0;
}





template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
bool const FacilityArrangementPoint<CoordinateType, AreaType, UnitType>::addSubject(std::string const type_name, AreaType const area)
{
	if (this->remaining_capacity < area)
		return false;
	if (this->subject_count.contains(type_name))
		++this->subject_count[type_name];
	else
		this->subject_count[type_name] = 1;
	this->remaining_capacity -= area;
	return true;
}





template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
void FacilityArrangementPoint<CoordinateType, AreaType, UnitType>::removeSubject(std::string const type_name, AreaType const area)
{
	if (!this->subject_count.contains(type_name) || this->subject_count.at(type_name) == 0)
		return;
	this->subject_count[type_name] -= 1;
	this->remaining_capacity += area;
	return;
}










// Namespaces





/**
 * @namespace points
 * @brief Tools to create facilities
 *
 * Functions for rapid facility layout design.
 */
namespace points
{





/**
 * @brief Create a regular grid
 *
 * Creates a facility layout in a form of a regular grid with preset equal distances
 * between neighbouring points and equal area capacities for all points.
 * 
 * @param row_count The number of rows in the grid.
 * @param column_count The number of columns in the grid.
 * @param distance The distance between neighbouring points.
 * @param capacity The area available in each point of the grid.
 * 
 * @returns Grid as a PointMap.
 */
template<typename CoordinateType, typename AreaType>
	requires numeric<CoordinateType> && numeric<AreaType>
PointMap<CoordinateType, AreaType> const grid(uint32_t const row_count, uint32_t const column_count, CoordinateType const distance, AreaType const capacity)
{
	PointMap<CoordinateType, AreaType> answer;

	//for (uint32_t row_i : std::views::iota(0, row_count))
	//	for (uint32_t column_i : std::views::iota(0, column_count))
	for (uint32_t row_i = 0; row_i < row_count; ++row_i)
		for (uint32_t column_i = 0; column_i < column_count; ++column_i)
			answer["(" + std::to_string(row_i) + "," + std::to_string(column_i) + ")"] = Point((CoordinateType)(row_i * distance), (CoordinateType)(column_i * distance), capacity);

	return answer;
}





} // points





#endif // __FASOLVER_POINT_HPP__
