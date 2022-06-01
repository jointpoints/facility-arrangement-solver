/**
 * @file Facility.cpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#include "Facility.hpp"










// Methods of Facility::FacilityArrangementStrategy










Facility::FacilityArrangementStrategy
	::FacilityArrangementStrategy(Algorithm const algorithm)
	: sequence{algorithm}
{}





Facility::FacilityArrangementStrategy
	::FacilityArrangementStrategy(std::vector<Algorithm> const &sequence)
	: sequence(sequence)
{}










// Operators of Facility::FacilityArrangementStrategy










Facility::FacilityArrangementStrategy const Facility::FacilityArrangementStrategy
	::operator>>(FacilityArrangementStrategy const &other)
{
	std::vector<Algorithm> tmp(this->sequence);
	tmp.insert(tmp.end(), other.sequence.begin(), other.sequence.end());
	return FacilityArrangementStrategy(tmp);
}
