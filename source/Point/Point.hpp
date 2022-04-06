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
using FacilityArrangementFlowMap = std::map<std::string, FlowMap<UnitType>>;





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
	CoordinateType x;
	CoordinateType y;
	AreaType       capacity;



	Point(void)
		: x(0), y(0), capacity(0) {};
	
	explicit
	Point(CoordinateType x, CoordinateType y, AreaType capacity)
		: x(x), y(y), capacity(capacity) {};

	inline bool const operator<(Point const& other) const
	{
		return (this->x < other.x) || ((this->x == other.x) && (this->y < other.y));
	}
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
	UnitMap<uint64_t>                    subject_count;
	FacilityArrangementFlowMap<UnitType> out_flows;



	FacilityArrangementPoint(Point const& point)
		: x(point.x), y(point.y), capacity(point.capacity), subject_count(), out_flows();
};





template<typename CoordinateType, typename AreaType>
	requires numeric<CoordinateType> && numeric<AreaType>
using PointMap = std::map<std::string, Point<CoordinateType, AreaType>>;

template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
using FacilityArrangementPointMap = std::map<std::string, FacilityArrangementPoint<CoordinateType, AreaType, UnitType>>;





/**
 * @namespace points
 * @brief Tools to create facilities
 *
 * Functions for convenient and rapid facility layout design.
 */
namespace points
{





/**
 * @brief Create a regular grid
 *
 * Creates a facility layout in a form of a regular grid with preset equal distances
 * between neighbouring points and equal area capacities of each point.
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
