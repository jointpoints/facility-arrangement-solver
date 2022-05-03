/**
 * @file main.cpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
//#include "FASolver/FASolver.hpp"
#include "PlanarMetric/PlanarMetric.hpp"





int main(void)
{
	/*auto point_map = points::grid(3, 3, 1, 5U);
	PlanarMetric m1 = metric::Minkowski(1, point_map);
	PlanarMetric m2 = metric::Minkowski(2, point_map);
	PlanarMetric mi = metric::Minkowski(oo, point_map);

	FacilityLayout const facility_layout(point_map, m1);
	SubjectTypeMap<uint32_t, uint32_t> types{{"A", {0,  25, 100, 2, 4, 400.L}},
											 {"B", {60, 50,   0, 3, 2, 600.L}},
											 {"C", {10, 10,   0, 2, 3, 200.L}},
											 {"D", {100, 0,   0, 5, 1, 900.L}}};
	FlowMap<uint32_t> total_flows{{"A", { {"A", 0}, {"B", 100}, {"C", 0},  {"D", 0}  }},
	                              {"B", { {"A", 0}, {"B", 0},   {"C", 25}, {"D", 75} }},
	                              {"C", { {"A", 0}, {"B", 0},   {"C", 0},  {"D", 10} }},
	                              {"D", { {"A", 0}, {"B", 0},   {"C", 0},  {"D", 0}  }}};
	/*FASolver solver(facility_layout, types, total_flows);
	solver.optimise(1.0);*/

	/*auto point_map = points::grid(400, 1, 1, 2U);
	PlanarMetric m1 = metric::Minkowski(1, point_map);
	PlanarMetric m2 = metric::Minkowski(2, point_map);
	PlanarMetric mi = metric::Minkowski(oo, point_map);

	FacilityLayout const facility_layout(point_map, m1);
	SubjectTypeMap<uint32_t, uint32_t> types{{"A", {0,  250, 10000, 1, 200, 400.L}},
											 {"B", {600, 500,   0, 2, 100, 600.L}},
											 {"C", {100, 100,   0, 2, 100, 200.L}},
											 {"D", {1000, 0,   0, 1, 100, 900.L}}};
	FlowMap<uint32_t> total_flows{{"A", { {"A", 0}, {"B", 10000}, {"C", 0},  {"D", 0}  }},
	                              {"B", { {"A", 0}, {"B", 0},   {"C", 2500}, {"D", 7500} }},
	                              {"C", { {"A", 0}, {"B", 0},   {"C", 0},  {"D", 1000} }},
	                              {"D", { {"A", 0}, {"B", 0},   {"C", 0},  {"D", 0}  }}};
	FASolver solver(facility_layout, types, total_flows);
	solver.optimise(1.0);
	/*int a = 0;
	Logger logger("test.txt", a);
	fa::GMC(facility_layout, types, total_flows, logger, 0, 3, 20 * 4 * 2);*/
	
	return 0;
}
