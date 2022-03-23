/**
 * @file main.cpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#include "FASolver/FASolver.hpp"
#include "PlanarMetric/PlanarMetric.hpp"
#include <iostream>





int main(void)
{
	PointMap<int> points{{"1", Point(1, 2, 5)}, {"2", Point(1, 3, 5)}};
	metric::PlanarMetric m1 = metric::Minkowski(1, points);
	metric::PlanarMetric m2 = metric::Minkowski(2, points);
	metric::PlanarMetric mi = metric::Minkowski(oo, points);
	Point p(2, 3, 5);
	std::cout << m1(points["1"], p) << '\n' << m2(points["1"], p) << '\n' << mi(points["1"], p) << '\n';

	FacilityLayout<int> const facility_layout{points, m1};
	SubjectTypeMap<uint64_t, uint64_t> types{{"A", {0,  25, 3, 4, 400.L}},
											 {"B", {60, 50, 4, 2, 600.L}},
											 {"C", {10, 10, 2, 3, 200.L}},
											 {"D", {100, 0, 5, 1, 900.L}}};
	FlowMap<uint64_t> total_flows{{"A", { {"A", 0}, {"B", 100}, {"C", 0},  {"D", 0}  }},
	                              {"B", { {"A", 0}, {"B", 0},   {"C", 25}, {"D", 75} }},
	                              {"C", { {"A", 0}, {"B", 0},   {"C", 0},  {"D", 10} }},
	                              {"D", { {"A", 0}, {"B", 0},   {"C", 0},  {"D", 0}  }}};
	FASolver solver(facility_layout, types, total_flows);
	
	return 0;
}
