/**
 * @file Point.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_POINT_HPP__
#define __FASOLVER_POINT_HPP__





#include "../Common/Common.hpp"
#include "../SubjectType/SubjectType.hpp"
#include <type_traits>
#include <limits>
//#include <ranges>





/**
 * @class Point
 * @brief A 2D point where subjects can be placed
 *
 * Represents a place within a facility where subjects can be placed.
 */
template<typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType = FASNone>
	requires fas_numeric<CoordinateType> && fas_numeric<AreaInputType> && fas_numeric_or_none<SubjectCountOutputType>
class Point final
{
	using AreaOutputType = std::conditional
	<
		std::is_same<SubjectCountOutputType, FASNone>::value,
		FASNone,
		std::conditional
		<
			std::is_same<SubjectCountOutputType, FASInteger>::value && std::is_same<AreaInputType, FASInteger>::value,
			FASInteger,
			FASFloat
		>::type
	>::type;



	/// @name Given data
	/// @{

	/// Abscissa, \b may be used by Metric to compute distances between several points
	CoordinateType _x;
	/// Ordinate, \b may be used by Metric to compute distances between several points
	CoordinateType _y;
	/// Total area available in the point
	AreaInputType _area_total;

	/// @}



	/// @name Arrangement data
	/// @{

	/// Free area available in the point
	AreaOutputType _area_free;
	/// Number of subjects of different types located in this point
	UnaryMap<SubjectCountOutputType> _subject_count;

	/// @}



	/// @name Modifiers
	/// @{

	/**
	 * @brief Put a new subject into the point
	 *
	 * Tries to put a new subject of the specified type into the point.
	 * 
	 * @param type_name The name of the subject type.
	 * @param type The type itself.
	 * @param count The number of subjects to be added.
	 * 
	 * @returns \c true if the subject was placed successfully, \c false if there is not
	 * enough free area in the point.
	 */
	template<typename SubjectCountInputType, typename UnitType, typename PriceType>
	bool const _addSubject(std::string const type_name, SubjectType<AreaInputType, SubjectCountInputType, UnitType, PriceType> const& type, SubjectCountOutputType const count);

	/**
	 * @brief Remove a subject from the point
	 *
	 * Removes a subject of the given type from the point, if there are any.
	 * 
	 * @param type_name The name of the subject type.
	 * @param type The type itself.
	 * @param count The number of subjects to be removed.
	 */
	template<typename SubjectCountInputType, typename UnitType, typename PriceType>
	void _removeSubject(std::string const type_name, SubjectType<AreaInputType, SubjectCountInputType, UnitType, PriceType> const& type, SubjectCountOutputType const count);

	/// @}



public:
	/// @name Constructors & destructors
	/// @{
	
	/// Default constructor
	Point(void) = default;
	
	/// By-values constructor
	explicit
	Point(CoordinateType const x, CoordinateType const y, AreaInputType const area);
	
	/// Conversion constructor
	template<typename Old_CoordinateType, typename Old_AreaInputType, typename Old_SubjectCountOutputType>
	Point(Point<Old_CoordinateType, Old_AreaInputType, Old_SubjectCountOutputType> const &point);
	
	/// @}



	/// @name Access
	/// @{
	
	/// Get the abscissa
	inline
	CoordinateType const x(void) const;

	/// Get the ordinate
	inline
	CoordinateType const y(void) const;

	/// Get the total area
	inline
	AreaInputType const areaTotal(void) const;

	/// Get the available area
	inline
	AreaOutputType const areaFree(void) const;

	/// Get the total number of subjects in the point
	inline
	SubjectCountOutputType const countSubjects(void) const;

	/// Get the number of subjects of the set type in the point
	inline
	SubjectCountOutputType const countSubjects(std::string const type_name) const;

	/// @}
};










// Methods of Point










template<typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType>
template<typename SubjectCountInputType, typename UnitType, typename PriceType>
bool const Point<CoordinateType, AreaInputType, SubjectCountOutputType>
	::_addSubject(std::string const type_name, SubjectType<AreaInputType, SubjectCountInputType, UnitType, PriceType> const& type, SubjectCountOutputType const count)
{
	if (this->_area_free < type.area * count)
		return false;
	if (this->_subject_count.contains(type_name))
		this->_subject_count[type_name] += count;
	else
		this->_subject_count[type_name] = count;
	this->_area_free -= type.area * count;
	return true;
}



