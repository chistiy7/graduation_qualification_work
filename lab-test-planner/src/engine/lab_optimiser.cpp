#include "engine/lab_optimiser.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace lab {

LabOptimiser::LabOptimiser(MetricsEngine metrics) : metrics_(std::move(metrics)) {}

TestRoute LabOptimiser::buildRoute(const ProblemDefinition& problem,
                                   RouteOrderingStrategy strategy) const {
    switch (strategy) {
    case RouteOrderingStrategy::BySpecimenThenOperation:
        return routes_.buildBaselineRoute(problem);
    case RouteOrderingStrategy::ByOperationThenSpecimen:
        return routes_.buildGroupedRoute(problem);
    }
    return {};
}

OptimisationResult LabOptimiser::compareStrategies(
    const ProblemDefinition& problem,
    RouteOrderingStrategy baseline,
    RouteOrderingStrategy optimized) const {
    OptimisationResult r;
    r.baselineRoute = buildRoute(problem, baseline);
    r.optimizedRoute = buildRoute(problem, optimized);
    r.comparison = metrics_.compare(problem, r.baselineRoute, r.optimizedRoute);
    return r;
}

OptimisationResult LabOptimiser::run(const ScenarioBundle& bundle) const {
    return compareStrategies(bundle.problem, bundle.baselineStrategy, bundle.optimizedStrategy);
}

std::string LabOptimiser::formatReport(const OptimisationResult& result) {
    const auto& b = result.comparison.baseline;
    const auto& o = result.comparison.optimized;
    const auto& c = result.comparison;

    std::ostringstream out;
    out << std::fixed << std::setprecision(3);
    out << "=== LabPlanner — сравнение вариантов ===\n\n";
    out << "| Показатель | Вариант 0 | Вариант 1 | Δ% |\n";
    out << "|------------|-----------|-----------|-----|\n";
    out << "| T, мин     | " << b.T << " | " << o.T << " | " << c.timeReductionPct << " |\n";
    out << "| C, руб     | " << b.C << " | " << o.C << " | " << c.costReductionPct << " |\n";
    out << "| N          | " << b.N << " | " << o.N << " | "
        << (b.N > 0 ? (b.N - o.N) * 100.0 / b.N : 0) << " |\n";
    out << "| L, шаг     | " << b.L << " | " << o.L << " | "
        << (b.L > 0 ? (b.L - o.L) * 100.0 / b.L : 0) << " |\n";
    out << "| η_sr       | " << b.etaAvg << " | " << o.etaAvg << " | — |\n";
    out << "| ЦФ         | " << b.K << " | " << o.K << " | " << c.objectiveReductionPct << " |\n";

    const auto& bc = b.cost;
    const auto& oc = o.cost;
    out << "\n--- Разложение себестоимости C, руб ---\n";
    out << "| Статья | Вариант 0 | Вариант 1 |\n";
    out << "|--------|-----------|-----------|\n";
    out << "| Подготовка образца (труд) | " << bc.prepLabor << " | " << oc.prepLabor << " |\n";
    out << "| Операции (материалы+энергия) | " << bc.operations << " | " << oc.operations << " |\n";
    out << "| Труд по операциям | " << bc.operationLabor << " | " << oc.operationLabor << " |\n";
    out << "| Перенастройки | " << bc.setup << " | " << oc.setup << " |\n";
    out << "| Амортизация | " << bc.amortization << " | " << oc.amortization << " |\n";
    out << "| Транспорт (перемещения) | " << bc.transport << " | " << oc.transport << " |\n";
    out << "| Размещение ячеек | " << bc.cellPlacement << " | " << oc.cellPlacement << " |\n";
    out << "| Итого C | " << bc.total() << " | " << oc.total() << " |\n";
    out << "| C без размещения | " << bc.withoutPlacement() << " | " << oc.withoutPlacement()
        << " |\n";

    out << "\n--- Маршрут (вариант 0) ---\n";
    for (const auto& step : result.baselineRoute.steps()) {
        out << "  " << step.specimenId << " -> " << step.operationId << " @ "
            << step.equipmentId << "\n";
    }
    out << "--- Маршрут (вариант 1) ---\n";
    for (const auto& step : result.optimizedRoute.steps()) {
        out << "  " << step.specimenId << " -> " << step.operationId << " @ "
            << step.equipmentId << "\n";
    }

    out << "\nРекомендация: "
        << (o.K < b.K ? "вариант 1 (оптимизированный)" : "вариант 0") << "\n";
    return out.str();
}

void LabOptimiser::printReport(const OptimisationResult& result) const {
    std::cout << formatReport(result);
}

}  // namespace lab
