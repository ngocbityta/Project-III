#include "phase1.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <algorithm>

using json = nlohmann::json;

// Helper: đọc JSON file
template <typename T>
T read_json(const std::string& filename) {
    std::ifstream f(filename);
    if (!f.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    json j;
    f >> j;
    return j.get<T>();
}

// Chuyển đổi json sang struct
ProblemData initialize_problem() {
    ProblemData data;

    // Đọc dữ liệu
    std::ifstream f1("teachers.json"), f2("courses.json"), f3("classrooms.json");
    json j_teachers, j_courses, j_classrooms;
    f1 >> j_teachers;
    f2 >> j_courses;
    f3 >> j_classrooms;

    // Parse teachers
    for (auto& t : j_teachers) {
        Teacher teacher;
        teacher.id = t["id"];
        teacher.name = t["name"];
        teacher.max_courses = t["max_courses"];
        teacher.eligible_courses = t["eligible_courses"].get<std::vector<std::string>>();
        teacher.course_pref = t["course_preferences"].get<std::map<std::string, int>>();

        // flatten PT_ilm
        for (auto& [day, periods] : t["day_time_preferences"].items()) {
            for (auto& [period, score] : periods.items()) {
                teacher.time_pref.push_back({day, period, score});
            }
        }

        // sort by preference
        std::sort(teacher.time_pref.begin(), teacher.time_pref.end(),
                  [](const TimePref& a, const TimePref& b) {
                      return a.score > b.score;
                  });
        teacher.LMi = teacher.time_pref;
        data.teachers.push_back(teacher);
    }

    // Parse courses
    for (auto& c : j_courses) {
        Course course;
        course.id = c["id"];
        course.name = c["name"];
        course.min_teachers = c["min_teachers"];
        course.max_teachers = c["max_teachers"];

        for (auto& s : c["sections"]) {
            course.sections.push_back({s["id"], s["required_periods"]});
        }

        // Construct Ij
        std::vector<std::pair<std::string, int>> teacher_scores;
        for (auto& t : data.teachers) {
            if (std::find(t.eligible_courses.begin(), t.eligible_courses.end(), course.id) != t.eligible_courses.end()) {
                int score = t.course_pref.count(course.id) ? t.course_pref.at(course.id) : 0;
                teacher_scores.push_back({t.id, score});
            }
        }
        std::sort(teacher_scores.begin(), teacher_scores.end(),
                  [](auto& a, auto& b) { return a.second > b.second; });

        for (auto& [tid, _] : teacher_scores)
            course.Ij.push_back(tid);

        data.courses.push_back(course);
    }

    // Parse classrooms
    data.classrooms.days = j_classrooms["days"].get<std::vector<std::string>>();
    data.classrooms.periods = j_classrooms["periods"].get<std::vector<std::string>>();
    data.classrooms.Clm = j_classrooms["classrooms_per_slot"].get<std::map<std::string, std::map<std::string, int>>>();

    std::cout << "✅ Phase 1 initialized: "
              << data.teachers.size() << " teachers, "
              << data.courses.size() << " courses loaded.\n";
    return data;
}
