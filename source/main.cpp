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
	std::set<std::string> const types{"A", "B", "C", "D"};
	UnitCapacityMap<uint64_t> const in_capacities{{"A", 0}, {"B", 60}, {"C", 10}, {"D", 100}};
	UnitCapacityMap<uint64_t> const out_capacities{{"A", 25}, {"B", 50}, {"C", 10}, {"D", 0}};
	FASolver solver(facility_layout, types, in_capacities, out_capacities);
	
	return 0;
}
