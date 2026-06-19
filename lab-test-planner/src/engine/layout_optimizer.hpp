#pragma once

#include "model/problem.hpp"

#include <optional>
#include <string>

namespace lab {

struct LayoutOptimizationResult {
    ProblemDefinition problem;  // постановка с оптимальным размещением стендов
    double bestCost = 0.0;      // минимальная C на найденной планировке
    bool bruteForce = false;    // использован полный перебор (иначе эвристика)
    long long evaluated = 0;    // число просмотренных вариантов размещения
    bool autoArea = false;      // площадь не задана → спроектирована от минимума
    int gridRows = 0;           // итоговая сетка
    int gridCols = 0;
    double roomAreaM2 = 0.0;     // площадь итоговой сетки, м² (rows·cols·ячейка)
    std::string note;
};

// Размещение стендов на конкретной сетке gridRows×gridCols по min C (перебор для
// малых сеток, эвристика — для больших). Возвращает nullopt, если допустимой
// раскладки на этой сетке нет (стенды+зоны безопасности не помещаются).
[[nodiscard]] std::optional<LayoutOptimizationResult> placeOnGrid(const ProblemDefinition& base,
                                                                  int rows, int cols);

// Рациональное размещение стендов. Если сетка задана (gridRows,gridCols>0) —
// раскладка на ней (бросает при невозможности). Если НЕ задана (<=0) — САПР
// проектирует от МИНИМАЛЬНОЙ необходимой площади: наращивает сетку до первой
// допустимой раскладки. Брифинг 05.06.
[[nodiscard]] LayoutOptimizationResult optimizeLayout(const ProblemDefinition& base);

}  // namespace lab
