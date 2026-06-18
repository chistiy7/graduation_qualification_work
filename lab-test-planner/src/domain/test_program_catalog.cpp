#include "domain/test_program_catalog.hpp"

#include <cmath>
#include <stdexcept>

namespace lab {

namespace {

TestProgramDef makeProgram(TestOperationType type, std::string equipId, std::string opId,
                         std::string name, ProgramTimeModel model, double normMin,
                         double modeP, double setupMin, double setupP,
                         std::string stop, bool deformUsed, std::string source,
                         double sigma = 0.0, double deltaT = 0.0, int cycles = 0,
                         double freqHz = 0.0, double cycleMin = 0.0) {
    TestProgramDef p;
    p.equipmentId = std::move(equipId);
    p.operationId = std::move(opId);
    p.operationType = type;
    p.programName = std::move(name);
    p.timeModel = model;
    p.normativeTimeMin = normMin;
    p.modePowerKw = modeP;
    p.setupTimeMin = setupMin;
    p.setupPowerKw = setupP;
    p.stopCriterion = std::move(stop);
    p.deformationEnergyUsed = deformUsed;
    p.sourceType = std::move(source);
    p.sigmaMpa = sigma;
    p.deltaT_C = deltaT;
    p.cyclesCount = cycles;
    p.frequencyHz = freqHz;
    p.cycleTimeMin = cycleMin;
    return p;
}

std::vector<TestProgramDef> buildPrograms() {
    return {
        // 1. e_tension
        makeProgram(TestOperationType::Tension, "e_tension", "tension",
                    "Static tensile test to fracture", ProgramTimeModel::RegulatedTime, 20.0,
                    1.5, 10.0, 0.5, "fracture OR max_strain OR load_drop", true,
                    "стандарт (ГОСТ 1497-84) + расчётная технологическая база ПО", 600.0),

        // 2. e_compression
        makeProgram(TestOperationType::Compression, "e_compression", "compression",
                    "Static compression test", ProgramTimeModel::RegulatedTime, 15.0, 1.5, 10.0,
                    0.5, "target_force OR target_strain OR instability OR max_time", true,
                    "стандарт (ГОСТ 25.503-97) + расчётная технологическая база ПО", 500.0),

        // 3. e_torsion
        makeProgram(TestOperationType::Torsion, "e_torsion", "torsion",
                    "Static torsion test to fracture", ProgramTimeModel::RegulatedTime, 15.0,
                    1.0, 10.0, 0.5, "fracture OR target_angle OR torque_drop OR max_time", true,
                    "стандарт (ГОСТ 3565-80) + расчётная технологическая база ПО", 550.0),

        // 4. e_bending
        makeProgram(TestOperationType::Bending, "e_bending", "bending", "Static bending test",
                    ProgramTimeModel::RegulatedTime, 15.0, 1.5, 10.0, 0.5,
                    "fracture OR target_deflection OR load_drop", true,
                    "стандарт (ГОСТ 14019-2003) + расчётная технологическая база ПО", 450.0),

        // 5. e_fatigue
        makeProgram(TestOperationType::Fatigue, "e_fatigue", "fatigue", "Axial fatigue test",
                    ProgramTimeModel::CycleProgram, 166.7, 5.0, 20.0, 1.0,
                    "fracture OR target_cycles OR stiffness_drop", true,
                    "стандарт (ГОСТ 25.502-79) + расчётная технологическая база ПО", 300.0, 0.0,
                    100000, 10.0),

        // 6. e_tension_torsion
        makeProgram(TestOperationType::TensionPlusTorsion, "e_tension_torsion", "tension_torsion",
                    "Combined axial tension and torsion test", ProgramTimeModel::RegulatedTime,
                    30.0, 2.3, 15.0, 0.8,
                    "fracture OR target_axial_strain_and_twist OR load_drop", true,
                    "расчётная технологическая база ПО", 630.0),

        // 7. e_bending_torsion
        makeProgram(TestOperationType::BendingPlusTorsion, "e_bending_torsion", "bending_torsion",
                    "Combined bending and torsion test", ProgramTimeModel::RegulatedTime, 30.0,
                    2.3, 15.0, 0.8, "fracture OR target_moment_and_twist OR torque_drop", true,
                    "расчётная технологическая база ПО", 580.0),

        // 8. e_thermo_mech
        makeProgram(TestOperationType::Thermomechanical, "e_thermo_mech", "thermo_mech",
                    "Thermomechanical loading", ProgramTimeModel::ThermomechanicalProgram, 100.0,
                    10.0, 20.0, 1.0,
                    "fracture OR target_cycles OR strain_limit OR temperature_cycle_complete",
                    true, "стандарт (ГОСТ 9651-84) + паспорт стенда + расчётная технологическая база ПО",
                    480.0, 275.0, 20, 0.0, 5.0),
    };
}

}  // namespace

const std::vector<TestProgramDef>& approvedTestPrograms() {
    static const auto programs = buildPrograms();
    return programs;
}

const TestProgramDef* findTestProgram(const std::string& equipmentId,
                                      const std::string& operationId) {
    for (const auto& p : approvedTestPrograms()) {
        if (p.equipmentId == equipmentId && p.operationId == operationId) return &p;
    }
    return nullptr;
}

const TestProgramDef* findTestProgramForOperation(TestOperationType type) {
    for (const auto& p : approvedTestPrograms()) {
        if (p.operationType == type) return &p;
    }
    return nullptr;
}

double resolveProgramTimeMin(const TestProgramDef& program) {
    switch (program.timeModel) {
    case ProgramTimeModel::RegulatedTime:
        return program.normativeTimeMin;
    case ProgramTimeModel::CycleProgram:
        if (program.frequencyHz <= 0.0) {
            throw std::runtime_error("CycleProgram: frequencyHz must be > 0 for " +
                                     program.operationId);
        }
        return static_cast<double>(program.cyclesCount) / program.frequencyHz / 60.0;
    case ProgramTimeModel::ThermalProgram: {
        const double sum = program.heatingTimeMin + program.holdingTimeMin + program.coolingTimeMin;
        return sum > 0.0 ? sum : program.normativeTimeMin;
    }
    case ProgramTimeModel::ThermalCycleProgram:
    case ProgramTimeModel::ThermomechanicalProgram:
        return static_cast<double>(program.cyclesCount) * program.cycleTimeMin;
    }
    return program.normativeTimeMin;
}

std::string programTimeModelName(ProgramTimeModel model) {
    switch (model) {
    case ProgramTimeModel::RegulatedTime:
        return "RegulatedTime";
    case ProgramTimeModel::CycleProgram:
        return "CycleProgram";
    case ProgramTimeModel::ThermalProgram:
        return "ThermalProgram";
    case ProgramTimeModel::ThermalCycleProgram:
        return "ThermalCycleProgram";
    case ProgramTimeModel::ThermomechanicalProgram:
        return "ThermomechanicalProgram";
    }
    return "Unknown";
}

}  // namespace lab
