#pragma once

#include "app/pipeline.hpp"
#include "model/scenario_input.hpp"

#include <string>

namespace lab {

// Текстовое представление результата расчёта (как в CLI после Pipeline).
[[nodiscard]] std::string formatScenarioRunOutput(const PipelineOutput& out,
                                                  const ScenarioInput* inputForGridInfo = nullptr);

[[nodiscard]] PipelineOutput runUserScenario(const ScenarioInput& in, bool exportFiles = true);

}  // namespace lab
