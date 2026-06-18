#pragma once

#include "app/pipeline.hpp"
#include "model/scenario_input.hpp"

#include <QString>
#include <optional>

namespace lab::qtui {

struct CalculationJobResult {
    bool ok = false;
    QString error;
    PipelineOutput output;
    std::optional<ScenarioInput> input;
};

// Выполняется в фоновом потоке (QtConcurrent).
[[nodiscard]] CalculationJobResult executeCalculation(bool customMode,
                                                      const QString& presetId,
                                                      const ScenarioInput& customInput);

[[nodiscard]] CalculationJobResult executeJsonScenario(const QString& jsonPath);

}  // namespace lab::qtui
