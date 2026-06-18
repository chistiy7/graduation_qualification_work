#pragma once

#include "domain/test_program_catalog.hpp"
#include "model/problem.hpp"

#include "domain/test_stage.hpp"

#include <string>

namespace lab {

// Нормативные параметры операции (legacy-обёртка над каталогом программ).
struct OperationNorms {
    double durationNormMin = 0.0;
    double sigmaMpa = 0.0;
    double deltaT_C = 0.0;
};

[[nodiscard]] OperationNorms defaultOperationNorms(TestOperationType type);

struct OperationEnergyInput {
    TestKind kind = TestKind::Mechanical;
    double volumeM3 = 0.0;
    double durationNormMin = 0.0;
    double sigmaMpa = 0.0;
    double deltaT_C = 0.0;
    bool deformationEnergyUsed = false;
    double modePowerKw = 0.0;
    double checkPowerKw = 0.0;
    double efficiency = 0.88;
    double tariffPerKwh = 0.0;
};

struct OperationEnergyResult {
    double specificEnergyJm3 = 0.0;
    double deformationEnergyJ = 0.0;
    double minLoadTimeMin = 0.0;
    double durationMin = 0.0;
    double cycleTimeMin = 0.0;
    double energyKwh = 0.0;
    double costEnergyRub = 0.0;
    std::string warning;
};

// t_program из техкарты; E_work = modePowerKw × t_program / 60.
[[nodiscard]] OperationEnergyResult computeOperationEnergy(const OperationEnergyInput& in);

void finalizeOperationTimingAndEnergy(ProblemDefinition& problem, double volumeM3);

}  // namespace lab
