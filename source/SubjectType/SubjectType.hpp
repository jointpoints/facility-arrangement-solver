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
};





#endif // __FASOLVER_SUBJECT_TYPE_HPP__
