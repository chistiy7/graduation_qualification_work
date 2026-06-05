#include "app/interactive.hpp"

#include "app/paths.hpp"
#include "app/postprocessor.hpp"
#include "app/preprocessor.hpp"
#include "engine/lab_optimiser.hpp"
#include "engine/layout_map.hpp"
#include "engine/layout_optimizer.hpp"
#include "model/scenario_input.hpp"

#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace lab {

namespace {

double askDouble(const std::string& prompt, double fallback) {
    std::cout << prompt << " [" << fallback << "]: ";
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) return fallback;
    try {
        return std::stod(line);
    } catch (...) {
        return fallback;
    }
}

int askInt(const std::string& prompt, int fallback) {
    std::cout << prompt << " [" << fallback << "]: ";
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) return fallback;
    try {
        return std::stoi(line);
    } catch (...) {
        return fallback;
    }
}

std::vector<TestOperationType> askTestTypes() {
    const auto& types = selectableTestTypes();
    std::cout << "\nДоступные виды испытаний:\n";
    for (size_t i = 0; i < types.size(); ++i) {
        std::cout << "  " << (i + 1) << ") " << testTypeNameRu(types[i]) << "\n";
    }
    std::cout << "Выберите номера через пробел (например: 1 2): ";

    std::string line;
    std::getline(std::cin, line);
    std::vector<TestOperationType> chosen;
    std::istringstream ss(line);
    int idx = 0;
    while (ss >> idx) {
        if (idx >= 1 && idx <= static_cast<int>(types.size())) {
            chosen.push_back(types[idx - 1]);
        }
    }
    if (chosen.empty()) {
        chosen = {TestOperationType::Tension, TestOperationType::Torsion};
        std::cout << "Ничего не выбрано — взяты растяжение и кручение.\n";
    }
    return chosen;
}

}  // namespace

int runInteractive() {
    std::cout << "=== LabPlanner — расчёт испытательной лаборатории ===\n";
    std::cout << "Введите параметры (Enter — значение по умолчанию).\n\n";

    ScenarioInput in;
    in.roomWidthM = askDouble("Ширина помещения, м", in.roomWidthM);
    in.roomLengthM = askDouble("Длина помещения, м", in.roomLengthM);
    in.cellSizeM = askDouble("Сторона ячейки сетки, м", in.cellSizeM);
    in.batchSize = askInt("Размер партии образцов", in.batchSize);
    in.laborRatePerHour = askDouble("Ставка труда, руб/ч", in.laborRatePerHour);
    in.testTypes = askTestTypes();

    const int rows = gridRowsFromInput(in);
    const int cols = gridColsFromInput(in);
    std::cout << "\nСетка помещения: " << rows << " x " << cols << " ячеек ("
              << in.cellSizeM << " м/ячейка).\n";

    try {
        ScenarioBundle bundle = buildFromInput(in);

        const auto layout = optimizeLayout(bundle.problem);
        bundle.problem = layout.problem;

        Preprocessor preprocessor;
        preprocessor.ensureValid(bundle);

        LabOptimiser optimiser;
        const auto result = optimiser.run(bundle);

        std::cout << "\n" << LabOptimiser::formatReport(result);
        std::cout << "\nРазмещение: " << layout.note << " (вариантов просмотрено: "
                  << layout.evaluated << ")\n";
        std::cout << "\n" << renderLayoutMatrix(bundle.problem, result.optimizedRoute);

        Postprocessor post;
        const auto report = post.exportTextReport(bundle, result);
        const auto csv = post.exportCsvComparison(result);
        std::cout << "\nОтчёт: " << report << "\nCSV: " << csv << "\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "\nОшибка: " << ex.what() << "\n";
        return 1;
    }
}

}  // namespace lab
