#include"BTSolver.hpp"
#include <queue>
using namespace std;

// =====================================================================
// Constructors
// =====================================================================

BTSolver::BTSolver ( SudokuBoard input, Trail* _trail,  string val_sh, string var_sh, string cc )
: sudokuGrid( input.get_p(), input.get_q(), input.get_board() ), network( input )
{
	valHeuristics = val_sh;
	varHeuristics = var_sh;
	cChecks =  cc;

	trail = _trail;
}

// =====================================================================
// Consistency Checks
// =====================================================================

// Basic consistency check, no propagation done
bool BTSolver::assignmentsCheck ( void )
{
	for ( Constraint c : network.getConstraints() )
		if ( ! c.isConsistent() )
			return false;

	return true;
}

/**
 * Part 1 TODO: Implement the Forward Checking Heuristic
 *
 * This function will do both Constraint Propagation and check
 * the consistency of the network
 *
 * (1) If a variable is assigned then eliminate that value from
 *     the square's neighbors.
 *
 * Note: remember to trail.push variables before you change their domain
 * Return: true is assignment is consistent, false otherwise
 */
bool BTSolver::forwardChecking ( void )
{
	vector<pair<Variable*, vector<int>>> vars;

	for (Variable * index : network.getVariables())
	{
		if(!index->isAssigned())
		{
			vector<int> temp;
			bool willBeMod = false;
			for (Variable * var : network.getNeighborsOfVariable(index))
			{
				if (var->isAssigned() && index->getDomain().contains(var->getAssignment()))
				{
						willBeMod = true;
						temp.push_back(var->getAssignment());
				}
			}
			if (willBeMod)
				vars.push_back(pair<Variable*, vector<int>>(index, temp));
		}
	}

	//Constraint Propagation && Consistency Check
	vector<pair<Variable*, vector<int>>> ::iterator it;
	for (it = vars.begin(); it != vars.end(); it++)
	{
		trail->push(it->first);
		vector<int>::iterator jt;
		for (jt = it->second.begin(); jt != it->second.end(); jt++)
			it->first->removeValueFromDomain(*jt);
		
		if (it->first->size() == 0)
			return false;
	}
	return true;
}

/**
 * Part 2 TODO: Implement both of Norvig's Heuristics
 *
 * This function will do both Constraint Propagation and check
 * the consistency of the network
 *
 * (1) If a variable is assigned then eliminate that value from
 *     the square's neighbors.
 *
 * (2) If a constraint has only one possible place for a value
 *     then put the value there.
 *
 * Note: remember to trail.push variables before you change their domain
 * Return: true is assignment is consistent, false otherwise
 */
bool BTSolver::norvigCheck ( void )
{
	if (forwardChecking()) //perform constraint propagation. 
	{
		vector<pair<Constraint, vector<int>>> temp;
		//initialize a checklist that contains a list of copied constraints and a vector of possible values.
		for (Constraint c : network.getConstraints())
		{
			vector<int> values;
			for (int i = 0; i < sudokuGrid.get_p() * sudokuGrid.get_q(); i++)
				values.push_back(i+1);
			temp.push_back(pair<Constraint, vector<int>>(c,values));
		}

		//eliminate the values of assigned variables in the list.
		vector<pair<Constraint, vector<int>>> ::iterator it;
		for (Variable * v : network.getVariables())
		{
			if (v->isAssigned())
			{
				int val = v->getAssignment();
				for (Constraint * c : network.getConstraintsContainingVariable(v))
				{
					for (it = temp.begin(); it != temp.end(); ++it)
					{
						if (*c == it->first)
							if(find(it->second.begin(), it->second.end(), val) != it->second.end())
								it->second.erase(find(it->second.begin(), it->second.end(), val));
					}
				}
			}
		}

		//check the list if a constraint has only one possible place for a value
		//then put the value there.
		for (Variable * v : network.getVariables())
		{
			if (!v->isAssigned())
			{
				for (Constraint * c : network.getConstraintsContainingVariable(v))
				{
					for (it = temp.begin(); it != temp.end(); it++)
					{
						if (it->first == *c && it->second.size() == 1)
						{
							trail->push(v);
							v->assignValue(it->second[0]);
							//check consistency after assignment.
							if (!forwardChecking())
								return false;
						}
					}
				}
			}
		}
		return true;
	}
	return false;
}
/**
My FC Check
**/
bool BTSolver::forwardCheck2point0(std::vector<Variable*> &s)
{
}
/**
 * Optional TODO: Implement your own advanced Constraint Propagation
 *
 * Completing the three tourn heuristic will automatically enter
 * your program into a tournament.
 */
