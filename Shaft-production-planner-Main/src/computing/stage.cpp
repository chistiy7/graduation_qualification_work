#include "stage.hpp"


Stage::Stage() {}

void Stage::setCapacity(int _capacity) {
    capacity = _capacity;
}

void Stage::setOperationTime(float _time, int _batchVolume) {
    operationTime = _time;
    batchTime = getTimePerBatch(_batchVolume);
}

void Stage::setRequiredQuantity(float _settedPartCapacity, int _batchVolume) {
    requiredQuantity = std::ceilf(_settedPartCapacity*operationTime/capacity);
    // Check, if produce less parts than required
    if (requiredQuantity > _batchVolume) {
        requiredQuantity = _batchVolume;
    }
}

void Stage::setPowerConsumption(float _powerConsumtion) {
    powerConsumption = _powerConsumtion;
}

void Stage::setCost(int cycles, float activeTime, float additions) {
    float cycleCost = powerConsumption*activeTime + employerCost*operationTime + additions;
    partCost = cycleCost/capacity;
    batchCost = partCost*cycles;
}

int Stage::getReqieredQuantity() {
    return requiredQuantity;
}

int Stage::getCapacity() {
    return capacity;
}

float Stage::getTimePerOperation() {
    return operationTime;
}

float Stage::getTimePerBatch(int _batchVolume) {
    return ceilf((float)ceilf((float)_batchVolume/capacity)/requiredQuantity)*operationTime;
}

float Stage::getTimePerBatch() {
    return batchTime;
}

float Stage::getPartCost() {
    return partCost;
}

float Stage::getBatchCost() {
    return batchCost;
}
