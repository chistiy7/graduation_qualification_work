#include "domain/stand_catalog.hpp"

namespace lab {

namespace {

std::vector<StandDef> buildStands() {
  return {
      {StandType::UniversalAxialTorsion, "e_axial_torsion",
       "Осевая-крутильная сервогидравлическая испытательная машина",
       {TestOperationType::Tension, TestOperationType::Torsion,
        TestOperationType::TensionPlusTorsion},
       false, false, {1, 0, 1}, 14.0},

      {StandType::CompressionStand, "e_compression", "Стенд испытания на сжатие",
       {TestOperationType::Compression}, false, false, {1, 0, 1}, 15.0},

      {StandType::BendingRig, "e_bending", "Стенд испытания на изгиб",
       {TestOperationType::Bending}, false, false, {1, 0, 1}, 10.0},

      {StandType::FatigueStand, "e_fatigue", "Стенд усталостных испытаний",
       {TestOperationType::Fatigue}, true, false, {2, 2, 1}, 8.0},

      {StandType::BendingTorsionRig, "e_bending_torsion",
       "Стенд испытания на изгиб с кручением",
       {TestOperationType::BendingPlusTorsion}, false, false, {1, 0, 1}, 13.0},

      {StandType::ThermomechanicalStand, "e_thermo_mech",
       "Термомеханический испытательный комплекс",
       {TestOperationType::Thermomechanical}, false, true, {2, 1, 2}, 40.0},
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

const StandDef* findStandDefById(const std::string& equipmentId) {
  for (const auto& def : approvedStands()) {
    if (def.idPrefix == equipmentId) return &def;
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
