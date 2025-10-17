// phase3.cpp
#include "phase3.h"
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <cmath>
#include <iostream>
#include <deque>
#include <numeric>
#include <unordered_set>
#include <sstream>
#include <map>

using namespace std;

namespace {
    // RNG global for file
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

    // ---------- Utility helpers ----------
    static int get_required_periods(const ProblemData &data, const string &course_id, const string &section_id) {
        for (const auto &c : data.courses) {
            if (c.id == course_id) {
                for (const auto &s : c.sections)
                    if (s.id == section_id) return s.required_periods;
            }
        }
        return 1;
    }

    static string assignment_sig(const OptimalSolution::Assignment &a) {
        ostringstream oss;
        oss << a.teacher_id << "|" << a.course_id << "|" << a.section_id << "|" << a.day << "|" << a.period;
        return oss.str();
    }

    static string pair_sig(const OptimalSolution::Assignment &a, const OptimalSolution::Assignment &b) {
        string s1 = assignment_sig(a), s2 = assignment_sig(b);
        if (s1 < s2) return s1 + "<=>" + s2;
        return s2 + "<=>" + s1;
    }

    static int count_in_slot(const OptimalSolution &sol, const string &day, const string &period) {
        int cnt = 0;
        for (const auto &b : sol.assignments) if (b.day == day && b.period == period) ++cnt;
        return cnt;
    }

    static bool teacher_has_slot(const OptimalSolution &sol, const string &teacher_id, const string &day, const string &period) {
        for (const auto &a : sol.assignments) if (a.teacher_id == teacher_id && a.day == day && a.period == period) return true;
        return false;
    }

    // Find period index map
    static unordered_map<string,int> build_period_index(const ProblemData &data) {
        unordered_map<string,int> idx;
        for (int i = 0; i < (int)data.classrooms.periods.size(); ++i) idx[data.classrooms.periods[i]] = i;
        return idx;
    }

    // ---------- Feasibility ----------
    static bool IsFeasibleBlock(const string &teacher_id,
                                const string &course_id,
                                const string &section_id,
                                const string &day,
                                const string &start_period,
                                const OptimalSolution &sol,
                                const ProblemData &data)
    {
        // teacher exists and eligible
        auto it_teacher = find_if(data.teachers.begin(), data.teachers.end(),
                                  [&](const Teacher &t){ return t.id == teacher_id; });
        if (it_teacher == data.teachers.end()) return false;
        if (find(it_teacher->eligible_courses.begin(), it_teacher->eligible_courses.end(), course_id) == it_teacher->eligible_courses.end())
            return false;

        int r = get_required_periods(data, course_id, section_id);
        // find start idx
        int start_idx = -1;
        for (int m = 0; m < (int)data.classrooms.periods.size(); ++m)
            if (data.classrooms.periods[m] == start_period) { start_idx = m; break; }
        if (start_idx < 0 || start_idx + r - 1 >= (int)data.classrooms.periods.size()) return false;

        for (int t = 0; t < r; ++t) {
            string period = data.classrooms.periods[start_idx + t];
            if (teacher_has_slot(sol, teacher_id, day, period)) return false;
            int cap = data.classrooms.Clm.at(day).at(period);
            int cnt = count_in_slot(sol, day, period);
            if (cnt >= cap) return false;
        }
        return true;
    }

    static bool IsFeasible(const string &teacher_id,
                           const string &course_id,
                           const string &section_id,
                           const string &day,
                           const string &period,
                           const OptimalSolution &sol,
                           const ProblemData &data)
    {
        // For simplicity, treat as block-check (works for r=1 as well)
        return IsFeasibleBlock(teacher_id, course_id, section_id, day, period, sol, data);
    }

    // ---------- Objective Evaluation ----------
    static double compute_stddev(const vector<int> &vals) {
        if (vals.empty()) return 0.0;
        double mean = accumulate(vals.begin(), vals.end(), 0.0) / vals.size();
        double s = 0.0;
        for (double v : vals) s += (v - mean) * (v - mean);
        return sqrt(s / vals.size());
    }

