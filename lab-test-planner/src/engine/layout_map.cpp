#include "engine/layout_map.hpp"

#include "domain/laboratory.hpp"

#include <algorithm>
#include <map>
#include <sstream>
#include <vector>

namespace lab {

namespace {

constexpr int kEmpty = 0;
constexpr int kRoute = -1;  // отображается как 'R'

struct Coord {
    int row = 0;
    int col = 0;
};

const LabEquipment* findEquipment(const ProblemDefinition& p, const std::string& id) {
    for (const auto& e : p.equipment) {
        if (e.id == id) return &e;
    }
    return nullptr;
}

}  // namespace

std::string renderLayoutMatrix(const ProblemDefinition& problem, const TestRoute& route) {
    const auto& laboratory = problem.laboratory;

    // Код типа стенда: по порядку появления StandType среди оборудования
    std::map<StandType, int> typeCode;
    int nextCode = 1;
    for (const auto& e : problem.equipment) {
        if (typeCode.find(e.standType) == typeCode.end()) {
            typeCode[e.standType] = nextCode++;
        }
    }

    // Координаты ячеек стендов
    std::map<std::string, Coord> equipmentCoord;  // equipmentId -> coord
    int minRow = 0, maxRow = 0, minCol = 0, maxCol = 0;
    bool first = true;
    for (const auto& c : laboratory.cells()) {
        if (first) {
            minRow = maxRow = c.row;
            minCol = maxCol = c.col;
            first = false;
        } else {
            minRow = std::min(minRow, c.row);
            maxRow = std::max(maxRow, c.row);
            minCol = std::min(minCol, c.col);
            maxCol = std::max(maxCol, c.col);
        }
    }
    if (first) {
        return "Карта размещения: сетка не задана.\n";
    }

    for (const auto& [eqId, cellId] : laboratory.equipmentPlacements()) {
        if (const auto* c = laboratory.cell(cellId)) {
            equipmentCoord[eqId] = {c->row, c->col};
        }
    }

    const int rows = maxRow - minRow + 1;
    const int cols = maxCol - minCol + 1;
    std::vector<std::vector<int>> grid(rows, std::vector<int>(cols, kEmpty));

    for (const auto& [eqId, coord] : equipmentCoord) {
        if (const auto* e = findEquipment(problem, eqId)) {
            grid[coord.row - minRow][coord.col - minCol] = typeCode[e->standType];
        }
    }

    // Наложение маршрута: между соседними шагами с разными стендами
    auto markRoute = [&](Coord a, Coord b) {
        int r = a.row, c = a.col;
        while (c != b.col) {
            c += (b.col > c) ? 1 : -1;
            int gr = r - minRow, gc = c - minCol;
            if (grid[gr][gc] == kEmpty) grid[gr][gc] = kRoute;
        }
        while (r != b.row) {
            r += (b.row > r) ? 1 : -1;
            int gr = r - minRow, gc = c - minCol;
            if (grid[gr][gc] == kEmpty) grid[gr][gc] = kRoute;
        }
    };

    const auto& steps = route.steps();
    for (size_t i = 1; i < steps.size(); ++i) {
        const auto prev = equipmentCoord.find(steps[i - 1].equipmentId);
        const auto cur = equipmentCoord.find(steps[i].equipmentId);
        if (prev == equipmentCoord.end() || cur == equipmentCoord.end()) continue;
        if (prev->second.row == cur->second.row && prev->second.col == cur->second.col) continue;
        markRoute(prev->second, cur->second);
    }

    std::ostringstream out;
    out << "--- Карта размещения (ячейки сетки) ---\n";
    out << "Легенда: 0 — пусто, R — маршрут";
    for (const auto& [type, code] : typeCode) {
        std::string name;
        for (const auto& e : problem.equipment) {
            if (e.standType == type) {
                name = e.nameRu + " (" + e.id + ")";
                break;
            }
        }
        out << ", " << code << " — " << name;
    }
    out << "\n";

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const int v = grid[r][c];
            if (v == kEmpty) {
                out << " 0";
            } else if (v == kRoute) {
                out << " R";
            } else {
                out << " " << v;
            }
        }
        out << "\n";
    }
    return out.str();
}

}  // namespace lab
