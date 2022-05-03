/**
 * @file Point.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_POINT_HPP__
#define __FASOLVER_POINT_HPP__





#include "../Common/Common.hpp"
#include "../SubjectType/SubjectType.hpp"
#include <limits>
//#include <ranges>





/**
 * @class Point
 * @brief A 2D point where subjects can be placed
 *
 * Represents a place within a facility where subjects can be placed.
 */
template<typename CoordinateType, typename AreaInputType, typename AreaOutputType = FASNone, typename SubjectCountOutputType = FASNone>
	requires fas_numeric<CoordinateType> && fas_numeric<AreaInputType> && fas_numeric_or_none<AreaOutputType> && fas_numeric_or_none<SubjectCountOutputType>
class Point final
{
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
	bool const _addSubject(std::string const type_name, SubjectType<AreaOutputType, SubjectCountInputType, UnitType, PriceType> const& type, SubjectCountOutputType const count);

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
	void _removeSubject(std::string const type_name, SubjectType<AreaOutputType, SubjectCountInputType, UnitType, PriceType> const& type, SubjectCountOutputType const count);

	/// @}



public:
	/// @name Constructors & destructors
	/// @{
	
	/// Default constructor
	Point(void) = default;
	
	/// By-values constructor
	explicit
	Point(CoordinateType const x, CoordinateType const y, AreaInputType const area);
	
	/// Copy constructor
	template<typename AnotherAreaOutputType, typename AnotherSubjectCountOutputType>
	Point(Point<CoordinateType, AreaInputType, AnotherAreaOutputType, AnotherSubjectCountOutputType> &point);
	
	/// Move constructor
	template<typename AnotherAreaOutputType, typename AnotherSubjectCountOutputType>
	Point(Point<CoordinateType, AreaInputType, AnotherAreaOutputType, AnotherSubjectCountOutputType> &&point);
	
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










template<typename CoordinateType, typename AreaInputType, typename AreaOutputType, typename SubjectCountOutputType>
template<typename SubjectCountInputType, typename UnitType, typename PriceType>
bool const Point<CoordinateType, AreaInputType, AreaOutputType, SubjectCountOutputType>
	::_addSubject(std::string const type_name, SubjectType<AreaOutputType, SubjectCountInputType, UnitType, PriceType> const& type, SubjectCountOutputType const count)
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



template<typename CoordinateType, typename AreaInputType, typename AreaOutputType, typename SubjectCountOutputType>
template<typename SubjectCountInputType, typename UnitType, typename PriceType>
void Point<CoordinateType, AreaInputType, AreaOutputType, SubjectCountOutputType>
	::_removeSubject(std::string const type_name, SubjectType<AreaOutputType, SubjectCountInputType, UnitType, PriceType> const& type, SubjectCountOutputType const count)
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
	requires numeric<CoordinateType> && numeric<AreaInputType>
PointMap<CoordinateType, AreaInputType> const grid(uint32_t const row_count, uint32_t const column_count, CoordinateType const distance, AreaInputType const capacity)
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
