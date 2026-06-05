#pragma once

#include "domain/test_route.hpp"
#include "model/problem.hpp"

#include <string>

namespace lab {

// Карта размещения лаборатории (брифинг 05.06): матрица ячеек с кодами
//   0 — пустая ячейка; 1..K — тип испытательного стенда; R — ячейка маршрута.
// Возвращает текстовый блок: легенда + матрица.
[[nodiscard]] std::string renderLayoutMatrix(const ProblemDefinition& problem,
                                             const TestRoute& route);

}  // namespace lab
