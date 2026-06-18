#include "app/pipeline.hpp"
#include "engine/lab_optimiser.hpp"
#include "engine/metrics.hpp"
#include "engine/operation_energy.hpp"
#include "engine/program_efficiency.hpp"
#include "engine/route_analysis.hpp"
#include "engine/route_planner.hpp"
#include "engine/stand_state_matrix.hpp"
#include "app/preprocessor.hpp"
#include "domain/laboratory.hpp"
#include "domain/stand_catalog.hpp"
#include "domain/test_program_catalog.hpp"
#include "model/lab_program_mode.hpp"
#include "model/scenario_bundle.hpp"
#include "model/scenario_validation.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <set>
#include <stdexcept>

namespace {

bool testCatalog8ProgramParams() {
    struct Expected {
        lab::TestOperationType type;
        const char* equipId;
        double tMin;
        double modeKw;
        double energyKwh;
    };
    const Expected expected[] = {
        {lab::TestOperationType::Tension, "e_tension", 20.0, 1.5, 0.50},
        {lab::TestOperationType::Compression, "e_compression", 15.0, 1.5, 0.375},
        {lab::TestOperationType::Torsion, "e_torsion", 15.0, 1.0, 0.25},
        {lab::TestOperationType::Bending, "e_bending", 15.0, 1.5, 0.375},
        {lab::TestOperationType::Fatigue, "e_fatigue", 166.7, 5.0, 13.89},
        {lab::TestOperationType::TensionPlusTorsion, "e_tension_torsion", 30.0, 2.3, 1.15},
        {lab::TestOperationType::BendingPlusTorsion, "e_bending_torsion", 30.0, 2.3, 1.15},
        {lab::TestOperationType::Thermomechanical, "e_thermo_mech", 100.0, 10.0, 16.67},
    };

    if (lab::approvedTestPrograms().size() != 8) {
        std::cerr << "Catalog8: expected 8 programs, got " << lab::approvedTestPrograms().size()
                  << "\n";
        return false;
    }

    bool ok = true;
    for (const auto& exp : expected) {
        const auto* prog = lab::findTestProgramForOperation(exp.type);
        if (!prog) {
            std::cerr << "Catalog8: missing program for operation type\n";
            ok = false;
            continue;
        }
        const double t = lab::resolveProgramTimeMin(*prog);
        const double e = exp.modeKw * t / 60.0;
        ok &= prog->equipmentId == exp.equipId;
        ok &= std::abs(t - exp.tMin) < 0.15;
        ok &= std::abs(prog->modePowerKw - exp.modeKw) < 1e-6;
        ok &= std::abs(e - exp.energyKwh) < 0.02;
        ok &= lab::findStandDefById(prog->equipmentId) != nullptr;
        if (!ok) {
            std::cerr << "Catalog8 mismatch for " << prog->operationId << ": t=" << t
                      << " P=" << prog->modePowerKw << " E=" << e << "\n";
        }
    }

    return ok;
}

bool testPipelineAppliesLayoutForUserInput() {
    lab::ScenarioInput in;
    in.roomAreaM2 = 100.0;
    in.batchSize = 2;
    in.laborRatePerHour = 450.0;
    in.energyTariffPerKwh = 6.5;
    in.groups = {{1, lab::TestOperationType::Tension}, {1, lab::TestOperationType::Torsion}};

    lab::Pipeline pipeline;
    const auto out = pipeline.runFromInput(in, false);
    if (out.bundle.problem.laboratory.cells().empty()) {
        std::cerr << "Pipeline should place stands on grid for user input\n";
        return false;
    }
    if (out.layoutEvaluated <= 0) {
        std::cerr << "Pipeline should report layout evaluation count\n";
        return false;
    }
    return true;
}

bool testSetupOnModeChange() {
    lab::ProblemDefinition p;
    lab::LabEquipment eq;
    eq.id = "e_mock";
    eq.setupTimeMin = 10.0;
    eq.setupCost = 100.0;
    p.equipment.push_back(eq);
    p.capable["e_mock"]["tension"] = true;
    p.capable["e_mock"]["torsion"] = true;

    lab::TestStage tension;
    tension.id = "tension";
    tension.durationMin = 20.0;
    lab::TestStage combo;
    combo.id = "torsion";
    combo.durationMin = 30.0;
    p.operations = {tension, combo};

    lab::TestRoute grouped;
    grouped.addStep({"s1", "tension", "e_mock"});
    grouped.addStep({"s2", "tension", "e_mock"});
    grouped.addStep({"s3", "torsion", "e_mock"});
    grouped.addStep({"s4", "torsion", "e_mock"});

    lab::TestRoute interleaved;
    interleaved.addStep({"s1", "tension", "e_mock"});
    interleaved.addStep({"s2", "torsion", "e_mock"});
    interleaved.addStep({"s3", "tension", "e_mock"});
    interleaved.addStep({"s4", "torsion", "e_mock"});

    lab::TestRoute sameMode;
    sameMode.addStep({"s1", "tension", "e_mock"});
    sameMode.addStep({"s2", "tension", "e_mock"});

    const auto aGrouped = lab::RouteAnalyzer{}.analyze(p, grouped);
    const auto aInterleaved = lab::RouteAnalyzer{}.analyze(p, interleaved);
    const auto aSame = lab::RouteAnalyzer{}.analyze(p, sameMode);
    const auto mGrouped = lab::StandStateAnalyzer{}.analyze(p, grouped);
    const auto mInterleaved = lab::StandStateAnalyzer{}.analyze(p, interleaved);

    bool ok = true;
    ok &= aGrouped.setupCount == 2;
    ok &= aInterleaved.setupCount == 4;
    ok &= aSame.setupCount == 1;
    ok &= mGrouped.setupCount == aGrouped.setupCount;
    ok &= mInterleaved.setupCount == aInterleaved.setupCount;
    ok &= aGrouped.setupCount < aInterleaved.setupCount;
    if (!ok) {
        std::cerr << "Setup mode-change test FAILED: grouped N=" << aGrouped.setupCount
                  << " interleaved N=" << aInterleaved.setupCount
                  << " same N=" << aSame.setupCount << "\n";
    }
    return ok;
}

bool testCycleTimeAndUtilization() {
    lab::ProblemDefinition p;
    p.gridCellSizeM = 2.0;
    p.rentRatePerM2Hour = 10.0;

    lab::LabEquipment fast;
    fast.id = "e_tension";
    fast.amortPerHour = 1000.0;
    fast.fundTimeMin = 480.0;
    lab::LabEquipment slow;
    slow.id = "e_thermo_mech";
    slow.amortPerHour = 2000.0;
    slow.fundTimeMin = 480.0;
    p.equipment = {fast, slow};
    p.capable["e_tension"]["tension"] = true;
    p.capable["e_thermo_mech"]["thermo_mech"] = true;

    lab::TestStage tension;
    tension.id = "tension";
    tension.durationMin = 100.0;
    lab::TestStage thermo;
    thermo.id = "thermo_mech";
    thermo.durationMin = 500.0;
    p.operations = {tension, thermo};

    lab::LabCell standA;
    standA.id = "c1";
    standA.row = 0;
    standA.col = 0;
    standA.kind = lab::LabCellKind::Stand;
    lab::LabCell standB;
    standB.id = "c2";
    standB.row = 0;
    standB.col = 2;
    standB.kind = lab::LabCellKind::Stand;
    lab::LabCell buffer;
    buffer.id = "b1";
    buffer.row = 0;
    buffer.col = 1;
    buffer.kind = lab::LabCellKind::Buffer;
    p.laboratory.addCell(standA);
    p.laboratory.addCell(buffer);
    p.laboratory.addCell(standB);

    lab::TestRoute route;
    route.addStep({"s1", "tension", "e_tension"});
    route.addStep({"s2", "thermo_mech", "e_thermo_mech"});

    const auto m = lab::MetricsEngine{}.compute(p, route);
    bool ok = true;
    ok &= m.T_cycle == 500.0;
    ok &= m.T_sum > m.T_cycle;
    ok &= m.etaAvg <= 1.0;
    ok &= m.equipmentStats.size() == 2;

    const lab::EquipmentUtilization* tensile = nullptr;
    const lab::EquipmentUtilization* furnace = nullptr;
    for (const auto& u : m.equipmentStats) {
        if (u.equipmentId == "e_tension") tensile = &u;
        if (u.equipmentId == "e_thermo_mech") furnace = &u;
    }
    ok &= tensile != nullptr && furnace != nullptr;
    if (tensile && furnace) {
        ok &= tensile->busyMin == 100.0;
        ok &= furnace->busyMin == 500.0;
        ok &= tensile->idleMin == 400.0;
        ok &= furnace->idleMin == 0.0;
        ok &= tensile->utilization == 0.2;
        ok &= furnace->utilization == 1.0;
        ok &= m.bottleneckEquipmentId == "e_thermo_mech";
    }

    ok &= m.cost.area > 0.0;
    ok &= m.C > m.cost.amortization;

    if (!ok) {
        std::cerr << "Cycle/utilization test FAILED: T_cycle=" << m.T_cycle
                  << " etaAvg=" << m.etaAvg << " bottleneck=" << m.bottleneckEquipmentId
                  << "\n";
    }
    return ok;
}

bool testValidIndependentSpecimens() {
    lab::ProblemDefinition p;
    lab::LabEquipment tensile;
    tensile.id = "e_tension";
    lab::LabEquipment torsion;
    torsion.id = "e_torsion";
    p.equipment = {tensile, torsion};
    p.capable["e_tension"]["tension"] = true;
    p.capable["e_torsion"]["torsion"] = true;

    lab::TestStage tension;
    tension.id = "tension";
    lab::TestStage torsionOp;
    torsionOp.id = "torsion";
    p.operations = {tension, torsionOp};

    p.specimens = {{"s1"}, {"s2"}};
    p.required["s1"]["tension"] = true;
    p.required["s2"]["torsion"] = true;

    lab::ScenarioBundle bundle;
    bundle.problem = p;
    const auto errors = lab::Preprocessor{}.validate(bundle);
    if (!errors.empty()) {
        std::cerr << "Valid independent specimens FAILED: " << errors.front() << "\n";
        return false;
    }
    return true;
}

bool testValidCombinedOperation() {
    lab::ProblemDefinition p;
    lab::LabEquipment eq;
    eq.id = "e_tension_torsion";
    p.equipment.push_back(eq);
    p.capable["e_tension_torsion"]["tension_torsion"] = true;

    lab::TestStage combo;
    combo.id = "tension_torsion";
    p.operations = {combo};
    p.specimens = {{"s1"}};
    p.required["s1"]["tension_torsion"] = true;

    lab::ScenarioBundle bundle;
    bundle.problem = p;
    return lab::Preprocessor{}.validate(bundle).empty();
}

bool testRejectReusedSpecimen() {
    lab::ProblemDefinition p;
    lab::LabEquipment tensile;
    tensile.id = "e_tension";
    lab::LabEquipment torsion;
    torsion.id = "e_torsion";
    p.equipment = {tensile, torsion};
    p.capable["e_tension"]["tension"] = true;
    p.capable["e_torsion"]["torsion"] = true;

    lab::TestStage tension;
    tension.id = "tension";
    lab::TestStage torsionOp;
    torsionOp.id = "torsion";
    p.operations = {tension, torsionOp};
    p.specimens = {{"s1"}};
    p.required["s1"]["tension"] = true;
    p.required["s1"]["torsion"] = true;

    lab::ScenarioBundle bundle;
    bundle.problem = p;
    const auto errors = lab::Preprocessor{}.validate(bundle);
    if (errors.empty()) {
        std::cerr << "Reject reused specimen FAILED: expected validation error\n";
        return false;
    }

    try {
        lab::Preprocessor{}.ensureValid(bundle);
        std::cerr << "Reject reused specimen FAILED: ensureValid should throw\n";
        return false;
    } catch (const std::runtime_error&) {
        return true;
    }
}

bool testRejectBeforeMetrics() {
    lab::ProblemDefinition p;
    lab::LabEquipment eq;
    eq.id = "e_tension";
    p.equipment.push_back(eq);
    p.capable["e_tension"]["tension"] = true;
    p.capable["e_tension"]["torsion"] = true;

    lab::TestStage tension;
    tension.id = "tension";
    lab::TestStage torsionOp;
    torsionOp.id = "torsion";
    p.operations = {tension, torsionOp};

    p.specimens = {{"s1"}};
    p.required["s1"]["tension"] = true;
    p.required["s1"]["torsion"] = true;

    lab::ScenarioBundle bundle;
    bundle.problem = p;
    try {
        lab::Preprocessor{}.ensureValid(bundle);
    } catch (const std::runtime_error&) {
        return true;
    }
    std::cerr << "Reject before metrics FAILED: ensureValid should throw\n";
    return false;
}

bool testDemoSimpleOneTestPerSpecimen() {
    const auto bundle = lab::buildDemoSimple();
    if (bundle.problem.specimens.size() != 2) {
        std::cerr << "demo_simple should have 2 specimens\n";
        return false;
    }

    std::set<std::string> routeSpecimens;
    for (const auto& step : lab::RoutePlanner{}.buildBaselineRoute(bundle.problem).steps()) {
        if (!routeSpecimens.insert(step.specimenId).second) {
            std::cerr << "demo_simple route repeats specimen " << step.specimenId << "\n";
            return false;
        }
    }
    return lab::Preprocessor{}.validate(bundle).empty();
}

bool testAllDemoScenariosValid() {
    const std::vector<lab::ScenarioBundle> demos = {
        lab::buildDemoSimple(),
        lab::buildDemoTwoSpecimens(),
        lab::buildDemoForMode(lab::LabProgramMode::MechanicalExtended),
        lab::buildDemoForMode(lab::LabProgramMode::Thermomechanical),
    };

    lab::Preprocessor preprocessor;
    for (const auto& demo : demos) {
        const auto errors = preprocessor.validate(demo);
        if (!errors.empty()) {
            std::cerr << "Demo '" << demo.name << "' invalid: " << errors.front() << "\n";
            return false;
        }
        const auto route = lab::RoutePlanner{}.buildBaselineRoute(demo.problem);
        if (!lab::specimenAppearsOnceInRoute(route)) {
            std::cerr << "Demo '" << demo.name << "' route repeats specimen\n";
            return false;
        }
    }
    return true;
}

bool testThermalCycleAliasMerged() {
    const auto viaAlias = lab::buildDemoForMode(lab::LabProgramMode::ThermalCycle);
    const auto direct = lab::buildDemoForMode(lab::LabProgramMode::Thermomechanical);
    if (viaAlias.name != direct.name ||
        viaAlias.programMode != lab::LabProgramMode::Thermomechanical) {
        std::cerr << "ThermalCycle alias should resolve to Thermomechanical demo\n";
        return false;
    }
    const auto* spec = lab::findProgramMode(lab::LabProgramMode::ThermalCycle);
    if (!spec || spec->mode != lab::LabProgramMode::Thermomechanical) {
        std::cerr << "findProgramMode(ThermalCycle) should map to Thermomechanical spec\n";
        return false;
    }
    return true;
}

bool testProgramWorkTime() {
    const auto bundle = lab::buildDemoSimple();
    const auto route = lab::RoutePlanner{}.buildBaselineRoute(bundle.problem);
    const auto analysis = lab::RouteAnalyzer{}.analyze(bundle.problem, route);
    const auto eff = lab::ProgramEfficiencyEngine{}.compute(bundle.problem, route, analysis);

    bool ok = true;
    ok &= eff.time.workTimeMin > 0.0;
    ok &= eff.time.workTimeMin ==
          eff.time.prepMin + eff.time.testMin + eff.time.setupMin + eff.time.moveMin;
    ok &= eff.time.cycleMin > 0.0;
    ok &= eff.time.cycleMin <= eff.time.workTimeMin + 1e-6;
    ok &= eff.time.parallelismIndex > 0.0 && eff.time.parallelismIndex <= 1.0;

    const auto m = lab::MetricsEngine{}.compute(bundle.problem, route);
    ok &= std::abs(eff.time.workTimeMin - m.T_sum) < 1e-6;
    ok &= std::abs(eff.time.cycleMin - m.T_cycle) < 1e-6;

    if (!ok) {
        std::cerr << "Program work time FAILED: work=" << eff.time.workTimeMin
                  << " cycle=" << eff.time.cycleMin << " T_sum=" << m.T_sum << "\n";
    }
    return ok;
}

}  // namespace

