#pragma once

#include "domain/test_operation_catalog.hpp"

#include <string>
#include <vector>

namespace lab {

// Направленная зона безопасности стенда (гл. 2, §2.2, ГОСТ-обоснование)
// front — сторона оператора / ось нагружения (опасная)
// back  — нерабочая сторона; 0 означает «допускается примыкание к стене»
// side  — боковые стороны (доступ для обслуживания и измерений)
struct DirectionalBuffer {
    int front = 1;
    int back  = 0;
    int side  = 1;
};

// v1: 8 стендов — один основной вид испытания на стенд (id = e_*).
enum class StandType {
    TensionStand,           // e_tension
    CompressionStand,       // e_compression
    TorsionMachine,         // e_torsion
    BendingRig,             // e_bending
    FatigueStand,           // e_fatigue
    AxialTorsionRig,        // e_tension_torsion
    BendingTorsionRig,      // e_bending_torsion
    ThermomechanicalStand,  // e_thermo_mech
};

// Вспомогательные зоны (гл. 2, §2.1)
enum class AuxiliaryZoneType {
    PreparationTable,
    CoolingZone,
    MeasurementZone,
    ToolingStorage,
    OperatorDesk,
};

struct StandDef {
    StandType type{};
    std::string idPrefix;
    std::string nameRu;
    std::vector<TestOperationType> capableOperations;
    bool highVibration = false;       // для информации / правил совместимости §2.2
    bool heatSource    = false;
    DirectionalBuffer defaultBuffer;  // нормативный буфер по направлениям (§2.2)
    double nominalPowerKw = 10.0;     // P_nom — паспортная мощность, кВт (переналадка)
};

struct AuxiliaryZoneDef {
    AuxiliaryZoneType type{};
    std::string idPrefix;
    std::string nameRu;
    std::vector<TestOperationType> linkedOperations;
};

[[nodiscard]] const std::vector<StandDef>& approvedStands();
[[nodiscard]] const StandDef* findStandDef(StandType type);
[[nodiscard]] const StandDef* findStandDefById(const std::string& equipmentId);
[[nodiscard]] bool standCanExecute(StandType stand, TestOperationType operation);
[[nodiscard]] const std::vector<AuxiliaryZoneDef>& approvedAuxiliaryZones();

}  // namespace lab
