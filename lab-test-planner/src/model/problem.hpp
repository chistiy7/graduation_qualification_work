#pragma once

#include "domain/lab_equipment.hpp"
#include "domain/laboratory.hpp"
#include "domain/specimen.hpp"
#include "domain/test_stage.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace lab {

struct ProblemDefinition {
    std::vector<Specimen> specimens;
    std::vector<TestStage> operations;
    std::vector<LabEquipment> equipment;
    Laboratory laboratory;

    std::unordered_map<std::string, std::unordered_map<std::string, bool>> required;
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> capable;
    std::vector<std::pair<std::string, std::string>> precedence;

    double laborRatePerHour = 0.0;
    double electricityTariffPerKwh = 0.0;
    double rentRatePerM2Hour = 0.0;  // 0 → RENT_RATE_DEFAULT_RUB_M2_HOUR
    double gridCellSizeM = 2.0;
    double minutesPerGridStep = 1.0;
    int gridRows = 0;
    int gridCols = 0;
};

struct EquipmentUtilization {
    std::string equipmentId;
    double testMin = 0.0;
    double setupMin = 0.0;
    double busyMin = 0.0;
    double idleMin = 0.0;
    double utilization = 0.0;  // t_busy / T_cycle, доля 0..1
    double energyWorkKwh = 0.0;
    double energySetupKwh = 0.0;
    double costEnergyWork = 0.0;
    double costEnergySetup = 0.0;
    double costSetup = 0.0;
    double costAmort = 0.0;
};

struct CostBreakdown {
    double prepLabor = 0.0;
    double operationMaterials = 0.0;
    double energyWork = 0.0;
    double operationLabor = 0.0;
    double setup = 0.0;
    double energySetup = 0.0;
    double amortization = 0.0;
    double transport = 0.0;
    double area = 0.0;  // C_area

    [[nodiscard]] double total() const {
        return prepLabor + operationMaterials + energyWork + operationLabor + setup +
               energySetup + amortization + transport + area;
    }
    [[nodiscard]] double withoutArea() const { return total() - area; }
};

struct VariantMetrics {
    double T_sum = 0.0;
    double T_cycle = 0.0;
    double T = 0.0;  // = T_sum (совместимость)
    double C = 0.0;
    int N = 0;
    double L = 0.0;
    double etaAvg = 0.0;  // доля 0..1
    std::string bottleneckEquipmentId;
    CostBreakdown cost;
    double totalIdleMin = 0.0;
    std::vector<EquipmentUtilization> equipmentStats;
};

}  // namespace lab
