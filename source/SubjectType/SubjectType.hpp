/**
 * @file SubjectType.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_SUBJECT_TYPE_HPP__
#define __FASOLVER_SUBJECT_TYPE_HPP__





#include "../Common/Common.hpp"





/**
 * @class SubjectType
 * @brief A type of subjects
 *
 * Subjects of the same type share the same features and a common position in the
 * production chain or, in other words, are indistinguishable from one another from the
 * functional point of view.
 */
template<typename AreaInputType, typename SubjectCountInputType, typename UnitInputType, typename PriceType>
	requires fas_numeric<AreaInputType> && fas_numeric<SubjectCountInputType> && fas_numeric<UnitInputType> && fas_numeric<PriceType>
struct SubjectType final
{
	/// @name Data
	/// @{

	/// Input capacity of a single subject of the type
	UnitInputType in_capacity;
	/// Output capacity of a single subject of the type
	UnitInputType out_capacity;
	/// Area occupied by a single subject of the type
	AreaInputType area;
	/// The number of subjects initially available for the placement
	SubjectCountInputType initially_available;
	/// The price for each additional subject of the type
	PriceType price;

	/// @}



	/// @name Constructors & destructors
	/// @{

	SubjectType(void) = default;

	SubjectType(UnitInputType in_capacity, UnitInputType out_capacity, AreaInputType area, SubjectCountInputType initially_available, PriceType price);

	template<typename Old_AreaInputType, typename Old_SubjectCountInputType, typename Old_UnitInputType, typename Old_PriceType>
	SubjectType(SubjectType<Old_AreaInputType, Old_SubjectCountInputType, Old_UnitInputType, Old_PriceType> const &subject_type);

	/// @}
};










// Methods of SubjectType










template<typename AreaInputType, typename SubjectCountInputType, typename UnitInputType, typename PriceType>
	requires fas_numeric<AreaInputType> && fas_numeric<SubjectCountInputType> && fas_numeric<UnitInputType> && fas_numeric<PriceType>
SubjectType<AreaInputType, SubjectCountInputType, UnitInputType, PriceType>
	::SubjectType(UnitInputType in_capacity, UnitInputType out_capacity, AreaInputType area, SubjectCountInputType initially_available, PriceType price)
	: in_capacity(in_capacity)
	, out_capacity(out_capacity)
	, area(area)
	, initially_available(initially_available)
	, price(price)
{}





template<typename AreaInputType, typename SubjectCountInputType, typename UnitInputType, typename PriceType>
	requires fas_numeric<AreaInputType> && fas_numeric<SubjectCountInputType> && fas_numeric<UnitInputType> && fas_numeric<PriceType>
template<typename Old_AreaInputType, typename Old_SubjectCountInputType, typename Old_UnitInputType, typename Old_PriceType>
SubjectType<AreaInputType, SubjectCountInputType, UnitInputType, PriceType>
	::SubjectType(SubjectType<Old_AreaInputType, Old_SubjectCountInputType, Old_UnitInputType, Old_PriceType> const &subject_type)
	: in_capacity(subject_type.in_capacity)
	, out_capacity(subject_type.out_capacity)
	, area(subject_type.area)
	, initially_available(subject_type.initially_available)
	, price(subject_type.price)
{}





#endif // __FASOLVER_SUBJECT_TYPE_HPP__
