#include "app/pipeline.hpp"

namespace lab {

PipelineOutput Pipeline::run(ScenarioBundle bundle, bool exportFiles) const {
    preprocessor_.ensureValid(bundle);
    PipelineOutput out;
    out.bundle = std::move(bundle);
    out.result = optimiser_.run(out.bundle);
    if (exportFiles) {
        out.reportPath = postprocessor_.exportTextReport(out.bundle, out.result);
        out.csvPath = postprocessor_.exportCsvComparison(out.result);
    }
    return out;
}

}  // namespace lab
