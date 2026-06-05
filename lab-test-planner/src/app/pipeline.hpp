#pragma once

#include "app/postprocessor.hpp"
#include "app/preprocessor.hpp"
#include "engine/lab_optimiser.hpp"
#include "model/scenario_bundle.hpp"

#include <filesystem>

namespace lab {

struct PipelineOutput {
    ScenarioBundle bundle;
    OptimisationResult result;
    std::filesystem::path reportPath;
    std::filesystem::path csvPath;
};

// Связка: препроцессор → ядро → постпроцессор
class Pipeline {
public:
    [[nodiscard]] PipelineOutput run(ScenarioBundle bundle, bool exportFiles = true) const;

private:
    Preprocessor preprocessor_;
    LabOptimiser optimiser_;
    Postprocessor postprocessor_;
};

}  // namespace lab
