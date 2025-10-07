#include "phase1.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <vector>
#include <string>
#include <map>

using json = nlohmann::json;
using namespace std;

// ---------------------------------------------------------
// Helper: đọc file JSON thành kiểu T
// ---------------------------------------------------------
template <typename T>
T read_json(const string &filename) {
    ifstream f(filename);
    if (!f.is_open()) {
        throw runtime_error("❌ Cannot open file: " + filename);
    }
    json j;
    f >> j;
    return j.get<T>();
}

// ---------------------------------------------------------
// Khởi tạo dữ liệu bài toán từ JSON
// ---------------------------------------------------------
ProblemData initialize_problem() {
    ProblemData data;

    // Đọc dữ liệu JSON
    ifstream f_teachers("example-data/teachers.json");
    ifstream f_courses("example-data/courses.json");
    ifstream f_classrooms("example-data/classrooms.json");

    if (!f_teachers.is_open() || !f_courses.is_open() || !f_classrooms.is_open()) {
        throw runtime_error("❌ One or more input JSON files not found in ./example-data/");
    }

    json j_teachers, j_courses, j_classrooms;
    f_teachers >> j_teachers;
    f_courses >> j_courses;
    f_classrooms >> j_classrooms;

    // ----------------------------
    // Parse teachers
    // ----------------------------
    for (const auto &t : j_teachers) {
        Teacher teacher;
        teacher.id = t["id"];
        teacher.name = t["name"];
        teacher.max_courses = t["max_courses"];
        teacher.eligible_courses = t["eligible_courses"].get<vector<string>>();
        teacher.course_pref = t["course_preferences"].get<map<string, int>>();

        // Flatten day_time_preferences
        for (const auto &[day, periods] : t["day_time_preferences"].items()) {
            for (const auto &[period, score] : periods.items()) {
                teacher.time_pref.push_back({day, period, score});
            }
        }

        // Sort theo mức độ ưu tiên thời gian
        sort(teacher.time_pref.begin(), teacher.time_pref.end(),
             [](const TimePref &a, const TimePref &b) {
                 return a.score > b.score;
             });

        teacher.LMi = teacher.time_pref;
        data.teachers.push_back(teacher);
    }

    // ----------------------------
    // Parse courses
    // ----------------------------
    for (const auto &c : j_courses) {
        Course course;
        course.id = c["id"];
        course.name = c["name"];
        course.min_teachers = c["min_teachers"];
        course.max_teachers = c["max_teachers"];

        for (const auto &s : c["sections"]) {
            course.sections.push_back({s["id"], s["required_periods"]});
        }

        // Construct Ij (teachers phù hợp với course này)
        vector<pair<string, int>> teacher_scores;
        for (const auto &t : data.teachers) {
            if (find(t.eligible_courses.begin(), t.eligible_courses.end(), course.id) != t.eligible_courses.end()) {
                int score = t.course_pref.count(course.id) ? t.course_pref.at(course.id) : 0;
                teacher_scores.push_back({t.id, score});
            }
        }

        sort(teacher_scores.begin(), teacher_scores.end(),
             [](const auto &a, const auto &b) { return a.second > b.second; });

        for (const auto &[tid, _] : teacher_scores) {
            course.Ij.push_back(tid);
        }

        data.courses.push_back(course);
    }

    // ----------------------------
    // Parse classrooms
    // ----------------------------
    data.classrooms.days = j_classrooms["days"].get<vector<string>>();
    data.classrooms.periods = j_classrooms["periods"].get<vector<string>>();
    data.classrooms.Clm = j_classrooms["classrooms_per_slot"]
                              .get<map<string, map<string, int>>>();

    cout << "✅ Phase 1 initialized: "
         << data.teachers.size() << " teachers, "
         << data.courses.size() << " courses loaded.\n";

    return data;
}
