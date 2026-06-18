#include "config/config_overrides.hpp"

namespace lab {

TestProgramDef applyProgramOverride(const TestProgramDef& base, const TestProgramOverride& patch) {
    TestProgramDef out = base;
    if (patch.timeModel) out.timeModel = *patch.timeModel;
    if (patch.normativeTimeMin) out.normativeTimeMin = *patch.normativeTimeMin;
    if (patch.modePowerKw) out.modePowerKw = *patch.modePowerKw;
    if (patch.setupTimeMin) out.setupTimeMin = *patch.setupTimeMin;
    if (patch.setupPowerKw) out.setupPowerKw = *patch.setupPowerKw;
    if (patch.deformationEnergyUsed) out.deformationEnergyUsed = *patch.deformationEnergyUsed;
    if (patch.cyclesCount) out.cyclesCount = *patch.cyclesCount;
    if (patch.frequencyHz) out.frequencyHz = *patch.frequencyHz;
    if (patch.cycleTimeMin) out.cycleTimeMin = *patch.cycleTimeMin;
    if (patch.heatingTimeMin) out.heatingTimeMin = *patch.heatingTimeMin;
    if (patch.holdingTimeMin) out.holdingTimeMin = *patch.holdingTimeMin;
    if (patch.coolingTimeMin) out.coolingTimeMin = *patch.coolingTimeMin;
    if (patch.sigmaMpa) out.sigmaMpa = *patch.sigmaMpa;
    if (patch.deltaT_C) out.deltaT_C = *patch.deltaT_C;
    return out;
}

TestProgramDef resolveTestProgram(const TestProgramDef& base, const LabConfigOverrides* overrides) {
    if (!overrides) return base;
    for (const auto& patch : overrides->programs) {
        if (patch.equipmentId == base.equipmentId && patch.operationId == base.operationId) {
            return applyProgramOverride(base, patch);
        }
    }
    return base;
}

}  // namespace lab
