#include "model/lab_program_mode.hpp"

#include "domain/stand_catalog.hpp"

namespace lab {

namespace {

ProgramModeSpec makeSpec(LabProgramMode mode, std::string id, std::string nameRu,
                         std::string desc, std::vector<TestOperationType> ops,
                         std::vector<StandType> stands,
                         std::vector<AuxiliaryZoneType> zones) {
    return {mode, std::move(id), std::move(nameRu), std::move(desc), std::move(ops),
            std::move(stands), std::move(zones)};
}

}  // namespace

const std::vector<ProgramModeSpec>& approvedProgramModes() {
    static const std::vector<ProgramModeSpec> modes = {
        makeSpec(LabProgramMode::BasicMechanical, "basic_mechanical",
                 "Базовая механическая программа",
                 "Растяжение и кручение цилиндрических образцов (гл. 1–2)",
                 {TestOperationType::SpecimenPreparation, TestOperationType::GeometryControl,
                  TestOperationType::Tension, TestOperationType::Torsion,
                  TestOperationType::ResultsProcessing},
                 {StandType::UniversalTensile, StandType::TorsionMachine},
                 {AuxiliaryZoneType::PreparationTable, AuxiliaryZoneType::MeasurementZone,
                  AuxiliaryZoneType::OperatorDesk}),
        makeSpec(LabProgramMode::MechanicalExtended, "mechanical_extended",
                 "Расширенная механическая программа",
                 "Добавлены изгиб и усталость",
                 {TestOperationType::SpecimenPreparation, TestOperationType::GeometryControl,
                  TestOperationType::Tension, TestOperationType::Torsion,
                  TestOperationType::Bending, TestOperationType::Fatigue,
                  TestOperationType::ResultsProcessing},
                 {StandType::UniversalTensile, StandType::TorsionMachine, StandType::BendingRig,
                  StandType::FatigueStand},
                 {AuxiliaryZoneType::PreparationTable, AuxiliaryZoneType::MeasurementZone,
                  AuxiliaryZoneType::ToolingStorage, AuxiliaryZoneType::OperatorDesk}),
        makeSpec(LabProgramMode::ThermalCycle, "thermal_cycle", "Термический цикл",
                 "Статический и индукционный нагрев, охлаждение",
                 {TestOperationType::SpecimenPreparation, TestOperationType::GeometryControl,
                  TestOperationType::ThermalStatic, TestOperationType::InductionHeating,
                  TestOperationType::CoolingHold, TestOperationType::ResultsProcessing},
                 {StandType::ThermalFurnace, StandType::InductionHeater},
                 {AuxiliaryZoneType::PreparationTable, AuxiliaryZoneType::CoolingZone,
                  AuxiliaryZoneType::OperatorDesk}),
        makeSpec(LabProgramMode::Thermomechanical, "thermomechanical", "Термомеханическая программа",
                 "Комбинированные и термомеханические режимы (гл. 1, §1.3)",
                 {TestOperationType::SpecimenPreparation, TestOperationType::GeometryControl,
                  TestOperationType::ThermalStatic, TestOperationType::Thermomechanical,
                  TestOperationType::TensionPlusTorsion, TestOperationType::BendingPlusTorsion,
                  TestOperationType::CoolingHold, TestOperationType::ResultsProcessing},
                 {StandType::ThermalFurnace, StandType::ThermomechanicalUnit,
                  StandType::UniversalTensile, StandType::TorsionMachine, StandType::BendingRig},
                 {AuxiliaryZoneType::PreparationTable, AuxiliaryZoneType::CoolingZone,
                  AuxiliaryZoneType::MeasurementZone, AuxiliaryZoneType::OperatorDesk}),
    };
    return modes;
}

const ProgramModeSpec* findProgramMode(LabProgramMode mode) {
    for (const auto& spec : approvedProgramModes()) {
        if (spec.mode == mode) return &spec;
    }
    return nullptr;
}

std::string programModeNameRu(LabProgramMode mode) {
    if (const auto* spec = findProgramMode(mode)) return spec->nameRu;
    return "неизвестный режим";
}

}  // namespace lab
