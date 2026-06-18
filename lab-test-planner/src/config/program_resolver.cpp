#include "config/program_resolver.hpp"

#include "domain/stand_catalog.hpp"
#include "domain/test_operation_catalog.hpp"

#include <stdexcept>

namespace lab {

namespace {

TestKind kindForOperation(const TestOperationDef& def) {
    switch (def.regime) {
    case WorkloadRegime::Thermal:
        return TestKind::Thermal;
    case WorkloadRegime::Thermomechanical:
    case WorkloadRegime::CombinedSequential:
    case WorkloadRegime::CombinedSimultaneous:
        return TestKind::Combined;
    default:
        return TestKind::Mechanical;
    }
}

}  // namespace

StandEconomicsDefaults defaultStandEconomics(StandType type) {
    constexpr double kCell = 5000.0;
    switch (type) {
    case StandType::UniversalAxialTorsion:
        // Единая амортизация универсальной осевой-крутильной машины (Instron 8874):
        // одна физическая установка вместо трёх → одна статья C_аморт.
        return {220.0, 2800.0, 280.0, kCell};
    case StandType::CompressionStand:
        return {200.0, 2000.0, 240.0, kCell};
    case StandType::BendingRig:
        return {220.0, 1800.0, 200.0, kCell};
    case StandType::FatigueStand:
        return {300.0, 3000.0, 480.0, kCell * 1.5};
    case StandType::BendingTorsionRig:
        return {220.0, 2600.0, 260.0, kCell};
    case StandType::ThermomechanicalStand:
        return {500.0, 4000.0, 400.0, kCell * 2.0};
    default:
        return {200.0, 2000.0, 240.0, kCell};
    }
}

OperationEconomicsDefaults defaultOperationEconomics(TestOperationType type) {
    switch (type) {
    case TestOperationType::Tension:
        return {200.0, 0.20};
    case TestOperationType::Torsion:
        return {250.0, 0.25};
    case TestOperationType::Bending:
        return {220.0, 0.22};
    case TestOperationType::Compression:
        return {200.0, 0.20};
    case TestOperationType::Fatigue:
        return {400.0, 0.40};
    case TestOperationType::TensionPlusTorsion:
        return {350.0, 0.35};
    case TestOperationType::BendingPlusTorsion:
        return {360.0, 0.35};
    case TestOperationType::ThermalStatic:
        return {150.0, 0.20};
    case TestOperationType::ThermalCyclic:
        return {200.0, 0.25};
    case TestOperationType::InductionHeating:
        return {120.0, 0.15};
    case TestOperationType::Thermomechanical:
        return {350.0, 0.35};
    default:
        return {80.0, 0.10};
    }
}

TestProgramDef programForOperation(TestOperationType type, const LabConfigOverrides* overrides) {
    const auto* base = findTestProgramForOperation(type);
    if (!base) {
        throw std::runtime_error("нет техкарты программы для операции");
    }
    return resolveTestProgram(*base, overrides);
}

TestStage buildStageFromProgram(TestOperationType type, const LabConfigOverrides* overrides) {
    const auto program = programForOperation(type, overrides);
    const auto* def = findOperationDef(type);
    if (!def) throw std::runtime_error("unknown operation type");

    const double tProgram = resolveProgramTimeMin(program);
    const auto econ = defaultOperationEconomics(type);

    TestStage stage;
    stage.id = def->id;
    stage.operationType = type;
    stage.regime = def->regime;
    stage.nameRu = def->nameRu;
    stage.destructive = def->destructive;
    stage.combined = def->combined;
    stage.kind = kindForOperation(*def);
    stage.durationNormMin = tProgram;
    stage.durationMin = tProgram;
    stage.cycleTimeMin = tProgram;
    stage.modePowerKw = program.modePowerKw;
    stage.setupPowerKw = program.setupPowerKw;
    stage.programSetupTimeMin = program.setupTimeMin;
    stage.deformationEnergyUsed = program.deformationEnergyUsed;
    stage.sigmaMpa = program.sigmaMpa;
    stage.deltaT_C = program.deltaT_C;
    stage.costOp = econ.costOp;
    stage.laborHours = econ.laborHours;
    return stage;
}

LabEquipment buildEquipmentFromProgram(const TestProgramDef& program, StandType standType,
                                       const LabConfigOverrides* overrides) {
    const auto* sdef = findStandDef(standType);
    if (!sdef) throw std::runtime_error("unknown stand type");

    const auto& resolved = resolveTestProgram(program, overrides);
    const auto econ = defaultStandEconomics(standType);

    LabEquipment eq;
    eq.id = sdef->idPrefix;
    eq.standType = standType;
    eq.nameRu = sdef->nameRu;
    eq.setupTimeMin = resolved.setupTimeMin;
    eq.setupCost = econ.setupCost;
    eq.amortPerHour = econ.amortPerHour;
    eq.fundTimeMin = econ.fundTimeMin;
    eq.cellPlacementCost = econ.cellPlacementCost;
    eq.buffer = sdef->defaultBuffer;
    eq.orientation = 1;
    eq.nominalPowerKw = sdef->nominalPowerKw;
    return eq;
}

}  // namespace lab
