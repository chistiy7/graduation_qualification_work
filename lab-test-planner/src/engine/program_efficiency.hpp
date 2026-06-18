#pragma once

#include "domain/test_route.hpp"
#include "engine/route_analysis.hpp"
#include "model/problem.hpp"

namespace lab {

// Время испытательной программы (гл. 2, §2.4; гл. 3, §3.3):
// T_sum = t_prep + t_test + t_setup + t_move
struct ProgramTimeMetrics {
    double prepMin = 0.0;
    double testMin = 0.0;
    double setupMin = 0.0;
    double moveMin = 0.0;
    double workTimeMin = 0.0;  // T_sum — суммарное время работы программы, мин
    double cycleMin = 0.0;     // T_cycle — длительность выполнения партии, мин
    double parallelismIndex = 0.0;  // T_cycle / T_sum (0..1), степень «сжатия» по времени
};

struct ProgramEfficiencyMetrics {
    ProgramTimeMetrics time;
};

class ProgramEfficiencyEngine {
public:
    [[nodiscard]] ProgramEfficiencyMetrics compute(const ProblemDefinition& problem,
                                                   const TestRoute& route) const;

    [[nodiscard]] ProgramEfficiencyMetrics compute(const ProblemDefinition& problem,
                                                   const TestRoute& route,
                                                   const RouteAnalysis& analysis) const;
};

}  // namespace lab
