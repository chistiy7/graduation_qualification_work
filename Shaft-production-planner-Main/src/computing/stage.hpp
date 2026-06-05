#pragma once

#include "part.hpp"


// One step of process, need for understanding requered part and parameters 
class Stage {
private:
    // Constants
    const float energyCost = 7.0;    // Rubles/kW*h
    const float employerCost = 500;  // Rubles/h

    // Variables
    int capacity;
    float operationTime, batchTime;  // In hours
    int requiredQuantity;
    float powerConsumption;
    float partCost, batchCost;  // In rubbles

protected:
    void setCapacity(int capacity);
    void setOperationTime(float time, int batchVolume);
    void setRequiredQuantity(float settedPartCapacity, int batchVolume);
    void setPowerConsumption(float powerConsumtion);
    void setCost(int cycles, float activeTime, float additions=0);

public:
    Stage();
    int getReqieredQuantity();
    int getCapacity();
    float getTimePerOperation();
    float getTimePerBatch();
    float getTimePerBatch(int batchVolume);
    float getPartCost();
    float getBatchCost();
};
