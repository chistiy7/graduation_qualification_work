#pragma once

#include "domain/lab_equipment.hpp"
#include "domain/laboratory.hpp"
#include "domain/specimen.hpp"
#include "domain/test_stage.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace lab {

// Целевая функция: K (взвешенная, гл. 2) или суммарная стоимость в рублях (открытая постановка, брифинг)
enum class ObjectiveMode {
    WeightedK,      // min K = α₁T̃ + … (нормированные показатели)
    TotalCostRub,   // min C — сумма затрат в рублях без обезразмеривания
};

struct ObjectiveWeights {
    double alphaT = 0.25;
    double alphaC = 0.35;
    double alphaN = 0.20;
    double alphaL = 0.10;
    double alphaEta = 0.10;
};

// Описание задачи (гл. 3, брифинг 28.05): S, D, E, Z, G
struct ProblemDefinition {
    // S — партия образцов (не «множество»)
    std::vector<Specimen> specimens;

    // D — перечень испытательных операций
    std::vector<TestStage> operations;

    // E — номенклатура испытательного оборудования (стенды)
    std::vector<LabEquipment> equipment;

    // G, Z — сетка и вспомогательные зоны
    Laboratory laboratory;

    // Матрица требований Req(s,d): образец → операция
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> required;
    // Cap(e,d): стенд → операция
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> capable;
    // Pred(d₁,d₂): предшествование операций
    std::vector<std::pair<std::string, std::string>> precedence;

    double laborRatePerHour = 0.0;       // c_lab (руб/ч)
    double electricityTariffPerKwh = 0.0;  // тариф (руб/кВт·ч), если задано по мощности
    double gridCellSizeM = 2.0;            // ячейка 2×2 м (брифинг)
    double minutesPerGridStep = 1.0;     // норматив перемещения: 1 шаг сетки = N мин
    int gridRows = 0;                    // размеры сетки помещения (для оптимизатора размещения)
    int gridCols = 0;

    ObjectiveMode objectiveMode = ObjectiveMode::TotalCostRub;
    ObjectiveWeights weights;
};

// Разложение себестоимости по составляющим (брифинг 05.06)
struct CostBreakdown {
    double prepLabor = 0.0;      // подготовка образца (труд)
    double operations = 0.0;     // материалы + энергия по операциям (c_op + c_en)
    double operationLabor = 0.0; // труд по испытательным операциям
    double setup = 0.0;          // перенастройки (c_set)
    double amortization = 0.0;   // амортизация стендов
    double transport = 0.0;      // перемещение образцов по ячейкам (зависит от размещения)
    double cellPlacement = 0.0;  // размещение ячеек стендов

    [[nodiscard]] double total() const {
        return prepLabor + operations + operationLabor + setup + amortization + transport +
               cellPlacement;
    }
    [[nodiscard]] double withoutPlacement() const { return total() - cellPlacement; }
};

struct VariantMetrics {
    double T = 0.0;   // цикл: подготовка + операции + перенастройка + перемещение (мин)
    double C = 0.0;   // суммарные затраты (руб)
    int N = 0;        // перенастройки
    double L = 0.0;   // длина маршрута (шаги ячеек, штучная модель)
    double etaAvg = 0.0;
    double K = 0.0;   // целевая функция (K или C в зависимости от режима)
    double Tn = 1.0, Cn = 1.0, Nn = 1.0, Ln = 1.0, EtaN = 1.0;
    CostBreakdown cost;  // разложение C по составляющим
};

struct ComparisonRow {
    VariantMetrics baseline;
    VariantMetrics optimized;
    double timeReductionPct = 0.0;
    double costReductionPct = 0.0;
    double objectiveReductionPct = 0.0;
};

}  // namespace lab
