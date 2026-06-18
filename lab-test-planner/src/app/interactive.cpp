#include "app/interactive.hpp"

#include "app/pipeline.hpp"
#include "engine/lab_optimiser.hpp"
#include "engine/layout_map.hpp"
#include "model/scenario_input.hpp"

#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace lab {

namespace {

// Читает строку; завершает программу при EOF
bool readLine(std::string& out) {
    if (!std::getline(std::cin, out)) {
        std::cerr << "\nВвод прерван (EOF).\n";
        std::exit(1);
    }
    return true;
}

// Обязательный ввод вещественного числа > 0; повторяет до корректного ввода.
double requirePositiveDouble(const std::string& prompt) {
    while (true) {
        std::cout << prompt << ": ";
        std::string line;
        readLine(line);
        if (line.empty()) {
            std::cout << "  ! Значение обязательно. Введите положительное число.\n";
            continue;
        }
        double val = 0.0;
        try {
            size_t pos = 0;
            val = std::stod(line, &pos);
            if (pos != line.size()) {
                std::cout << "  ! Введите число без лишних символов.\n";
                continue;
            }
        } catch (...) {
            std::cout << "  ! Не удалось распознать число. Попробуйте ещё раз.\n";
            continue;
        }
        if (val <= 0.0) {
            std::cout << "  ! Число должно быть больше нуля.\n";
            continue;
        }
        return val;
    }
}

// Обязательный ввод целого числа в диапазоне [minVal, maxVal]
int requireInt(const std::string& prompt, int minVal, int maxVal) {
    while (true) {
        std::cout << prompt << " (" << minVal << "–" << maxVal << "): ";
        std::string line;
        readLine(line);
        if (line.empty()) {
            std::cout << "  ! Значение обязательно.\n";
            continue;
        }
        int val = 0;
        try {
            size_t pos = 0;
            val = std::stoi(line, &pos);
            if (pos != line.size()) {
                std::cout << "  ! Введите целое число без лишних символов.\n";
                continue;
            }
        } catch (...) {
            std::cout << "  ! Не удалось распознать целое число. Попробуйте ещё раз.\n";
            continue;
        }
        if (val < minVal || val > maxVal) {
            std::cout << "  ! Допустимый диапазон: " << minVal << " – " << maxVal << ".\n";
            continue;
        }
        return val;
    }
}

// Выбор одного вида испытания из каталога; повторяет до корректного ввода
TestOperationType requireTestType(const std::string& groupLabel) {
    const auto& types = selectableTestTypes();
    while (true) {
        std::cout << "\n  Вид испытания для " << groupLabel << ":\n";
        for (size_t i = 0; i < types.size(); ++i) {
            std::cout << "    " << (i + 1) << ") " << testTypeNameRu(types[i]) << "\n";
        }
        std::cout << "  Выберите номер (1–" << types.size() << "): ";
        std::string line;
        readLine(line);
        if (line.empty()) {
            std::cout << "  ! Введите номер из списка.\n";
            continue;
        }
        int idx = 0;
        try {
            size_t pos = 0;
            idx = std::stoi(line, &pos);
            if (pos != line.size()) {
                std::cout << "  ! Введите только цифру.\n";
                continue;
            }
        } catch (...) {
            std::cout << "  ! Введите целое число.\n";
            continue;
        }
        if (idx < 1 || idx > static_cast<int>(types.size())) {
            std::cout << "  ! Номер вне диапазона 1–" << types.size() << ".\n";
            continue;
        }
        return types[idx - 1];
    }
}

// Вопрос yes/no (y/n); повторяет до корректного ввода
bool requireYesNo(const std::string& prompt, bool defaultYes) {
    const std::string hint = defaultYes ? " [y/n, Enter=y]: " : " [y/n, Enter=n]: ";
    while (true) {
        std::cout << prompt << hint;
        std::string line;
        readLine(line);
        if (line.empty()) return defaultYes;
        if (line[0] == 'y' || line[0] == 'Y') return true;
        if (line[0] == 'n' || line[0] == 'N') return false;
        std::cout << "  ! Введите y (да) или n (нет).\n";
    }
}

std::vector<SpecimenGroup> askSpecimenGroups(int totalBatch) {
    std::vector<SpecimenGroup> groups;
    int assigned = 0;

    std::cout << "\n--- Распределение партии по видам испытаний ---\n";
    std::cout << "Всего образцов: " << totalBatch << "\n";
    std::cout << "Каждая группа — одно испытание (простое или комбинированное).\n";
    std::cout << "Сумма образцов по группам должна равняться " << totalBatch << ".\n";

    while (assigned < totalBatch) {
        const int remaining = totalBatch - assigned;
        const int groupNum = static_cast<int>(groups.size()) + 1;
        const std::string label =
            "группы " + std::to_string(groupNum) +
            " (осталось " + std::to_string(remaining) + ")";

        std::cout << "\nГруппа " << groupNum
                  << " (осталось распределить: " << remaining << ")\n";

        const int count = requireInt("  Число образцов в группе", 1, remaining);

        SpecimenGroup g;
        g.count = count;
        g.testType = requireTestType(label);
        groups.push_back(g);
        assigned += count;

        if (assigned < totalBatch) {
            if (!requireYesNo("Добавить ещё одну группу?", true)) {
                std::cout << "  Оставшиеся " << (totalBatch - assigned)
                          << " образцов добавлены с тем же видом ("
                          << testTypeNameRu(g.testType) << ").\n";
                SpecimenGroup rest;
                rest.count = totalBatch - assigned;
                rest.testType = g.testType;
                groups.push_back(rest);
                assigned = totalBatch;
            }
        }
    }
    return groups;
}

}  // namespace

