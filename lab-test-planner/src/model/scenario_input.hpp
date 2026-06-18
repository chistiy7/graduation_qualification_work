#pragma once

#include "config/config_overrides.hpp"
#include "domain/test_operation_catalog.hpp"
#include "model/scenario_bundle.hpp"

#include <filesystem>
#include <vector>

namespace lab {

// Группа образцов: часть партии с одним видом испытания (одиночным или комбинированным).
//   Пример: партия 100 → группа 35 (растяжение) + группа 65 (изгиб+кручение).
//   Комбинированные нагрузки уже закодированы как единый TestOperationType
//   (TensionPlusTorsion, BendingPlusTorsion, Thermomechanical).
struct SpecimenGroup {
    int count = 0;
    TestOperationType testType{};
};

// Пользовательский ввод (брифинг 05.06):
//   площадь помещения, общее число образцов, разбиение партии на группы по типам испытаний.
struct ScenarioInput {
    double roomAreaM2 = 36.0;                // площадь помещения, м²
    double cellSizeM = 2.0;                  // сторона унифицированной ячейки стенда (2 м)
    int batchSize = 0;                       // общее число образцов (= сумме групп)
    std::vector<SpecimenGroup> groups;       // разбиение партии по видам испытаний
    double laborRatePerHour = 1000.0;
    double energyTariffPerKwh = 6.5;        // тариф, руб/кВт·ч (ENERGY_TARIFF_DEFAULT_RUB_KWH)
    // Объём рабочей части образца, м³: V = π·(d₀/2)²·l₀ (ГОСТ 1497-84).
    double specimenVolumeM3 = 3.93e-6;      // по умолчанию: d₀=10 мм, l₀=50 мм
    // Переопределение техкарт (код) и/или JSON-файл с частичными правками.
    LabConfigOverrides configOverrides;
    std::filesystem::path overrideConfigPath;
};

// Число ячеек сетки из площади (площадь / площадь ячейки), сетка близка к квадрату.
[[nodiscard]] int gridCellCount(const ScenarioInput& in);
[[nodiscard]] int gridRowsFromInput(const ScenarioInput& in);
[[nodiscard]] int gridColsFromInput(const ScenarioInput& in);

// Сборка постановки БЕЗ размещения: стенды без координат (их расставит оптимизатор).
[[nodiscard]] ScenarioBundle buildFromInput(const ScenarioInput& in);

// Пресет: 80 образцов — по 10 на каждый из 8 видов испытаний (см. data/scenarios/demo_8_types_80.json).
[[nodiscard]] ScenarioInput makeScenario8Types80Input();
[[nodiscard]] ScenarioBundle buildScenario8Types80();

// Виды испытаний, доступные пользователю в демонстрационной версии.
[[nodiscard]] const std::vector<TestOperationType>& selectableTestTypes();
[[nodiscard]] std::string testTypeNameRu(TestOperationType type);

}  // namespace lab
