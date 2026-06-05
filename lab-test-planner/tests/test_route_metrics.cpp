#include "engine/lab_optimiser.hpp"
#include "engine/route_analysis.hpp"
#include "engine/stand_state_matrix.hpp"
#include "model/scenario_bundle.hpp"

#include <cstdlib>
#include <iostream>

int main() {
    const auto bundle = lab::buildDemoSimple();
    lab::LabOptimiser optimiser;

    const auto result = optimiser.run(bundle);
    const auto& b = result.comparison.baseline;
    const auto& o = result.comparison.optimized;

    bool ok = true;

    ok &= o.N <= b.N;
    ok &= o.L <= b.L;
    ok &= b.T > 0 && o.T > 0;
    ok &= b.C > 0 && o.C > 0;

    if (bundle.problem.objectiveMode == lab::ObjectiveMode::TotalCostRub) {
        ok &= o.C <= b.C + 1e-6;
        ok &= o.K == o.C && b.K == b.C;
    } else {
        ok &= o.K <= b.K + 1e-6;
        ok &= b.K >= 0.99 && b.K <= 1.01;
    }

    const auto analysis =
        lab::RouteAnalyzer{}.analyze(bundle.problem, result.baselineRoute);
    ok &= analysis.setupCount == b.N;
    ok &= analysis.routeLengthSteps == b.L;

    const auto stateMatrix =
        lab::StandStateAnalyzer{}.analyze(bundle.problem, result.baselineRoute);
    ok &= stateMatrix.setupCount == b.N;
    for (const auto& ev : stateMatrix.events) {
        ok &= ev.operationAllowed;
    }

    if (!ok) {
        std::cerr << "Route metrics test FAILED\n";
        std::cerr << "N0=" << b.N << " N1=" << o.N << " L0=" << b.L << " L1=" << o.L
                  << " C0=" << b.C << " C1=" << o.C << " K0=" << b.K << " K1=" << o.K << "\n";
        return EXIT_FAILURE;
    }
    std::cout << "Route metrics properties OK (demo_simple)\n";
    return EXIT_SUCCESS;
}
