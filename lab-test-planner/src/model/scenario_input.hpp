#pragma once

#include "domain/test_operation_catalog.hpp"
#include "model/scenario_bundle.hpp"

#include <vector>

namespace lab {

// Пользовательский ввод демонстрационной версии (брифинг 05.06):
//   площадь помещения, размер партии, требуемые типы испытаний, тарифы.
struct ScenarioInput {
    double roomWidthM = 6.0;
    double roomLengthM = 6.0;
    double cellSizeM = 2.0;                 // сторона ячейки (по умолчанию 2 м)
    int batchSize = 1;                      // число образцов в партии
    std::vector<TestOperationType> testTypes;  // выбранные виды испытаний (на всю партию)
    double laborRatePerHour = 1000.0;
    double energyTariffPerKwh = 0.0;
};

[[nodiscard]] int gridRowsFromInput(const ScenarioInput& in);
[[nodiscard]] int gridColsFromInput(const ScenarioInput& in);

// Сборка постановки БЕЗ размещения: стенды без координат (их расставит оптимизатор).
[[nodiscard]] ScenarioBundle buildFromInput(const ScenarioInput& in);

// Виды испытаний, доступные пользователю в демонстрационной версии.
[[nodiscard]] const std::vector<TestOperationType>& selectableTestTypes();
[[nodiscard]] std::string testTypeNameRu(TestOperationType type);

}  // namespace lab
