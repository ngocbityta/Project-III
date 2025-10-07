#pragma once
#include "phase1.h"

struct InitialSolution {
    struct Assignment {
        std::string teacher_id;
        std::string course_id;
        std::string section_id;
        std::string day;
        std::string period;
    };

    std::vector<Assignment> assignments;
};

// Hàm xây dựng phương án khởi đầu bằng Integer Programming
InitialSolution construct_initial_solution(const ProblemData &data);
