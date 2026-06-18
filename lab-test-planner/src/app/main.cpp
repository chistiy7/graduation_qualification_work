#include "app/interactive.hpp"
#include "app/pipeline.hpp"
#include "app/preprocessor.hpp"
#include "engine/lab_optimiser.hpp"
#include "engine/layout_map.hpp"
#include "model/scenario_bundle.hpp"
#include "ui/gui_app.hpp"

#include <iostream>
#include <string>

namespace {

// Отладочные пресеты. Сценарии из ScenarioInput (demo_8_types_80) — тот же Pipeline, что и --cli.
void runDebugPreset(const std::string& scenarioId) {
    lab::Pipeline pipeline;
    lab::PipelineOutput out;

    if (scenarioId == "8_types_80" || scenarioId == "demo_8_types_80" ||
        scenarioId == "8x10") {
        out = pipeline.run(lab::buildScenario8Types80());
    } else {
        lab::Preprocessor preprocessor;
        out = pipeline.run(preprocessor.loadBuiltin(scenarioId));
    }

    lab::LabOptimiser{}.printReport(out.result, out.bundle.problem);
    if (out.layoutEvaluated > 0) {
        std::cout << "\nРазмещение: " << out.layoutNote << " (вариантов просмотрено: "
                  << out.layoutEvaluated << ")\n";
        std::cout << "\n"
                  << lab::renderLayoutMatrix(out.bundle.problem, out.result.route,
                                           out.result.metrics.T_cycle);
    }
    std::cout << "\nСценарий (debug): " << out.bundle.name << "\n";
    std::cout << "Отчёт: " << out.reportPath << "\nCSV: " << out.csvPath << "\n";
}

void printUsage() {
    std::cout << "LabPlanner — испытательная лаборатория\n\n"
              << "  LabPlanner                         GUI\n"
              << "  LabPlanner --cli                   интерактивный ввод параметров\n"
              << "  LabPlanner --debug demo_simple     отладочный пресет (2 образца)\n"
              << "  LabPlanner --debug demo_two_specimens  отладочный пресет (2 образца)\n"
              << "  LabPlanner --debug demo_8_types_80   80 образцов, 8 видов × 10\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        if (argc >= 2) {
            const std::string arg = argv[1];
            if (arg == "--help" || arg == "-h") {
                printUsage();
                return 0;
            }
            if (arg == "--cli") {
                return lab::runInteractive();
            }
            if (arg == "--debug") {
                const std::string id = argc >= 3 ? argv[2] : "demo_simple";
                runDebugPreset(id);
                return 0;
            }
        }
        lab::ui::runGui();
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Fatal: " << ex.what() << "\n";
        return 1;
    }
}
