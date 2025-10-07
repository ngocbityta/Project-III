#include "phase3.h"
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <cmath>
#include <iostream>

using namespace std;

namespace
{
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

    int Evaluate(const OptimalSolution &sol, const ProblemData &data)
    {
        int score = 0;
        for (const auto &a : sol.assignments)
        {
            auto it_t = find_if(data.teachers.begin(), data.teachers.end(),
                                [&](const Teacher &t)
                                { return t.id == a.teacher_id; });
            if (it_t != data.teachers.end())
            {
                auto pc_it = it_t->course_pref.find(a.course_id);
                if (pc_it != it_t->course_pref.end())
                    score += pc_it->second;

                for (const auto &tp : it_t->time_pref)
                    if (tp.day == a.day && tp.period == a.period)
                        score += tp.score;
            }
        }
        return score;
    }

    bool IsFeasible(const string &teacher_id,
                    const string &course_id,
                    const string &section_id,
                    const string &day,
                    const string &period,
                    const OptimalSolution &sol,
                    const ProblemData &data)
    {
        auto it_teacher = find_if(data.teachers.begin(), data.teachers.end(),
                                  [&](const Teacher &t)
                                  { return t.id == teacher_id; });
        if (it_teacher == data.teachers.end())
            return false;

        const auto &teacher = *it_teacher;
        if (find(teacher.eligible_courses.begin(), teacher.eligible_courses.end(),
                 course_id) == teacher.eligible_courses.end())
            return false;

        for (const auto &a : sol.assignments)
        {
            if (a.teacher_id == teacher_id && a.day == day && a.period == period)
                return false;

            int count_in_slot = 0;
            for (const auto &b : sol.assignments)
                if (b.day == day && b.period == period)
                    ++count_in_slot;

            int cap = data.classrooms.Clm.at(day).at(period);
            if (count_in_slot >= cap)
                return false;
        }
        return true;
    }

    bool NeighborhoodImprovement(OptimalSolution &current, const ProblemData &data)
    {
        if (current.assignments.empty())
            return false;

        uniform_int_distribution<int> dist(0, (int)current.assignments.size() - 1);
        int idx = dist(rng);
        auto &a = current.assignments[idx];

        auto it_course = find_if(data.courses.begin(), data.courses.end(),
                                 [&](const Course &c)
                                 { return c.id == a.course_id; });
        if (it_course == data.courses.end() || it_course->Ij.empty())
            return false;

        const auto &course = *it_course;
        uniform_int_distribution<int> tdist(0, (int)course.Ij.size() - 1);
        string new_teacher = course.Ij[tdist(rng)];

        if (new_teacher != a.teacher_id &&
            IsFeasible(new_teacher, a.course_id, a.section_id, a.day, a.period, current, data))
        {
            a.teacher_id = new_teacher;
            return true;
        }

        uniform_int_distribution<int> ldist(0, (int)data.classrooms.days.size() - 1);
        uniform_int_distribution<int> pdist(0, (int)data.classrooms.periods.size() - 1);
        for (int trial = 0; trial < 5; ++trial)
        {
            string new_day = data.classrooms.days[ldist(rng)];
            string new_period = data.classrooms.periods[pdist(rng)];
            if (IsFeasible(a.teacher_id, a.course_id, a.section_id, new_day, new_period, current, data))
            {
                a.day = new_day;
                a.period = new_period;
                return true;
            }
        }

        return false;
    }
}

OptimalSolution find_optimal_solution(const ProblemData &data, const InitialSolution &initial)
{
    OptimalSolution current = initial;
    OptimalSolution temp = initial;
    OptimalSolution best = initial;

    // Khởi tạo objective_value
    current.objective_value = Evaluate(current, data);
    temp.objective_value = current.objective_value;
    best.objective_value = current.objective_value;

    double T = 1.0;
    int numb_iter_no_improv = 0;
    int max_iterations = 1000;
    int neighbor_moves = 50;
    double alpha = 0.95;
    int limit_no_improv = 100;

    for (int iter = 0; iter < max_iterations; ++iter)
    {
        for (int move = 0; move < neighbor_moves; ++move)
        {
            bool changed = NeighborhoodImprovement(current, data);
            if (changed)
            {
                int new_score = Evaluate(current, data);
                int delta = new_score - temp.objective_value;

                if (delta >= 0)
                {
                    temp = current;
                    temp.objective_value = new_score;

                    if (new_score > best.objective_value)
                    {
                        best = current;
                        best.objective_value = new_score;
                    }
                    numb_iter_no_improv = 0;
                }
                else
                {
                    uniform_real_distribution<double> uni(0.0, 1.0);
                    double prob = exp(delta / T);
                    if (uni(rng) < prob)
                    {
                        temp = current;
                        temp.objective_value = new_score;
                    }
                    else
                    {
                        current = temp;
                    }
                    ++numb_iter_no_improv;
                }
            }
        }

        T *= alpha;

        if (numb_iter_no_improv > limit_no_improv)
        {
            current = best;
            temp = best;
            numb_iter_no_improv = 0;
        }

        if (iter % 50 == 0)
        {
            cout << "[Phase3] Iteration " << iter
                 << ", Current obj: " << temp.objective_value
                 << ", Best obj: " << best.objective_value
                 << ", Temp: " << T << "\n";
        }
    }

    cout << "[Phase3] Finished. Best objective: " << best.objective_value
         << ", Total assignments: " << best.assignments.size() << "\n";

    return best;
}
