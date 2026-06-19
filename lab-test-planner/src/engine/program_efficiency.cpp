#include "engine/program_efficiency.hpp"

#include "domain/schedule.hpp"
#include "engine/scheduler.hpp"

#include <algorithm>

namespace lab {

namespace {

const TestStage* findOperation(const ProblemDefinition& p, const std::string& id) {
    for (const auto& op : p.operations) {
        if (op.id == id) return &op;
    }
    return nullptr;
}

double operationTimeSum(const ProblemDefinition& problem, const TestRoute& route) {
    double sum = 0.0;
    for (const auto& step : route.steps()) {
        if (const auto* op = findOperation(problem, step.operationId)) {
            const double busy =
                op->cycleTimeMin > 0.0 ? op->cycleTimeMin : op->durationMin;
            sum += busy;
        }
    }
    return sum;
}

double prepTimeSum(const ProblemDefinition& problem) {
    double sum = 0.0;
    for (const auto& s : problem.specimens) {
        sum += s.prepTimeMin;
    }
    return sum;
}

}  // namespace

ProgramEfficiencyMetrics ProgramEfficiencyEngine::compute(
    const ProblemDefinition& problem, const TestRoute& route) const {
    return compute(problem, route, RouteAnalyzer{}.analyze(problem, route));
}

ProgramEfficiencyMetrics ProgramEfficiencyEngine::compute(const ProblemDefinition& problem,
                                                          const TestRoute& route,
                                                          const RouteAnalysis& analysis) const {
    // DES-расписание даёт реальный цикл (makespan) и фактический путь оператора.
    const Schedule sched = Scheduler{}.build(problem, route);

    ProgramEfficiencyMetrics out;
    auto& t = out.time;

    t.prepMin = prepTimeSum(problem);
    t.testMin = operationTimeSum(problem, route);
    t.setupMin = analysis.setupTimeMin;
    t.moveMin = sched.operatorTravelSteps * problem.minutesPerGridStep;
    t.workTimeMin = t.prepMin + t.testMin + t.setupMin + t.moveMin;
    t.cycleMin = sched.makespanMin;

    if (t.workTimeMin > 0.0) {
        t.parallelismIndex = std::min(1.0, t.cycleMin / t.workTimeMin);
    }

    return out;
}

}  // namespace lab
