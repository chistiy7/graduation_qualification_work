#pragma once

#include "model/lab_program_mode.hpp"
#include "model/problem.hpp"

namespace lab {

// Сценарий: входные данные и режим программы
struct ScenarioBundle {
    std::string name;
    std::string description;
    LabProgramMode programMode = LabProgramMode::BasicMechanical;
    ProblemDefinition problem;
};

ScenarioBundle buildDemoSimple();
ScenarioBundle buildDemoTwoSpecimens();
ScenarioBundle buildDemoForMode(LabProgramMode mode);

}  // namespace lab
