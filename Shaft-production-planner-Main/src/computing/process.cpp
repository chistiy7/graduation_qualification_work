#include "process.hpp"


// Input part
Part Process::targetPart{60, 200, 4};
Part Process::blankPart{80, 220, 240};
float Process::partProductionTarget = 10;
int Process::targetBatchVolume = 100;
// Process itself
MechanicalStage Process::mechanicalStages[4];
ThermalStage Process::thermalStages[4];
Part Process::semiproducts[5];
// Calculated part (resetted for savety)
int Process::startStep = 0, Process::endStep = 0;
float Process::timePerPart = 0, Process::timePerBatch = 0;
float Process::totalPartCost = 0, Process::totalBatchCost = 0;


Process::Process() {}

void Process::recalculate() {
    // Reset
    timePerPart = 0;
    totalPartCost = 0;
    totalBatchCost = 0;

    // Update step count
    endStep = MechanicalStage::getStepNumber(targetPart.rougness)+1;

    if (blankPart.rougness < targetPart.rougness) {
        startStep = endStep-1;
    } else {
        startStep = MechanicalStage::getStepNumber(blankPart.rougness);
        // Check, if at same step
        if (startStep == endStep-1) {
            startStep = endStep-1;
        }
    }

    // Getting input diamaters from step of production and putting it as out for next stage
    semiproducts[endStep].set(targetPart);
    for (int i=endStep-1; i >= startStep; --i) {
        // Check, if input rougness not from previous step
        if (i == startStep && semiproducts[i+1].rougness < blankPart.rougness) {
            mechanicalStages[i].setFirst(semiproducts[i+1], i, blankPart.rougness, partProductionTarget, targetBatchVolume);
        } else {
            mechanicalStages[i].set(semiproducts[i+1], i, partProductionTarget, targetBatchVolume);
        }
        semiproducts[i].set(mechanicalStages[i].getInputPart());
        thermalStages[i].set(semiproducts[i], partProductionTarget, targetBatchVolume);

        // Updating time counters
        timePerPart += mechanicalStages[i].getTimePerOperation();
        timePerPart += thermalStages[i].getTimePerOperation();

        // Update cost counters
        totalPartCost += mechanicalStages[i].getPartCost();
        totalPartCost += thermalStages[i].getPartCost();
        totalBatchCost += mechanicalStages[i].getBatchCost();
        totalBatchCost += thermalStages[i].getBatchCost();
    }
    calculateBatchTime();
}

void Process::calculateBatchTime() {
    // Resetting
    timePerBatch = 0;

    // Counting whole volume time to produce
    float endTime[4];
    for (int i=0; i < 4; ++i) {
        endTime[i] = 0;
    }

    for (int i=startStep; i < endStep; ++i) {
        int stepCapacity = thermalStages[i].getCapacity();
        if (stepCapacity > targetBatchVolume) {
            stepCapacity = targetBatchVolume;
        }
        // Adding start time to all steps, before them can start work
        for (int j=i+1; j < endStep; ++j) {
            endTime[j] += mechanicalStages[i].getTimePerBatch(stepCapacity);
            endTime[j] += thermalStages[i].getTimePerOperation();
        }
        float machineTime = mechanicalStages[i].getTimePerBatch(targetBatchVolume);
        float furnaceTime = thermalStages[i].getTimePerBatch(targetBatchVolume);

        if (furnaceTime > machineTime) {
            endTime[i] += furnaceTime;
            endTime[i] += mechanicalStages[i].getTimePerBatch(stepCapacity);
        } else {
            endTime[i] += machineTime;
            endTime[i] += thermalStages[i].getTimePerOperation();
        }

        // Adding end time to all step before, so them can finish work
        for (int j=i-1; j >= startStep; --j) {
            endTime[j] += mechanicalStages[i].getTimePerBatch(stepCapacity);
            endTime[j] += thermalStages[i].getTimePerOperation();
        }
    }
    // Finding maximal time
    for (int i=startStep; i < endStep; ++i) {
        if (endTime[i] > timePerBatch) {
            timePerBatch = endTime[i];
        }
    }
}

unsigned Process::getLatheCount() {
    unsigned count = 0;
    for (int i=startStep; i < endStep; ++i) {
        count += mechanicalStages[i].getReqieredQuantity();
    }
    return count;
}

unsigned Process::getFurnaceCount() {
    unsigned count = 0;
    for (int i=startStep; i < endStep; ++i) {
        count += thermalStages[i].getReqieredQuantity();
    }
    return count;
}

unsigned Process::getWarehouseCount() {
    return endStep-startStep;
}

std::vector<unsigned> Process::getMachinesCount() {
    std::vector<unsigned> counts;
    for (int i=startStep; i < endStep; ++i) {
        counts.push_back(mechanicalStages[i].getReqieredQuantity());
        counts.push_back(thermalStages[i].getReqieredQuantity());
        counts.push_back(1);
    }
    return counts;
}

float Process::getPartProductionTime() {
    return timePerPart;
}

float Process::getVolumeProductionTime() {
    return timePerBatch;
}

float Process::getPartCost() {
    return totalPartCost;
}

float Process::getBatchCost() {
    return totalBatchCost;
}

float Process::getAvaragePartCost() {
    return totalBatchCost/targetBatchVolume;
}

const Part Process::getInputPart() {
    return semiproducts[startStep];
}
