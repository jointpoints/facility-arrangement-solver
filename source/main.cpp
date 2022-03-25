/**
 * @file main.cpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#include "FASolver/FASolver.hpp"
#include "PlanarMetric/PlanarMetric.hpp"
#include <iostream>





int main(void)
{
	PointMap<int> points{{"(0,0)", Point(0, 0, 5)}, {"(0,1)", Point(0, 1, 5)}, {"(0,2)", Point(0, 2, 5)},
	{"(1,0)", Point(1, 0, 5)}, {"(1,1)", Point(1, 1, 5)}, {"(1,2)", Point(1, 2, 5)},
	{"(2,0)", Point(2, 0, 5)}, {"(2,1)", Point(2, 1, 5)}, {"(2,2)", Point(2, 2, 5)}};
	metric::PlanarMetric m1 = metric::Minkowski(1, points);
	metric::PlanarMetric m2 = metric::Minkowski(2, points);
	metric::PlanarMetric mi = metric::Minkowski(oo, points);
	Point p(2, 3, 5);
	//std::cout << m1(points["1"], p) << '\n' << m2(points["1"], p) << '\n' << mi(points["1"], p) << '\n';

	FacilityLayout const facility_layout(points, m1);
	SubjectTypeMap<uint64_t, uint64_t> types{{"A", {0,  25, 100, 2, 4, 400.L}},
											 {"B", {60, 50,   0, 3, 2, 600.L}},
											 {"C", {10, 10,   0, 2, 3, 200.L}},
											 {"D", {100, 0,   0, 5, 1, 900.L}}};
	FlowMap<uint64_t> total_flows{{"A", { {"A", 0}, {"B", 100}, {"C", 0},  {"D", 0}  }},
	                              {"B", { {"A", 0}, {"B", 0},   {"C", 25}, {"D", 75} }},
	                              {"C", { {"A", 0}, {"B", 0},   {"C", 0},  {"D", 10} }},
	                              {"D", { {"A", 0}, {"B", 0},   {"C", 0},  {"D", 0}  }}};
	FASolver solver(facility_layout, types, total_flows);
	solver.optimise(1.0);
	
	return 0;
}
