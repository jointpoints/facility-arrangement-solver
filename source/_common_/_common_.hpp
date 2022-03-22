/**
 * @file _common_.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_COMMON_HPP__
#define __FASOLVER_COMMON_HPP__





#include <cstdint> // fixed-width integers





template <class CoordinateType>
struct Point final
{
	CoordinateType x;
	CoordinateType y;
	uint64_t       capacity;



	Point(CoordinateType x, CoordinateType y, uint64_t capacity) : x(x), y(y), capacity(capacity) {};

	inline bool const operator<(Point const& other) const
	{
		return (this->x < other.x) || ((this->x == other.x) && (this->y < other.y));
	}
};





#endif // __FASOLVER_COMMON_HPP__
