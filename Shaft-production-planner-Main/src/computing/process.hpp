#pragma once

#include <vector>
#include "mechanicalStage.hpp"
#include "thermalStage.hpp"


class Process {
private:
    // Time on production of one part
    static float timePerPart;
    // Time to produce whole batch
    static float timePerBatch;
    // Total cost of single part and whole batch
    static float totalPartCost, totalBatchCost;

protected:
    // Input part
    // Parts for calculations
    static Part targetPart, blankPart;
    static float partProductionTarget;
    static int targetBatchVolume;

    // Function for update process
    static void recalculate();
    static void calculateBatchTime();

    // Calculated parameters
    // Step counts
    static int startStep, endStep;
    // Process mechanical steps
    static MechanicalStage mechanicalStages[4];
    // Process thermal steps
    static ThermalStage thermalStages[4];
    // Semiproducts
    static Part semiproducts[5];

public:
    Process();

    // Options for get objects count
    static unsigned getLatheCount();
    static unsigned getFurnaceCount();
    static unsigned getWarehouseCount();
    static std::vector<unsigned> getMachinesCount();
    static float getPartProductionTime();
    static float getVolumeProductionTime();
    static float getPartCost();
    static float getBatchCost();
    static float getAvaragePartCost();
    static const Part getInputPart();
};
