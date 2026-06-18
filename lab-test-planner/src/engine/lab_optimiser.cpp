#include "engine/lab_optimiser.hpp"

#include "engine/layout_area.hpp"

#include <cctype>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

namespace lab {

namespace {

std::optional<int> specimenIndex(const std::string& id) {
    if (id.empty() || (id[0] != 's' && id[0] != 'S')) return std::nullopt;
    int value = 0;
    bool digits = false;
    for (size_t i = 1; i < id.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(id[i]))) return std::nullopt;
        digits = true;
        value = value * 10 + (id[i] - '0');
    }
    if (!digits) return std::nullopt;
    return value;
}

std::string formatSpecimenRange(const std::vector<std::string>& ids) {
    if (ids.empty()) return "";
    if (ids.size() == 1) return ids.front();

    std::ostringstream out;
    size_t i = 0;
    while (i < ids.size()) {
        if (i > 0) out << ", ";
        const auto startIdx = specimenIndex(ids[i]);
        size_t j = i + 1;
        if (startIdx.has_value()) {
            while (j < ids.size()) {
                const auto cur = specimenIndex(ids[j]);
                const auto prev = specimenIndex(ids[j - 1]);
                if (!cur.has_value() || !prev.has_value() || *cur != *prev + 1) break;
                ++j;
            }
        }
        if (j - i >= 2 && startIdx.has_value()) {
            out << ids[i] << "–" << ids[j - 1];
        } else {
            for (size_t k = i; k < j; ++k) {
                if (k > i) out << ", ";
                out << ids[k];
            }
        }
        i = j;
    }
    return out.str();
}

std::string formatAggregatedRoute(const TestRoute& route) {
    std::ostringstream out;
    const auto& steps = route.steps();
    if (steps.empty()) return out.str();

    size_t i = 0;
    while (i < steps.size()) {
        const auto& head = steps[i];
        std::vector<std::string> specimens;
        specimens.push_back(head.specimenId);
        size_t j = i + 1;
        while (j < steps.size() && steps[j].operationId == head.operationId &&
               steps[j].equipmentId == head.equipmentId) {
            specimens.push_back(steps[j].specimenId);
            ++j;
        }
        out << "  " << formatSpecimenRange(specimens) << " -> " << head.operationId << " @ "
            << head.equipmentId << "\n";
        i = j;
    }
    return out.str();
}

double percent(double fraction) { return fraction * 100.0; }

}  // namespace

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

PlanResult LabOptimiser::plan(const ProblemDefinition& problem) const {
    const TestRoute bySpecimen = buildRoute(problem, RouteOrderingStrategy::BySpecimenThenOperation);
    const TestRoute byOperation = buildRoute(problem, RouteOrderingStrategy::ByOperationThenSpecimen);
    const VariantMetrics mSpecimen = metrics_.compute(problem, bySpecimen);
    const VariantMetrics mOperation = metrics_.compute(problem, byOperation);

    PlanResult r;
    if (mOperation.C <= mSpecimen.C) {
        r.route = byOperation;
        r.metrics = mOperation;
        r.orderingNote = "по операциям (группировка снижает перенастройки)";
    } else {
        r.route = bySpecimen;
        r.metrics = mSpecimen;
        r.orderingNote = "по образцам";
    }

    for (const auto& op : problem.operations) {
        if (!op.energyWarning.empty()) {
            r.energyWarnings.push_back(op.nameRu + ": " + op.energyWarning);
        }
    }
    r.efficiency = ProgramEfficiencyEngine{}.compute(problem, r.route);
    return r;
}

PlanResult LabOptimiser::run(const ScenarioBundle& bundle) const {
    return plan(bundle.problem);
}

