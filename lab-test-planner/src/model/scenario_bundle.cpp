#include "model/scenario_bundle.hpp"

#include "model/program_builder.hpp"

#include <stdexcept>

namespace lab {

ScenarioBundle buildDemoSimple() {
    ProgramBuildRequest req;
    req.mode = LabProgramMode::BasicMechanical;
    req.scenarioName = "demo_simple";
    req.description =
        "Два образца, два независимых испытания. Метрики вычисляются программой из маршрута.";
    req.laborRatePerHour = 800.0;
    req.specimens = {
        {"s1", SpecimenRole::Primary, 5.0, 0.10, {TestOperationType::Tension}},
        {"s2", SpecimenRole::Primary, 5.0, 0.10, {TestOperationType::Torsion}},
    };
    req.operationParams[TestOperationType::Tension] = {20.0, 100.0, 150.0, 0.15};
    req.operationParams[TestOperationType::Torsion] = {25.0, 120.0, 180.0, 0.18};
    req.standParams[StandType::TensionStand] = {8.0, 150.0, 1500.0, 120.0, 5000.0, 0, 0};
    req.standParams[StandType::TorsionMachine] = {8.0, 150.0, 1800.0, 120.0, 5000.0, 0, 3};
    return buildFromProgram(req);
}

ScenarioBundle buildDemoTwoSpecimens() {
    ProgramBuildRequest req;
    req.mode = LabProgramMode::BasicMechanical;
    req.scenarioName = "demo_two_specimens";
    req.description = "Два образца: растяжение и кручение на разных стендах (BasicMechanical)";
    req.laborRatePerHour = 1000.0;
    req.specimens = {
        {"s1", SpecimenRole::Primary, 10.0, 0.15, {TestOperationType::Tension}},
        {"s2", SpecimenRole::Primary, 10.0, 0.15, {TestOperationType::Torsion}},
    };
    req.standParams[StandType::TensionStand] = {10.0, 200.0, 2000.0, 240.0, 5000.0, 0, 0};
    req.standParams[StandType::TorsionMachine] = {10.0, 200.0, 2500.0, 240.0, 5000.0, 0, 4};
    return buildFromProgram(req);
}

ScenarioBundle buildDemoForMode(LabProgramMode mode) {
    if (mode == LabProgramMode::ThermalCycle) {
        mode = LabProgramMode::Thermomechanical;
    }
    ProgramBuildRequest req;
    req.mode = mode;
    req.laborRatePerHour = 1000.0;
    req.gridCellSizeM = 2.0;

    switch (mode) {
    case LabProgramMode::BasicMechanical:
        return buildDemoSimple();
    case LabProgramMode::MechanicalExtended:
        req.scenarioName = "demo_mechanical_extended";
        req.description = "Расширенная механическая программа: растяжение и изгиб (2 образца)";
        req.specimens = {
            {"s1", SpecimenRole::Primary, 10.0, 0.15, {TestOperationType::Tension}},
            {"s2", SpecimenRole::Primary, 10.0, 0.15, {TestOperationType::Bending}},
        };
        break;
    case LabProgramMode::Thermomechanical:
        req.scenarioName = "demo_thermomechanical";
        req.description = "Термомеханическая программа (1 образец, thermo_mech)";
        req.specimens = {{"s1", SpecimenRole::Primary, 12.0, 0.18,
                          {TestOperationType::Thermomechanical}}};
        break;
    default:
        throw std::runtime_error("unsupported program mode in buildDemoForMode");
    }
    return buildFromProgram(req);
}

}  // namespace lab
