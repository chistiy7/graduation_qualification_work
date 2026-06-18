#include "engine/route_analysis.hpp"

#include "domain/physics_constants.hpp"
#include "domain/test_stage.hpp"

#include <optional>
#include <stdexcept>
#include <unordered_map>

namespace lab {

namespace {

const LabEquipment* findEquipment(const ProblemDefinition& p, const std::string& id) {
    for (const auto& e : p.equipment) {
        if (e.id == id) return &e;
    }
    return nullptr;
}

const TestStage* findOperation(const ProblemDefinition& p, const std::string& id) {
    for (const auto& o : p.operations) {
        if (o.id == id) return &o;
    }
    return nullptr;
}

double setupPowerKw(const LabEquipment& e, const TestStage* op) {
    if (op && op->setupPowerKw > 0.0) return op->setupPowerKw;
    return K_POWER_SETUP * e.nominalPowerKw;
}

double setupTimeMin(const LabEquipment& e, const TestStage* op) {
    if (op && op->programSetupTimeMin > 0.0) return op->programSetupTimeMin;
    return e.setupTimeMin;
}

}  // namespace

RouteAnalysis RouteAnalyzer::analyze(const ProblemDefinition& problem,
                                     const TestRoute& route) const {
    RouteAnalysis out;
    const auto& steps = route.steps();

    std::unordered_map<std::string, std::optional<std::string>> lastModeOnEquipment;
    std::unordered_map<std::string, double> testMinByEquipment;
    std::unordered_map<std::string, double> setupMinByEquipment;

    for (const auto& step : steps) {
        const auto* equipment = findEquipment(problem, step.equipmentId);
        if (!equipment) {
            throw std::runtime_error("unknown equipment in route: " + step.equipmentId);
        }

        bool needsSetup = true;
        if (auto it = lastModeOnEquipment.find(equipment->id);
            it != lastModeOnEquipment.end() && it->second.has_value()) {
            needsSetup = it->second.value() != step.operationId;
        }

        if (needsSetup) {
            const auto* op = findOperation(problem, step.operationId);
            const double tSetup = setupTimeMin(*equipment, op);
            const double pSetup = setupPowerKw(*equipment, op);

            ++out.setupCount;
            out.setupTimeMin += tSetup;
            out.setupCost += equipment->setupCost;
            setupMinByEquipment[equipment->id] += tSetup;
            out.setupCostByEquipment[equipment->id] += equipment->setupCost;
            out.busyMinutesByEquipment[equipment->id] += tSetup;

            if (problem.electricityTariffPerKwh > 0.0 && pSetup > 0.0) {
                const double kwh = pSetup * (tSetup / 60.0);
                out.energySetupCost += kwh * problem.electricityTariffPerKwh;
            }
        }

        if (const auto* op = findOperation(problem, step.operationId)) {
            const double busy =
                op->cycleTimeMin > 0.0 ? op->cycleTimeMin : op->durationMin;
            testMinByEquipment[equipment->id] += busy;
            out.busyMinutesByEquipment[equipment->id] += busy;
        }

        lastModeOnEquipment[equipment->id] = step.operationId;
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
    out.testMinByEquipment = std::move(testMinByEquipment);
    out.setupMinByEquipment = std::move(setupMinByEquipment);
    return out;
}

}  // namespace lab
