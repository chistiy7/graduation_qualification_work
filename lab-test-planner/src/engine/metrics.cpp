#include "engine/metrics.hpp"

#include "engine/layout_area.hpp"
#include "engine/route_analysis.hpp"

#include <unordered_map>

namespace lab {

namespace {

const TestStage* findOperation(const ProblemDefinition& p, const std::string& id) {
    for (const auto& op : p.operations) {
        if (op.id == id) return &op;
    }
    return nullptr;
}

const LabEquipment* findEquipment(const ProblemDefinition& p, const std::string& id) {
    for (const auto& e : p.equipment) {
        if (e.id == id) return &e;
    }
    return nullptr;
}

void accumulateWorkEnergyByEquipment(
    const ProblemDefinition& problem, const TestRoute& route,
    std::unordered_map<std::string, double>& energyKwh,
    std::unordered_map<std::string, double>& costRub) {
    for (const auto& step : route.steps()) {
        const auto* op = findOperation(problem, step.operationId);
        if (!op) continue;
        energyKwh[step.equipmentId] += op->energyKwh;
        costRub[step.equipmentId] += op->costEnergy;
    }
}

double busyMinutes(const RouteAnalysis& analysis, const std::string& equipmentId) {
    const auto it = analysis.busyMinutesByEquipment.find(equipmentId);
    return it != analysis.busyMinutesByEquipment.end() ? it->second : 0.0;
}

}  // namespace

double MetricsEngine::operationTimeSum(const ProblemDefinition& problem,
                                       const TestRoute& route) const {
    double sum = 0.0;
    for (const auto& step : route.steps()) {
        if (const auto* op = findOperation(problem, step.operationId)) {
            const double busy =
                op->cycleTimeMin > 0.0 ? op->cycleTimeMin : op->durationMin;
            sum += busy;
        }
    }
    return sum;
}

double MetricsEngine::operationMaterialsCost(const ProblemDefinition& problem,
                                             const TestRoute& route) const {
    double sum = 0.0;
    for (const auto& step : route.steps()) {
        if (const auto* op = findOperation(problem, step.operationId)) {
            sum += op->costOp;
        }
    }
    return sum;
}

double MetricsEngine::energyWorkCost(const ProblemDefinition& problem,
                                     const TestRoute& route) const {
    double sum = 0.0;
    for (const auto& step : route.steps()) {
        if (const auto* op = findOperation(problem, step.operationId)) {
            sum += op->costEnergy;
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

double MetricsEngine::amortizationCost(const ProblemDefinition& problem) const {
    double sum = 0.0;
    for (const auto& e : problem.equipment) {
        if (e.fundTimeMin <= 0.0) continue;
        sum += e.amortPerHour * (e.fundTimeMin / 60.0);
    }
    return sum;
}

double MetricsEngine::averageLoad(const std::vector<EquipmentUtilization>& stats) const {
    if (stats.empty()) return 0.0;
    double total = 0.0;
    for (const auto& u : stats) {
        total += u.utilization;
    }
    return total / static_cast<double>(stats.size());
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

    m.T_sum = prepTime + opTime + analysis.setupTimeMin + analysis.moveTimeMin;
    m.T = m.T_sum;
    m.N = analysis.setupCount;
    m.L = analysis.routeLengthSteps;

    double tCycle = 0.0;
    for (const auto& e : problem.equipment) {
        tCycle = std::max(tCycle, busyMinutes(analysis, e.id));
    }
    m.T_cycle = tCycle;

    m.cost.prepLabor = prepLaborCost(problem);
    m.cost.operationMaterials = operationMaterialsCost(problem, route);
    m.cost.energyWork = energyWorkCost(problem, route);
    m.cost.operationLabor = operationLaborCost(problem, route);
    m.cost.setup = analysis.setupCost;
    m.cost.energySetup = analysis.energySetupCost;
    m.cost.amortization = amortizationCost(problem);
    m.cost.transport = (analysis.moveTimeMin / 60.0) * problem.laborRatePerHour;
    m.cost.area = areaOccupancyCost(problem, m.T_cycle);

    std::unordered_map<std::string, double> workKwh;
    std::unordered_map<std::string, double> workCost;
    accumulateWorkEnergyByEquipment(problem, route, workKwh, workCost);

    double maxBusy = 0.0;
    for (const auto& e : problem.equipment) {
        EquipmentUtilization u;
        u.equipmentId = e.id;
        u.testMin = analysis.testMinByEquipment.count(e.id) ? analysis.testMinByEquipment.at(e.id)
                                                            : 0.0;
        u.setupMin = analysis.setupMinByEquipment.count(e.id)
                         ? analysis.setupMinByEquipment.at(e.id)
                         : 0.0;
        u.busyMin = busyMinutes(analysis, e.id);
        if (u.busyMin > maxBusy) {
            maxBusy = u.busyMin;
            m.bottleneckEquipmentId = e.id;
        }

        if (m.T_cycle > 0.0) {
            u.idleMin = std::max(0.0, m.T_cycle - u.busyMin);
            u.utilization = std::min(1.0, u.busyMin / m.T_cycle);
        }

        u.energyWorkKwh = workKwh.count(e.id) ? workKwh[e.id] : 0.0;
        u.costEnergyWork = workCost.count(e.id) ? workCost[e.id] : 0.0;
        u.costSetup = analysis.setupCostByEquipment.count(e.id)
                          ? analysis.setupCostByEquipment.at(e.id)
                          : 0.0;
        u.costAmort = e.fundTimeMin > 0.0 ? e.amortPerHour * (e.fundTimeMin / 60.0) : 0.0;

        m.totalIdleMin += u.idleMin;
        m.equipmentStats.push_back(u);
    }

    m.etaAvg = averageLoad(m.equipmentStats);
    m.C = m.cost.total();

    return m;
}

}  // namespace lab
