/**
 * @file SubjectType.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_SUBJECT_TYPE_HPP__
#define __FASOLVER_SUBJECT_TYPE_HPP__





#include <map>
#include <cstdint> // fixed-width integers





/**
 * @class SubjectType
 * @brief A type of subjects
 *
 * Subjects of the same type share the same features and a common position in the
 * production chain or, in other words, are indistinguishable from one another from the
 * functional point of view.
 */
template<typename AreaType, typename UnitType>
struct SubjectType final
{
	/// The input capacity of a single subject of the type.
	UnitType    in_capacity;
	/// The output capacity of a single subject of the type.
	UnitType    out_capacity;
	/// The number of objects all subjects of the type \b must produce.
	UnitType    production_target;
	/// The area occupied by a single subject of the type.
	AreaType    area;
	/// The number of subjects initially available for the placement.
	uint64_t    initially_available;
	/// The price for each additional subject of the type.
	long double price;
};





template<typename AreaType, typename UnitType>
using SubjectTypeMap = std::map<std::string, SubjectType<AreaType, UnitType>>;





#endif // __FASOLVER_SUBJECT_TYPE_HPP__
