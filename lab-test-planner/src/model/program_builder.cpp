#include "model/program_builder.hpp"

#include "config/program_resolver.hpp"
#include "domain/physics_constants.hpp"
#include "domain/stand_catalog.hpp"
#include "domain/test_operation_catalog.hpp"
#include "domain/test_program_catalog.hpp"
#include "engine/operation_energy.hpp"

#include <algorithm>
#include <stdexcept>

namespace lab {

namespace {

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

TestStage makeStageFromCatalog(TestOperationType type, const OperationParams* overridePar) {
    auto stage = buildStageFromProgram(type);
    if (!overridePar) return stage;

    if (overridePar->durationMin > 0.0) {
        stage.durationNormMin = overridePar->durationMin;
        stage.durationMin = overridePar->durationMin;
        stage.cycleTimeMin = overridePar->durationMin;
    }
    if (overridePar->costOp > 0.0) stage.costOp = overridePar->costOp;
    if (overridePar->laborHours > 0.0) stage.laborHours = overridePar->laborHours;
    return stage;
}

StandParams defaultStandParams(StandType type, int col) {
    const auto* sdef = findStandDef(type);
    const auto econ = defaultStandEconomics(type);
    StandParams sp;
    sp.setupCost = econ.setupCost;
    sp.amortPerHour = econ.amortPerHour;
    sp.fundTimeMin = econ.fundTimeMin;
    sp.cellPlacementCost = econ.cellPlacementCost;
    sp.gridCol = col;
    if (sdef) {
        for (const auto& prog : approvedTestPrograms()) {
            if (prog.equipmentId == sdef->idPrefix) {
                sp.setupTimeMin = prog.setupTimeMin;
                break;
            }
        }
    }
    if (sp.setupTimeMin <= 0.0) sp.setupTimeMin = 10.0;
    return sp;
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
    p.electricityTariffPerKwh = request.energyTariffPerKwh > 0.0
                                    ? request.energyTariffPerKwh
                                    : ENERGY_TARIFF_DEFAULT_RUB_KWH;
    p.minutesPerGridStep = request.minutesPerGridStep;
    p.gridCellSizeM = request.gridCellSizeM;

    std::vector<TestOperationType> routableOps;
    for (const auto opType : modeSpec->operations) {
        if (!findTestProgramForOperation(opType)) continue;

        routableOps.push_back(opType);

        const OperationParams* opPar = nullptr;
        OperationParams stored;
        if (auto it = request.operationParams.find(opType); it != request.operationParams.end()) {
            stored = it->second;
            opPar = &stored;
        }
        p.operations.push_back(makeStageFromCatalog(opType, opPar));
    }

    applyDefaultPrecedence(p, modeSpec->operations);

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
        eq.nominalPowerKw = sdef->nominalPowerKw;
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

    for (const auto& spec : request.specimens) {
        Specimen s;
        s.id = spec.id;
        s.role = spec.role;
        s.prepTimeMin = spec.prepTimeMin;
        s.prepLaborHours = spec.prepLaborHours;
        s.volumeM3 = request.specimenVolumeM3;
        p.specimens.push_back(s);
        for (const auto opType : spec.requiredOperations) {
            if (const auto* odef = findOperationDef(opType)) {
                p.required[spec.id][odef->id] = true;
            }
        }
    }

    finalizeOperationTimingAndEnergy(p, request.specimenVolumeM3);

    return bundle;
}

}  // namespace lab
