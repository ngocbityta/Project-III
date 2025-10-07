#pragma once
#include <string>
#include <vector>
#include <map>
#include <utility>
using namespace std;

struct TimePref {
    string day;
    string period;
    int score;
};

struct Teacher {
    string id;
    string name;
    int max_courses;
    map<string, int> course_pref; // PC_ij
    vector<TimePref> time_pref; // PT_ilm
    vector<string> eligible_courses;
    vector<TimePref> LMi;
};

struct Section {
    string id;
    int required_periods;
};

struct Course {
    string id;
    string name;
    vector<Section> sections;
    int min_teachers;
    int max_teachers;
    vector<string> Ij;
};

struct ClassroomInfo {
    vector<string> days;
    vector<string> periods;
    map<string, map<string, int>> Clm;
};

struct ProblemData {
    vector<Teacher> teachers;
    vector<Course> courses;
    ClassroomInfo classrooms;
};

ProblemData initialize_problem();
