/**
 * @file PlanarMetric.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_PLANAR_METRIC_HPP__
#define __FASOLVER_PLANAR_METRIC_HPP__





#include "../Common/Common.hpp"
#include "../Point/Point.hpp"
#include <functional> // std::function
#include <cmath>

#define CORE_FUNCTION_TYPE std::function<DistanceType(Point<CoordinateType, AreaInputType, SubjectCountOutputType> const &, Point<CoordinateType, AreaInputType, SubjectCountOutputType> const &)>

/// Infinity to use as metrics parameter
#define oo (uint8_t)0





/**
 * @class PlanarMetric
 * @brief Functor to measure distances between points in 2D
 *
 * This functor (a class whose objects provide an interface similar to Callable) enables
 * the computation of distance between any two points from two-dimensional space with
 * coordinates of the specified in advance type. Even though it is called \e metric,
 * distance functions are not really expected to be metrics in the mathematical sense of
 * this word.
 * 
 * @note Intended to only be used within FacilityLayout, FacilityArrangement, and
 * FASolver.
 */
template<typename DistanceType>
class PlanarMetric
{
	/// Stored Callable object to measure distances
	std::shared_ptr<void> dist;



public:
	/// @name Constructors and destructors
	/// @{

	/// Default constructor
	PlanarMetric(void) = default;

	/// Copy constructor
	//PlanarMetric(PlanarMetric const&) = default;

	/**
	 * @brief By-value constructor
	 *
	 * Constructs a new PlanarMetric object.
	 * 
	 * @param distance_function A pointer to a \e dynamically allocated object of type
	 *                          \c CORE_FUNCTION_TYPE cast to `void*`.
	 */
	explicit inline
	PlanarMetric(void *distance_function);

	/**
	 * @brief Destructor
	 *
	 * Properly destroys the PlanarMetric object.
	 */
	//~PlanarMetric(void);

	/// @}



	/// @name Operators
	/// @{

	/**
	 * @brief Compute distance
	 *
	 * Makes a call of the distance function that was passed as the constructor argument.
	 * 
	 * @param point1 One Point from the 2D plane.
	 * @param point2 Another Point from the 2D plane.
	 * 
	 * @returns The distance between \c point1 and \c point2 as an instance of
	 * \c METRIC_RETURN_TYPE.
	 */
	template<typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType>
	inline
	DistanceType const operator()(Point<CoordinateType, AreaInputType, SubjectCountOutputType> const& point1, Point<CoordinateType, AreaInputType, SubjectCountOutputType> const& point2) const;

	/// @}
};










// Methods of PlanarMetric










template<typename DistanceType>
PlanarMetric<DistanceType>::PlanarMetric(void *distance_function)
	: dist(distance_function)
{}










// Operators of PlanarMetric










template<typename DistanceType>
template<typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType>
DistanceType const PlanarMetric<DistanceType>::operator()(Point<CoordinateType, AreaInputType, SubjectCountOutputType> const &point1, Point<CoordinateType, AreaInputType, SubjectCountOutputType> const &point2) const
{
	return *reinterpret_cast<CORE_FUNCTION_TYPE*>(this->dist.get())(point1, point2);
}










// Namespaces











/**
 * @namespace metric
 * @brief Convenience tools to measure distances in 2D
 *
 * This namespace contains utilities related to measuring distances between points in 2D.
 */
namespace metric
{





/**
 * @brief Get an instance of Minkowski metric
 *
 * Generates an instance of Minkowski metric of the given order for a coordinate type
 * used in the given map of points.
 * 
 * Minkowski metric of order @f$n \in \mathbb{N}@f$ for points @f$\overrightarrow{x}@f$
 * and @f$\overrightarrow{y}@f$ from @f$\mathbb{R}^d@f$ is defined as follows:
 * 
 * @f[M_n(\overrightarrow{x}, \overrightarrow{y}) = \left(  \sum_{i = 1}^d |x_i - y_i|^n  \right)^\frac{1}{n} @f]
 * 
 * Minkowski metric of order @f$n = \infty@f$ for points @f$\overrightarrow{x}@f$ and
 * @f$\overrightarrow{y}@f$ from @f$\mathbb{R}^d@f$ is defined as follows:
 * 
 * @f[M_\infty(\overrightarrow{x}, \overrightarrow{y}) = \max_{i = 1}^d \left(  |x_i - y_i|  \right) @f]
 * 
 * @param order An integer number @f$n \in \mathbb{N}_{\leq 255}@f$ or \c oo (stands for
 *              infinity).
 * @param points A map of points. Only needed to derive the type of their coordinates,
 *               never actually used. I.e., no distances are precomputed in advance.
 * 
 * @returns A constant PlanarMetric instance ready to use for computation of Minkowski
 * metric.
 */
template<uint8_t order, typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType, typename DistanceType = FASFloat>
PlanarMetric<DistanceType> const Minkowski(UnaryMap<Point<CoordinateType, AreaInputType, SubjectCountOutputType>> const& points)
{
	CORE_FUNCTION_TYPE* dist = new CORE_FUNCTION_TYPE
	([order](Point<CoordinateType, AreaInputType, SubjectCountOutputType> const& point1, Point<CoordinateType, AreaInputType, SubjectCountOutputType> const& point2)
	{
		return std::pow(std::pow(std::abs(point1.x - point2.x), order) + std::pow(std::abs(point1.y - point2.y), order), 1.L / order);
	});

	return PlanarMetric<DistanceType>(reinterpret_cast<void *>(dist));
}
// If the order is 1, use simpler formulae
template<typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType>
PlanarMetric<CoordinateType> const Minkowski<1, CoordinateType, AreaInputType, SubjectCountOutputType, CoordinateType>(UnaryMap<Point<FASInteger, AreaInputType, SubjectCountOutputType>> const& points)
{
	CORE_FUNCTION_TYPE* dist = new CORE_FUNCTION_TYPE
	([order](Point<CoordinateType, AreaInputType, SubjectCountOutputType> const& point1, Point<CoordinateType, AreaInputType, SubjectCountOutputType> const& point2)
	{
		return std::abs(point1.x - point2.x) + std::abs(point1.y - point2.y);
	});

	return PlanarMetric<CoordinateType>(reinterpret_cast<void *>(dist));
}
// If the order is infinity, use simpler formulae
template<typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType>
PlanarMetric<CoordinateType> const Minkowski<oo, CoordinateType, AreaInputType, SubjectCountOutputType, CoordinateType>(UnaryMap<Point<FASInteger, AreaInputType, SubjectCountOutputType>> const& points)
{
	CORE_FUNCTION_TYPE* dist = new CORE_FUNCTION_TYPE
	([order](Point<CoordinateType, AreaInputType, SubjectCountOutputType> const& point1, Point<CoordinateType, AreaInputType, SubjectCountOutputType> const& point2)
	{
		return return std::max(std::abs(point1.x - point2.x), std::abs(point1.y - point2.y));
	});

	return PlanarMetric<CoordinateType>(reinterpret_cast<void *>(dist));
}





/*template <typename CoordinateType, typename AreaType>
	requires numeric<CoordinateType> && numeric<AreaType>
PlanarMetric const PostOffice(uint8_t order, PointMap<CoordinateType, AreaType> const& points, Point<CoordinateType, AreaType> const post_office = {0, 0, 0})
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
}*/





} // metric





#undef CORE_FUNCTION_TYPE





#endif // __FASOLVER_PLANAR_METRIC_HPP__
