#include "engine/route_analysis.hpp"

#include "domain/changeover_matrix.hpp"
#include "domain/physics_constants.hpp"
#include "domain/stand_catalog.hpp"
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
    std::unordered_map<std::string, std::optional<TestOperationType>> lastTypeOnEquipment;
    std::unordered_map<std::string, double> testMinByEquipment;
    std::unordered_map<std::string, double> setupMinByEquipment;

    for (const auto& step : steps) {
        const auto* equipment = findEquipment(problem, step.equipmentId);
        if (!equipment) {
            throw std::runtime_error("unknown equipment in route: " + step.equipmentId);
        }

        const auto* op = findOperation(problem, step.operationId);
        const bool universal = equipment->standType == StandType::UniversalAxialTorsion;

        auto modeIt = lastModeOnEquipment.find(equipment->id);
        const bool firstUse =
            modeIt == lastModeOnEquipment.end() || !modeIt->second.has_value();
        const bool programChanged =
            !firstUse && modeIt->second.value() != step.operationId;
        // Событие переналадки N — первичная установка или смена ПРОГРАММЫ. Чистая
        // смена образца (та же программа на универсальном стенде) в N не входит, но
        // занимает время/энергию по матрице переходов.
        const bool isChangeover = firstUse || programChanged;

        // Время фазы настройки/перезагрузки стенда.
        double tSetup = 0.0;
        if (universal) {
            if (firstUse) {
                tSetup = setupTimeMin(*equipment, op);  // первичная установка из техкарты
            } else {
                const auto typeIt = lastTypeOnEquipment.find(equipment->id);
                const TestOperationType prevType = typeIt->second.value();
                const TestOperationType curType = op ? op->operationType : prevType;
                tSetup = changeoverTimeMin(prevType, curType);  // переход или смена образца
            }
        } else if (isChangeover) {
            tSetup = setupTimeMin(*equipment, op);  // обычный стенд — только при смене программы
        }

        // N и фиксированная плата за переналадку — только на реальном переходе программы.
        if (isChangeover) {
            ++out.setupCount;
            out.setupCost += equipment->setupCost;
            out.setupCostByEquipment[equipment->id] += equipment->setupCost;
        }

        // Время и энергия фазы настройки/перезагрузки (универсальный стенд — каждый шаг).
        if (tSetup > 0.0) {
            out.setupTimeMin += tSetup;
            setupMinByEquipment[equipment->id] += tSetup;
            out.busyMinutesByEquipment[equipment->id] += tSetup;

            const double pSetup = setupPowerKw(*equipment, op);
            if (problem.electricityTariffPerKwh > 0.0 && pSetup > 0.0) {
                const double kwh = pSetup * (tSetup / 60.0);
                out.energySetupCost += kwh * problem.electricityTariffPerKwh;
            }
        }

        if (op) {
            const double busy =
                op->cycleTimeMin > 0.0 ? op->cycleTimeMin : op->durationMin;
            testMinByEquipment[equipment->id] += busy;
            out.busyMinutesByEquipment[equipment->id] += busy;
        }

        lastModeOnEquipment[equipment->id] = step.operationId;
        if (op) lastTypeOnEquipment[equipment->id] = op->operationType;
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
