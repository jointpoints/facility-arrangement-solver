/**
 * @file FAHeuristic.hpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FAHEURISTIC_HPP__
#define __FASOLVER_FAHEURISTIC_HPP__





#include "../Facility/Facility.hpp"
#include "../SubjectType/SubjectType.hpp"





/**
 * @namespace fa::util
 * @brief Utilities used for facility arrangement generation
 *
 * These functions are not expected to be called from outside of fa namespace.
 */
namespace fa::util
{





/**
 * @class ThreadReturn
 * @brief Structure returned by each generating thread
 */
template<typename CoordinateType, typename AreaType, typename UnitType>
	requires numeric<CoordinateType> && numeric<AreaType> && numeric<UnitType>
struct ThreadReturn
{
	FacilityArrangement<CoordinateType, AreaType, UnitType> facility_arrangement;
	long double objective_value;
};





} // fa::util





#endif // __FASOLVER_FAHEURISTIC_HPP__
