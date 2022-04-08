/**
 * @file SubjectType.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_SUBJECT_TYPE_HPP__
#define __FASOLVER_SUBJECT_TYPE_HPP__





#include <map>
#include <cstdint> // fixed-width integers





template<typename AreaType, typename UnitType>
struct SubjectType final
{
	UnitType    in_capacity;
	UnitType    out_capacity;
	UnitType    total_generated_units;
	AreaType    area;
	uint64_t    initially_available;
	long double price;
};





template<typename AreaType, typename UnitType>
using SubjectTypeMap = std::map<std::string, SubjectType<AreaType, UnitType>>;





#endif // __FASOLVER_SUBJECT_TYPE_HPP__
