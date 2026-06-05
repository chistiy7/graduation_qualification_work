#pragma once

#include "engine/lab_optimiser.hpp"
#include "model/scenario_bundle.hpp"

#include <filesystem>

namespace lab {

// Постпроцессор: отчёты и экспорт для анализа / главы 3–4
class Postprocessor {
public:
    [[nodiscard]] std::filesystem::path exportTextReport(const ScenarioBundle& bundle,
                                                       const OptimisationResult& result) const;

    [[nodiscard]] std::filesystem::path exportCsvComparison(
        const OptimisationResult& result) const;
};

}  // namespace lab
