#pragma once

#include "app/postprocessor.hpp"
#include "app/preprocessor.hpp"
#include "engine/lab_optimiser.hpp"
#include "model/scenario_bundle.hpp"
#include "model/scenario_input.hpp"

#include <filesystem>
#include <string>

namespace lab {

struct PipelineOutput {
    ScenarioBundle bundle;
    PlanResult result;
    std::filesystem::path reportPath;
    std::filesystem::path csvPath;
    std::string layoutNote;
    long long layoutEvaluated = 0;
    // Раскладка: режим минимальной площади (площадь не задавалась) и итоговая сетка.
    bool autoArea = false;
    int gridRows = 0;
    int gridCols = 0;
    double roomAreaM2 = 0.0;
    double elapsedMs = -1.0;
};

// Связка: размещение (если нужно) → препроцессор → ядро → постпроцессор
class Pipeline {
public:
    [[nodiscard]] PipelineOutput run(ScenarioBundle bundle, bool exportFiles = true) const;
    [[nodiscard]] PipelineOutput runFromInput(const ScenarioInput& in,
                                              bool exportFiles = true) const;

private:
    Preprocessor preprocessor_;
    LabOptimiser optimiser_;
    Postprocessor postprocessor_;
};

}  // namespace lab
