#pragma once
#include "phase1.h"
#include "phase2.h"
#include <vector>
#include <string>
using namespace std;

// Cấu trúc lưu kết quả tối ưu sau phase 3
struct OptimalSolution {
    struct Assignment {
        string teacher_id;
        string course_id;
        string section_id;
        string day;
        string period;
    };

    vector<Assignment> assignments;
    int objective_value = 0;
    OptimalSolution() = default;
    OptimalSolution(const InitialSolution &init) {
        assignments.clear();
        for (const auto &a : init.assignments) {
            assignments.push_back({a.teacher_id, a.course_id, a.section_id, a.day, a.period});
        }
    }
};

// Hàm tìm phương án tối ưu sử dụng Simulated Annealing + Neighborhood Improvement
OptimalSolution find_optimal_solution(const ProblemData& data, const InitialSolution& init_sol);
