/**
 * @file Facility.cpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#include "Facility.hpp"










// Methods of Facility::_FacilityArrangementStrategy










Facility::_FacilityArrangementStrategy
	::_FacilityArrangementStrategy(_Algorithm const algorithm)
	: _sequence{algorithm}
{}





Facility::_FacilityArrangementStrategy
	::_FacilityArrangementStrategy(std::vector<_Algorithm> const &sequence)
	: _sequence(sequence)
{}










// Operators of Facility::_FacilityArrangementStrategy










Facility::_FacilityArrangementStrategy const Facility::_FacilityArrangementStrategy
	::operator>>(_FacilityArrangementStrategy const &other)
{
	std::vector<_Algorithm> tmp(this->_sequence);
	tmp.insert(tmp.end(), other._sequence.begin(), other._sequence.end());
	return _FacilityArrangementStrategy(tmp);
}










// Methods of Facility memory management nested classes










Facility::_PointsDeleter::_PointsDeleter(Facility *const owner)
	: owner(owner)
{}

void Facility::_PointsDeleter::operator()(void *pointer) const
{
	RUNTIME_CONDITIONAL(CoordinateType, this->owner->_coordinate_type_integral, FASInteger, FASFloat,
	RUNTIME_CONDITIONAL(AreaInputType, this->owner->_area_input_type_integral, FASInteger, FASFloat,
	RUNTIME_CONDITIONAL(SubjectCountOutputType, this->owner->_subject_count_output_type_integral, FASInteger, FASFloat,
		delete static_cast<UnaryMap<Point<CoordinateType, AreaInputType, SubjectCountOutputType>> *>(pointer);
	)))

	return;
}

Facility::_DistanceDeleter::_DistanceDeleter(Facility *const owner)
	: owner(owner)
{}

void Facility::_DistanceDeleter::operator()(void *pointer) const
{
	RUNTIME_CONDITIONAL(DistanceType, this->owner->_distance_type_integral, FASInteger, FASFloat,
		delete static_cast<PlanarMetric<DistanceType> *>(pointer);
	)

	return;
}

Facility::_FlowsDeleter::_FlowsDeleter(Facility *const owner)
	: owner(owner)
{}

void Facility::_FlowsDeleter::operator()(void *pointer) const
{
	RUNTIME_CONDITIONAL(UnitOutputType, this->owner->_unit_output_type_integral, FASInteger, FASFloat,
		delete static_cast<BinaryPairMap<UnitOutputType> *>(pointer);
	)

	return;
}
