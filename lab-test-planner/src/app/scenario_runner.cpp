#include "app/scenario_runner.hpp"

#include "engine/lab_optimiser.hpp"
#include "engine/layout_map.hpp"

#include <sstream>

namespace lab {

std::string formatScenarioRunOutput(const PipelineOutput& out,
                                    const ScenarioInput* inputForGridInfo) {
    std::ostringstream ss;

    if (inputForGridInfo) {
        ss << formatGridSetupNote(*inputForGridInfo) << "\n";
    }

    ss << LabOptimiser::formatReport(out.result, out.bundle.problem);
    if (out.layoutEvaluated > 0) {
        ss << "\nРазмещение: " << out.layoutNote << " (вариантов просмотрено: "
           << out.layoutEvaluated << ")\n";
    }
    ss << "\n"
       << renderLayoutMatrix(out.bundle.problem, out.result.route, out.result.metrics.T_cycle);
    ss << "\nОтчёт: " << out.reportPath.string() << "\n";
    ss << "CSV: " << out.csvPath.string() << "\n";
    if (out.elapsedMs >= 0.0) {
        ss << "Время работы: " << out.elapsedMs << " мс\n";
    }
    return ss.str();
}

PipelineOutput runUserScenario(const ScenarioInput& in, bool exportFiles) {
    Pipeline pipeline;
    return pipeline.runFromInput(in, exportFiles);
}

}  // namespace lab
