/**
 * @file main.cpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#include "PlanarMetric/PlanarMetric.hpp"
#include <ilcplex/ilocplex.h>
#include <iostream>





int main(void)
{
	IloEnv cplex_environment;

	metric::Minkowski<1, int> d;
	Point<int> p1(1, 2, 5);
	Point<int> p2(1, 3, 5);
	std::cout << d(p1, p2) << '\n' << typeid(d(p1, p2)).name() << '\n';

	cplex_environment.end();

	return 0;
}