int runInteractive() {
    std::cout << "=== LabPlanner — расчёт испытательной лаборатории ===\n\n";

    ScenarioInput in;

    in.roomAreaM2 = requirePositiveDouble("Площадь помещения, м²");
    // cellSizeM = 2.0 — унифицированный размер стенда, не запрашивается
    in.batchSize = requireInt("Общее число образцов в партии", 1,
                              std::numeric_limits<int>::max() / 2);
    in.laborRatePerHour = requirePositiveDouble("Ставка труда, руб/ч");
    // Объём рабочей части образца — нужен для C_энерг = σ²·V/(2E·η)·тариф (§3.2).
    // Подсказка: V = π·(d₀/2)²·l₀; для d₀=10 мм, l₀=50 мм → V ≈ 3.93e-6 м³ (3.93 см³).
    std::cout << "  (Подсказка: V = π·(d/2)²·l; d₀=10 мм, l₀=50 мм → ≈3.93e-6 м³)\n";
    in.specimenVolumeM3 = requirePositiveDouble("Объём рабочей части образца, м³");
    in.energyTariffPerKwh = requirePositiveDouble("Тариф электроэнергии, руб/кВт·ч");

    in.groups = askSpecimenGroups(in.batchSize);

    try {
        const int rows = gridRowsFromInput(in);
        const int cols = gridColsFromInput(in);
        const int cells = rows * cols;
        std::cout << "\nСетка помещения: " << rows << " × " << cols << " = " << cells
                  << " ячеек (ячейка 2×2 м, стенд = 1 ячейка).\n";
        std::cout << "Запретная зона вокруг каждого стенда: 1 ячейка.\n";

        Pipeline pipeline;
        const auto out = pipeline.runFromInput(in);

        std::cout << "\n" << LabOptimiser::formatReport(out.result, out.bundle.problem);
        if (out.layoutEvaluated > 0) {
            std::cout << "\nРазмещение: " << out.layoutNote << " (вариантов просмотрено: "
                      << out.layoutEvaluated << ")\n";
        }
        std::cout << "\n"
                  << renderLayoutMatrix(out.bundle.problem, out.result.route,
                                        out.result.metrics.T_cycle);
        std::cout << "\nОтчёт: " << out.reportPath << "\nCSV: " << out.csvPath << "\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "\nОшибка: " << ex.what() << "\n";
        return 1;
    }
}

}  // namespace lab