bool BTSolver::getTournCC ( void )
{
	return norvigCheck();
}

// =====================================================================
// Variable Selectors
// =====================================================================

// Basic variable selector, returns first unassigned variable
Variable* BTSolver::getfirstUnassignedVariable ( void )
{
	for ( Variable* v : network.getVariables() )
		if ( !(v->isAssigned()) )
			return v;

	// Everything is assigned
	return nullptr;
}

/**
 * Part 1 TODO: Implement the Minimum Remaining Value Heuristic
 *
 * Return: The unassigned variable with the smallest domain
 */
Variable* BTSolver::getMRV ( void )
{
	Variable * v = nullptr;
	priority_queue<pair<int, Variable*>, vector<pair<int, Variable*>>, OrderByDomain> q;
	for (Variable * index : network.getVariables())
	{
		if (!index->isAssigned())
		{
			pair<int, Variable*> temp(index->size(), index);
			q.push(temp);
		}
	}
	if (q.size() != 0)
	{
		v = q.top().second;
	}
	return v;
}

/**
 * Part 2 TODO: Implement the Degree Heuristic
 *
 * Return: The unassigned variable with the most unassigned neighbors
 */
Variable* BTSolver::getDegree ( void )
{
	priority_queue<pair<int,Variable*>, vector<pair<int,Variable*>>,OrderByMostNeighbors> VariableWithMostNeighbors;
	for (Variable * index : network.getVariables())
	{
		if (!index->isAssigned())
		{
			int neighborhood = 0; //count unassign neighbors.
			for (Variable * neighbor : network.getNeighborsOfVariable(index))
			{
				if (!neighbor->isAssigned())
					++neighborhood;				
			}
			VariableWithMostNeighbors.push(pair<int, Variable*>(neighborhood,index));
		}
	}
	if (VariableWithMostNeighbors.size() != 0)
		return VariableWithMostNeighbors.top().second;
	return nullptr;
}

/**
 * Part 2 TODO: Implement the Minimum Remaining Value Heuristic
 *                with Degree Heuristic as a Tie Breaker
 *
 * Return: The unassigned variable with the smallest domain and involved
 *             in the most constraints
 */
Variable* BTSolver::MRVwithTieBreaker ( void )
{
	Variable * v = getMRV();
	if (v == getDegree())
		return v; //return the best variable if getMRV and getDegree function both return the same variable.
	else
	{
		priority_queue<pair<int,Variable*>, vector<pair<int,Variable*>>,OrderByDomain> MRVQueue;
		priority_queue<pair<int, Variable*>, vector<pair<int, Variable*>>, OrderByMostNeighbors> DEGQueue;

		//select variable with smallest Domain.
		for (Variable * index : network.getVariables())
		{
			if (!index->isAssigned()) 
				MRVQueue.push(pair<int, Variable*>(index->size(), index));
		}
		if (MRVQueue.size() == 0)
			return nullptr;
		else
		{
			//select variable with most neighbors in the list of smallest domain variables.
			int minDom = MRVQueue.top().first;
			while (MRVQueue.top().first <= minDom && !MRVQueue.empty())
			{
				int neighborhood = 0;
				for (Variable * neighbor : network.getNeighborsOfVariable(MRVQueue.top().second))
				{
					if (!neighbor->isAssigned())
						++neighborhood;
				}
				DEGQueue.push(pair<int, Variable*>(neighborhood, MRVQueue.top().second));
				MRVQueue.pop();
			}
			if (DEGQueue.size() == 0)
				return nullptr;
			return DEGQueue.top().second;
		}
	}
}

/**
 * Optional TODO: Implement your own advanced Variable Heuristic
 *
 * Completing the three tourn heuristic will automatically enter
 * your program into a tournament.
 */
