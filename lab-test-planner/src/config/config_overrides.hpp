#pragma once

#include "domain/test_program_catalog.hpp"

#include <optional>
#include <string>
#include <vector>

namespace lab {

// Частичное переопределение техкарты (не заданное поле → значение из каталога).
struct TestProgramOverride {
    std::string equipmentId;
    std::string operationId;
    std::optional<ProgramTimeModel> timeModel;
    std::optional<double> normativeTimeMin;
    std::optional<double> modePowerKw;
    std::optional<double> setupTimeMin;
    std::optional<double> setupPowerKw;
    std::optional<bool> deformationEnergyUsed;
    std::optional<int> cyclesCount;
    std::optional<double> frequencyHz;
    std::optional<double> cycleTimeMin;
    std::optional<double> heatingTimeMin;
    std::optional<double> holdingTimeMin;
    std::optional<double> coolingTimeMin;
    std::optional<double> sigmaMpa;
    std::optional<double> deltaT_C;
};

struct LabConfigOverrides {
    std::vector<TestProgramOverride> programs;
};

[[nodiscard]] TestProgramDef applyProgramOverride(const TestProgramDef& base,
                                                  const TestProgramOverride& patch);
[[nodiscard]] TestProgramDef resolveTestProgram(const TestProgramDef& base,
                                                const LabConfigOverrides* overrides);

}  // namespace lab
