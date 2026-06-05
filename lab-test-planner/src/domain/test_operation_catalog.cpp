#include "domain/test_operation_catalog.hpp"

#include <algorithm>
#include <stdexcept>

namespace lab {

namespace {

std::vector<TestOperationDef> buildCatalog() {
    return {
        {TestOperationType::SpecimenPreparation, "prep", "Подготовка образца",
         WorkloadRegime::Auxiliary, false, false, {}},
        {TestOperationType::GeometryControl, "geom", "Контроль геометрии",
         WorkloadRegime::Auxiliary, false, false, {TestOperationType::SpecimenPreparation}},
        {TestOperationType::Tension, "tension", "Испытание на растяжение",
         WorkloadRegime::ShortTerm, true, false, {TestOperationType::GeometryControl}},
        {TestOperationType::Compression, "compression", "Испытание на сжатие",
         WorkloadRegime::ShortTerm, true, false, {TestOperationType::GeometryControl}},
        {TestOperationType::Torsion, "torsion", "Испытание на кручение",
         WorkloadRegime::ShortTerm, true, false, {TestOperationType::GeometryControl}},
        {TestOperationType::Bending, "bending", "Испытание на изгиб",
         WorkloadRegime::ShortTerm, true, false, {TestOperationType::GeometryControl}},
        {TestOperationType::Fatigue, "fatigue", "Испытание на усталость",
         WorkloadRegime::CyclicLong, true, false, {TestOperationType::GeometryControl}},
        {TestOperationType::Hardness, "hardness", "Измерение твердости",
         WorkloadRegime::ShortTerm, false, false, {TestOperationType::GeometryControl}},
        {TestOperationType::ThermalStatic, "thermal_static", "Статический нагрев",
         WorkloadRegime::Thermal, true, false, {TestOperationType::GeometryControl}},
        {TestOperationType::ThermalCyclic, "thermal_cyclic", "Циклический термический режим",
         WorkloadRegime::Thermal, true, false, {TestOperationType::ThermalStatic}},
        {TestOperationType::InductionHeating, "induction", "Индукционный нагрев",
         WorkloadRegime::Thermal, true, false, {TestOperationType::GeometryControl}},
        {TestOperationType::Thermomechanical, "thermo_mech", "Термомеханическое испытание",
         WorkloadRegime::Thermomechanical, true, false,
         {TestOperationType::ThermalStatic, TestOperationType::InductionHeating}},
        {TestOperationType::CoolingHold, "cooling", "Охлаждение и выдержка",
         WorkloadRegime::Auxiliary, false, false,
         {TestOperationType::ThermalStatic, TestOperationType::InductionHeating,
          TestOperationType::Thermomechanical}},
        {TestOperationType::ResultsProcessing, "results", "Обработка результатов",
         WorkloadRegime::Auxiliary, false, false, {}},
        {TestOperationType::TensionPlusTorsion, "tension_torsion", "Растяжение + кручение",
         WorkloadRegime::CombinedSequential, true, true,
         {TestOperationType::Tension, TestOperationType::Torsion}},
        {TestOperationType::BendingPlusTorsion, "bending_torsion", "Изгиб + кручение",
         WorkloadRegime::CombinedSequential, true, true,
         {TestOperationType::Bending, TestOperationType::Torsion}},
        {TestOperationType::CompressionPlusBending, "compression_bending", "Сжатие + изгиб",
         WorkloadRegime::CombinedSequential, true, true,
         {TestOperationType::Compression, TestOperationType::Bending}},
    };
}

}  // namespace

const std::vector<TestOperationDef>& approvedOperations() {
    static const auto catalog = buildCatalog();
    return catalog;
}

const TestOperationDef* findOperationDef(TestOperationType type) {
    for (const auto& def : approvedOperations()) {
        if (def.type == type) return &def;
    }
    return nullptr;
}

const TestOperationDef* findOperationDefById(const std::string& id) {
    for (const auto& def : approvedOperations()) {
        if (def.id == id) return &def;
    }
    return nullptr;
}

std::vector<TestOperationType> operationsForRegime(WorkloadRegime regime) {
    std::vector<TestOperationType> out;
    for (const auto& def : approvedOperations()) {
        if (def.regime == regime) out.push_back(def.type);
    }
    return out;
}

}  // namespace lab
