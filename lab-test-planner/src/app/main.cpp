#include "app/interactive.hpp"
#include "app/pipeline.hpp"
#include "app/preprocessor.hpp"
#include "app/scenario_runner.hpp"
#include "model/scenario_input.hpp"

#include <iostream>
#include <string>

namespace {

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

    std::cout << lab::formatScenarioRunOutput(out);
}

void printUsage() {
    std::cout << "LabPlanner — испытательная лаборатория\n\n"
              << "  LabPlanner --cli                   интерактивный ввод параметров\n"
              << "  LabPlanner --debug demo_simple     отладочный пресет (2 образца)\n"
              << "  LabPlanner --debug demo_two_specimens  отладочный пресет (2 образца)\n"
              << "  LabPlanner --debug demo_8_types_80   80 образцов, 8 видов × 10\n"
              << "  lab-test-planner-gui.app           Qt GUI (macOS)\n";
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
            std::cerr << "Неизвестный аргумент: " << arg << "\n\n";
            printUsage();
            return 1;
        }

        printUsage();
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Fatal: " << ex.what() << "\n";
        return 1;
    }
}
