#pragma once

#include "model/lab_program_mode.hpp"
#include "model/planning_strategy.hpp"
#include "model/problem.hpp"

namespace lab {

// Сценарий: входные данные, режим программы, стратегии упорядочивания
struct ScenarioBundle {
    std::string name;
    std::string description;
    LabProgramMode programMode = LabProgramMode::BasicMechanical;
    ProblemDefinition problem;
    RouteOrderingStrategy baselineStrategy = RouteOrderingStrategy::BySpecimenThenOperation;
    RouteOrderingStrategy optimizedStrategy = RouteOrderingStrategy::ByOperationThenSpecimen;
};

ScenarioBundle buildDemoSimple();
ScenarioBundle buildDemoTwoSpecimens();
ScenarioBundle buildDemoForMode(LabProgramMode mode);

}  // namespace lab
