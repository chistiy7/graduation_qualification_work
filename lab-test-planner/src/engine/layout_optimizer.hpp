#pragma once

#include "model/problem.hpp"

#include <string>

namespace lab {

struct LayoutOptimizationResult {
    ProblemDefinition problem;  // постановка с оптимальным размещением стендов
    double bestCost = 0.0;      // минимальная C на найденной планировке
    bool bruteForce = false;    // использован полный перебор (иначе эвристика)
    long long evaluated = 0;    // число просмотренных вариантов размещения
    std::string note;
};

// Оптимальное размещение стендов на сетке gridRows×gridCols по min C
// (перебор для малых сеток, эвристика — для больших). Брифинг 05.06.
[[nodiscard]] LayoutOptimizationResult optimizeLayout(const ProblemDefinition& base);

}  // namespace lab
