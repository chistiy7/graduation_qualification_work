#include "engine/layout_map.hpp"

#include "domain/laboratory.hpp"
#include "engine/layout_area.hpp"

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>
#include <vector>

namespace lab {

namespace {

constexpr int kVoid    =  0;
constexpr int kPassage = -3;
constexpr int kBuffer  = -2;
constexpr int kRoute   = -1;

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

std::string renderLayoutMatrix(const ProblemDefinition& problem, const TestRoute& route,
                               double cycleTimeMin) {
    const auto& laboratory = problem.laboratory;

    std::map<StandType, int> typeCode;
    int nextCode = 1;
    for (const auto& e : problem.equipment) {
        if (typeCode.find(e.standType) == typeCode.end()) {
            typeCode[e.standType] = nextCode++;
        }
    }

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

    const int rows = maxRow - minRow + 1;
    const int cols = maxCol - minCol + 1;
    std::vector<std::vector<int>> grid(rows, std::vector<int>(cols, kVoid));

    for (const auto& c : laboratory.cells()) {
        const int gr = c.row - minRow;
        const int gc = c.col - minCol;
        if (c.kind == LabCellKind::Buffer || c.kind == LabCellKind::Forbidden) {
            grid[gr][gc] = kBuffer;
        } else if (c.kind != LabCellKind::Stand) {
            grid[gr][gc] = kPassage;
        }
    }

    std::map<std::string, Coord> equipmentCoord;
    for (const auto& [eqId, cellId] : laboratory.equipmentPlacements()) {
        if (const auto* c = laboratory.cell(cellId)) {
            equipmentCoord[eqId] = {c->row, c->col};
            if (const auto* e = findEquipment(problem, eqId)) {
                grid[c->row - minRow][c->col - minCol] = typeCode[e->standType];
            }
        }
    }

    auto markRoute = [&](Coord a, Coord b) {
        int r = a.row, c = a.col;
        while (c != b.col) {
            c += (b.col > c) ? 1 : -1;
            const int gr = r - minRow, gc = c - minCol;
            if (grid[gr][gc] == kPassage) grid[gr][gc] = kRoute;
        }
        while (r != b.row) {
            r += (b.row > r) ? 1 : -1;
            const int gr = r - minRow, gc = c - minCol;
            if (grid[gr][gc] == kPassage) grid[gr][gc] = kRoute;
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

    bool hasRoute = false;
    for (int r = 0; r < rows && !hasRoute; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (grid[r][c] == kRoute) {
                hasRoute = true;
                break;
            }
        }
    }

    std::ostringstream out;
    out << std::fixed << std::setprecision(2);
    out << "--- Карта размещения (сетка " << problem.gridCellSizeM << "×"
        << problem.gridCellSizeM << " м) ---\n";
    out << "Ячейка стенда включает: зону загрузки/снятия, зону испытания,\n"
        << "  зону контроля/измерений, зону наблюдения и зону обслуживания.\n";
    out << "Легенда:\n"
        << "  .  — зона прохода (свободная ячейка, доступна для перемещения)\n"
        << "  X  — зона безопасности/обслуживания стенда; в этой зоне нельзя "
        << "размещать другое оборудование\n";
    if (hasRoute) {
        out << "  R  — маршрут перемещения образцов\n";
    }
    for (const auto& [type, code] : typeCode) {
        std::string name;
        for (const auto& e : problem.equipment) {
            if (e.standType == type) {
                name = e.nameRu + " [" + e.id + "]";
                break;
            }
        }
        out << "  " << code << "  — " << name << "\n";
    }
    out << "\n";

    out << "     ";
    for (int c = 0; c < cols; ++c) {
        out << (c % 10);
    }
    out << "\n";

    for (int r = 0; r < rows; ++r) {
        out << " " << r << " |";
        for (int c = 0; c < cols; ++c) {
            const int v = grid[r][c];
            if (v == kVoid) {
                out << "  ";
            } else if (v == kPassage) {
                out << " .";
            } else if (v == kRoute) {
                out << " R";
            } else if (v == kBuffer) {
                out << " X";
            } else {
                out << " " << v;
            }
        }
        out << "\n";
    }

    const auto areaStats = computeLayoutAreaStats(problem);
    const double cArea = cycleTimeMin > 0.0 ? areaOccupancyCost(problem, cycleTimeMin) : 0.0;
    out << "\n--- Площадь и C_area ---\n";
    out << "  equipment_cells = " << areaStats.equipmentCells << "\n";
    out << "  reserved_cells  = " << areaStats.reservedCells << "\n";
    out << "  free_cells      = " << areaStats.freeCells << "\n";
    out << "  used_area_m2    = " << areaStats.usedAreaM2 << "\n";
    if (cycleTimeMin > 0.0) {
        out << "  T_cycle, ч      = " << (cycleTimeMin / 60.0) << "\n";
        out << "  C_area, руб     = " << cArea << "\n";
    }
    return out.str();
}

}  // namespace lab
