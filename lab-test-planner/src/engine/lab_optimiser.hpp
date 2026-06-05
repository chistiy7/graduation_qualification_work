#pragma once

#include "engine/metrics.hpp"
#include "engine/route_planner.hpp"
#include "model/planning_strategy.hpp"
#include "model/scenario_bundle.hpp"

namespace lab {

struct OptimisationResult {
    TestRoute baselineRoute;
    TestRoute optimizedRoute;
    ComparisonRow comparison;
};

// Сравнение двух стратегий упорядочивания маршрута (без заданных N/L извне)
class LabOptimiser {
public:
    explicit LabOptimiser(MetricsEngine metrics = {});

    [[nodiscard]] OptimisationResult compareStrategies(
        const ProblemDefinition& problem,
        RouteOrderingStrategy baseline = RouteOrderingStrategy::BySpecimenThenOperation,
        RouteOrderingStrategy optimized = RouteOrderingStrategy::ByOperationThenSpecimen) const;

    [[nodiscard]] OptimisationResult run(const ScenarioBundle& bundle) const;

    [[nodiscard]] static std::string formatReport(const OptimisationResult& result);

    void printReport(const OptimisationResult& result) const;

private:
    [[nodiscard]] TestRoute buildRoute(const ProblemDefinition& problem,
                                       RouteOrderingStrategy strategy) const;

    MetricsEngine metrics_;
    RoutePlanner routes_;
};

}  // namespace lab
