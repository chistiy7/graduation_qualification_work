#pragma once

#include "domain/test_operation_catalog.hpp"

#include <optional>
#include <string>
#include <vector>

namespace lab {

enum class ProgramTimeModel {
    RegulatedTime,
    CycleProgram,
    ThermalProgram,
    ThermalCycleProgram,
    ThermomechanicalProgram,
};

// Техкарта программы испытания (equipmentId + operationId).
struct TestProgramDef {
    std::string equipmentId;
    std::string operationId;
    TestOperationType operationType{};
    std::string programName;
    ProgramTimeModel timeModel = ProgramTimeModel::RegulatedTime;
    double normativeTimeMin = 0.0;
    double modePowerKw = 0.0;
    double setupTimeMin = 0.0;
    double setupPowerKw = 0.0;
    std::string stopCriterion;
    bool deformationEnergyUsed = false;
    std::string sourceType;
    // Параметры CycleProgram / ThermalCycleProgram / ThermomechanicalProgram
    int cyclesCount = 0;
    double frequencyHz = 0.0;
    double cycleTimeMin = 0.0;
    // ThermalProgram (если заданы — t = сумма; иначе normativeTimeMin)
    double heatingTimeMin = 0.0;
    double holdingTimeMin = 0.0;
    double coolingTimeMin = 0.0;
    // Контроль энергии деформации (не для времени)
    double sigmaMpa = 0.0;
    double deltaT_C = 0.0;
};

[[nodiscard]] const std::vector<TestProgramDef>& approvedTestPrograms();
[[nodiscard]] const TestProgramDef* findTestProgram(const std::string& equipmentId,
                                                    const std::string& operationId);
[[nodiscard]] const TestProgramDef* findTestProgramForOperation(TestOperationType type);
[[nodiscard]] double resolveProgramTimeMin(const TestProgramDef& program);
[[nodiscard]] std::string programTimeModelName(ProgramTimeModel model);

}  // namespace lab