Variable* BTSolver::getTournVar ( void )
{
	return MRVwithTieBreaker();
}

// =====================================================================
// Value Selectors
// =====================================================================

// Default Value Ordering
vector<int> BTSolver::getValuesInOrder ( Variable* v )
{
	vector<int> values = v->getDomain().getValues();
	sort( values.begin(), values.end() );
	return values;
}

/**
 * Part 1 TODO: Implement the Least Constraining Value Heuristic
 *
 * The Least constraining value is the one that will knock the least
 * values out of it's neighbors domain.
 *
 * Return: A list of v's domain sorted by the LCV heuristic
 *         The LCV is first and the MCV is last
 */
vector<int> BTSolver::getValuesLCVOrder ( Variable* v )
{
	priority_queue<pair<int, int>, vector<pair<int, int>>, OrderByConstraint> CQueue;
	vector<pair<int, int>> LCV;
	vector<int> Values = v->getValues();
	vector<int>::iterator it;

	for (it = Values.begin(); it != Values.end(); it++)
	{
		int constraint = 0;
		for (Variable * var : network.getNeighborsOfVariable(v))
		{
			if (!var->isAssigned() && var->getDomain().contains(*it))
			{
				++constraint;
			}
		}
		CQueue.push(pair<int, int>(constraint, *it));
		LCV.push_back(pair<int, int>(constraint, *it));
	}
	vector<int> returnValues;
	int minVal = CQueue.top().first;
	while (CQueue.top().first <= minVal && !CQueue.empty())
	{
		returnValues.push_back(CQueue.top().second);
		CQueue.pop();
	}
	return returnValues;
}

/**
 * Optional TODO: Implement your own advanced Value Heuristic
 *
 * Completing the three tourn heuristic will automatically enter
 * your program into a tournament.
 */
vector<int> BTSolver::getTournVal ( Variable* v )
{
	return getValuesLCVOrder(v);
}

// =====================================================================
// Engine Functions
// =====================================================================

void BTSolver::solve ( void )
{
	if ( hasSolution )
		return;

	// Variable Selection
	Variable* v = selectNextVariable();

	if ( v == nullptr )
	{
		for ( Variable* var : network.getVariables() )
		{
			// If all variables haven't been assigned
			if ( ! ( var->isAssigned() ) )
			{
				cout << "Error" << endl;
				return;
			}
		}

		// Success
		hasSolution = true;
		return;
	}

	// Attempt to assign a value
	for ( int i : getNextValues( v ) )
	{
		// Store place in trail and push variable's state on trail
		trail->placeTrailMarker();
		trail->push( v );

		// Assign the value
		v->assignValue( i );

		// Propagate constraints, check consistency, recurse
		if ( checkConsistency() )
			solve();

		// If this assignment succeeded, return
		if ( hasSolution )
			return;

		// Otherwise backtrack
		trail->undo();
	}
}

bool BTSolver::checkConsistency ( void )
{
	if ( cChecks == "forwardChecking" )
		return forwardChecking();

	if ( cChecks == "norvigCheck" )
		return norvigCheck();

	if ( cChecks == "tournCC" )
		return getTournCC();

	return assignmentsCheck();
}

Variable* BTSolver::selectNextVariable ( void )
{
	if ( varHeuristics == "MinimumRemainingValue" )
		return getMRV();

	if ( varHeuristics == "Degree" )
		return getDegree();

	if ( varHeuristics == "MRVwithTieBreaker" )
		return MRVwithTieBreaker();

	if ( varHeuristics == "tournVar" )
		return getTournVar();

	return getfirstUnassignedVariable();
}

vector<int> BTSolver::getNextValues ( Variable* v )
{
	if ( valHeuristics == "LeastConstrainingValue" )
		return getValuesLCVOrder( v );

	if ( valHeuristics == "tournVal" )
		return getTournVal( v );

	return getValuesInOrder( v );
}

bool BTSolver::haveSolution ( void )
{
	return hasSolution;
}

SudokuBoard BTSolver::getSolution ( void )
{
	return network.toSudokuBoard ( sudokuGrid.get_p(), sudokuGrid.get_q() );
}

ConstraintNetwork BTSolver::getNetwork ( void )
{
	return network;
}
