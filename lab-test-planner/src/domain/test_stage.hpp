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
    double durationMin = 0.0;
    double costOp = 0.0;
    double costEnergy = 0.0;
    double laborHours = 0.0;
};

struct MechanicalTestStage : TestStage {
    MechanicalTestStage() { kind = TestKind::Mechanical; }
};

struct ThermalTestStage : TestStage {
    ThermalTestStage() { kind = TestKind::Thermal; }
};

}  // namespace lab
