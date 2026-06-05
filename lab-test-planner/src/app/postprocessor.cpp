#include "app/postprocessor.hpp"

#include "app/paths.hpp"
#include "engine/layout_map.hpp"
#include "io/scenario_json.hpp"

#include <fstream>
#include <iomanip>

namespace lab {

std::filesystem::path Postprocessor::exportTextReport(const ScenarioBundle& bundle,
                                                     const OptimisationResult& result) const {
    const auto path = outputDir() / ("report_" + bundle.name + ".txt");
    std::ofstream out(path);
    out << "Сценарий: " << bundle.name << "\n\n";
    out << LabOptimiser::formatReport(result);
    out << "\n" << renderLayoutMatrix(bundle.problem, result.optimizedRoute);
    return path;
}

std::filesystem::path Postprocessor::exportCsvComparison(const OptimisationResult& result) const {
    const auto path = outputDir() / "comparison.csv";
    std::ofstream out(path);
    const auto& b = result.comparison.baseline;
    const auto& o = result.comparison.optimized;
    const auto& c = result.comparison;

    out << std::fixed << std::setprecision(4);
    out << "metric,baseline,optimized,delta_pct\n";
    out << "T_min," << b.T << "," << o.T << "," << c.timeReductionPct << "\n";
    out << "C_rub," << b.C << "," << o.C << "," << c.costReductionPct << "\n";
    out << "N," << b.N << "," << o.N << ","
        << (b.N > 0 ? (b.N - o.N) * 100.0 / b.N : 0) << "\n";
    out << "L_steps," << b.L << "," << o.L << ","
        << (b.L > 0 ? (b.L - o.L) * 100.0 / b.L : 0) << "\n";
    out << "eta_sr," << b.etaAvg << "," << o.etaAvg << ",0\n";
    out << "objective," << b.K << "," << o.K << "," << c.objectiveReductionPct << "\n";

    const auto& bc = b.cost;
    const auto& oc = o.cost;
    auto pct = [](double base, double opt) { return base > 0 ? (base - opt) / base * 100.0 : 0.0; };
    out << "cost_prep_labor," << bc.prepLabor << "," << oc.prepLabor << ","
        << pct(bc.prepLabor, oc.prepLabor) << "\n";
    out << "cost_operations," << bc.operations << "," << oc.operations << ","
        << pct(bc.operations, oc.operations) << "\n";
    out << "cost_operation_labor," << bc.operationLabor << "," << oc.operationLabor << ","
        << pct(bc.operationLabor, oc.operationLabor) << "\n";
    out << "cost_setup," << bc.setup << "," << oc.setup << "," << pct(bc.setup, oc.setup) << "\n";
    out << "cost_amortization," << bc.amortization << "," << oc.amortization << ","
        << pct(bc.amortization, oc.amortization) << "\n";
    out << "cost_transport," << bc.transport << "," << oc.transport << ","
        << pct(bc.transport, oc.transport) << "\n";
    out << "cost_cell_placement," << bc.cellPlacement << "," << oc.cellPlacement << ","
        << pct(bc.cellPlacement, oc.cellPlacement) << "\n";
    out << "cost_without_placement," << bc.withoutPlacement() << "," << oc.withoutPlacement() << ","
        << pct(bc.withoutPlacement(), oc.withoutPlacement()) << "\n";
    return path;
}

}  // namespace lab
