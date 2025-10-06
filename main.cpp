#include "phase1.h"
#include "phase2.h"
#include "phase3.h"
#include <iostream>

int main() {
    ProblemData data = initialize_problem();
    Solution sol = solve_with_ip(data);
    Solution best = optimize_with_sa(sol, data);
    return 0;
}
