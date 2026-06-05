#pragma once

#include "domain/test_operation_catalog.hpp"

#include <string>
#include <vector>

namespace lab {

// Типы испытательных стендов (гл. 2, §2.1)
enum class StandType {
    UniversalTensile,       // разрывная машина
    TorsionMachine,         // машина кручения
    BendingRig,             // стенд изгиба
    FatigueStand,           // усталостные испытания
    HardnessTester,         // твердомер
    ThermalFurnace,         // термическая печь
    InductionHeater,        // индукционный нагреватель
    ThermomechanicalUnit,   // комбинированная термомеханическая установка
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
    bool highVibration = false;   // для правил соседства §2.2
    bool heatSource = false;
};

struct AuxiliaryZoneDef {
    AuxiliaryZoneType type{};
    std::string idPrefix;
    std::string nameRu;
    std::vector<TestOperationType> linkedOperations;
};

[[nodiscard]] const std::vector<StandDef>& approvedStands();
[[nodiscard]] const StandDef* findStandDef(StandType type);
[[nodiscard]] bool standCanExecute(StandType stand, TestOperationType operation);
[[nodiscard]] const std::vector<AuxiliaryZoneDef>& approvedAuxiliaryZones();

}  // namespace lab
