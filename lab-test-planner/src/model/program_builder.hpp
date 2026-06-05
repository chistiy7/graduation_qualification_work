#pragma once

#include "model/lab_program_mode.hpp"
#include "model/problem.hpp"
#include "model/scenario_bundle.hpp"

#include <unordered_map>

namespace lab {

struct OperationParams {
    double durationMin = 0.0;
    double costOp = 0.0;
    double costEnergy = 0.0;
    double laborHours = 0.0;
};

struct StandParams {
    double setupTimeMin = 0.0;
    double setupCost = 0.0;
    double amortPerHour = 0.0;
    double fundTimeMin = 0.0;
    double cellPlacementCost = 0.0;  // руб за ячейку стенда (штучная модель)
    int gridRow = 0;
    int gridCol = 0;
};

struct SpecimenProgram {
    std::string id;
    SpecimenRole role = SpecimenRole::Primary;
    double prepTimeMin = 0.0;
    double prepLaborHours = 0.0;
    // какие операции из режима требуются этому образцу
    std::vector<TestOperationType> requiredOperations;
};

struct ProgramBuildRequest {
    LabProgramMode mode = LabProgramMode::BasicMechanical;
    std::string scenarioName;
    std::string description;
    std::vector<SpecimenProgram> specimens;
    double laborRatePerHour = 1000.0;
    double minutesPerGridStep = 1.0;
    double gridCellSizeM = 2.0;
    ObjectiveMode objectiveMode = ObjectiveMode::TotalCostRub;
    ObjectiveWeights weights{};
    std::unordered_map<TestOperationType, OperationParams> operationParams;
    std::unordered_map<StandType, StandParams> standParams;
};

// Сборка постановки из утверждённого режима программы
[[nodiscard]] ScenarioBundle buildFromProgram(const ProgramBuildRequest& request);

}  // namespace lab