    static int Evaluate(const OptimalSolution &sol, const ProblemData &data, int history_count = 0) {
        int score = 0;
        for (const auto &a : sol.assignments) {
            auto it_t = find_if(data.teachers.begin(), data.teachers.end(),
                                [&](const Teacher &t){ return t.id == a.teacher_id; });
            if (it_t != data.teachers.end()) {
                auto pc_it = it_t->course_pref.find(a.course_id);
                if (pc_it != it_t->course_pref.end()) score += pc_it->second;
                for (const auto &tp : it_t->time_pref)
                    if (tp.day == a.day && tp.period == a.period) score += tp.score;
            }
        }
        if (history_count > 3) score -= (history_count - 3);
        return score;
    }

    // ---------- Move operators (free functions) ----------
    // Each returns pair<changed, signature-string>

    static pair<bool,string> move_single_change(OptimalSolution &sol, const ProblemData &data) {
        if (sol.assignments.empty()) return {false, ""};
        uniform_int_distribution<int> dist(0, (int)sol.assignments.size() - 1);
        int idx = dist(rng);
        auto &a = sol.assignments[idx];

        // find course object
        auto it_course = find_if(data.courses.begin(), data.courses.end(),
                                 [&](const Course &c){ return c.id == a.course_id; });
        if (it_course == data.courses.end() || it_course->Ij.empty()) return {false, ""};
        const auto &course = *it_course;

        // try teacher change
        uniform_int_distribution<int> tdist(0, (int)course.Ij.size() - 1);
        string new_teacher = course.Ij[tdist(rng)];
        if (new_teacher != a.teacher_id && IsFeasible(new_teacher, a.course_id, a.section_id, a.day, a.period, sol, data)) {
            OptimalSolution::Assignment old = a;
            a.teacher_id = new_teacher;
            return {true, pair_sig(old, a)};
        }

        // try relocate (few attempts)
        uniform_int_distribution<int> ldist(0, (int)data.classrooms.days.size() - 1);
        uniform_int_distribution<int> pdist(0, (int)data.classrooms.periods.size() - 1);
        for (int t = 0; t < 6; ++t) {
            string new_day = data.classrooms.days[ldist(rng)];
            string new_period = data.classrooms.periods[pdist(rng)];
            if (IsFeasible(a.teacher_id, a.course_id, a.section_id, new_day, new_period, sol, data)) {
                OptimalSolution::Assignment old = a;
                a.day = new_day; a.period = new_period;
                return {true, pair_sig(old, a)};
            }
        }
        return {false, ""};
    }

    static pair<bool,string> move_teacher_swap(OptimalSolution &sol, const ProblemData &data) {
        if (sol.assignments.size() < 2) return {false, ""};
        uniform_int_distribution<int> d(0, (int)sol.assignments.size() - 1);
        int i = d(rng), j = d(rng);
        if (i == j) return {false, ""};
        auto &A = sol.assignments[i];
        auto &B = sol.assignments[j];
        if (A.teacher_id == B.teacher_id) return {false, ""};

        if (IsFeasible(B.teacher_id, A.course_id, A.section_id, A.day, A.period, sol, data) &&
            IsFeasible(A.teacher_id, B.course_id, B.section_id, B.day, B.period, sol, data)) {
            OptimalSolution::Assignment oldA = A, oldB = B;
            swap(A.teacher_id, B.teacher_id);
            return {true, pair_sig(oldA, oldB)};
        }
        return {false, ""};
    }

    static pair<bool,string> move_pair_swap(OptimalSolution &sol, const ProblemData &data) {
        if (sol.assignments.size() < 2) return {false, ""};
        uniform_int_distribution<int> d(0, (int)sol.assignments.size() - 1);
        int i = d(rng), j = d(rng);
        if (i == j) return {false, ""};
        auto a = sol.assignments[i];
        auto b = sol.assignments[j];

        // attempt swap entire (teacher+slot)
        if (IsFeasible(b.teacher_id, a.course_id, a.section_id, b.day, b.period, sol, data) &&
            IsFeasible(a.teacher_id, b.course_id, b.section_id, a.day, a.period, sol, data)) {
            swap(sol.assignments[i].teacher_id, sol.assignments[j].teacher_id);
            swap(sol.assignments[i].day, sol.assignments[j].day);
            swap(sol.assignments[i].period, sol.assignments[j].period);
            return {true, pair_sig(a, b)};
        }
        return {false, ""};
    }

