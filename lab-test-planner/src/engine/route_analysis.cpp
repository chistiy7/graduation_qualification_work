#include "engine/route_analysis.hpp"

#include <optional>
#include <stdexcept>

namespace lab {

namespace {

const LabEquipment* findEquipment(const ProblemDefinition& p, const std::string& id) {
    for (const auto& e : p.equipment) {
        if (e.id == id) return &e;
    }
    return nullptr;
}

}  // namespace

RouteAnalysis RouteAnalyzer::analyze(const ProblemDefinition& problem,
                                     const TestRoute& route) const {
    RouteAnalysis out;
    const auto& steps = route.steps();
    if (steps.empty()) return out;

    std::unordered_map<std::string, std::optional<std::string>> lastOperationOnEquipment;

    for (const auto& step : steps) {
        const auto* equipment = findEquipment(problem, step.equipmentId);
        if (!equipment) {
            throw std::runtime_error("unknown equipment in route: " + step.equipmentId);
        }

        bool needsSetup = true;
        if (auto it = lastOperationOnEquipment.find(equipment->id);
            it != lastOperationOnEquipment.end() && it->second.has_value()) {
            needsSetup = it->second.value() != step.operationId;
        }

        if (needsSetup) {
            ++out.setupCount;
            out.setupTimeMin += equipment->setupTimeMin;
            out.setupCost += equipment->setupCost;
            out.busyMinutesByEquipment[equipment->id] += equipment->setupTimeMin;
        }

        if (const auto* op = [&]() -> const TestStage* {
                for (const auto& o : problem.operations) {
                    if (o.id == step.operationId) return &o;
                }
                return nullptr;
            }()) {
            out.busyMinutesByEquipment[equipment->id] += op->durationMin;
        }

        lastOperationOnEquipment[equipment->id] = step.operationId;
    }

    for (size_t i = 1; i < steps.size(); ++i) {
        const auto& prevEq = steps[i - 1].equipmentId;
        const auto& curEq = steps[i].equipmentId;
        if (prevEq == curEq) continue;

        const auto* prev = findEquipment(problem, prevEq);
        const auto* cur = findEquipment(problem, curEq);
        if (!prev || !cur) continue;

        const int dist = problem.laboratory.manhattanDistance(prev->cellId, cur->cellId);
        out.routeLengthSteps += static_cast<double>(dist);
    }

    out.moveTimeMin = out.routeLengthSteps * problem.minutesPerGridStep;
    return out;
}

}  // namespace lab
