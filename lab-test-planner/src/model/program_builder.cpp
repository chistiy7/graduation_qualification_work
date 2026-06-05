#include "model/program_builder.hpp"

#include "domain/stand_catalog.hpp"
#include "domain/test_operation_catalog.hpp"

#include <algorithm>
#include <stdexcept>

namespace lab {

namespace {

TestStage makeStage(TestOperationType type, const OperationParams& p) {
    const auto* def = findOperationDef(type);
    if (!def) throw std::runtime_error("unknown operation type in program");

    TestStage stage;
    stage.id = def->id;
    stage.operationType = type;
    stage.regime = def->regime;
    stage.nameRu = def->nameRu;
    stage.destructive = def->destructive;
    stage.combined = def->combined;
    stage.durationMin = p.durationMin;
    stage.costOp = p.costOp;
    stage.costEnergy = p.costEnergy;
    stage.laborHours = p.laborHours;

    switch (def->regime) {
    case WorkloadRegime::Thermal:
    case WorkloadRegime::Thermomechanical:
        stage.kind = TestKind::Thermal;
        break;
    case WorkloadRegime::CombinedSequential:
    case WorkloadRegime::CombinedSimultaneous:
        stage.kind = TestKind::Combined;
        break;
    default:
        stage.kind = TestKind::Mechanical;
        break;
    }
    return stage;
}

void addPrecedence(ProblemDefinition& problem, TestOperationType before,
                   TestOperationType after) {
    const auto* a = findOperationDef(after);
    const auto* b = findOperationDef(before);
    if (!a || !b) return;
    problem.precedence.emplace_back(b->id, a->id);
}

void applyDefaultPrecedence(ProblemDefinition& problem,
                            const std::vector<TestOperationType>& ops) {
    for (const auto type : ops) {
        const auto* def = findOperationDef(type);
        if (!def) continue;
        for (const auto pred : def->mustFollow) {
            if (std::find(ops.begin(), ops.end(), pred) == ops.end()) continue;
            addPrecedence(problem, pred, type);
        }
    }
}

OperationParams defaultOpParams(TestOperationType type) {
    switch (type) {
    case TestOperationType::Tension:
        return {30.0, 200.0, 300.0, 0.20};
    case TestOperationType::Torsion:
        return {40.0, 250.0, 350.0, 0.25};
    case TestOperationType::Bending:
        return {35.0, 220.0, 320.0, 0.22};
    case TestOperationType::Fatigue:
        return {120.0, 400.0, 500.0, 0.40};
    case TestOperationType::ThermalStatic:
        return {60.0, 180.0, 400.0, 0.18};
    case TestOperationType::Thermomechanical:
        return {90.0, 350.0, 600.0, 0.35};
    default:
        return {15.0, 80.0, 100.0, 0.10};
    }
}

StandParams defaultStandParams(StandType type, int col) {
    constexpr double kCellCost = 5000.0;  // руб/ячейка (2×2 м)
    switch (type) {
    case StandType::UniversalTensile:
        return {10.0, 200.0, 2000.0, 240.0, kCellCost, 0, col};
    case StandType::TorsionMachine:
        return {10.0, 200.0, 2500.0, 240.0, kCellCost, 0, col};
    case StandType::BendingRig:
        return {12.0, 220.0, 1800.0, 200.0, kCellCost, 0, col};
    case StandType::FatigueStand:
        return {15.0, 300.0, 3000.0, 480.0, kCellCost * 1.5, 0, col};
    case StandType::ThermalFurnace:
        return {20.0, 350.0, 2200.0, 300.0, kCellCost * 1.2, 0, col};
    case StandType::ThermomechanicalUnit:
        return {25.0, 450.0, 3500.0, 360.0, kCellCost * 2.0, 0, col};
    default:
        return {10.0, 200.0, 2000.0, 240.0, kCellCost, 0, col};
    }
}

}  // namespace

ScenarioBundle buildFromProgram(const ProgramBuildRequest& request) {
    const auto* modeSpec = findProgramMode(request.mode);
    if (!modeSpec) throw std::runtime_error("unknown program mode");

    ScenarioBundle bundle;
    bundle.name = request.scenarioName.empty() ? modeSpec->id : request.scenarioName;
    bundle.description = request.description.empty() ? modeSpec->descriptionRu : request.description;
    bundle.programMode = request.mode;

    auto& p = bundle.problem;
    p.laborRatePerHour = request.laborRatePerHour;
    p.minutesPerGridStep = request.minutesPerGridStep;
    p.gridCellSizeM = request.gridCellSizeM;
    p.objectiveMode = request.objectiveMode;
    p.weights = request.weights;

    // Операции режима (без чисто вспомогательных prep/results в маршруте стендов)
    std::vector<TestOperationType> routableOps;
    for (const auto opType : modeSpec->operations) {
        if (opType == TestOperationType::SpecimenPreparation ||
            opType == TestOperationType::ResultsProcessing) {
            continue;
        }
        routableOps.push_back(opType);

        OperationParams opPar = defaultOpParams(opType);
        if (auto it = request.operationParams.find(opType); it != request.operationParams.end()) {
            opPar = it->second;
        }
        p.operations.push_back(makeStage(opType, opPar));
    }

    applyDefaultPrecedence(p, modeSpec->operations);

    // Стенды режима
    int col = 0;
    for (const auto standType : modeSpec->stands) {
        const auto* sdef = findStandDef(standType);
        if (!sdef) continue;

        StandParams sp = defaultStandParams(standType, col);
        if (auto it = request.standParams.find(standType); it != request.standParams.end()) {
            sp = it->second;
        }

        const std::string eqId = sdef->idPrefix;
        const std::string cellId = eqId + "_cell";
        p.laboratory.addCell({cellId, sp.gridRow, sp.gridCol, LabCellKind::Stand});
        p.laboratory.placeEquipment(eqId, cellId);

        LabEquipment eq;
        eq.id = eqId;
        eq.cellId = cellId;
        eq.standType = standType;
        eq.nameRu = sdef->nameRu;
        eq.setupTimeMin = sp.setupTimeMin;
        eq.setupCost = sp.setupCost;
        eq.amortPerHour = sp.amortPerHour;
        eq.fundTimeMin = sp.fundTimeMin;
        eq.cellPlacementCost = sp.cellPlacementCost;
        p.equipment.push_back(eq);

        for (const auto opType : sdef->capableOperations) {
            if (std::find(routableOps.begin(), routableOps.end(), opType) == routableOps.end()) {
                continue;
            }
            if (const auto* odef = findOperationDef(opType)) {
                p.capable[eqId][odef->id] = true;
            }
        }
        col += 4;
    }

    // Вспомогательные зоны (атрибуты планировки, §2.1)
    int zcol = 0;
    for (const auto zoneType : modeSpec->zones) {
        for (const auto& zdef : approvedAuxiliaryZones()) {
            if (zdef.type != zoneType) continue;
            const std::string cellId = zdef.idPrefix + "_cell";
            p.laboratory.addCell({cellId, 2, zcol, LabCellKind::Buffer});
            p.laboratory.placeZone(zdef.idPrefix, cellId);
            zcol += 2;
            break;
        }
    }

    // Образцы и Req
    for (const auto& spec : request.specimens) {
        p.specimens.push_back(
            {spec.id, spec.role, spec.prepTimeMin, spec.prepLaborHours});
        for (const auto opType : spec.requiredOperations) {
            if (const auto* odef = findOperationDef(opType)) {
                p.required[spec.id][odef->id] = true;
            }
        }
    }

    return bundle;
}

}  // namespace lab
