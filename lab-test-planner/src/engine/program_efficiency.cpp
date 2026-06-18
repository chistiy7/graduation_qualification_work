#include "engine/program_efficiency.hpp"

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

double cycleTimeMin(const ProblemDefinition& problem, const RouteAnalysis& analysis) {
    double tCycle = 0.0;
    for (const auto& e : problem.equipment) {
        const auto it = analysis.busyMinutesByEquipment.find(e.id);
        const double busy = it != analysis.busyMinutesByEquipment.end() ? it->second : 0.0;
        tCycle = std::max(tCycle, busy);
    }
    return tCycle;
}

}  // namespace

ProgramEfficiencyMetrics ProgramEfficiencyEngine::compute(
    const ProblemDefinition& problem, const TestRoute& route) const {
    return compute(problem, route, RouteAnalyzer{}.analyze(problem, route));
}

ProgramEfficiencyMetrics ProgramEfficiencyEngine::compute(const ProblemDefinition& problem,
                                                          const TestRoute& route,
                                                          const RouteAnalysis& analysis) const {
    ProgramEfficiencyMetrics out;
    auto& t = out.time;

    t.prepMin = prepTimeSum(problem);
    t.testMin = operationTimeSum(problem, route);
    t.setupMin = analysis.setupTimeMin;
    t.moveMin = analysis.moveTimeMin;
    t.workTimeMin = t.prepMin + t.testMin + t.setupMin + t.moveMin;
    t.cycleMin = cycleTimeMin(problem, analysis);

    if (t.workTimeMin > 0.0) {
        t.parallelismIndex = std::min(1.0, t.cycleMin / t.workTimeMin);
    }

    return out;
}

}  // namespace lab
