#pragma once

#include "domain/stand_catalog.hpp"
#include "domain/test_operation_catalog.hpp"

#include <string>
#include <vector>

namespace lab {

// Режим / программа испытаний лаборатории (набор допустимых операций и стендов)
enum class LabProgramMode {
  // Демо: базовая механика — растяжение + кручение
  BasicMechanical,
  // Расширенная механика: + изгиб, усталость
  MechanicalExtended,
  // Устаревший алиас (брифинг 28.05): не в approvedProgramModes(); см. findProgramMode()
  ThermalCycle,
  // Термомеханика и комбинированные режимы (термика только здесь)
  Thermomechanical,
};

struct ProgramModeSpec {
    LabProgramMode mode{};
    std::string id;
    std::string nameRu;
    std::string descriptionRu;
    std::vector<TestOperationType> operations;
    std::vector<StandType> stands;
    std::vector<AuxiliaryZoneType> zones;
};

[[nodiscard]] const std::vector<ProgramModeSpec>& approvedProgramModes();
[[nodiscard]] const ProgramModeSpec* findProgramMode(LabProgramMode mode);
[[nodiscard]] std::string programModeNameRu(LabProgramMode mode);

}  // namespace lab
