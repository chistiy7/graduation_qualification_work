#pragma once

#include "domain/test_route.hpp"
#include "model/problem.hpp"

namespace lab {

class MetricsEngine {
public:
    [[nodiscard]] VariantMetrics compute(const ProblemDefinition& problem,
                                         const TestRoute& route) const;

    [[nodiscard]] ComparisonRow compare(const ProblemDefinition& problem,
                                      const TestRoute& baselineRoute,
                                      const TestRoute& optimizedRoute) const;

private:
    [[nodiscard]] double operationTimeSum(const ProblemDefinition& problem,
                                          const TestRoute& route) const;
    [[nodiscard]] double operationCostSum(const ProblemDefinition& problem,
                                          const TestRoute& route) const;
    [[nodiscard]] double laborCost(const ProblemDefinition& problem,
                                   const TestRoute& route) const;
    [[nodiscard]] double prepLaborCost(const ProblemDefinition& problem) const;
    [[nodiscard]] double operationLaborCost(const ProblemDefinition& problem,
                                            const TestRoute& route) const;
    [[nodiscard]] double amortizationCost(
        const ProblemDefinition& problem,
        const std::unordered_map<std::string, double>& busyByEquipment) const;
    [[nodiscard]] double averageLoad(
        const ProblemDefinition& problem,
        const std::unordered_map<std::string, double>& busyByEquipment) const;
    [[nodiscard]] double objectiveK(const ObjectiveWeights& w, const VariantMetrics& m) const;
    [[nodiscard]] double cellPlacementCost(const ProblemDefinition& problem,
                                           const TestRoute& route) const;
};

}  // namespace lab
