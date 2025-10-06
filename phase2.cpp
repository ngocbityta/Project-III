#include "phase1.h"

ProblemData initialize_problem() {
    ProblemData d;
    d.num_teachers = 2;
    d.num_classes = 2;
    d.cost = {{10, 20}, {30, 15}};
    return d;
}
