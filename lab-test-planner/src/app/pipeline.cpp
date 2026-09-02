#include "app/pipeline.hpp"

#include "engine/layout_optimizer.hpp"
#include "util/run_timer.hpp"

namespace lab {

namespace {

void applyLayoutIfNeeded(ScenarioBundle& bundle, PipelineOutput& out) {
    auto& p = bundle.problem;
    if (!p.laboratory.cells().empty()) return;
    if (p.equipment.empty()) return;
    // gridRows/gridCols<=0 — допустимо: режим минимальной площади (optimizeLayout
    // сам нарастит сетку до первой допустимой раскладки).

    const auto layout = optimizeLayout(p);
    p = layout.problem;
    out.layoutNote = layout.note;
    out.layoutEvaluated = layout.evaluated;
    out.autoArea = layout.autoArea;
    out.gridRows = layout.gridRows;
    out.gridCols = layout.gridCols;
    out.roomAreaM2 = layout.roomAreaM2;
}

}  // namespace

PipelineOutput Pipeline::run(ScenarioBundle bundle, bool exportFiles) const {
    RunTimer timer;

    PipelineOutput out;
    applyLayoutIfNeeded(bundle, out);

    preprocessor_.ensureValid(bundle);
    out.bundle = std::move(bundle);
    out.result = optimiser_.run(out.bundle);
    if (exportFiles) {
        out.reportPath = postprocessor_.exportTextReport(out.bundle, out.result);
        out.csvPath = postprocessor_.exportCsv(out.result);
    }

    out.elapsedMs = timer.elapsedMs();
    printRunTimeMs(out.elapsedMs);
    return out;
}

PipelineOutput Pipeline::runFromInput(const ScenarioInput& in, bool exportFiles) const {
    return run(buildFromInput(in), exportFiles);
}

}  // namespace lab