int main() {
    if (!testCatalog8ProgramParams()) return EXIT_FAILURE;
    std::cout << "Catalog 8 program params OK\n";
    if (!testPipelineAppliesLayoutForUserInput()) return EXIT_FAILURE;
    std::cout << "Pipeline layout for user input OK\n";
    if (!testSetupOnModeChange()) return EXIT_FAILURE;
    std::cout << "Setup mode-change OK\n";
    if (!testCycleTimeAndUtilization()) return EXIT_FAILURE;
    std::cout << "Cycle time and utilization OK\n";
    if (!testValidIndependentSpecimens()) return EXIT_FAILURE;
    std::cout << "Valid s1 tension + s2 torsion OK\n";
    if (!testValidCombinedOperation()) return EXIT_FAILURE;
    std::cout << "Valid tension_torsion OK\n";
    if (!testRejectReusedSpecimen()) return EXIT_FAILURE;
    std::cout << "Reject reused specimen OK\n";
    if (!testRejectBeforeMetrics()) return EXIT_FAILURE;
    std::cout << "Reject before metrics OK\n";
    if (!testDemoSimpleOneTestPerSpecimen()) return EXIT_FAILURE;
    std::cout << "demo_simple one test per specimen OK\n";
    if (!testThermalCycleAliasMerged()) return EXIT_FAILURE;
    std::cout << "ThermalCycle alias merged OK\n";
    if (!testAllDemoScenariosValid()) return EXIT_FAILURE;
    std::cout << "All demo scenarios valid OK\n";
    if (!testProgramWorkTime()) return EXIT_FAILURE;
    std::cout << "Program work time OK\n";

    const auto bundle = lab::buildDemoSimple();
    lab::Preprocessor{}.ensureValid(bundle);
    lab::LabOptimiser optimiser;

    const auto result = optimiser.run(bundle);
    const auto& m = result.metrics;

    bool ok = true;

    ok &= m.T_sum > 0.0;
    ok &= m.T_cycle > 0.0;
    ok &= m.C > 0.0;
    ok &= m.N >= 0;
    ok &= m.L >= 0.0;
    ok &= m.etaAvg <= 1.0;

    lab::RoutePlanner planner;
    lab::MetricsEngine metrics;
    const auto bySpecimen = planner.buildBaselineRoute(bundle.problem);
    const auto byOperation = planner.buildGroupedRoute(bundle.problem);
    const double cMin = std::min(metrics.compute(bundle.problem, bySpecimen).C,
                                 metrics.compute(bundle.problem, byOperation).C);
    ok &= m.C <= cMin + 1e-6;

    const auto analysis = lab::RouteAnalyzer{}.analyze(bundle.problem, result.route);
    ok &= analysis.setupCount == m.N;
    ok &= analysis.routeLengthSteps == m.L;

    const auto stateMatrix = lab::StandStateAnalyzer{}.analyze(bundle.problem, result.route);
    ok &= stateMatrix.setupCount == m.N;
    for (const auto& ev : stateMatrix.events) {
        ok &= ev.operationAllowed;
    }

    if (!ok) {
        std::cerr << "Route metrics test FAILED\n";
        std::cerr << "T_sum=" << m.T_sum << " T_cycle=" << m.T_cycle << " C=" << m.C
                  << " N=" << m.N << " L=" << m.L << "\n";
        return EXIT_FAILURE;
    }
    std::cout << "Plan metrics properties OK (demo_simple)\n";
    return EXIT_SUCCESS;
}
