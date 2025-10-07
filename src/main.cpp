#include "phase1.h"
#include "phase2.h"
#include "phase3.h"
#include <iostream>

int main() {
    ProblemData data = initialize_problem();
    InitialSolution init_sol = construct_initial_solution(data);
    OptimalSolution optimal_sol = find_optimal_solution(data, init_sol);
    return 0;
}
