/**
 * @file PlanarMetric.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_PLANAR_METRIC_HPP__
#define __FASOLVER_PLANAR_METRIC_HPP__





#include "../Common/Common.hpp"
#include "../Point/Point.hpp"
#include <functional> // std::function

#define CORE_FUNCTION_TYPE std::function<DistanceType(FASFloat const &, FASFloat const &, FASFloat const &, FASFloat const &)>

/// Infinity to use as metrics parameter
#define oo (uint8_t)0





/**
 * @class PlanarMetric
 * @brief Functor to measure distances between points in 2D
 *
 * This functor (a class that provides a Callable interface) enables
 * the computation of distance between any two points from two-dimensional space with
 * coordinates of the specified in advance type. Even though it is called \e metric,
 * distance functions are not really expected to be metrics in the mathematical sense of
 * this word.
 * 
 * @note Intended to only be used within FacilityLayout, FacilityArrangement, and
 * FASolver.
 */
template<typename DistanceType>
	requires fas_numeric<DistanceType>
class PlanarMetric
{
	/// Stored Callable object to measure distances
	CORE_FUNCTION_TYPE dist;



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
	PlanarMetric(CORE_FUNCTION_TYPE distance_function);

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
	 * @param point1 One point from the 2D plane.
	 * @param point2 Another point from the 2D plane.
	 * 
	 * @returns The distance between \c point1 and \c point2 as an instance of
	 * \c DistanceType.
	 */
	template<typename Op_CoordinateType, typename Op_AreaInputType, typename Op_SubjectCountOutputType>
	inline
	DistanceType const operator()(Point<Op_CoordinateType, Op_AreaInputType, Op_SubjectCountOutputType> const& point1, Point<Op_CoordinateType, Op_AreaInputType, Op_SubjectCountOutputType> const& point2) const;

	/// @}
};










// Methods of PlanarMetric










template<typename DistanceType>
	requires fas_numeric<DistanceType>
PlanarMetric<DistanceType>::PlanarMetric(CORE_FUNCTION_TYPE distance_function)
	: dist(distance_function)
{}










// Operators of PlanarMetric










template<typename DistanceType>
	requires fas_numeric<DistanceType>
template<typename Op_CoordinateType, typename Op_AreaInputType, typename Op_SubjectCountOutputType>
DistanceType const PlanarMetric<DistanceType>
	::operator()(Point<Op_CoordinateType, Op_AreaInputType, Op_SubjectCountOutputType> const &point1, Point<Op_CoordinateType, Op_AreaInputType, Op_SubjectCountOutputType> const &point2) const
{
	return this->dist(point1.x(), point1.y(), point2.x(), point2.y());
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





template<uint8_t order, typename CoordinateType>
struct MinkowskiDistanceTypeSelector {using type = FASFloat;};

template<typename CoordinateType>
struct MinkowskiDistanceTypeSelector<1, CoordinateType> {using type = CoordinateType;};

template<typename CoordinateType>
struct MinkowskiDistanceTypeSelector<oo, CoordinateType> {using type = CoordinateType;};





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
template<uint8_t order, typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType, typename DistanceType = MinkowskiDistanceTypeSelector<order, CoordinateType>::type>
PlanarMetric<DistanceType> const Minkowski
(
	UnaryMap<Point<CoordinateType, AreaInputType, SubjectCountOutputType>> const& points
)
{
	// If order is 1, use simpler formula
	if (order == 1)
		return PlanarMetric(CORE_FUNCTION_TYPE
		(
			[order](FASFloat const &point1_x, FASFloat const &point1_y, FASFloat const &point2_x, FASFloat const &point2_y)
			{
				return fasAbs(point1_x - point2_x) + fasAbs(point1_y - point2_y);
			}
		));
	// If order is infinity, use simpler formula
	else if (order == oo)
		return PlanarMetric(CORE_FUNCTION_TYPE
		(
			[order](FASFloat const &point1_x, FASFloat const &point1_y, FASFloat const &point2_x, FASFloat const &point2_y)
			{
				return std::max(fasAbs(point1_x - point2_x), fasAbs(point1_y - point2_y));
			}
		));
	// Otherwise, use the general formula
	else
		return PlanarMetric(CORE_FUNCTION_TYPE
		(
			[order](FASFloat const &point1_x, FASFloat const &point1_y, FASFloat const &point2_x, FASFloat const &point2_y)
			{
				return std::pow(std::pow(fasAbs(point1_x - point2_x), order) + std::pow(fasAbs(point1_y - point2_y), order), 1.L / order);
			}
		));
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
