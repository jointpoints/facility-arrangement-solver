/**
 * @file Point.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_POINT_HPP__
#define __FASOLVER_POINT_HPP__





#include <cstdint> // fixed-width integers
#include <map>
#include <string>





/**
 * @class Point
 * @brief 2D point with capacity
 *
 * Represents a place within a facility where subjects can be placed.
 */
template <typename CoordinateType>
struct Point final
{
	CoordinateType x;
	CoordinateType y;
	uint64_t       capacity;



	Point(void)
		: x(0), y(0), capacity(0) {};
	
	Point(CoordinateType x, CoordinateType y, uint64_t capacity)
		: x(x), y(y), capacity(capacity) {};

	inline bool const operator<(Point const& other) const
	{
		return (this->x < other.x) || ((this->x == other.x) && (this->y < other.y));
	}
};





template<typename CoordinateType>
using PointMap = std::map<std::string, Point<CoordinateType>>;





#endif // __FASOLVER_POINT_HPP__