    static pair<bool,string> move_block_relocate(OptimalSolution &sol, const ProblemData &data) {
        if (sol.assignments.empty()) return {false, ""};
        uniform_int_distribution<int> d(0, (int)sol.assignments.size() - 1);
        int idx = d(rng);
        auto &a = sol.assignments[idx];
        int attempts = 6;
        uniform_int_distribution<int> ldist(0, (int)data.classrooms.days.size() - 1);
        uniform_int_distribution<int> pdist(0, (int)data.classrooms.periods.size() - 1);
        for (int t = 0; t < attempts; ++t) {
            string nd = data.classrooms.days[ldist(rng)];
            string np = data.classrooms.periods[pdist(rng)];
            if (nd == a.day && np == a.period) continue;
            if (IsFeasibleBlock(a.teacher_id, a.course_id, a.section_id, nd, np, sol, data)) {
                OptimalSolution::Assignment old = a;
                a.day = nd; a.period = np;
                return {true, pair_sig(old, a)};
            }
        }
        return {false, ""};
    }

    static pair<bool,string> move_block_swap(OptimalSolution &sol, const ProblemData &data) {
        if (sol.assignments.size() < 2) return {false, ""};
        uniform_int_distribution<int> d(0, (int)sol.assignments.size() - 1);
        int i = d(rng), j = d(rng);
        if (i == j) return {false, ""};
        auto A = sol.assignments[i];
        auto B = sol.assignments[j];

        if (IsFeasibleBlock(A.teacher_id, A.course_id, A.section_id, B.day, B.period, sol, data) &&
            IsFeasibleBlock(B.teacher_id, B.course_id, B.section_id, A.day, A.period, sol, data)) {
            swap(sol.assignments[i].day, sol.assignments[j].day);
            swap(sol.assignments[i].period, sol.assignments[j].period);
            swap(sol.assignments[i].teacher_id, sol.assignments[j].teacher_id);
            return {true, pair_sig(A, B)};
        }
        return {false, ""};
    }
} // anonymous namespace

