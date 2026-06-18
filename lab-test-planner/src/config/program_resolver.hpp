#pragma once

#include "config/config_overrides.hpp"
#include "domain/lab_equipment.hpp"
#include "domain/test_stage.hpp"
#include "domain/test_program_catalog.hpp"

namespace lab {

struct StandEconomicsDefaults {
    double setupCost = 200.0;
    double amortPerHour = 2000.0;
    double fundTimeMin = 240.0;
    double cellPlacementCost = 5000.0;
};

struct OperationEconomicsDefaults {
    double costOp = 200.0;
    double laborHours = 0.20;
};

[[nodiscard]] StandEconomicsDefaults defaultStandEconomics(StandType type);
[[nodiscard]] OperationEconomicsDefaults defaultOperationEconomics(TestOperationType type);

[[nodiscard]] TestStage buildStageFromProgram(TestOperationType type,
                                              const LabConfigOverrides* overrides = nullptr);

[[nodiscard]] LabEquipment buildEquipmentFromProgram(const TestProgramDef& program,
                                                     StandType standType,
                                                     const LabConfigOverrides* overrides);

[[nodiscard]] TestProgramDef programForOperation(TestOperationType type,
                                                 const LabConfigOverrides* overrides = nullptr);

}  // namespace lab
