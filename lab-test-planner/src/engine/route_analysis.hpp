#pragma once

#include "domain/test_route.hpp"
#include "model/problem.hpp"

#include <string>
#include <unordered_map>

namespace lab {

// Результат анализа маршрута: все производные метрики из данных задачи
struct RouteAnalysis {
    int setupCount = 0;
    double setupTimeMin = 0.0;
    double setupCost = 0.0;
    double routeLengthSteps = 0.0;
    double moveTimeMin = 0.0;
    std::unordered_map<std::string, double> busyMinutesByEquipment;
};

class RouteAnalyzer {
public:
    [[nodiscard]] RouteAnalysis analyze(const ProblemDefinition& problem,
                                      const TestRoute& route) const;
};

}  // namespace lab
