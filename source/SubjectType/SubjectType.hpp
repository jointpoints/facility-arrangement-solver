/**
 * @file SubjectType.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_SUBJECT_TYPE_HPP__
#define __FASOLVER_SUBJECT_TYPE_HPP__





#include <map>
#include <cstdint> // fixed-width integers





template<typename UnitType, typename AreaType>
struct SubjectType final
{
	UnitType    in_capacity;
	UnitType    out_capacity;
	UnitType    total_generated_units;
	AreaType    area;
	uint64_t    initially_available;
	long double price;
};





template<typename UnitType, typename AreaType>
using SubjectTypeMap = std::map<std::string, SubjectType<UnitType, AreaType>>;





#endif // __FASOLVER_SUBJECT_TYPE_HPP__
