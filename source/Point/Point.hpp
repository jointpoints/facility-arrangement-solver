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





/**
 * @class Point
 * @brief 2D point with capacity
 *
 * Represents a place within a facility where subjects can be placed.
 */
template<typename CoordinateType>
	requires numeric<CoordinateType>
struct Point final
{
	CoordinateType x;
	CoordinateType y;
	uint64_t       capacity;



	Point(void)
		: x(0), y(0), capacity(0) {};
	
	explicit
	Point(CoordinateType x, CoordinateType y, uint64_t capacity)
		: x(x), y(y), capacity(capacity) {};

	inline bool const operator<(Point const& other) const
	{
		return (this->x < other.x) || ((this->x == other.x) && (this->y < other.y));
	}
};





template<typename CoordinateType>
	requires numeric<CoordinateType>
using PointMap = std::map<std::string, Point<CoordinateType>>;





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
template<typename CoordinateType>
	requires numeric<CoordinateType>
PointMap<CoordinateType> const grid(uint32_t const row_count, uint32_t const column_count, CoordinateType const distance, uint64_t capacity)
{
	PointMap<CoordinateType> answer;

	//for (uint32_t row_i : std::views::iota(0, row_count))
	//	for (uint32_t column_i : std::views::iota(0, column_count))
	for (uint32_t row_i = 0; row_i < row_count; ++row_i)
		for (uint32_t column_i = 0; column_i < column_count; ++column_i)
			answer["(" + std::to_string(row_i) + "," + std::to_string(column_i) + ")"] = Point((CoordinateType)(row_i * distance), (CoordinateType)(column_i * distance), capacity);

	return answer;
}





} // points





#endif // __FASOLVER_POINT_HPP__
