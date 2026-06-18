#include "engine/operation_energy.hpp"

#include "domain/physics_constants.hpp"
#include "domain/test_program_catalog.hpp"
#include "model/problem.hpp"

#include <cmath>

namespace lab {

namespace {

double specificEnergyJm3(TestKind kind, double sigmaMpa, double deltaT_C) {
    double u = 0.0;
    if (kind == TestKind::Mechanical || kind == TestKind::Combined) {
        const double sigma_pa = sigmaMpa * 1e6;
        const double E_pa = E_STEEL_GPA * 1e9;
        u += sigma_pa * sigma_pa / (2.0 * E_pa);
    }
    if (kind == TestKind::Thermal || kind == TestKind::Combined) {
        u += RHO_STEEL_KG_M3 * CP_STEEL_J_KG_K * deltaT_C;
    }
    return u;
}

}  // namespace

OperationNorms defaultOperationNorms(TestOperationType type) {
    OperationNorms n;
    if (const auto* prog = findTestProgramForOperation(type)) {
        n.durationNormMin = resolveProgramTimeMin(*prog);
        n.sigmaMpa = prog->sigmaMpa;
        n.deltaT_C = prog->deltaT_C;
        return n;
    }
    n = {15.0, 0.0, 0.0};
    return n;
}

OperationEnergyResult computeOperationEnergy(const OperationEnergyInput& in) {
    OperationEnergyResult out;
    out.specificEnergyJm3 = specificEnergyJm3(in.kind, in.sigmaMpa, in.deltaT_C);
    out.deformationEnergyJ = out.specificEnergyJm3 * in.volumeM3;
    out.durationMin = in.durationNormMin;
    out.cycleTimeMin = in.durationNormMin;

    if (in.deformationEnergyUsed && in.efficiency > 0.0 && in.checkPowerKw > 0.0 &&
        out.deformationEnergyJ > 0.0) {
        const double power_w = in.checkPowerKw * 1000.0;
        out.minLoadTimeMin = out.deformationEnergyJ / (in.efficiency * power_w) / 60.0;
        if (out.minLoadTimeMin > in.durationNormMin + 1e-9) {
            out.warning =
                "Нормативное время программы меньше физически оценочного времени передачи "
                "энергии деформации. Проверьте параметры материала, мощности или норматив.";
        }
    }

    if (in.modePowerKw > 0.0 && out.durationMin > 0.0) {
        out.energyKwh = in.modePowerKw * (out.durationMin / 60.0);
        out.costEnergyRub = out.energyKwh * in.tariffPerKwh;
    }

    return out;
}

void finalizeOperationTimingAndEnergy(ProblemDefinition& problem, double volumeM3) {
    const double V = volumeM3 > 0.0 ? volumeM3 : 3.93e-6;
    const double tariff = problem.electricityTariffPerKwh > 0.0
                              ? problem.electricityTariffPerKwh
                              : ENERGY_TARIFF_DEFAULT_RUB_KWH;

    for (auto& op : problem.operations) {
        const double tProgram =
            op.durationNormMin > 0.0 ? op.durationNormMin : op.durationMin;

        OperationEnergyInput in;
        in.kind = op.kind;
        in.volumeM3 = V;
        in.durationNormMin = tProgram;
        in.sigmaMpa = op.sigmaMpa;
        in.deltaT_C = op.deltaT_C;
        in.deformationEnergyUsed = op.deformationEnergyUsed;
        in.modePowerKw = op.modePowerKw;
        in.checkPowerKw = op.modePowerKw > 0.0 ? op.modePowerKw : 0.0;
        in.efficiency = 0.88;
        in.tariffPerKwh = tariff;

        const auto result = computeOperationEnergy(in);

        op.durationNormMin = tProgram;
        op.durationMin = result.durationMin;
        op.cycleTimeMin = result.cycleTimeMin;
        op.deformationEnergyJ = result.deformationEnergyJ;
        op.minLoadTimeMin = result.minLoadTimeMin;
        op.energyKwh = result.energyKwh;
        op.costEnergy = result.costEnergyRub;
        op.energyWarning = result.warning;
    }
}

}  // namespace lab
