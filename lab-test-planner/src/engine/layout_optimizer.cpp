#include "engine/layout_optimizer.hpp"

#include "engine/metrics.hpp"
#include "engine/route_planner.hpp"

#include <algorithm>
#include <functional>
#include <limits>
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

// Постановка с размещением стендов по позициям positions[i] (индекс ячейки для equipment[i])
ProblemDefinition applyPlacement(const ProblemDefinition& base, const std::vector<Cell>& cells,
                                 const std::vector<int>& positions) {
    ProblemDefinition p = base;
    Laboratory grid;
    for (const auto& c : cells) {
        grid.addCell({c.id, c.row, c.col, LabCellKind::Passage});
    }
    for (size_t i = 0; i < p.equipment.size(); ++i) {
        const auto& cell = cells[positions[i]];
        p.equipment[i].cellId = cell.id;
        grid.placeEquipment(p.equipment[i].id, cell.id);
    }
    p.laboratory = grid;
    return p;
}

// Минимальная C по двум стратегиям упорядочивания на данной планировке
double evaluateCost(const ProblemDefinition& placed) {
    RoutePlanner planner;
    MetricsEngine metrics;
    const auto baseline = planner.buildBaselineRoute(placed);
    const auto grouped = planner.buildGroupedRoute(placed);
    const double cb = metrics.compute(placed, baseline).C;
    const double cg = metrics.compute(placed, grouped).C;
    return std::min(cb, cg);
}

long long permutations(int m, int k) {
    long long acc = 1;
    for (int i = 0; i < k; ++i) {
        acc *= (m - i);
        if (acc > 1'000'000'000LL) return acc;  // защита от переполнения
    }
    return acc;
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

    constexpr long long kBruteLimit = 200'000;
    const bool brute = permutations(m, k) <= kBruteLimit;

    if (brute) {
        std::vector<int> positions(k, -1);
        std::vector<bool> used(m, false);
        // Рекурсивный перебор упорядоченных размещений
        const std::function<void(int)> place = [&](int idx) {
            if (idx == k) {
                ++evaluated;
                const double cost = evaluateCost(applyPlacement(base, cells, positions));
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
        // Эвристика: компактная раскладка (первые K ячеек) + локальный поиск перестановок
        std::vector<int> positions(k);
        for (int i = 0; i < k; ++i) positions[i] = i;
        bestPositions = positions;
        bestCost = evaluateCost(applyPlacement(base, cells, positions));
        ++evaluated;

        bool improved = true;
        int guard = 0;
        while (improved && guard++ < 50) {
            improved = false;
            // перенос стенда в свободную ячейку
            std::vector<bool> used(m, false);
            for (int v : bestPositions) used[v] = true;
            for (int i = 0; i < k; ++i) {
                for (int c = 0; c < m; ++c) {
                    if (used[c]) continue;
                    auto trial = bestPositions;
                    used[trial[i]] = false;
                    trial[i] = c;
                    const double cost = evaluateCost(applyPlacement(base, cells, trial));
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
        out.bruteForce = false;
    }

    out.problem = applyPlacement(base, cells, bestPositions);
    out.bestCost = bestCost;
    out.evaluated = evaluated;
    out.note = brute ? "полный перебор размещений" : "эвристика (компактная раскладка + локальный поиск)";
    return out;
}

}  // namespace lab
