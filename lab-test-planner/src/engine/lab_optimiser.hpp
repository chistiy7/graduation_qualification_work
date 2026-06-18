#pragma once

#include "engine/metrics.hpp"
#include "engine/program_efficiency.hpp"
#include "engine/route_planner.hpp"
#include "model/planning_strategy.hpp"
#include "model/scenario_bundle.hpp"

#include <string>
#include <vector>

namespace lab {

// Единый оптимальный план: маршрут испытаний и его показатели/себестоимость
struct PlanResult {
    TestRoute route;
    VariantMetrics metrics;
    ProgramEfficiencyMetrics efficiency;
    std::string orderingNote;
    std::vector<std::string> energyWarnings;  // t_norm < t_load_min
};

// Подбор лучшего порядка операций на заданном размещении (min C) → один план
class LabOptimiser {
public:
    explicit LabOptimiser(MetricsEngine metrics = {});

    [[nodiscard]] PlanResult plan(const ProblemDefinition& problem) const;

    [[nodiscard]] PlanResult run(const ScenarioBundle& bundle) const;

    [[nodiscard]] static std::string formatReport(const PlanResult& result,
                                                  const ProblemDefinition& problem);

    void printReport(const PlanResult& result, const ProblemDefinition& problem) const;

private:
    [[nodiscard]] TestRoute buildRoute(const ProblemDefinition& problem,
                                       RouteOrderingStrategy strategy) const;

    MetricsEngine metrics_;
    RoutePlanner routes_;
};

}  // namespace lab
