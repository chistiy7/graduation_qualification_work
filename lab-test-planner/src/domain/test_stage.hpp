#pragma once

#include "domain/test_operation_catalog.hpp"

#include <string>

namespace lab {

enum class TestKind { Mechanical, Thermal, Combined };

// Испытательная операция o ∈ O
struct TestStage {
    std::string id;
    TestOperationType operationType{};
    std::string nameRu;
    TestKind kind = TestKind::Mechanical;
    WorkloadRegime regime = WorkloadRegime::ShortTerm;
    bool destructive = true;
    bool combined = false;
    double durationMin = 0.0;        // t_program — норматив программы, мин
    double durationNormMin = 0.0;   // t_program (источник: техкарта), мин
    double cycleTimeMin = 0.0;       // = t_program (занятость стенда на операцию)
    double modePowerKw = 0.0;        // P_work — мощность в рабочем режиме программы
    double setupPowerKw = 0.0;       // P_setup — мощность при переналадке программы
    double programSetupTimeMin = 0.0; // t_setup из техкарты программы
    bool deformationEnergyUsed = false;
    double costOp = 0.0;
    // C_энерг = modePowerKw · t_program / 60 · тариф; W — только контроль.
    double costEnergy = 0.0;
    double laborHours = 0.0;
    double sigmaMpa = 0.0;
    double deltaT_C = 0.0;
    double deformationEnergyJ = 0.0;  // W = u·V
    double minLoadTimeMin = 0.0;      // t_load_min = W/(η·P_nom)
    double energyKwh = 0.0;
    std::string energyWarning;        // предупреждение при t_norm < t_load_min
};

struct MechanicalTestStage : TestStage {
    MechanicalTestStage() { kind = TestKind::Mechanical; }
};

struct ThermalTestStage : TestStage {
    ThermalTestStage() { kind = TestKind::Thermal; }
};

}  // namespace lab
