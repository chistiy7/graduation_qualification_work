#include "domain/laboratory.hpp"

#include <cmath>
#include <stdexcept>

namespace lab {

void Laboratory::addCell(LabCell cell) {
    cells_.push_back(cell);
    cellById_[cell.id] = cell;
}

void Laboratory::placeEquipment(const std::string& equipmentId, const std::string& cellId) {
    equipmentCell_[equipmentId] = cellId;
}

void Laboratory::placeZone(const std::string& zoneId, const std::string& cellId) {
    zoneCell_[zoneId] = cellId;
}

const LabCell* Laboratory::cell(const std::string& cellId) const {
    auto it = cellById_.find(cellId);
    return it == cellById_.end() ? nullptr : &it->second;
}

std::string Laboratory::equipmentCell(const std::string& equipmentId) const {
    auto it = equipmentCell_.find(equipmentId);
    if (it == equipmentCell_.end()) {
        throw std::runtime_error("equipment not placed: " + equipmentId);
    }
    return it->second;
}

int Laboratory::manhattanDistance(const std::string& cellA, const std::string& cellB) const {
    auto a = cellById_.find(cellA);
    auto b = cellById_.find(cellB);
    if (a == cellById_.end() || b == cellById_.end()) {
        return 0;
    }
    return std::abs(a->second.row - b->second.row) + std::abs(a->second.col - b->second.col);
}

}  // namespace lab
