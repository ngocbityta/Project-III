#pragma once
#include "phase1.h"

struct Solution {
    std::vector<int> assignment; // assignment[j] = teacher của lớp j
    double cost;
};

Solution solve_with_ip(const ProblemData& data);
