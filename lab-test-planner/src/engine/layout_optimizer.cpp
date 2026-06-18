#include "engine/layout_optimizer.hpp"

#include "engine/layout_area.hpp"
#include "engine/route_analysis.hpp"
#include "engine/route_planner.hpp"

#include <algorithm>
#include <limits>
#include <numeric>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace lab {

namespace {

struct Cell {
    int row;
    int col;
    std::string id;
};

std::vector<Cell> makeCells(int rows, int cols) {
    std::vector<Cell> cells;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            cells.push_back({r, c, "c_" + std::to_string(r) + "_" + std::to_string(c)});
        }
    }
    return cells;
}

int autoOrient(int r, int c, int rows, int cols, const DirectionalBuffer& buf) {
    if (buf.back == buf.front) return 1;
    const int distN = r, distS = rows - 1 - r;
    const int distW = c, distE = cols - 1 - c;
    const int minDist = std::min({distN, distS, distW, distE});
    if (minDist == distW) return 1;
    if (minDist == distE) return 3;
    if (minDist == distN) return 2;
    return 0;
}

bool inForbiddenZone(int dr, int dc, int orient, const DirectionalBuffer& buf) {
    int axial, lateral;
    switch (orient) {
    case 0:
        axial = -dr;
        lateral = dc;
        break;
    case 1:
        axial = dc;
        lateral = dr;
        break;
    case 2:
        axial = dr;
        lateral = dc;
        break;
    default:
        axial = -dc;
        lateral = dr;
        break;
    }
    const int axialBuf = (axial >= 0) ? buf.front : buf.back;
    return (std::abs(axial) <= axialBuf && std::abs(lateral) <= buf.side);
}

bool placementValid(const ProblemDefinition& base, const std::vector<Cell>& cells,
                    const std::vector<int>& positions, int rows, int cols, int activeCount = -1) {
    const int k = activeCount >= 0 ? activeCount : static_cast<int>(positions.size());
    for (int i = 0; i < k; ++i) {
        for (int j = i + 1; j < k; ++j) {
            if (positions[i] == positions[j]) return false;

            const auto& ci = cells[positions[i]];
            const auto& cj = cells[positions[j]];
            const auto& bufi = base.equipment[i].buffer;
            const auto& bufj = base.equipment[j].buffer;
            const int ori = autoOrient(ci.row, ci.col, rows, cols, bufi);
            const int orj = autoOrient(cj.row, cj.col, rows, cols, bufj);

            const int dr = cj.row - ci.row;
            const int dc = cj.col - ci.col;

            if (inForbiddenZone(dr, dc, ori, bufi)) return false;
            if (inForbiddenZone(-dr, -dc, orj, bufj)) return false;
        }
    }
    return true;
}

ProblemDefinition applyPlacement(const ProblemDefinition& base, const std::vector<Cell>& cells,
                                 const std::vector<int>& positions) {
    ProblemDefinition p = base;
    const int rows = p.gridRows > 0 ? p.gridRows : 1;
    const int cols = p.gridCols > 0 ? p.gridCols : 1;

    std::set<std::string> standCellIds;
    for (size_t i = 0; i < p.equipment.size(); ++i) {
        standCellIds.insert(cells[positions[i]].id);
    }

    std::set<std::string> bufferCellIds;
    for (size_t i = 0; i < p.equipment.size(); ++i) {
        const auto& cell = cells[positions[i]];
        const auto& buf = p.equipment[i].buffer;
        const int orient = autoOrient(cell.row, cell.col, rows, cols, buf);
        p.equipment[i].orientation = orient;

        for (const auto& c : cells) {
            if (standCellIds.count(c.id)) continue;
            const int dr = c.row - cell.row;
            const int dc = c.col - cell.col;
            if (inForbiddenZone(dr, dc, orient, buf)) {
                bufferCellIds.insert(c.id);
            }
        }
    }

    Laboratory grid;
    for (const auto& c : cells) {
        LabCellKind kind = LabCellKind::Passage;
        bool banned = false;
        if (standCellIds.count(c.id)) {
            kind = LabCellKind::Stand;
        } else if (bufferCellIds.count(c.id)) {
            kind = LabCellKind::Buffer;
            banned = true;
        }
        grid.addCell({c.id, c.row, c.col, kind, banned});
    }

    for (size_t i = 0; i < p.equipment.size(); ++i) {
        const auto& cell = cells[positions[i]];
        p.equipment[i].cellId = cell.id;
        grid.placeEquipment(p.equipment[i].id, cell.id);
    }
    p.laboratory = grid;
    return p;
}

// Только слагаемые C, зависящие от координат стендов (транспорт + C_area).
double evaluateLayoutCost(const ProblemDefinition& placed) {
    const TestRoute route = RoutePlanner{}.buildGroupedRoute(placed);
    const RouteAnalysis analysis = RouteAnalyzer{}.analyze(placed, route);

    double tCycle = 0.0;
    for (const auto& e : placed.equipment) {
        const auto it = analysis.busyMinutesByEquipment.find(e.id);
        const double busy = it != analysis.busyMinutesByEquipment.end() ? it->second : 0.0;
        tCycle = std::max(tCycle, busy);
    }

    const double transport = (analysis.moveTimeMin / 60.0) * placed.laborRatePerHour;
    const double area = areaOccupancyCost(placed, tCycle);
    return transport + area;
}