std::string LabOptimiser::formatReport(const PlanResult& result,
                                       const ProblemDefinition& problem) {
    const auto& m = result.metrics;
    const auto& c = m.cost;

    std::ostringstream out;
    out << std::fixed << std::setprecision(2);
    out << "=== LabPlanner — оптимальный план лаборатории ===\n\n";

    out << "Показатели плана:\n";
    out << "  T_sum — суммарная трудоёмкость, мин = " << m.T_sum << "\n";
    out << "  T_cycle — цикл выполнения партии, мин = " << m.T_cycle << "\n";
    out << "  N — первичные подготовки и переналадки = " << m.N << "\n";
    out << "  L — маршрут, шаги ячеек           = " << m.L << "\n";
    out << "  η_avg — средняя загрузка стендов, % = " << percent(m.etaAvg) << "\n";
    if (!m.bottleneckEquipmentId.empty()) {
        out << "  bottleneck — узкое место          = " << m.bottleneckEquipmentId << "\n";
    }
    out << "  Порядок операций: " << result.orderingNote << "\n";

    const auto& pt = result.efficiency.time;
    out << "\n--- Эффективность программы (время) ---\n";
    out << "  Время работы программы (T_sum), мин = " << pt.workTimeMin << "\n";
    out << "    подготовка образцов, мин        = " << pt.prepMin << "\n";
    out << "    испытания на стендах, мин       = " << pt.testMin << "\n";
    out << "    переналадки стендов, мин        = " << pt.setupMin << "\n";
    out << "    перемещения, мин                = " << pt.moveMin << "\n";
    out << "  Цикл выполнения партии (T_cycle), мин = " << pt.cycleMin << "\n";
    out << "  Индекс параллельности (T_cycle/T_sum) = " << pt.parallelismIndex << "\n";

    out << "\nЦелевая функция (минимизирована): C = " << m.C << " руб\n";

    out << "\n--- Составляющие целевой функции C, руб ---\n";
    out << "  Подготовка образцов (труд)      = " << c.prepLabor << "\n";
    out << "  Материалы испытаний             = " << c.operationMaterials << "\n";
    out << "  Энергия испытаний (работа)      = " << c.energyWork << "\n";
    out << "  Труд по операциям               = " << c.operationLabor << "\n";
    out << "  Перенастройки стендов           = " << c.setup << "\n";
    out << "  Энергия переналадки             = " << c.energySetup << "\n";
    out << "  Амортизация стендов             = " << c.amortization << "\n";
    out << "  Транспорт (перемещения)         = " << c.transport << "\n";
    out << "  Занятость площади (C_area)      = " << c.area << "\n";
    out << "  --------------------------------------------\n";
    out << "  Итого C                         = " << c.total() << "\n";

    out << "\n--- Простой и загрузка стендов ---\n";
    out << "  Суммарный простой, мин          = " << m.totalIdleMin << "\n";
    out << "  Средняя загрузка η_avg, %       = " << percent(m.etaAvg) << "\n";
    for (const auto& u : m.equipmentStats) {
        const auto* eq = [&]() -> const LabEquipment* {
            for (const auto& e : problem.equipment) {
                if (e.id == u.equipmentId) return &e;
            }
            return nullptr;
        }();
        double energySetupKwh = 0.0;
        double costEnergySetup = 0.0;
        if (eq && problem.electricityTariffPerKwh > 0.0 && eq->nominalPowerKw > 0.0 &&
            u.setupMin > 0.0) {
            energySetupKwh = 0.2 * eq->nominalPowerKw * (u.setupMin / 60.0);
            costEnergySetup = energySetupKwh * problem.electricityTariffPerKwh;
        }

        out << "\n  [" << u.equipmentId << "]\n";
        out << "    t_test, мин                   = " << u.testMin << "\n";
        out << "    t_setup, мин                  = " << u.setupMin << "\n";
        out << "    t_busy, мин                    = " << u.busyMin << "\n";
        out << "    t_idle, мин                    = " << u.idleMin << "\n";
        out << "    η_busy, %                      = " << percent(u.utilization) << "\n";
        out << "    E_work, кВт·ч                  = " << u.energyWorkKwh << "\n";
        out << "    E_setup, кВт·ч                 = " << energySetupKwh << "\n";
        out << "    E_idle, кВт·ч                  = " << 0.0 << "\n";
        out << "    C_energy_work, руб             = " << u.costEnergyWork << "\n";
        out << "    C_energy_setup, руб            = " << costEnergySetup << "\n";
        out << "    C_energy_idle, руб             = " << 0.0 << "\n";
        out << "    C_setup, руб                   = " << u.costSetup << "\n";
        out << "    C_idle, руб                    = " << 0.0 << "\n";
        out << "    C_amort, руб                   = " << u.costAmort << "\n";
    }

    out << "\n--- Маршрут испытаний (агрегированный) ---\n";
    out << formatAggregatedRoute(result.route);

    if (!result.energyWarnings.empty()) {
        out << "\n--- Предупреждения по энергии / времени ---\n";
        for (const auto& w : result.energyWarnings) {
            out << "  ! " << w << "\n";
        }
    }
    return out.str();
}

void LabOptimiser::printReport(const PlanResult& result,
                               const ProblemDefinition& problem) const {
    std::cout << formatReport(result, problem);
}

}  // namespace lab