// ---------- Main Phase3 ----------
OptimalSolution find_optimal_solution(const ProblemData &data, const InitialSolution &initial) {
    OptimalSolution current = initial;
    OptimalSolution temp = initial;
    OptimalSolution best = initial;

    current.objective_value = Evaluate(current, data);
    temp.objective_value = current.objective_value;
    best.objective_value = current.objective_value;

    // Tabu + history
    deque<string> tabu_q;
    unordered_set<string> tabu_set;
    size_t tabu_tenure = 150; // tuneable
    unordered_map<string,int> history_count;

    // Neighborhoods ordered by increasing strength
    using MoveFn = pair<bool,string>(*)(OptimalSolution&, const ProblemData&);
    vector<MoveFn> neighborhoods = {
        &move_single_change,
        &move_teacher_swap,
        &move_pair_swap,
        &move_block_relocate,
        &move_block_swap
    };

    // SA/VNS params
    double T = 1.0;
    double alpha = 0.98;
    int max_iterations = 1200;
    int moves_per_nb = 40;
    int limit_no_improv = 300;

    vector<int> recent_objs; recent_objs.reserve(200);
    int numb_iter_no_improv = 0;

    for (int iter = 0; iter < max_iterations; ++iter) {
        bool any_improved = false;

        for (size_t nb = 0; nb < neighborhoods.size(); ++nb) {
            bool improved_in_nb = false;

            for (int mv = 0; mv < moves_per_nb; ++mv) {
                OptimalSolution cand = current;
                auto res = neighborhoods[nb](cand, data);
                if (!res.first) continue;
                string sig = res.second;

                // check tabu
                if (!sig.empty() && tabu_set.find(sig) != tabu_set.end()) {
                    int cand_score = Evaluate(cand, data, history_count[sig]);
                    if (cand_score <= best.objective_value) continue; // reject unless aspiration
                }

                int cand_score = Evaluate(cand, data, history_count[sig]);
                int delta = cand_score - temp.objective_value;

                bool accept = false;
                if (delta >= 0) accept = true;
                else {
                    recent_objs.push_back(temp.objective_value);
                    if (recent_objs.size() > 100) recent_objs.erase(recent_objs.begin());
                    double sigma = compute_stddev(recent_objs);
                    double adaptive_T = 0.25 * T + 0.75 * (0.01 + sigma);
                    uniform_real_distribution<double> u(0.0, 1.0);
                    double prob = exp(delta / adaptive_T);
                    if (u(rng) < prob) accept = true;
                }

                if (accept) {
                    current = cand;
                    temp = cand;
                    temp.objective_value = cand_score;

                    if (!sig.empty()) {
                        history_count[sig] += 1;
                        tabu_q.push_back(sig);
                        tabu_set.insert(sig);
                        if (tabu_q.size() > tabu_tenure) {
                            string old = tabu_q.front(); tabu_q.pop_front();
                            tabu_set.erase(old);
                        }
                    }

                    if (cand_score > best.objective_value) {
                        best = cand;
                        best.objective_value = cand_score;
                        numb_iter_no_improv = 0;
                    } else ++numb_iter_no_improv;

                    improved_in_nb = true;
                    any_improved = true;
                    // intensify: restart from smallest neighborhood
                    nb = 0;
                    break;
                }
            } // moves per neighborhood

            if (!improved_in_nb) {
                // go to next neighborhood (diversify)
                continue;
            }
        } // neighborhoods

        if (!any_improved) {
            // diversification: random shakes
            int shakes = 4;
            for (int s = 0; s < shakes; ++s) {
                OptimalSolution cand = current;
                pair<bool,string> res = move_block_relocate(cand, data);
                if (!res.first) res = move_teacher_swap(cand, data);
                if (!res.first) res = move_pair_swap(cand, data);
                if (!res.first) continue;
                string sig = res.second;
                int cand_score = Evaluate(cand, data, history_count[sig]);

                uniform_real_distribution<double> u(0.0, 1.0);
                recent_objs.push_back(temp.objective_value);
                if (recent_objs.size() > 100) recent_objs.erase(recent_objs.begin());
                double sigma = compute_stddev(recent_objs);
                double adaptive_T = 0.25 * T + 0.75 * (0.01 + sigma);
                double prob = exp((cand_score - temp.objective_value) / adaptive_T);
                if (u(rng) < prob) {
                    current = cand; temp = cand; temp.objective_value = cand_score;
                    if (!sig.empty()) {
                        history_count[sig] += 1;
                        tabu_q.push_back(sig); tabu_set.insert(sig);
                        if (tabu_q.size() > tabu_tenure) { string old = tabu_q.front(); tabu_q.pop_front(); tabu_set.erase(old); }
                    }
                    if (cand_score > best.objective_value) { best = cand; best.objective_value = cand_score; numb_iter_no_improv = 0; }
                    else ++numb_iter_no_improv;
                    break;
                }
            }
        }

        // cooling & adapt alpha
        T *= alpha;
        if (any_improved) alpha = min(0.995, alpha + 0.0006);
        else alpha = max(0.92, alpha - 0.0009);

        // restart if stuck
        if (numb_iter_no_improv > limit_no_improv) {
            current = best; temp = best; numb_iter_no_improv = 0;
            shuffle(current.assignments.begin(), current.assignments.end(), rng);
            // small shake
            for (int k = 0; k < (int)current.assignments.size() / 12; ++k) move_single_change(current, data);
        }

        if (iter % 50 == 0) {
            cout << "[Phase3] Iter " << iter
                 << ", Best " << best.objective_value
                 << ", Temp " << T
                 << ", Tabu " << tabu_set.size()
                 << ", HistoryEntries " << history_count.size()
                 << "\n";
        }
    } // main loop

    cout << "[Phase3] Finished. Best objective: " << best.objective_value
         << ", Total assignments: " << best.assignments.size() << "\n";
    return best;
}
