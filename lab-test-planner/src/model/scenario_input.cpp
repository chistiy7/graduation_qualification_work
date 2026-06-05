#include "model/scenario_input.hpp"

#include "domain/stand_catalog.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace lab {

namespace {

struct OpPar {
    double durationMin, costOp, costEnergy, laborHours;
};

OpPar defaultOpPar(TestOperationType type) {
    switch (type) {
    case TestOperationType::Tension:        return {30.0, 200.0, 300.0, 0.20};
    case TestOperationType::Torsion:        return {40.0, 250.0, 350.0, 0.25};
    case TestOperationType::Bending:        return {35.0, 220.0, 320.0, 0.22};
    case TestOperationType::Compression:    return {30.0, 200.0, 300.0, 0.20};
    case TestOperationType::Fatigue:        return {120.0, 400.0, 500.0, 0.40};
    case TestOperationType::TensionPlusTorsion:  return {60.0, 350.0, 450.0, 0.35};
    case TestOperationType::BendingPlusTorsion:  return {60.0, 360.0, 460.0, 0.35};
    case TestOperationType::Thermomechanical:    return {90.0, 350.0, 600.0, 0.35};
    default:                                return {15.0, 80.0, 100.0, 0.10};
    }
}

struct StandPar {
    double setupTimeMin, setupCost, amortPerHour, fundTimeMin, cellPlacementCost;
};

StandPar defaultStandPar(StandType type) {
    constexpr double kCell = 5000.0;
    switch (type) {
    case StandType::UniversalTensile:     return {10.0, 200.0, 2000.0, 240.0, kCell};
    case StandType::TorsionMachine:       return {10.0, 200.0, 2500.0, 240.0, kCell};
    case StandType::BendingRig:           return {12.0, 220.0, 1800.0, 200.0, kCell};
    case StandType::FatigueStand:         return {15.0, 300.0, 3000.0, 480.0, kCell * 1.5};
    case StandType::ThermomechanicalUnit: return {25.0, 450.0, 3500.0, 360.0, kCell * 2.0};
    default:                              return {10.0, 200.0, 2000.0, 240.0, kCell};
    }
}

// Стенд, способный выполнить операцию
const StandDef* standForOperation(TestOperationType op) {
    for (const auto& s : approvedStands()) {
        for (const auto cap : s.capableOperations) {
            if (cap == op) return &s;
        }
    }
    return nullptr;
}

TestStage makeStage(TestOperationType type) {
    const auto* def = findOperationDef(type);
    if (!def) throw std::runtime_error("unknown operation type");
    const auto par = defaultOpPar(type);

    TestStage stage;
    stage.id = def->id;
    stage.operationType = type;
    stage.regime = def->regime;
    stage.nameRu = def->nameRu;
    stage.destructive = def->destructive;
    stage.combined = def->combined;
    stage.durationMin = par.durationMin;
    stage.costOp = par.costOp;
    stage.costEnergy = par.costEnergy;
    stage.laborHours = par.laborHours;

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
    }
    return stage;
}

}  // namespace

int gridRowsFromInput(const ScenarioInput& in) {
    return std::max(1, static_cast<int>(std::lround(in.roomLengthM / in.cellSizeM)));
}

int gridColsFromInput(const ScenarioInput& in) {
    return std::max(1, static_cast<int>(std::lround(in.roomWidthM / in.cellSizeM)));
}

const std::vector<TestOperationType>& selectableTestTypes() {
    static const std::vector<TestOperationType> types = {
        TestOperationType::Tension,
        TestOperationType::Torsion,
        TestOperationType::Bending,
        TestOperationType::Compression,
        TestOperationType::Fatigue,
        TestOperationType::TensionPlusTorsion,
        TestOperationType::BendingPlusTorsion,
        TestOperationType::Thermomechanical,
    };
    return types;
}

std::string testTypeNameRu(TestOperationType type) {
    if (const auto* def = findOperationDef(type)) return def->nameRu;
    return "?";
}

ScenarioBundle buildFromInput(const ScenarioInput& in) {
    if (in.testTypes.empty()) {
        throw std::runtime_error("не выбран ни один тип испытаний");
    }

    ScenarioBundle bundle;
    bundle.name = "user_scenario";
    bundle.description = "Пользовательский сценарий (площадь, партия, типы испытаний)";

    auto& p = bundle.problem;
    p.laborRatePerHour = in.laborRatePerHour;
    p.electricityTariffPerKwh = in.energyTariffPerKwh;
    p.gridCellSizeM = in.cellSizeM;
    p.minutesPerGridStep = 1.0;
    p.gridRows = gridRowsFromInput(in);
    p.gridCols = gridColsFromInput(in);
    p.objectiveMode = ObjectiveMode::TotalCostRub;

    // Операции из выбранных типов (без дублей)
    std::vector<TestOperationType> ops;
    for (const auto t : in.testTypes) {
        if (std::find(ops.begin(), ops.end(), t) == ops.end()) ops.push_back(t);
    }
    for (const auto t : ops) {
        p.operations.push_back(makeStage(t));
    }

    // Стенды: уникальные по типу, способные выполнить выбранные операции
    std::vector<StandType> standTypes;
    for (const auto t : ops) {
        const auto* sdef = standForOperation(t);
        if (!sdef) throw std::runtime_error("нет стенда для операции: " + testTypeNameRu(t));
        if (std::find(standTypes.begin(), standTypes.end(), sdef->type) == standTypes.end()) {
            standTypes.push_back(sdef->type);
        }
    }
    for (const auto st : standTypes) {
        const auto* sdef = findStandDef(st);
        const auto par = defaultStandPar(st);
        LabEquipment eq;
        eq.id = sdef->idPrefix;
        eq.cellId.clear();  // координаты назначит оптимизатор
        eq.standType = st;
        eq.nameRu = sdef->nameRu;
        eq.setupTimeMin = par.setupTimeMin;
        eq.setupCost = par.setupCost;
        eq.amortPerHour = par.amortPerHour;
        eq.fundTimeMin = par.fundTimeMin;
        eq.cellPlacementCost = par.cellPlacementCost;
        p.equipment.push_back(eq);

        for (const auto t : ops) {
            if (standCanExecute(st, t)) {
                if (const auto* odef = findOperationDef(t)) p.capable[eq.id][odef->id] = true;
            }
        }
    }

    // Партия образцов: каждый требует все выбранные операции
    for (int i = 0; i < std::max(1, in.batchSize); ++i) {
        Specimen s;
        s.id = "s" + std::to_string(i + 1);
        s.role = SpecimenRole::Primary;
        s.prepTimeMin = 10.0;
        s.prepLaborHours = 0.15;
        p.specimens.push_back(s);
        for (const auto t : ops) {
            if (const auto* odef = findOperationDef(t)) p.required[s.id][odef->id] = true;
        }
    }

    // Предшествование из каталога (mustFollow) среди выбранных операций
    for (const auto t : ops) {
        const auto* def = findOperationDef(t);
        if (!def) continue;
        for (const auto pred : def->mustFollow) {
            if (std::find(ops.begin(), ops.end(), pred) == ops.end()) continue;
            const auto* pdef = findOperationDef(pred);
            if (pdef) p.precedence.emplace_back(pdef->id, def->id);
        }
    }

    return bundle;
}

}  // namespace lab
