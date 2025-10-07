#pragma once
#include <string>
#include <vector>
#include <map>
#include <utility>

struct TimePref {
    std::string day;
    std::string period;
    int score;
};

struct Teacher {
    std::string id;
    std::string name;
    int max_courses;
    std::map<std::string, int> course_pref; // PC_ij
    std::vector<TimePref> time_pref;        // PT_ilm
    std::vector<std::string> eligible_courses;
    std::vector<TimePref> LMi;              // sorted by preference
};

struct Section {
    std::string id;
    int required_periods;
};

struct Course {
    std::string id;
    std::string name;
    std::vector<Section> sections;
    int min_teachers;
    int max_teachers;
    std::vector<std::string> Ij;
};

struct ClassroomInfo {
    std::vector<std::string> days;
    std::vector<std::string> periods;
    std::map<std::string, std::map<std::string, int>> Clm;
};

struct ProblemData {
    std::vector<Teacher> teachers;
    std::vector<Course> courses;
    ClassroomInfo classrooms;
};

ProblemData initialize_problem();
