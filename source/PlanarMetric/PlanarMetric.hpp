/**
 * @file PlanarMetric.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_PLANAR_METRIC_HPP__
#define __FASOLVER_PLANAR_METRIC_HPP__





#include "../_common_/_common_.hpp"
#include <map>
#include <cmath>
#include <type_traits> // std::conditional





namespace metric
{





/**
 * @class PlanarMetric
 * @brief Base template class for planar distances
 *
 * Base abstract template class for distance functors (not necessarily metrics
 * in the mathematical sense of this word).
 */
template <class ReturnType, class CoordinateType>
class PlanarMetric
{
	/// Predefined distances between certain points
	std::map<Point<CoordinateType>, std::map<Point<CoordinateType>, ReturnType>> predefined;



public:
	/// @name Constructors and destructors
	/// @{

	/**
	 * @brief Desctructor
	 *
	 * A virtual destructor.
	 */
	virtual ~PlanarMetric(void) {};

	/// @}



	/// @name Operators
	/// @{

	/**
	 * @brief Functor call
	 *
	 * Calculate distance between the two given points.
	 */
	virtual ReturnType const operator()(Point<CoordinateType> const& point1, Point<CoordinateType> const& point2) const = 0;

	/// @}
};





/**
 * @class Minkowski
 * @brief A template class for Minkowski distances
 *
 * Mikowski metric @f$p_n@f$ for @f$n \in \mathbb{N}@f$ for @f$\mathbb{R}^d@f$ is defined as
 * 
 * @f[p_n(\vec{x}, \vec{y}) = \left(  \sum_{i = 1}^d  |x_i - y_i|^n  \right)^{\frac{1}{n}}@f]
 */
template <uint8_t order, class CoordinateType>
class Minkowski : PlanarMetric<long double, CoordinateType>
{
public:
	long double const operator()(Point<CoordinateType> const& point1, Point<CoordinateType> const& point2) const override
	{
		return std::pow(std::pow(std::abs(point1.x - point2.x), order) + std::pow(std::abs(point1.y - point2.y), order), 1.L / order);
	}
};



template <class CoordinateType>
class Minkowski<1, CoordinateType> : PlanarMetric<typename std::conditional<std::is_integral<CoordinateType>::value, uint64_t, long double>::type, CoordinateType>
{
public:
	typename std::conditional<std::is_integral<CoordinateType>::value, uint64_t, long double>::type const operator()(Point<CoordinateType> const& point1, Point<CoordinateType> const& point2) const override
	{
		return std::abs(point1.x - point2.x) + std::abs(point1.y - point2.y);
	}
};





} // metric





#endif // __FASOLVER_PLANAR_METRIC_HPP__
