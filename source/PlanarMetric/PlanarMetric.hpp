/**
 * @file PlanarMetric.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_PLANAR_METRIC_HPP__
#define __FASOLVER_PLANAR_METRIC_HPP__





#include "../Point/Point.hpp"
#include <functional> // std::function
#include <map>
#include <cmath>

#define METRIC_RETURN_TYPE long double
#define CORE_FUNCTION_TYPE std::function<METRIC_RETURN_TYPE(Point<CoordinateType> const&, Point<CoordinateType> const&)>

/// Infinity to use as metrics parameter
#define oo (uint8_t)0





/**
 * @namespace metric
 * @brief Tools to measure distances in 2D
 *
 * This namespace contains utilities related to measuring distances between points in 2D.
 * Even though it is called \c metric, distance functions are not really expected to be
 * metrics in the mathematical sense of this word.
 */
namespace metric
{





/**
 * @class PlanarMetric
 * @brief Generalised functor to measure distances between points in 2D
 *
 * This functor (a class whose objects provide an interface similar to Callable) enables
 * the computation of distance between any two points from two-dimensional space with
 * coordinates of the specified in advance type.
 * 
 * @note Intended to only be used within FacilityLayout class.
 */
class PlanarMetric
{
	/// Stored Callable object to measure distances
	void* const dist;



public:
	/// @name Constructors and destructors
	/// @{

	PlanarMetric(void) = delete;

	/**
	 * @brief Constructor
	 *
	 * Constructs a new PlanarMetric object.
	 * 
	 * @param distance_function A pointer to a \e dynamically allocated object of type
	 *                          \c CORE_FUNCTION_TYPE cast to `void*`.
	 */
	explicit PlanarMetric(void* distance_function);

	/**
	 * @brief Destructor
	 *
	 * Properly destroys the PlanarMetric object.
	 */
	~PlanarMetric(void);

	/// @}



	/// @name Operators
	/// @{

	/**
	 * @brief Compute distance
	 *
	 * Makes a call of distance function passed as the constructor argument.
	 * 
	 * @param point1 One Point from the 2D plane.
	 * @param point2 Another Point from the 2D plane.
	 * 
	 * @returns The distance between \c point1 and \c point2 as an instance of
	 * \c METRIC_RETURN_TYPE.
	 */
	template<typename CoordinateType>
	METRIC_RETURN_TYPE const operator()(Point<CoordinateType> const& point1, Point<CoordinateType> const& point2) const
	{
		return (*reinterpret_cast<CORE_FUNCTION_TYPE*>(this->dist))(point1, point2);
	}

	/// @}
};





/**
 * @brief Get an instance of Minkowski metric
 *
 * Generates an instance of Minkowski metric with the given order for a coordinate type
 * used in the given map of points.
 * 
 * Minkowski metric for points @f$\overrightarrow{x}@f$ and @f$\overrightarrow{y}@f$
 * from @f$\mathbb{R}^d@f$ of order @f$n \in \mathbb{N}@f$ is defined as follows:
 * 
 * @f[M_n(\overrightarrow{x}, \overrightarrow{y}) = \left(  \sum_{i = 1}^d |x_i - y_i|^n  \right)^\frac{1}{n} @f]
 * 
 * Minkowski metric for points @f$\overrightarrow{x}@f$ and @f$\overrightarrow{y}@f$
 * from @f$\mathbb{R}^d@f$ of order @f$n = \infty$ is defined as follows:
 * 
 * @f[M_\infty(\overrightarrow{x}, \overrightarrow{y}) = \max_{i = 1}^d \left(  |x_i - y_i|  \right) @f]
 * 
 * @param order An integer number @f$n \in \mathbb{N}_{\leq 255}@f$ or \c oo (stands for
 *              infinity).
 * @param points A map of points. Only needed to derive the type of their coordinates,
 *               never actually used. I.e., no distances are precomputed in advance.
 * 
 * @returns A constant PlanarMetric instance ready to use to compute the Minkowski
 * distance.
 */
template <typename CoordinateType>
PlanarMetric const Minkowski(uint8_t order, POINT_MAP_TYPE const& points)
{
	CORE_FUNCTION_TYPE* dist = nullptr;

	// If the order is either 1 or infinity, use simpler formulae
	if (order == 1)
		dist = new CORE_FUNCTION_TYPE
		([order](Point<CoordinateType> const& point1, Point<CoordinateType> const& point2)
		{
			return std::abs(point1.x - point2.x) + std::abs(point1.y - point2.y);
		});
	else if (order == oo)
		dist = new CORE_FUNCTION_TYPE
		([order](Point<CoordinateType> const& point1, Point<CoordinateType> const& point2)
		{
			return std::max(std::abs(point1.x - point2.x), std::abs(point1.y - point2.y));
		});
	else
		dist = new CORE_FUNCTION_TYPE
		([order](Point<CoordinateType> const& point1, Point<CoordinateType> const& point2)
		{
			return std::pow(std::pow(std::abs(point1.x - point2.x), order) + std::pow(std::abs(point1.y - point2.y), order), 1.L / order);
		});

	return PlanarMetric(reinterpret_cast<void*>(dist));
}





template <typename CoordinateType>
PlanarMetric const PostOffice(uint8_t order, POINT_MAP_TYPE const& points, Point<CoordinateType> const post_office = {0, 0, 0})
{
	CORE_FUNCTION_TYPE* dist = nullptr;

	// If the order is either 1 or infinity, use simpler formulae
	if (order == 1)
		dist = new CORE_FUNCTION_TYPE
		([order](Point<CoordinateType> const& point1, Point<CoordinateType> const& point2)
		{
			return std::abs(point1.x - point2.x) + std::abs(point1.y - point2.y);
		});
	else if (order == oo)
		dist = new CORE_FUNCTION_TYPE
		([order](Point<CoordinateType> const& point1, Point<CoordinateType> const& point2)
		{
			return std::max(std::abs(point1.x - point2.x), std::abs(point1.y - point2.y));
		});
	else
		dist = new CORE_FUNCTION_TYPE
		([order](Point<CoordinateType> const& point1, Point<CoordinateType> const& point2)
		{
			return std::pow(std::pow(std::abs(point1.x - point2.x), order) + std::pow(std::abs(point1.y - point2.y), order), 1.L / order);
		});

	return PlanarMetric(reinterpret_cast<void*>(dist));
}





} // metric





#undef CORE_FUNCTION_TYPE
#undef METRIC_RETURN_TYPE





#endif // __FASOLVER_PLANAR_METRIC_HPP__