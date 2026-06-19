#include "model/scenario_input.hpp"

#include "config/program_resolver.hpp"
#include "domain/stand_catalog.hpp"
#include "domain/test_program_catalog.hpp"
#include "engine/operation_energy.hpp"
#include "io/config_json.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace lab {

namespace {

LabConfigOverrides mergedOverrides(const ScenarioInput& in) {
    LabConfigOverrides merged = in.configOverrides;
    if (!in.overrideConfigPath.empty()) {
        const auto fromFile = loadConfigOverridesJson(in.overrideConfigPath);
        for (const auto& patch : fromFile.programs) {
            merged.programs.push_back(patch);
        }
    }
    return merged;
}

StandType standTypeForOperation(TestOperationType op, const LabConfigOverrides& overrides) {
    const auto program = programForOperation(op, &overrides);
    if (const auto* sdef = findStandDefById(program.equipmentId)) {
        return sdef->type;
    }
    throw std::runtime_error("неизвестный стенд для операции: " + program.equipmentId);
}

}  // namespace

int gridCellCount(const ScenarioInput& in) {
    const double cellArea = in.cellSizeM * in.cellSizeM;
    return std::max(1, static_cast<int>(std::lround(in.roomAreaM2 / cellArea)));
}

int gridRowsFromInput(const ScenarioInput& in) {
    const int n = gridCellCount(in);
    return std::max(1, static_cast<int>(std::lround(std::sqrt(static_cast<double>(n)))));
}

int gridColsFromInput(const ScenarioInput& in) {
    const int n = gridCellCount(in);
    const int rows = gridRowsFromInput(in);
    return std::max(1, (n + rows - 1) / rows);
}

std::string formatGridSetupNote(const ScenarioInput& in) {
    const int rows = gridRowsFromInput(in);
    const int cols = gridColsFromInput(in);
    std::ostringstream ss;
    ss << "Сетка помещения: " << rows << " × " << cols << " = " << rows * cols
       << " ячеек (модуль " << in.cellSizeM << "×" << in.cellSizeM << " м, стенд = 1 ячейка).\n"
       << "Зоны безопасности: DirectionalBuffer (front/back/side) из stand_catalog по §2.2 "
          "(ГОСТ 12.2.061-81 и др.), с учётом ориентации стенда.\n"
       << "  механические стенды: front=1, back=0 у стены, side=1;\n"
       << "  усталость: {2,2,1}; термомеханика: {2,1,2}. На карте — клетки X (Buffer).\n";
    return ss.str();
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

ScenarioInput makeScenario8Types80Input() {
    ScenarioInput in;
    in.roomAreaM2 = 100.0;
    in.cellSizeM = 2.0;
    in.batchSize = 80;
    in.laborRatePerHour = 450.0;
    in.energyTariffPerKwh = 6.5;
    in.specimenVolumeM3 = 3.93e-6;  // d₀=10 мм, l₀=50 мм
    for (const auto testType : selectableTestTypes()) {
        in.groups.push_back(SpecimenGroup{10, testType});
    }
    return in;
}

ScenarioBundle buildScenario8Types80() {
    auto bundle = buildFromInput(makeScenario8Types80Input());
    bundle.name = "demo_8_types_80";
    bundle.description =
        "80 образцов: по 10 на каждый из 8 видов испытаний; 100 м², ставка 450 руб/ч, "
        "тариф 6.5 руб/кВт·ч, объём образца базовый (d₀=10 мм, l₀=50 мм).";
    return bundle;
}

ScenarioBundle buildFromInput(const ScenarioInput& in) {
    if (in.groups.empty()) {
        throw std::runtime_error("не задано ни одной группы образцов");
    }

    int groupSum = 0;
    for (const auto& g : in.groups) {
        if (g.count <= 0) throw std::runtime_error("число образцов в группе должно быть > 0");
        groupSum += g.count;
    }
    if (in.batchSize > 0 && groupSum != in.batchSize) {
        throw std::runtime_error("сумма образцов по группам (" + std::to_string(groupSum) +
                                 ") не равна общему числу (" + std::to_string(in.batchSize) + ")");
    }

    const auto overrides = mergedOverrides(in);

    ScenarioBundle bundle;
    bundle.name = "user_scenario";
    bundle.description = "Пользовательский сценарий (площадь, партия, группы испытаний)";

    auto& p = bundle.problem;
    p.laborRatePerHour = in.laborRatePerHour;
    p.electricityTariffPerKwh = in.energyTariffPerKwh;
    p.gridCellSizeM = in.cellSizeM;
    p.minutesPerGridStep = 1.0;
    if (in.roomAreaM2 > 0.0) {
        p.gridRows = gridRowsFromInput(in);
        p.gridCols = gridColsFromInput(in);
    } else {
        // Площадь не задана → САПР проектирует от минимальной необходимой площади
        // (gridRows/gridCols=0 — сигнал режима минимальной площади для optimizeLayout).
        p.gridRows = 0;
        p.gridCols = 0;
    }

    std::vector<TestOperationType> allOps;
    for (const auto& g : in.groups) {
        if (std::find(allOps.begin(), allOps.end(), g.testType) == allOps.end())
            allOps.push_back(g.testType);
    }
    for (const auto t : allOps) {
        p.operations.push_back(buildStageFromProgram(t, &overrides));
    }

    std::vector<StandType> standTypes;
    for (const auto t : allOps) {
        const auto st = standTypeForOperation(t, overrides);
        if (std::find(standTypes.begin(), standTypes.end(), st) == standTypes.end()) {
            standTypes.push_back(st);
        }
    }
    for (const auto st : standTypes) {
        const auto* sdef = findStandDef(st);
        if (!sdef) continue;

        TestProgramDef programForStand;
        bool foundProgram = false;
        for (const auto t : allOps) {
            const auto prog = programForOperation(t, &overrides);
            if (prog.equipmentId == sdef->idPrefix) {
                programForStand = prog;
                foundProgram = true;
                break;
            }
        }
        if (!foundProgram) continue;

        auto eq = buildEquipmentFromProgram(programForStand, st, &overrides);
        eq.cellId.clear();
        p.equipment.push_back(eq);

        for (const auto t : allOps) {
            if (standCanExecute(st, t)) {
                if (const auto* odef = findOperationDef(t)) p.capable[eq.id][odef->id] = true;
            }
        }
    }

    int specimenIdx = 0;
    for (const auto& g : in.groups) {
        const auto* odef = findOperationDef(g.testType);
        for (int i = 0; i < g.count; ++i) {
            Specimen s;
            s.id = "s" + std::to_string(++specimenIdx);
            s.role = SpecimenRole::Primary;
            s.prepTimeMin = 10.0;
            s.prepLaborHours = 0.15;
            s.volumeM3 = in.specimenVolumeM3;
            p.specimens.push_back(s);
            if (odef) p.required[s.id][odef->id] = true;
        }
    }

    for (const auto t : allOps) {
        const auto* def = findOperationDef(t);
        if (!def) continue;
        for (const auto pred : def->mustFollow) {
            if (std::find(allOps.begin(), allOps.end(), pred) == allOps.end()) continue;
            const auto* pdef = findOperationDef(pred);
            if (pdef) p.precedence.emplace_back(pdef->id, def->id);
        }
    }

    const double V = p.specimens.empty() ? in.specimenVolumeM3 : p.specimens.front().volumeM3;
    finalizeOperationTimingAndEnergy(p, V);

    return bundle;
}

}  // namespace lab
