#include "engine/metrics.hpp"

#include "domain/schedule.hpp"
#include "engine/layout_area.hpp"
#include "engine/route_analysis.hpp"
#include "engine/scheduler.hpp"

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
    // DES-расписание: реальный T_cycle (makespan) с перекрытием автономных прогонов,
    // путь оператора L и счёт переналадок N. Себестоимостные рычаги (C_setup,
    // C_energy_setup) берём из RouteAnalyzer — они согласованы с фазой 2.
    const Schedule sched = Scheduler{}.build(problem, route);

    // Раскладка фаз расписания по стендам: настройка / прогон / снятие.
    std::unordered_map<std::string, double> schedTestMin;
    std::unordered_map<std::string, double> schedSetupMin;
    for (const auto& ph : sched.phases) {
        const double dur = ph.endMin - ph.startMin;
        if (ph.phase == SchedulePhase::Setup) {
            schedSetupMin[ph.equipmentId] += dur;
        } else if (ph.phase == SchedulePhase::Run) {
            schedTestMin[ph.equipmentId] += dur;
        }
    }
    std::unordered_map<std::string, double> schedBusyMin;
    for (const auto& [id, busy] : sched.standBusyMin) {
        schedBusyMin[id] = busy;
    }

    VariantMetrics m;
    double prepTime = 0.0;
    for (const auto& s : problem.specimens) {
        prepTime += s.prepTimeMin;
    }

    const double opTime = operationTimeSum(problem, route);
    const double moveTime = sched.operatorTravelSteps * problem.minutesPerGridStep;

    m.T_sum = prepTime + opTime + analysis.setupTimeMin + moveTime;
    m.T = m.T_sum;
    m.N = sched.changeoverCount;
    m.L = sched.operatorTravelSteps;
    m.T_cycle = sched.makespanMin;  // реальный цикл партии из DES-ядра

    m.cost.prepLabor = prepLaborCost(problem);
    m.cost.operationMaterials = operationMaterialsCost(problem, route);
    m.cost.energyWork = energyWorkCost(problem, route);
    m.cost.operationLabor = operationLaborCost(problem, route);
    m.cost.setup = analysis.setupCost;
    m.cost.energySetup = analysis.energySetupCost;
    m.cost.amortization = amortizationCost(problem);
    m.cost.transport = (moveTime / 60.0) * problem.laborRatePerHour;
    m.cost.area = areaOccupancyCost(problem, m.T_cycle);

    std::unordered_map<std::string, double> workKwh;
    std::unordered_map<std::string, double> workCost;
    accumulateWorkEnergyByEquipment(problem, route, workKwh, workCost);

    double maxBusy = 0.0;
    for (const auto& e : problem.equipment) {
        EquipmentUtilization u;
        u.equipmentId = e.id;
        u.testMin = schedTestMin.count(e.id) ? schedTestMin.at(e.id) : 0.0;
        u.setupMin = schedSetupMin.count(e.id) ? schedSetupMin.at(e.id) : 0.0;
        u.busyMin = schedBusyMin.count(e.id) ? schedBusyMin.at(e.id) : 0.0;
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
