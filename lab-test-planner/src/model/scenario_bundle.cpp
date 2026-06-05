#include "model/scenario_bundle.hpp"

#include "model/program_builder.hpp"

namespace lab {

ScenarioBundle buildDemoSimple() {
    ProgramBuildRequest req;
    req.mode = LabProgramMode::BasicMechanical;
    req.scenarioName = "demo_simple";
    req.description = "1 образец: растяжение + кручение (режим BasicMechanical)";
    req.laborRatePerHour = 800.0;
    req.weights = {0.3, 0.3, 0.2, 0.1, 0.1};
    req.specimens = {
        {"s1", SpecimenRole::Primary, 5.0, 0.10,
         {TestOperationType::Tension, TestOperationType::Torsion}},
    };
    req.operationParams[TestOperationType::Tension] = {20.0, 100.0, 150.0, 0.15};
    req.operationParams[TestOperationType::Torsion] = {25.0, 120.0, 180.0, 0.18};
    req.standParams[StandType::UniversalTensile] = {8.0, 150.0, 1500.0, 120.0, 5000.0, 0, 0};
    req.standParams[StandType::TorsionMachine] = {8.0, 150.0, 1800.0, 120.0, 5000.0, 0, 3};
    return buildFromProgram(req);
}

ScenarioBundle buildDemoTwoSpecimens() {
    ProgramBuildRequest req;
    req.mode = LabProgramMode::BasicMechanical;
    req.scenarioName = "demo_two_specimens";
    req.description = "2 образца: растяжение + кручение (режим BasicMechanical)";
    req.laborRatePerHour = 1000.0;
    req.weights = {0.25, 0.35, 0.20, 0.10, 0.10};
    req.specimens = {
        {"s1", SpecimenRole::Primary, 10.0, 0.15,
         {TestOperationType::Tension, TestOperationType::Torsion}},
        {"s2", SpecimenRole::Primary, 10.0, 0.15, {TestOperationType::Tension}},
    };
    req.standParams[StandType::UniversalTensile] = {10.0, 200.0, 2000.0, 240.0, 5000.0, 0, 0};
    req.standParams[StandType::TorsionMachine] = {10.0, 200.0, 2500.0, 240.0, 5000.0, 0, 4};
    return buildFromProgram(req);
}

ScenarioBundle buildDemoForMode(LabProgramMode mode) {
    ProgramBuildRequest req;
    req.mode = mode;
    req.laborRatePerHour = 1000.0;
    req.objectiveMode = ObjectiveMode::TotalCostRub;
    req.gridCellSizeM = 2.0;

    switch (mode) {
    case LabProgramMode::BasicMechanical:
        return buildDemoSimple();
    case LabProgramMode::MechanicalExtended:
        req.scenarioName = "demo_mechanical_extended";
        req.description = "Расширенная механическая программа (1 образец)";
        req.specimens = {{"s1", SpecimenRole::Primary, 10.0, 0.15,
                          {TestOperationType::Tension, TestOperationType::Bending}}};
        break;
    case LabProgramMode::ThermalCycle:
        req.scenarioName = "demo_thermal_cycle";
        req.description = "Термический цикл (1 образец)";
        req.specimens = {{"s1", SpecimenRole::Primary, 8.0, 0.12,
                          {TestOperationType::ThermalStatic, TestOperationType::CoolingHold}}};
        break;
    case LabProgramMode::Thermomechanical:
        req.scenarioName = "demo_thermomechanical";
        req.description = "Термомеханическая программа (1 образец)";
        req.specimens = {{"s1", SpecimenRole::Primary, 12.0, 0.18,
                          {TestOperationType::ThermalStatic, TestOperationType::Thermomechanical}}};
        break;
    }
    return buildFromProgram(req);
}

}  // namespace lab