template<typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType>
template<typename SubjectCountInputType, typename UnitType, typename PriceType>
void Point<CoordinateType, AreaInputType, SubjectCountOutputType>
	::_removeSubject(std::string const type_name, SubjectType<AreaInputType, SubjectCountInputType, UnitType, PriceType> const& type, SubjectCountOutputType const count)
{
	if (this->_subject_count.contains(type_name))
	{
		if (this->_subject_count[type_name] == count)
			this->_subject_count.erase(type_name);
		else
			this->_subject_count[type_name] -= count;
		this->_area_free += type.area * count;
	}
}



template<typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType>
Point<CoordinateType, AreaInputType, SubjectCountOutputType>
	::Point(CoordinateType const x, CoordinateType const y, AreaInputType const area)
	: _x(x)
	, _y(y)
	, _area_total(area)
	, _area_free(area)
{}



template<typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType>
template<typename Old_CoordinateType, typename Old_AreaInputType, typename Old_SubjectCountOutputType>
Point<CoordinateType, AreaInputType, SubjectCountOutputType>
	::Point(Point<Old_CoordinateType, Old_AreaInputType, Old_SubjectCountOutputType> const &point)
	: _x(point._x)
	, _y(point._y)
	, _area_total(point._area_total)
	, _area_free(point._area_free)
{
	for (auto const &[type_name, type_count] : point._subject_count)
		this->_subject_count[type_name] = type_count;
	return;
}



#define DEFINE_GETTER(getterName, ReturnType, getter_var)                                  \
template<typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType> \
ReturnType const Point<CoordinateType, AreaInputType, SubjectCountOutputType>              \
    :: ## getterName (void) const                                                          \
{                                                                                          \
    return this-> ## getter_var;                                                           \
}

DEFINE_GETTER(x, CoordinateType, _x)
DEFINE_GETTER(y, CoordinateType, _y)
DEFINE_GETTER(areaTotal, AreaInputType, _area_total)

template<typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType>
Point<CoordinateType, AreaInputType, SubjectCountOutputType>::AreaOutputType const Point<CoordinateType, AreaInputType, SubjectCountOutputType>
    ::areaFree(void) const
{
    return this->_area_free;
}

#undef DEFINE_GETTER



template<typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType>
SubjectCountOutputType const Point<CoordinateType, AreaInputType, SubjectCountOutputType>
	::countSubjects(void) const
{
	SubjectCountOutputType answer = 0;
	for (auto const &[type_name, type_count] : this->_subject_count)
		answer += type_count;
	return answer;
}



template<typename CoordinateType, typename AreaInputType, typename SubjectCountOutputType>
SubjectCountOutputType const Point<CoordinateType, AreaInputType, SubjectCountOutputType>
	::countSubjects(std::string const type_name) const
{
	return this->_subject_count.contains(type_name) ? this->_subject_count.at(type_name) : 0;
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
template<typename CoordinateType, typename AreaInputType>
	requires fas_numeric<CoordinateType> && fas_numeric<AreaInputType>
UnaryMap<Point<CoordinateType, AreaInputType>> const grid(uint32_t const row_count, uint32_t const column_count, CoordinateType const distance, AreaInputType const capacity)
{
	PointMap<CoordinateType, AreaInputType> answer;

	//for (uint32_t row_i : std::views::iota(0, row_count))
	//	for (uint32_t column_i : std::views::iota(0, column_count))
	for (uint32_t row_i = 0; row_i < row_count; ++row_i)
		for (uint32_t column_i = 0; column_i < column_count; ++column_i)
			answer["(" + std::to_string(row_i) + "," + std::to_string(column_i) + ")"] = Point((CoordinateType)(row_i * distance), (CoordinateType)(column_i * distance), capacity);

	return answer;
}





} // points





#endif // __FASOLVER_POINT_HPP__
