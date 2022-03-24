/**
 * @file PlanarMetric.cpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#include "PlanarMetric.hpp"





metric::PlanarMetric::PlanarMetric(void* distance_function)
	: dist(distance_function) {}





metric::PlanarMetric::~PlanarMetric(void) {}
