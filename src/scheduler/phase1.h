#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>

using json = nlohmann::json;
using namespace std;

struct TimePref
{
    string day;
    string period;
    int score;
};

struct Teacher
{
    string id;
    string name;
    int max_courses = 1;
    map<string, int> course_pref;
    vector<TimePref> time_pref;
    vector<string> eligible_courses;
    vector<TimePref> LMi;
};

struct Section
{
    string id;
    int required_periods = 1;
};

struct Course
{
    string id;
    string name;
    vector<Section> sections;
    int min_teachers = 1;
    int max_teachers = 1;
    vector<string> Ij; // eligible teacher ids sorted by preference
};

struct ClassroomInfo
{
    vector<string> days;
    vector<string> periods;
    map<string, map<string, int>> Clm;
};

struct ProblemData
{
    vector<Teacher> teachers;
    vector<Course> courses;
    ClassroomInfo classrooms;
};

ProblemData initialize_problem_from_json(const json &j_input);
