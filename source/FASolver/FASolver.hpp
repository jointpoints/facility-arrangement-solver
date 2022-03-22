/**
 * @file FASolver.hpp
 * @author JointPoints, 2022, github.com/jointpoints
 */
#include <cstring> // to make CPLEX compile with G++
#include <ilcplex/ilocplex.h>




/**
 * @class FASolver
 * @brief Facility Arrangement Problem solver
 *
 * Runs the optimisation process for the given Facility Arrangement Problem instance
 * with the help of CPLEX.
 */
class FASolver final
{
	/// CPLEX environemnt responsible for memory allocation of all Concert objects.
	IloEnv cplex_environment;



public:
	/// @name Constructors and destructors
	/// @{
	
	/**
	 * @brief Constructor from data
	 *
	 * Constructs a new FASolver object relying on the provided data about the instance
	 * of the problem to solve.
	 */
	FASolver(void);

	/**
	 * @brief Destructor
	 *
	 * Destroys the solver and the associated CPLEX environemnt.
	 */
	~FASolver(void);

	/// @}
};
