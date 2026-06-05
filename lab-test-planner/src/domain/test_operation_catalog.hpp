#pragma once

#include <string>
#include <vector>

namespace lab {

// Режим организации нагружения (гл. 1, §1.2–1.3)
enum class WorkloadRegime {
    ShortTerm,              // кратковременное статическое
    CyclicLong,             // усталость, длительное циклическое
    Thermal,                // статический / циклический нагрев
    Thermomechanical,       // нагрев под механической нагрузкой
    CombinedSequential,     // комбинированное последовательное
    CombinedSimultaneous,   // комбинированное одновременное
    Auxiliary,              // подготовка, охлаждение, контроль
};

// Утверждённые виды испытательных операций (гл. 1–2)
enum class TestOperationType {
    SpecimenPreparation,
    GeometryControl,
    Tension,
    Compression,
    Torsion,
    Bending,
    Fatigue,
    Hardness,
    ThermalStatic,
    ThermalCyclic,
    InductionHeating,
    Thermomechanical,
    CoolingHold,
    ResultsProcessing,
    TensionPlusTorsion,
    BendingPlusTorsion,
    CompressionPlusBending,
};

struct TestOperationDef {
    TestOperationType type{};
    std::string id;
    std::string nameRu;
    WorkloadRegime regime = WorkloadRegime::ShortTerm;
    bool destructive = true;
    bool combined = false;
    // операции, которые должны предшествовать данной (по умолчанию, §2.3)
    std::vector<TestOperationType> mustFollow;
};

[[nodiscard]] const std::vector<TestOperationDef>& approvedOperations();
[[nodiscard]] const TestOperationDef* findOperationDef(TestOperationType type);
[[nodiscard]] const TestOperationDef* findOperationDefById(const std::string& id);
[[nodiscard]] std::vector<TestOperationType> operationsForRegime(WorkloadRegime regime);

}  // namespace lab
