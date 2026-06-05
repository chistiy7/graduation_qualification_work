#include "domain/stand_catalog.hpp"

namespace lab {

namespace {

std::vector<StandDef> buildStands() {
    return {
        {StandType::UniversalTensile, "e_tensile", "Универсальная разрывная машина",
         {TestOperationType::Tension, TestOperationType::Compression,
          TestOperationType::TensionPlusTorsion},
         false, false},
        {StandType::TorsionMachine, "e_torsion", "Машина кручения",
         {TestOperationType::Torsion, TestOperationType::TensionPlusTorsion,
          TestOperationType::BendingPlusTorsion},
         false, false},
        {StandType::BendingRig, "e_bending", "Стенд изгиба",
         {TestOperationType::Bending, TestOperationType::BendingPlusTorsion,
          TestOperationType::CompressionPlusBending},
         false, false},
        {StandType::FatigueStand, "e_fatigue", "Стенд усталостных испытаний",
         {TestOperationType::Fatigue}, true, false},
        {StandType::HardnessTester, "e_hardness", "Твердомер",
         {TestOperationType::Hardness}, false, false},
        {StandType::ThermalFurnace, "e_furnace", "Термическая печь",
         {TestOperationType::ThermalStatic, TestOperationType::ThermalCyclic}, false, true},
        {StandType::InductionHeater, "e_induction", "Индукционный нагреватель",
         {TestOperationType::InductionHeating}, false, true},
        {StandType::ThermomechanicalUnit, "e_thermo_mech",
         "Комбинированная термомеханическая установка",
         {TestOperationType::Thermomechanical}, false, true},
    };
}

std::vector<AuxiliaryZoneDef> buildZones() {
    return {
        {AuxiliaryZoneType::PreparationTable, "z_prep", "Стол подготовки образцов",
         {TestOperationType::SpecimenPreparation, TestOperationType::GeometryControl}},
        {AuxiliaryZoneType::CoolingZone, "z_cool", "Зона охлаждения",
         {TestOperationType::CoolingHold}},
        {AuxiliaryZoneType::MeasurementZone, "z_measure", "Зона измерений",
         {TestOperationType::GeometryControl, TestOperationType::ResultsProcessing}},
        {AuxiliaryZoneType::ToolingStorage, "z_tooling", "Зона хранения оснастки", {}},
        {AuxiliaryZoneType::OperatorDesk, "z_operator", "Рабочее место оператора", {}},
    };
}

}  // namespace

const std::vector<StandDef>& approvedStands() {
    static const auto stands = buildStands();
    return stands;
}

const std::vector<AuxiliaryZoneDef>& approvedAuxiliaryZones() {
    static const auto zones = buildZones();
    return zones;
}

const StandDef* findStandDef(StandType type) {
    for (const auto& def : approvedStands()) {
        if (def.type == type) return &def;
    }
    return nullptr;
}

bool standCanExecute(StandType stand, TestOperationType operation) {
    const auto* def = findStandDef(stand);
    if (!def) return false;
    for (const auto op : def->capableOperations) {
        if (op == operation) return true;
    }
    return false;
}

}  // namespace lab
