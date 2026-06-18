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
                 {StandType::TensionStand, StandType::TorsionMachine},
                 {AuxiliaryZoneType::PreparationTable, AuxiliaryZoneType::MeasurementZone,
                  AuxiliaryZoneType::OperatorDesk}),
        makeSpec(LabProgramMode::MechanicalExtended, "mechanical_extended",
                 "Расширенная механическая программа",
                 "Добавлены изгиб и усталость",
                 {TestOperationType::SpecimenPreparation, TestOperationType::GeometryControl,
                  TestOperationType::Tension, TestOperationType::Torsion,
                  TestOperationType::Bending, TestOperationType::Fatigue,
                  TestOperationType::ResultsProcessing},
                 {StandType::TensionStand, StandType::TorsionMachine, StandType::BendingRig,
                  StandType::FatigueStand},
                 {AuxiliaryZoneType::PreparationTable, AuxiliaryZoneType::MeasurementZone,
                  AuxiliaryZoneType::ToolingStorage, AuxiliaryZoneType::OperatorDesk}),
        makeSpec(LabProgramMode::Thermomechanical, "thermomechanical", "Термомеханическая программа",
                 "Комбинированные и термомеханические режимы (8 стендов, 1 техкарта на стенд)",
                 {TestOperationType::SpecimenPreparation, TestOperationType::GeometryControl,
                  TestOperationType::Thermomechanical, TestOperationType::TensionPlusTorsion,
                  TestOperationType::BendingPlusTorsion, TestOperationType::ResultsProcessing},
                 {StandType::ThermomechanicalStand, StandType::AxialTorsionRig,
                  StandType::BendingTorsionRig},
                 {AuxiliaryZoneType::PreparationTable, AuxiliaryZoneType::MeasurementZone,
                  AuxiliaryZoneType::OperatorDesk}),
    };
    return modes;
}

const ProgramModeSpec* findProgramMode(LabProgramMode mode) {
    if (mode == LabProgramMode::ThermalCycle) {
        mode = LabProgramMode::Thermomechanical;
    }
    for (const auto& spec : approvedProgramModes()) {
        if (spec.mode == mode) return &spec;
    }
    return nullptr;
}

std::string programModeNameRu(LabProgramMode mode) {
    if (mode == LabProgramMode::ThermalCycle) {
        return programModeNameRu(LabProgramMode::Thermomechanical);
    }
    if (const auto* spec = findProgramMode(mode)) return spec->nameRu;
    return "неизвестный режим";
}

}  // namespace lab
