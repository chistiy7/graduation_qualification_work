#include "engine/metrics.hpp"

#include "engine/route_analysis.hpp"

#include <set>

namespace lab {

namespace {

const TestStage* findOperation(const ProblemDefinition& p, const std::string& id) {
    for (const auto& op : p.operations) {
        if (op.id == id) return &op;
    }
    return nullptr;
}

}  // namespace

double MetricsEngine::operationTimeSum(const ProblemDefinition& problem,
                                       const TestRoute& route) const {
    double sum = 0.0;
    for (const auto& step : route.steps()) {
        if (const auto* op = findOperation(problem, step.operationId)) {
            sum += op->durationMin;
        }
    }
    return sum;
}

double MetricsEngine::operationCostSum(const ProblemDefinition& problem,
                                       const TestRoute& route) const {
    double sum = 0.0;
    for (const auto& step : route.steps()) {
        if (const auto* op = findOperation(problem, step.operationId)) {
            // c_op + c_en (энергия × тариф задаётся в costEnergy)
            sum += op->costOp + op->costEnergy;
        }
    }
    return sum;
}

double MetricsEngine::laborCost(const ProblemDefinition& problem, const TestRoute& route) const {
    double sum = 0.0;
    for (const auto& s : problem.specimens) {
        sum += s.prepLaborHours * problem.laborRatePerHour;
    }
    for (const auto& step : route.steps()) {
        if (const auto* op = findOperation(problem, step.operationId)) {
            // труд + обработка результатов (внутри h(o), брифинг)
            sum += op->laborHours * problem.laborRatePerHour;
        }
    }
    return sum;
}

double MetricsEngine::prepLaborCost(const ProblemDefinition& problem) const {
    double sum = 0.0;
    for (const auto& s : problem.specimens) {
        sum += s.prepLaborHours * problem.laborRatePerHour;
    }
    return sum;
}

double MetricsEngine::operationLaborCost(const ProblemDefinition& problem,
                                         const TestRoute& route) const {
    double sum = 0.0;
    for (const auto& step : route.steps()) {
        if (const auto* op = findOperation(problem, step.operationId)) {
            sum += op->laborHours * problem.laborRatePerHour;
        }
    }
    return sum;
}

double MetricsEngine::amortizationCost(
    const ProblemDefinition& problem,
    const std::unordered_map<std::string, double>& busyByEquipment) const {
    double sum = 0.0;
    for (const auto& e : problem.equipment) {
        double busy = 0.0;
        if (auto it = busyByEquipment.find(e.id); it != busyByEquipment.end()) {
            busy = it->second;
        }
        sum += e.amortPerHour * (busy / 60.0);
    }
    return sum;
}

double MetricsEngine::averageLoad(
    const ProblemDefinition& problem,
    const std::unordered_map<std::string, double>& busyByEquipment) const {
    double total = 0.0;
    int count = 0;
    for (const auto& e : problem.equipment) {
        if (e.fundTimeMin <= 0.0) continue;
        double busy = 0.0;
        if (auto it = busyByEquipment.find(e.id); it != busyByEquipment.end()) {
            busy = it->second;
        }
        total += busy / e.fundTimeMin;
        ++count;
    }
    return count > 0 ? total / count : 0.0;
}

double MetricsEngine::objectiveK(const ObjectiveWeights& w, const VariantMetrics& m) const {
    return w.alphaT * m.Tn + w.alphaC * m.Cn + w.alphaN * m.Nn + w.alphaL * m.Ln +
           w.alphaEta * (2.0 - m.EtaN);
}

double MetricsEngine::cellPlacementCost(const ProblemDefinition& problem,
                                        const TestRoute& route) const {
    std::set<std::string> used;
    for (const auto& step : route.steps()) {
        used.insert(step.equipmentId);
    }
    double sum = 0.0;
    for (const auto& e : problem.equipment) {
        if (used.count(e.id)) sum += e.cellPlacementCost;
    }
    return sum;
}

VariantMetrics MetricsEngine::compute(const ProblemDefinition& problem,
                                      const TestRoute& route) const {
    const RouteAnalysis analysis = RouteAnalyzer{}.analyze(problem, route);

    VariantMetrics m;
    double prepTime = 0.0;
    for (const auto& s : problem.specimens) {
        prepTime += s.prepTimeMin;
    }

    const double opTime = operationTimeSum(problem, route);

    m.T = prepTime + opTime + analysis.setupTimeMin + analysis.moveTimeMin;
    m.N = analysis.setupCount;
    m.L = analysis.routeLengthSteps;

    m.cost.prepLabor = prepLaborCost(problem);
    m.cost.operations = operationCostSum(problem, route);
    m.cost.operationLabor = operationLaborCost(problem, route);
    m.cost.setup = analysis.setupCost;
    m.cost.amortization = amortizationCost(problem, analysis.busyMinutesByEquipment);
    // транспорт: время перемещения по ячейкам × ставка труда (зависит от размещения)
    m.cost.transport = (analysis.moveTimeMin / 60.0) * problem.laborRatePerHour;
    m.cost.cellPlacement = cellPlacementCost(problem, route);
    m.C = m.cost.total();
    m.etaAvg = averageLoad(problem, analysis.busyMinutesByEquipment);

    if (problem.objectiveMode == ObjectiveMode::TotalCostRub) {
        m.K = m.C;
    }

    return m;
}

ComparisonRow MetricsEngine::compare(const ProblemDefinition& problem,
                                     const TestRoute& baselineRoute,
                                     const TestRoute& optimizedRoute) const {
    ComparisonRow row;
    row.baseline = compute(problem, baselineRoute);
    row.optimized = compute(problem, optimizedRoute);

    if (problem.objectiveMode == ObjectiveMode::WeightedK) {
        auto normalize = [](VariantMetrics& cur, const VariantMetrics& base) {
            cur.Tn = base.T > 0 ? cur.T / base.T : 1.0;
            cur.Cn = base.C > 0 ? cur.C / base.C : 1.0;
            cur.Nn = base.N > 0 ? static_cast<double>(cur.N) / base.N : 1.0;
            cur.Ln = base.L > 0 ? cur.L / base.L : 1.0;
            cur.EtaN = base.etaAvg > 0 ? cur.etaAvg / base.etaAvg : 1.0;
        };

        normalize(row.baseline, row.baseline);
        normalize(row.optimized, row.baseline);

        row.baseline.K = objectiveK(problem.weights, row.baseline);
        row.optimized.K = objectiveK(problem.weights, row.optimized);
    } else {
        row.baseline.K = row.baseline.C;
        row.optimized.K = row.optimized.C;
    }

    row.timeReductionPct = row.baseline.T > 0
        ? (row.baseline.T - row.optimized.T) / row.baseline.T * 100.0
        : 0.0;
    row.costReductionPct = row.baseline.C > 0
        ? (row.baseline.C - row.optimized.C) / row.baseline.C * 100.0
        : 0.0;
    row.objectiveReductionPct =
        row.baseline.K > 0 ? (row.baseline.K - row.optimized.K) / row.baseline.K * 100.0 : 0.0;

    return row;
}

}  // namespace lab