long long permutations(int m, int k) {
    long long acc = 1;
    for (int i = 0; i < k; ++i) {
        acc *= (m - i);
        if (acc > 1'000'000'000LL) return acc;
    }
    return acc;
}

bool greedyPlacement(const ProblemDefinition& base, const std::vector<Cell>& cells,
                     const std::vector<int>& cellOrder, int rows, int cols, int k,
                     std::vector<int>& positions) {
    positions.assign(k, -1);
    std::vector<bool> used(cells.size(), false);

    for (int i = 0; i < k; ++i) {
        bool placed = false;
        for (const int c : cellOrder) {
            if (used[c]) continue;
            positions[i] = c;
            if (!placementValid(base, cells, positions, rows, cols, i + 1)) continue;
            used[c] = true;
            placed = true;
            break;
        }
        if (!placed) return false;
    }
    return placementValid(base, cells, positions, rows, cols);
}

bool findValidPlacement(const ProblemDefinition& base, const std::vector<Cell>& cells, int rows,
                        int cols, int m, int k, std::vector<int>& positions) {
    std::vector<int> cellOrder(m);
    std::iota(cellOrder.begin(), cellOrder.end(), 0);

    if (greedyPlacement(base, cells, cellOrder, rows, cols, k, positions)) {
        return true;
    }

    std::mt19937 rng(42);
    constexpr int kMaxTries = 64;
    for (int attempt = 0; attempt < kMaxTries; ++attempt) {
        std::shuffle(cellOrder.begin(), cellOrder.end(), rng);
        if (greedyPlacement(base, cells, cellOrder, rows, cols, k, positions)) {
            return true;
        }
    }
    return false;
}

}  // namespace

LayoutOptimizationResult optimizeLayout(const ProblemDefinition& base) {
    LayoutOptimizationResult out;

    const int rows = base.gridRows > 0 ? base.gridRows : 1;
    const int cols = base.gridCols > 0 ? base.gridCols : 1;
    const auto cells = makeCells(rows, cols);
    const int m = static_cast<int>(cells.size());
    const int k = static_cast<int>(base.equipment.size());

    if (k == 0) {
        out.problem = base;
        out.note = "нет стендов для размещения";
        return out;
    }
    if (k > m) {
        throw std::runtime_error("недостаточно ячеек: стендов " + std::to_string(k) +
                                 ", ячеек " + std::to_string(m) +
                                 " (увеличьте площадь помещения)");
    }

    double bestCost = std::numeric_limits<double>::max();
    std::vector<int> bestPositions;
    long long evaluated = 0;

    constexpr long long kBruteLimit = 50'000;
    const bool brute = permutations(m, k) <= kBruteLimit;

    if (brute) {
        std::vector<int> positions(k, -1);
        std::vector<bool> used(m, false);
        const std::function<void(int)> place = [&](int idx) {
            if (idx == k) {
                if (!placementValid(base, cells, positions, rows, cols)) return;
                ++evaluated;
                const double cost = evaluateLayoutCost(applyPlacement(base, cells, positions));
                if (cost < bestCost) {
                    bestCost = cost;
                    bestPositions = positions;
                }
                return;
            }
            for (int c = 0; c < m; ++c) {
                if (used[c]) continue;
                used[c] = true;
                positions[idx] = c;
                place(idx + 1);
                used[c] = false;
            }
        };
        place(0);
        out.bruteForce = true;
    } else {
        std::vector<int> positions;
        if (findValidPlacement(base, cells, rows, cols, m, k, positions)) {
            bestPositions = positions;
            bestCost = evaluateLayoutCost(applyPlacement(base, cells, positions));
            ++evaluated;
        }

        if (static_cast<int>(bestPositions.size()) == k) {
            bool improved = true;
            int guard = 0;
            while (improved && guard++ < 15) {
                improved = false;
                std::vector<bool> used(m, false);
                for (int v : bestPositions) used[v] = true;
                for (int i = 0; i < k; ++i) {
                    for (int c = 0; c < m; ++c) {
                        if (used[c]) continue;
                        auto trial = bestPositions;
                        used[trial[i]] = false;
                        trial[i] = c;
                        if (!placementValid(base, cells, trial, rows, cols)) continue;
                        const double cost = evaluateLayoutCost(applyPlacement(base, cells, trial));
                        ++evaluated;
                        if (cost < bestCost) {
                            bestCost = cost;
                            bestPositions = trial;
                            used.assign(m, false);
                            for (int v : bestPositions) used[v] = true;
                            improved = true;
                        } else {
                            used[bestPositions[i]] = true;
                        }
                    }
                }
            }
        }
        out.bruteForce = false;
    }

    if (bestPositions.empty()) {
        throw std::runtime_error(
            "не удалось разместить стенды с учётом направленных зон безопасности — "
            "увеличьте площадь помещения или уменьшите число типов испытаний");
    }

    out.problem = applyPlacement(base, cells, bestPositions);
    out.bestCost = bestCost;
    out.evaluated = evaluated;
    out.note = brute ? "полный перебор размещений" : "жадная раскладка + локальный поиск";
    return out;
}

}  // namespace lab
