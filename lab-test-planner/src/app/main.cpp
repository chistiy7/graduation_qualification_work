#include "app/interactive.hpp"
#include "app/pipeline.hpp"
#include "app/preprocessor.hpp"
#include "engine/lab_optimiser.hpp"
#include "model/scenario_bundle.hpp"
#include "ui/gui_app.hpp"

#include <iostream>
#include <string>

namespace {

// Отладочные пресеты (не основной сценарий): фиксированная партия.
void runDebugPreset(const std::string& scenarioId) {
    lab::Preprocessor preprocessor;
    lab::Pipeline pipeline;

    lab::ScenarioBundle bundle = preprocessor.loadBuiltin(scenarioId);
    const auto out = pipeline.run(bundle, true);
    lab::LabOptimiser{}.printReport(out.result);
    std::cout << "\nСценарий (debug): " << bundle.name << "\n";
    std::cout << "Отчёт: " << out.reportPath << "\nCSV: " << out.csvPath << "\n";
}

void printUsage() {
    std::cout << "LabPlanner — испытательная лаборатория\n\n"
              << "  LabPlanner                         GUI\n"
              << "  LabPlanner --cli                   интерактивный ввод параметров\n"
              << "  LabPlanner --debug demo_simple     отладочный пресет (1 образец)\n"
              << "  LabPlanner --debug demo_two_specimens  отладочный пресет (2 образца)\n";
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
