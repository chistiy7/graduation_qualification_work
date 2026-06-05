#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace lab {

enum class LabCellKind { Void, Passage, Stand, Buffer, Forbidden, Cooling, Staff };

struct LabCell {
    std::string id;
    int row = 0;
    int col = 0;
    LabCellKind kind = LabCellKind::Passage;
    bool banned = false;
};

struct LabZone {
    std::string id;
    std::string cellId;
};

// Лаборатория: сетка G, размещение стендов и зон
class Laboratory {
public:
    void addCell(LabCell cell);
    void placeEquipment(const std::string& equipmentId, const std::string& cellId);
    void placeZone(const std::string& zoneId, const std::string& cellId);

    [[nodiscard]] const std::vector<LabCell>& cells() const { return cells_; }
    [[nodiscard]] std::string equipmentCell(const std::string& equipmentId) const;
    [[nodiscard]] const std::unordered_map<std::string, std::string>& equipmentPlacements() const {
        return equipmentCell_;
    }
    [[nodiscard]] const LabCell* cell(const std::string& cellId) const;

    [[nodiscard]] int manhattanDistance(const std::string& cellA, const std::string& cellB) const;

private:
    std::vector<LabCell> cells_;
    std::unordered_map<std::string, LabCell> cellById_;
    std::unordered_map<std::string, std::string> equipmentCell_;
    std::unordered_map<std::string, std::string> zoneCell_;
};

}  // namespace lab
