#include "phase1.h"
#include "phase2.h"
#include "phase3.h"
#include <iostream>

int main() {
    ProblemData data = initialize_problem();
    InitialSolution init_sol = construct_initial_solution(data);
    print_initial_solution(init_sol);
    return 0;
}
