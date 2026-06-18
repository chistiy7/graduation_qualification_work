#include "app/postprocessor.hpp"

#include "app/paths.hpp"
#include "engine/layout_map.hpp"
#include "io/scenario_json.hpp"

#include <fstream>
#include <iomanip>

namespace lab {

std::filesystem::path Postprocessor::exportTextReport(const ScenarioBundle& bundle,
                                                     const PlanResult& result) const {
    const auto path = outputDir() / ("report_" + bundle.name + ".txt");
    std::ofstream out(path);
    out << "Сценарий: " << bundle.name << "\n\n";
    out << LabOptimiser::formatReport(result, bundle.problem);
    out << "\n" << renderLayoutMatrix(bundle.problem, result.route, result.metrics.T_cycle);
    return path;
}

std::filesystem::path Postprocessor::exportCsv(const PlanResult& result) const {
    const auto path = outputDir() / "plan.csv";
    std::ofstream out(path);
    const auto& m = result.metrics;
    const auto& c = m.cost;

    out << std::fixed << std::setprecision(4);
    out << "metric,value\n";
    out << "T_sum_min," << m.T_sum << "\n";
    out << "T_cycle_min," << m.T_cycle << "\n";
    out << "program_work_time_min," << result.efficiency.time.workTimeMin << "\n";
    out << "program_time_prep_min," << result.efficiency.time.prepMin << "\n";
    out << "program_time_test_min," << result.efficiency.time.testMin << "\n";
    out << "program_time_setup_min," << result.efficiency.time.setupMin << "\n";
    out << "program_time_move_min," << result.efficiency.time.moveMin << "\n";
    out << "program_parallelism_index," << result.efficiency.time.parallelismIndex << "\n";
    out << "C_rub," << m.C << "\n";
    out << "N," << m.N << "\n";
    out << "L_steps," << m.L << "\n";
    out << "eta_avg_pct," << (m.etaAvg * 100.0) << "\n";
    out << "bottleneck," << m.bottleneckEquipmentId << "\n";
    out << "cost_prep_labor," << c.prepLabor << "\n";
    out << "cost_operation_materials," << c.operationMaterials << "\n";
    out << "cost_energy_work," << c.energyWork << "\n";
    out << "cost_operation_labor," << c.operationLabor << "\n";
    out << "cost_setup," << c.setup << "\n";
    out << "cost_energy_setup," << c.energySetup << "\n";
    out << "cost_amortization," << c.amortization << "\n";
    out << "total_idle_min," << m.totalIdleMin << "\n";
    out << "cost_transport," << c.transport << "\n";
    out << "cost_area," << c.area << "\n";
    out << "cost_total," << c.total() << "\n";
    return path;
}

}  // namespace lab
