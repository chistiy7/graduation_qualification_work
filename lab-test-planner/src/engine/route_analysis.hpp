#pragma once

#include "domain/test_route.hpp"
#include "model/problem.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace lab {

struct RouteAnalysis {
    int setupCount = 0;
    double setupTimeMin = 0.0;
    double setupCost = 0.0;
    double routeLengthSteps = 0.0;
    double moveTimeMin = 0.0;
    std::unordered_map<std::string, double> busyMinutesByEquipment;
    std::unordered_map<std::string, double> testMinByEquipment;
    std::unordered_map<std::string, double> setupMinByEquipment;
    std::unordered_map<std::string, double> setupCostByEquipment;
    double energySetupCost = 0.0;
};

class RouteAnalyzer {
public:
    [[nodiscard]] RouteAnalysis analyze(const ProblemDefinition& problem,
                                      const TestRoute& route) const;
};

}  // namespace lab
